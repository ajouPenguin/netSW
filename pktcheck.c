#include "pktcheck.h"
#include "listControl.h"
#include "bittwistb.h"
#include "pi2srv.h"

struct http_request *http_req_ht[HTTP_REQ_HT_SIZE];
#define CUR_HTTP_REQ_HT (http_req_ht[cur_idx])

int check_packet(o, s, h, p)
int o;
int s;
const struct pcap_pkthdr *h;
u_char *p;
{
    struct ether_header eth_hdr;
    struct ip           ip_hdr;
    struct tcphdr       tcp_hdr;

    char *ptr;
    int remain;

    memset(&eth_hdr,    0, ETHER_HDR_LEN);
    memset(&ip_hdr,     0,    IP_HDR_LEN);
    memset(&tcp_hdr,    0,   TCP_HDR_LEN);

#define MOV_PTR(x) \
ptr		+= (x); \
remain	-= (x); \
if (remain == 0) return o;

    ptr = p;
    remain = h->caplen;

    memcpy(&eth_hdr, ptr, ETHER_HDR_LEN);
    if (ntohs(eth_hdr.ether_type) != ETHERTYPE_IP) {
        /* http is working on ip proto, not arp */
        goto end;
    }
    MOV_PTR(ETHER_HDR_LEN);

    memcpy(&ip_hdr, ptr, IP_HDR_LEN);
    if (ip_hdr.ip_v != 4) {
        /* only ip v4 supported. */
        goto end;
    }
    MOV_PTR(ip_hdr.ip_hl * 4);

    if (ip_hdr.ip_p != IPPROTO_TCP) {
        /* http is working on tcp */
        goto end;
    }
    memcpy(&tcp_hdr, ptr, TCP_HDR_LEN);

    if (ntohs(tcp_hdr.th_dport) != 80) {
        /* http's port is 80 */
        goto end;
    }

#define CHECK_TCP_FLAG(x) ((tcp_hdr.th_flags & (x)) == (x))
#define NEW_PKT_SET \
pkt = (struct pkt_set_t *) malloc(sizeof(struct pkt_set_t) * 1); \
pkt->h = h; \
pkt->p = p; \
pkt->next = NULL;
    if (CHECK_TCP_FLAG(TH_SYN) || CHECK_TCP_FLAG(TH_FIN) || CHECK_TCP_FLAG(TH_RST)) {
        goto end;
    }
    MOV_PTR(tcp_hdr.th_off * 4);

    if (remain == 0) {
        goto end;
    }

    int cur_idx = JumpConsistentHash((((tcp_hdr.th_sport + ip_hdr.ip_src.s_addr) << 32LL) + (tcp_hdr.th_dport + ip_hdr.ip_dst.s_addr)), HTTP_REQ_HT_SIZE);
#define CHECK_HTTP_METHOD(x) (strncmp(x, ptr, strlen(x)) == 0)
    if (CHECK_TCP_FLAG(TH_ACK)) {
        if (CHECK_HTTP_METHOD("GET ") || CHECK_HTTP_METHOD("POST ")) {
            struct http_request_t *cur;
            struct pkt_set_t *pkt;
            u_char *msg;
            if (CUR_HTTP_REQ_HT != NULL) {
                /* already exist... */
                goto end;
            }
            cur = (struct http_request_t *) malloc(sizeof(struct http_request_t) * 1);

            msg = (u_char *) calloc(remain + 1, sizeof(u_char));
            strncpy(msg, ptr, remain);

            NEW_PKT_SET;

            cur->pkt = cur->pkt_last = pkt;
            cur->o = o;
            cur->s = s;
            cur->msg = msg;

            CUR_HTTP_REQ_HT = cur;

            puts("CRET");
        } else if (CUR_HTTP_REQ_HT != NULL) {
            struct http_request_t *cur;
            struct pkt_set_t *pkt;
            u_char *msg;

            cur = CUR_HTTP_REQ_HT;

            NEW_PKT_SET;

            // append to last list
            cur->pkt_last->next = pkt;
            cur->pkt_last = pkt;
            /* cur->o = o; // maybe same o */
            /* cur->s = s; // maybe same s */

            msg = cur->msg;
            msg = realloc(msg, strlen(msg) + remain + 1);
            strncat(msg, ptr, remain);
            cur->msg = msg;

            puts("UPDATE");
        }
    }

    if (CHECK_TCP_FLAG(TH_PUSH)) {
        struct http_request_t *cur;
        struct list_item_t *item;
        struct pkt_set_t *ptr;
        u_char *msg;
        char *a, *b;

        /* check http req*/
        cur = CUR_HTTP_REQ_HT;
        msg = cur->msg;
        /*hexdump(msg, strlen(msg));*/
        a = b = NULL;
        parse_http_header(&a, &b, msg);
        if (a != NULL) { puts(a); } else { a = ""; }
        if (b != NULL) { puts(b); } else { b = ""; }

        item = select_item(a, b);

        if (item == NULL) {
            pthread_t th;
            struct thread_arg_t *arg;
            insert_item(a, b);
            arg = (struct thread_arg_t *) malloc(1 * sizeof(struct thread_arg_t));
            arg->cur_idx = cur_idx;
            arg->url = a;
            arg->cookie = b;

            pthread_create(&th, NULL, pi2sand, (void *) arg);
            //pi2sand(arg);
            return 1;
        }

        return process_by_list(cur_idx, item->status);
        /* del_http_req_ht(cur_idx); */
    }

    return 1;

    end:
    return 0;
}

