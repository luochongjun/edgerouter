#include "arp.h"

#include <sys/socket.h> 		/* socket() */
#include <sys/ioctl.h> 			/* ioctl() */
#include <arpa/inet.h> 			/* inet_aton(), inet_addr(), etc. */
#include <netinet/ether.h> 		/* ether_aton() etc. */
#include <net/if.h> 			/* struct ifreq */
#include <linux/if_packet.h> 	/* struct sockaddr_ll */
#include <unistd.h> 			/* close() */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

int arp_cache_lookup(in_addr_t ip, const char* dev_name,
                     int* arp_flags_out, struct ether_addr* ether_out)
{
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        perror("socket()");
        close(sock);
        return -1;
    }

    struct arpreq arp;
    struct sockaddr_in* sin;

    memset((void*) &arp, 0, sizeof(arp));
    strncpy(arp.arp_dev, dev_name, sizeof(arp.arp_dev));

    sin = (struct sockaddr_in*) &arp.arp_pa;
    sin->sin_family = AF_INET;
    sin->sin_addr.s_addr = ip;

    if (ioctl(sock, SIOCGARP, (void*) &arp) < 0) {
    	perror("SIOCGARP");
    	close(sock);
    	return -1;
    }

    close(sock);
    if (ether_out)
        memcpy(ether_out->ether_addr_octet, arp.arp_ha.sa_data, ETH_ALEN);

    if (arp_flags_out)
        *arp_flags_out = arp.arp_flags;

    return 1;
}

static int force_arp(in_addr_t dst_ip)
{
	int fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (fd < 0) {
		perror("socket()");
		return -1;
	}

	struct sockaddr_in sin;
	memset((void*) &sin, 0, sizeof(sin));

	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = dst_ip;
	sin.sin_port = htons(67);

	sendto(fd, NULL, 0, 0, (struct sockaddr*) &sin, sizeof(sin));
	close(fd);

	return 1;
}

int find_mac_addr(in_addr_t ip, const char* dev_name, struct ether_addr* ether_out)
{
    int max_loop = 3;
    int arp_flags = 0;
    do {
        if (arp_cache_lookup(ip, dev_name, &arp_flags, ether_out) > 0 && arp_flags == 0x2)
            return 1;

        force_arp(ip);
        sleep(1);
    } while (--max_loop > 0 && !arp_flags);

    return arp_cache_lookup(ip, dev_name, NULL, ether_out);
}

int send_arp_to(const struct arp_packet* pkt, int raw_sock_fd, int if_index)
{
	struct sockaddr_ll dst_addr;
	memset((void*) &dst_addr, 0, sizeof(dst_addr));

	dst_addr.sll_ifindex = if_index;
	dst_addr.sll_halen = ETH_ALEN;

	dst_addr.sll_addr[0] = pkt->ap_eth_dhost[0];
	dst_addr.sll_addr[1] = pkt->ap_eth_dhost[1];
	dst_addr.sll_addr[2] = pkt->ap_eth_dhost[2];
	dst_addr.sll_addr[3] = pkt->ap_eth_dhost[3];
	dst_addr.sll_addr[4] = pkt->ap_eth_dhost[4];
	dst_addr.sll_addr[5] = pkt->ap_eth_dhost[5];

	ssize_t sent = sendto(raw_sock_fd, (void*) pkt, sizeof(struct arp_packet), 
						  0, (struct sockaddr*) &dst_addr, sizeof(dst_addr));

	if (sent < 0) {
		perror("sendto()");
		return -1;
	}

	return sent;
}

struct arp_packet* create_arp_packet(const char* eth_dhost, const char* eth_shost,
									 int op_code,
									 const char* sendr_mac, const char* sendr_ip,
									 const char* trgt_mac, const char* trgt_ip)
{
	struct arp_packet* ap = (struct arp_packet*) malloc(sizeof(struct arp_packet));

	/* setup Ethernet frame header */
	memcpy(ap->ap_eth_dhost, ether_aton(eth_dhost)->ether_addr_octet, ETH_ALEN);
	memcpy(ap->ap_eth_shost, ether_aton(eth_shost)->ether_addr_octet, ETH_ALEN);
	ap->ap_eth_type = htons(ETHERTYPE_ARP);

	int spa = inet_addr(sendr_ip);
	int tpa = inet_addr(trgt_ip);

	ap->ap_hrd = htons(1); /* Ethernet */
	ap->ap_pro = htons(0x0800); /* Ipv4 */
	ap->ap_hln = ETH_ALEN;
	ap->ap_pln = 4;
	ap->ap_op = htons(op_code);
	memcpy(ap->ap_sha, ether_aton(sendr_mac)->ether_addr_octet, ETH_ALEN);
	memcpy(ap->ap_spa, (void*) &spa, sizeof(ap->ap_spa));
	memcpy(ap->ap_tha, ether_aton(trgt_mac)->ether_addr_octet, ETH_ALEN);
	memcpy(ap->ap_tpa, (void*) &tpa, sizeof(ap->ap_tpa));

	return ap;
}

struct arp_packet* create_arp_reply_packet(const char* sendr_mac, const char* sendr_ip,
										   const char* trgt_mac, const char* trgt_ip)
{
	return create_arp_packet(trgt_mac, sendr_mac, ARPOP_REPLY,
                             sendr_mac, sendr_ip, trgt_mac, trgt_ip);
}

struct arp_packet* create_arp_request_packet(const char* sendr_mac, const char* sendr_ip, const char* trgt_ip)
{
	return create_arp_packet("ff:ff:ff:ff:ff:ff", sendr_mac, ARPOP_REQUEST,
                             sendr_mac, sendr_ip, "00:00:00:00:00:00", trgt_ip);
}
