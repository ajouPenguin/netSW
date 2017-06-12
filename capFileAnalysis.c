#include <pcap.h>
#include <stdio.h>
#include <stdlib.h>

/* the include files are valid when it is complied on Linux 
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
 */

// litereals realted to distinguishing protocols
#define ETH_II_HSIZE		14		// frame size of ethernet v2
#define IP_PROTO_IP		    0		// IP
#define IP_PROTO_TCP		6		// TCP
#define IP_PROTO_UDP		17		// UDP

int cpkNum = 1;
int isFragment = 0;

unsigned long	net_ip_count;
unsigned long	net_etc_count;
unsigned long	trans_tcp_count;
unsigned long	trans_udp_count;
unsigned long	trans_etc_count;

// Macros
// pntohs : to convert network-aligned 16bit word to host-aligned one
#define pntoh16(p)  ((unsigned short)                       \
		((unsigned short)*((unsigned char *)(p)+0)<<8|  \
		 (unsigned short)*((unsigned char *)(p)+1)<<0))

// pntohl : to convert network-aligned 32bit word to host-aligned one
#define pntoh32(p)  ((unsigned short)*((unsigned char *)(p)+0)<<24|  \
		(unsigned short)*((unsigned char *)(p)+1)<<16|  \
		(unsigned short)*((unsigned char *)(p)+2)<<8|   \
		(unsigned short)*((unsigned char *)(p)+3)<<0)

void process_packet(u_char *args,
		const struct pcap_pkthdr *hdr,
		const u_char *packet)
{
	int i;
	unsigned char ip_ver, ip_proto;

	// ip_offset : 14
	ip_ver  = packet[ETH_II_HSIZE]>>4;     // IP version
	ip_proto    = packet[23];        // protocol above IP

	unsigned short type;

	// Frame Info
	// printf("Frame %d: %5d bytes on wire, %5d bytes captured.\n", cpkNum, hdr->len, hdr->caplen);

	// MAC Address
	//	printf("ETHERNET 2  src : ");
	//	for(i = 0; i < 6; i++) printf("%x%s", packet[i+6], i==5 ? " " : ":");
	//	printf(" dst : ");
	//	for(i = 0; i < 6; i++) printf("%x%s", packet[i], i==5 ? " " : ":");
	//	printf("\n");

	// IP datagram
	if((type = pntoh16(&packet[12])) == 0x0800) {
		net_ip_count++;
		//printf("Internet Protocol Version %d:  src : ", ip_ver);
		//for(i = 0; i < 4; i++)
		//		printf("%d%s", packet[i+26], i==3?" ":".");
		//	printf(" dst : ");
		//	for(i = 0; i < 4; i++)
		//		printf("%d%s", packet[i+30], i==3?" ":".");
		//	printf("\n");
		if(ip_proto == IP_PROTO_UDP) {
			trans_udp_count++;
			// UDP
			//			printf("User Datagram Protocol: ");
			//			printf("Src port : %d  ", packet[i+30] * 16 * 16 + packet[i+31]);
			//			printf("Dst port : %d\n", packet[i+32] * 16 * 16 + packet[i+33]);
			//printf("\n");
			//			printf("Src port : %x%x\n", packet[i+30], packet[i+31]);
		}
		if(ip_proto == IP_PROTO_TCP) { 
			trans_tcp_count++;
			int tcphdr_len = packet[16 * 3 - 2] / 4;

			// 'dst.port == 80'인 패킷처리
			if(packet[36] * 16 * 16 + packet[37] == 80) {
				printf("Frame %d: %5d bytes on wire, %5d bytes captured.\n\n", cpkNum, hdr->len, hdr->caplen);
				if(packet[tcphdr_len + 34] == 'G' &&
					packet[tcphdr_len + 35] == 'E' &&
					packet[tcphdr_len + 36] == 'T') {
					printf("GET REQUEST!\n");
					if(packet[tcphdr_len + 27] == 0x10) {
						isFragment = 1; // flag set
					}
				}
			}

			if(isFragment == 1) {
				if(packet[tcphdr_len + 27] == 0x10) {
					// 패킷을 큐에 push
					printf("[ACK] GET request\n");
					printf("push packet on queue\n");
				} else {
					isFragment = 0;
					// 패킷을 큐에 push
					printf("[PSH, ACK] GET request.\n");
					printf("여기서 큐에 있는 패킷들을 순서대로 pop하여 보냅니다.\n");
				}
			}
		}

	} else {
		net_etc_count++;
	}

	cpkNum++;
}

int main(int argc, char **argv)
{
	pcap_t *pcap;
	const unsigned char *packet;
	char errbuf[PCAP_ERRBUF_SIZE];
	struct pcap_pkthdr header;

	if(argc != 2) {
		printf("usage : %s <filename>\n", argv[0]);
		exit(1);
	}

	pcap = pcap_open_offline(argv[1], errbuf);

	if(pcap == NULL) {
		fprintf(stderr, "error reading pcap file: %s\n", errbuf);
		exit(1);
	}

	pcap_loop(pcap, -1, process_packet, 0);
	printf("%d-ip packets captured\n", net_ip_count);
	printf("%d-udp packets captured\n", trans_udp_count);
	printf("%d-tcp packets captured\n", trans_tcp_count);
	return 0;
}