void hexdump(data, size)
const void* data;
size_t size;
{
    char ascii[17];
    size_t i, j;
    ascii[16] = '\0';
    for (i = 0; i < size; ++i) {
        printf("%02X ", ((unsigned char*)data)[i]);
        if (((unsigned char*)data)[i] >= ' ' && ((unsigned char*)data)[i] <= '~') {
            ascii[i % 16] = ((unsigned char*)data)[i];
        } else {
            ascii[i % 16] = '.';
        }
        if ((i+1) % 8 == 0 || i+1 == size) {
            printf(" ");
            if ((i+1) % 16 == 0) {
                printf("|  %s \n", ascii);
            } else if (i+1 == size) {
                ascii[(i+1) % 16] = '\0';
                if ((i+1) % 16 <= 8) {
                    printf(" ");
                }
                for (j = (i+1) % 16; j < 16; ++j) {
                    printf("   ");
                }
                printf("|  %s \n", ascii);
            }
        }
    }
}

void parse_http_header(url, cookie, data)
char **url;
char **cookie;
char *data;
{
    char buff[BUFSIZ];
    char *host, *uri, *ptr, *tmp;
    strcpy(buff, data);

    ptr = strtok(buff, " ");
    ptr = strtok(NULL, " ");
    uri = ptr;

    ptr = strtok(NULL, "\r\n");
    do {
        if (strncmp(ptr, "Host:", 5) == 0) {
            host = ptr + 6;
        } else if (strncmp(ptr, "Cookie:", 7) == 0) {
            tmp = (char *) malloc(sizeof(char) * (strlen(ptr) - 8 + 1));
            strcpy(tmp, ptr + 8);
            *cookie = tmp;
        }
    } while ((ptr = strtok(NULL, "\r\n")) != NULL);

    tmp = (char *) malloc(sizeof(char) * (7 + strlen(host) + strlen(uri) + 1));
    strcpy(tmp, "http://");
    strcat(tmp, (const char *)host);
    strcat(tmp, (const char *)uri);
    *url = tmp;
}

int32_t JumpConsistentHash(uint64_t key, int32_t num_buckets) {
    int32_t b = 1, j = 0;
    while (j < num_buckets) {
        b = j;
        key = key * 2862933555777941757ULL + 1;
        j = (b + 1) * ((double)(1LL << 31) / (double)((key >> 33) + 1));
    }
    return b;
}


void del_http_req_ht(int cur_idx) {
    struct http_request_t *cur;
    struct pkt_set_t *pkt, *pkt2;
    u_char *msg;
    char *a, *b;

    cur = CUR_HTTP_REQ_HT;
    pkt = cur->pkt;
    while (pkt->next != NULL) {
        pkt2 = pkt->next;
        free(pkt);
        pkt = pkt2;
    }
    free(pkt);

    msg = cur->msg;
    free(msg);

    free(cur);
    CUR_HTTP_REQ_HT = NULL;
    puts("DELE");
}

int process_by_list(int cur_idx, char *status) {
    struct http_request_t *cur;
    struct pkt_set_t *ptr;
    struct ether_header eth_hdr;

    cur = CUR_HTTP_REQ_HT;
    if (strcmp(status, "WHITE") == 0) {
        ptr = cur->pkt;
        while (ptr != NULL) {
            memcpy(&eth_hdr, ptr->p, ETHER_HDR_LEN);
            send_packets(cur->o, cur->s,
                (const struct ether_addr *) eth_hdr.ether_dhost,
                (const struct ether_addr *) eth_hdr.ether_shost,
                ptr->p, ptr->h->caplen
            );
            ptr = ptr->next;
        }
        del_http_req_ht(cur_idx);
        return 1;
    } else if (strcmp(status, "BLACK") == 0) {
        del_http_req_ht(cur_idx);
        return 2;
    }
}
