#include "pktcheck.h"

int check_packet(o, h, p)
int o;
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

    if ((tcp_hdr.th_flags & TH_SYN) == TH_SYN ||
        (tcp_hdr.th_flags & TH_FIN) == TH_FIN ||
        (tcp_hdr.th_flags & TH_RST) == TH_RST) {
        goto end;
    }
    MOV_PTR(tcp_hdr.th_off * 4);

    if (remain == 0) {
        goto end;
    }
    if ((tcp_hdr.th_flags & TH_PUSH) == TH_PUSH)
        printf("PSH ");
    if ((tcp_hdr.th_flags & TH_ACK) == TH_ACK)
        printf("ACK ");
    puts("");
    hexdump(ptr, (remain >16)? 16: remain);

    /*
    ptr = packet content start
    remain = packet content size
    */
    //hexdump(ptr, remain);

    /*
    {
        char *url, *cookie;
        url = cookie = NULL;
        parse_http_header(&url, &cookie, ptr);
        if (url != NULL) {
            puts(url);
            free(url);
        }
        if (cookie != NULL){
            puts(cookie);
            free(cookie);
        }
    }
    */

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
