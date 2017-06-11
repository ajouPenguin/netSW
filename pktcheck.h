#ifndef __PKTCHECK_H__

#define __PKTCHECK_H__

#include <pthread.h>
#include "def.h"

struct blacklist_t {
    in_addr_t ip_src, ip_src_mask;
    in_addr_t ip_dst, ip_dst_mask;
    u_char l4_proto;
    u_short port_src;   // 0 is any port
    u_short port_dst;
};

int check_packet(int o, const struct pcap_pkthdr *h, u_char *p);
void hexdump(const void* data, size_t size);
void parse_http_header(char **url, char **cookie, char *data);
#endif
