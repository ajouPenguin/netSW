/*
 * bittwistb - pcap based ethernet bridge
 * Copyright (C) 2007 Addy Yeow Chin Heng <ayeowch@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#ifndef _BITTWIST_H_
#define _BITTWIST_H_

#include "def.h"
#include <pthread.h> // AAAA

void bridge_on(void);
void bridge_fwd(u_char *port, const struct pcap_pkthdr *header, const u_char *pkt_data);
void send_packets(int outport,
                  int sport,
                  const struct ether_addr *ether_dhost,
                  const struct ether_addr *ether_shost,
                  const u_char *pkt_data,
                  int pkt_len);
void hash_alarm_handler(int signum);
int hash_alarm(unsigned int seconds);
int gethwaddr(struct ether_addr *ether_addr, char *device);
void info(void);
void cleanup(int signum);
void notice(const char *fmt, ...);
void error(const char *fmt, ...);
void usage(void);


// AAAA START
int blacklist_flag = 0;

struct blacklist_t {
    in_addr_t ip_src, ip_src_mask;
    in_addr_t ip_dst, ip_dst_mask;
    u_char l4_proto;
    u_short port_src;   // 0 is any port
    u_short port_dst;
};

struct blacklist_t blist[100];
int blist_size;

int check_blacklist(int outport, struct ether_header *eth_hdr, const u_char *pkt_data);

void *blacklist_control(void *arg);
// AAAA END
#endif  /* !_BITTWIST_H_ */
