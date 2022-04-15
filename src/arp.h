#ifndef ARP_H
#define ARP_H

#include <sys/types.h>
#include <net/ethernet.h> 		/* struct ether_header */
#include <net/if_arp.h> 		/* struct arphdr, etc. */
#include <netinet/in.h> 		/* in_addr_t, struct in_addr, sockaddr_in, etc. */

struct arp_packet
{
	struct ether_header ap_eth;		/* ethernet frame header */

	struct	arphdr ap_f_hdr;		/* fixed-size header */
	u_int8_t ap_sha[ETH_ALEN];		/* sender hardware address */
	u_int8_t ap_spa[4];				/* sender protocol address */
	u_int8_t ap_tha[ETH_ALEN];		/* target hardware address */
	u_int8_t ap_tpa[4];				/* target protocol address */
} __attribute__ ((__packed__));

#define ap_eth_dhost ap_eth.ether_dhost
#define ap_eth_shost ap_eth.ether_shost
#define ap_eth_type  ap_eth.ether_type

#define	ap_hrd	ap_f_hdr.ar_hrd
#define	ap_pro	ap_f_hdr.ar_pro
#define	ap_hln	ap_f_hdr.ar_hln
#define	ap_pln	ap_f_hdr.ar_pln
#define	ap_op	ap_f_hdr.ar_op


int arp_cache_lookup(in_addr_t ip, const char* dev_name,
                     int* arp_flags, struct ether_addr* ether_out);

int find_mac_addr(in_addr_t ip, const char* dev_name, struct ether_addr* ether_out);

int send_arp_to(const struct arp_packet* pkt, int raw_sock_fd, int if_index);

struct arp_packet* create_arp_packet(const char* eth_dhost, const char* eth_shost,
									 int op_code,
									 const char* sendr_mac, const char* sendr_ip,
									 const char* trgt_mac, const char* trgt_ip);

struct arp_packet* create_arp_reply_packet(const char* sendr_mac, const char* sendr_ip,
										   const char* trgt_mac, const char* trgt_ip);

struct arp_packet* create_arp_request_packet(const char* sendr_mac, const char* sendr_ip, const char* trgt_ip);

#endif // ARP_H
