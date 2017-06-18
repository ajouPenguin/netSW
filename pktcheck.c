/*
* 아주대 소프트웨어학과
* Network software term project.
* 3조 teamPUB
* 2017-06-14
*/

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
	/* header structs are defined in 'def.h' */
    struct ether_header eth_hdr;
    struct ip           ip_hdr;
    struct tcphdr       tcp_hdr;

    char *ptr;	// pointer to handle packet
    int remain;	// number of bytes remaining from 'ptr'

    memset(&eth_hdr,    0, ETHER_HDR_LEN);
    memset(&ip_hdr,     0,    IP_HDR_LEN);
    memset(&tcp_hdr,    0,   TCP_HDR_LEN);

#define MOV_PTR(x) \
ptr		+= (x); \
remain	-= (x); \
if (remain == 0) return o;

    ptr = p;
    remain = h->caplen;

    memcpy(&eth_hdr, ptr, ETHER_HDR_LEN);				// eth_hdr initialization
    if (ntohs(eth_hdr.ether_type) != ETHERTYPE_IP) {	// ethernet type verification
        /* http is working on ip proto, not arp */
        goto end;
    }
    MOV_PTR(ETHER_HDR_LEN);	// Move the pointer by ethernet header length

    memcpy(&ip_hdr, ptr, IP_HDR_LEN);	// ip_hdr initialization
    if (ip_hdr.ip_v != 4) {				// ip version verification
        /* only ip v4 supported. */
        goto end;
    }
    MOV_PTR(ip_hdr.ip_hl * 4);	// move the pointer by ip header length

    if (ip_hdr.ip_p != IPPROTO_TCP) {	// ip protocol verification
        /* http is working on tcp */
        goto end;
    }
    memcpy(&tcp_hdr, ptr, TCP_HDR_LEN);	// tcp_hdr initialization

    if (ntohs(tcp_hdr.th_dport) != 80) {	// check dst port number
        /* http's port is 80 */
        goto end;
    }

#define CHECK_TCP_FLAG(x) ((tcp_hdr.th_flags & (x)) == (x))
#define NEW_PKT_SET \
pkt = (struct pkt_set_t *) malloc(sizeof(struct pkt_set_t) * 1); \
pkt->h = h; \
pkt->p = p; \
pkt->next = NULL;
	/* check tcp flag */
    if (CHECK_TCP_FLAG(TH_SYN) || CHECK_TCP_FLAG(TH_FIN) || CHECK_TCP_FLAG(TH_RST)) {
        goto end;
    }
	MOV_PTR(tcp_hdr.th_off * 4);	// Move the pointer by tcp header length

    if (remain == 0) {
        goto end;
    }

	/* find index from hash function */
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

			/* packet creation */
            NEW_PKT_SET;

			/* set cur to http request */
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

			/* packet creation */
            NEW_PKT_SET;

            /* append to last list */
            cur->pkt_last->next = pkt;
            cur->pkt_last = pkt;

			/* msg initialization */
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
        char *a, *b;	// a : url , b : cookie

        /* check http req */
        cur = CUR_HTTP_REQ_HT;
        msg = cur->msg;
        /*
		 * if you want to print hexdump of msg, use this code
		 * hexdump(msg, strlen(msg));
		 */
        a = b = NULL;
		/* parse url and cookie from http data */
        parse_http_header(&a, &b, msg);
        if (a != NULL) { puts(a); } else { a = ""; }
        if (b != NULL) { puts(b); } else { b = ""; }

		/* select item from db */
        item = select_item(a, b);

		/* if the item is not exist in the db, validate in sandbox and return 1 ('1' is white) */
        if (item == NULL) {
            pthread_t th;
            struct thread_arg_t *arg;
			/* insert new item into db */
            insert_item(a, b);
			/* arg for thread */
            arg = (struct thread_arg_t *) malloc(1 * sizeof(struct thread_arg_t));
            arg->cur_idx = cur_idx;
            arg->url = a;
            arg->cookie = b;

			/* create new thread with arg and run pi2sand */
            pthread_create(&th, NULL, pi2sand, (void *) arg);
            return 1;
        }

		/* if item is exist in the db, return 1(white) or 2(black) */
        return process_by_list(cur_idx, item->status);
    }

    return 1;

    end:
    return 0;
}

/* print data in hexadecimal for a given length */
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

/* parse url and cookie from data */
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
	/* get 'host' and 'cookie' via tokenize */
    do {
        if (strncmp(ptr, "Host:", 5) == 0) {
            host = ptr + 6;
        } else if (strncmp(ptr, "Cookie:", 7) == 0) {
            tmp = (char *) malloc(sizeof(char) * (strlen(ptr) - 8 + 1));
            strcpy(tmp, ptr + 8);
            *cookie = tmp;
        }
    } while ((ptr = strtok(NULL, "\r\n")) != NULL);

	/* get 'url' from host and uri */
    tmp = (char *) malloc(sizeof(char) * (7 + strlen(host) + strlen(uri) + 1));
    strcpy(tmp, "http://");
    strcat(tmp, (const char *)host);
    strcat(tmp, (const char *)uri);
    *url = tmp;
}

/*
 * consistent hash function
 * ref : "https://arxiv.org/pdf/1406.2294.pdf"
 */
int32_t JumpConsistentHash(uint64_t key, int32_t num_buckets) {
    int32_t b = 1, j = 0;
    while (j < num_buckets) {
        b = j;
        key = key * 2862933555777941757ULL + 1;
        j = (b + 1) * ((double)(1LL << 31) / (double)((key >> 33) + 1));
    }
    return b;
}

/* free memory associated with cur_idx */
void del_http_req_ht(int cur_idx) {
    struct http_request_t *cur;
    struct pkt_set_t *pkt, *pkt2;
    u_char *msg;
    char *a, *b;

    cur = CUR_HTTP_REQ_HT;
    pkt = cur->pkt;
	/* sequentially free memory from the tail */
    while (pkt->next != NULL) {
        pkt2 = pkt->next;
        free(pkt);
        pkt = pkt2;
    }
    free(pkt);

	/* free 'msg' of cur */
    msg = cur->msg;
    free(msg);

	/* free 'cur' */
    free(cur);
    CUR_HTTP_REQ_HT = NULL;
    puts("DELE");
}

/* get cur_idx's status. 1 is white, 2 is black */
int process_by_list(int cur_idx, char *status) {
    struct http_request_t *cur;
    struct pkt_set_t *ptr;
    struct ether_header eth_hdr;

    cur = CUR_HTTP_REQ_HT;
    if (strcmp(status, "WHITE") == 0) {
        ptr = cur->pkt;
		/* sequential packet transmission from head to tail */
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
