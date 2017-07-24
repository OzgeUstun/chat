#include <arpa/inet.h>
#include <linux/if_packet.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <net/if.h>
#include <netinet/ether.h>
#include <stdint.h>

#include "packets.h"

#define MY_NAME "ozge"
#define MY_SURNAME "ustun"

#define TARGET_NAME "devin"
#define TARGET_SURNAME "mungan"

#define MY_DEST_MAC0	0xff
#define MY_DEST_MAC1	0xff
#define MY_DEST_MAC2	0xff
#define MY_DEST_MAC3	0xff
#define MY_DEST_MAC4	0xff
#define MY_DEST_MAC5	0xff

#define DEFAULT_IF	"enp3s0"
#define BUF_SIZ		1024


static void fill_query_bcast(struct query_bcast *q);

static void fill_query_ucast(struct query_ucast *q);

static void hex_print(void *pack, size_t len);


int main(int argc, char *argv[])
{
	int sockfd;
	struct ifreq if_idx;
	struct ifreq if_mac;
	int tx_len = 0;
	char sendbuf[BUF_SIZ];
	struct ether_header *eh = (struct ether_header *) sendbuf;
	struct iphdr *iph = (struct iphdr *) (sendbuf + sizeof(struct ether_header));
	struct sockaddr_ll socket_address;
	char ifName[IFNAMSIZ];
	
	/* Get interface name */
	if (argc > 1)
		strcpy(ifName, argv[1]);
	else
		strcpy(ifName, DEFAULT_IF);

	/* Open RAW socket to send on */
	if ((sockfd = socket(AF_PACKET, SOCK_RAW, IPPROTO_RAW)) == -1) {
	    perror("socket");
	}

	/* Get the index of the interface to send on */
	memset(&if_idx, 0, sizeof(struct ifreq));
	strncpy(if_idx.ifr_name, ifName, IFNAMSIZ-1);
	if (ioctl(sockfd, SIOCGIFINDEX, &if_idx) < 0)
	    perror("SIOCGIFINDEX");
	/* Get the MAC address of the interface to send on */
	memset(&if_mac, 0, sizeof(struct ifreq));
	strncpy(if_mac.ifr_name, ifName, IFNAMSIZ-1);
	if (ioctl(sockfd, SIOCGIFHWADDR, &if_mac) < 0)
	    perror("SIOCGIFHWADDR");

	/* Construct the Ethernet header */
	memset(sendbuf, 0, BUF_SIZ);
	/* Ethernet header */
	eh->ether_shost[0] = ((uint8_t *)&if_mac.ifr_hwaddr.sa_data)[0];
	eh->ether_shost[1] = ((uint8_t *)&if_mac.ifr_hwaddr.sa_data)[1];
	eh->ether_shost[2] = ((uint8_t *)&if_mac.ifr_hwaddr.sa_data)[2];
	eh->ether_shost[3] = ((uint8_t *)&if_mac.ifr_hwaddr.sa_data)[3];
	eh->ether_shost[4] = ((uint8_t *)&if_mac.ifr_hwaddr.sa_data)[4];
	eh->ether_shost[5] = ((uint8_t *)&if_mac.ifr_hwaddr.sa_data)[5];
	eh->ether_dhost[0] = MY_DEST_MAC0;
	eh->ether_dhost[1] = MY_DEST_MAC1;
	eh->ether_dhost[2] = MY_DEST_MAC2;
	eh->ether_dhost[3] = MY_DEST_MAC3;
	eh->ether_dhost[4] = MY_DEST_MAC4;
	eh->ether_dhost[5] = MY_DEST_MAC5;
	
        /* Ethertype field */
	eh->ether_type = htons(0x1234);
	tx_len += sizeof(struct ether_header);

///////////////////////////////////////////////////////////////////////////////////////////

    struct query_bcast query_bcast;
    struct query_ucast query_ucast;


    memset(&query_bcast, 0, sizeof(struct query_bcast));
    memset(&query_ucast, 0, sizeof(struct query_ucast));

    fill_query_bcast(&query_bcast);
    //hex_print((void *) &query_bcast, sizeof(struct query_bcast));
    memcpy(&sendbuf[tx_len],&query_bcast,sizeof(struct query_bcast));
    tx_len=tx_len+sizeof(struct query_bcast);

    fill_query_ucast(&query_ucast);
    //hex_print((void *) &query_ucast, sizeof(struct query_ucast));

/////////////////////////////////////////////////////////////////////////////////////////// 

	//Packet data 

//broadcast için yolladğımız aslında sendbuf'un içinde sırayla. type,kivanc, cakmak
	
	/*sendbuf[tx_len++] = 0x00;
	sendbuf[tx_len++] = 0x6b;
	sendbuf[tx_len++] = 0x69;
	sendbuf[tx_len++] = 0x76;
  	sendbuf[tx_len++] = 0x61;
	sendbuf[tx_len++] = 0x6e;
	sendbuf[tx_len++] = 0x63;
	sendbuf[tx_len++] = 0x00;
	sendbuf[tx_len++] = 0x00;
	sendbuf[tx_len++] = 0x00;	
	sendbuf[tx_len++] = 0x00;
	sendbuf[tx_len++] = 0x63;
	sendbuf[tx_len++] = 0x61;
	sendbuf[tx_len++] = 0x6b;
	sendbuf[tx_len++] = 0x6d;
	sendbuf[tx_len++] = 0x61;
	sendbuf[tx_len++] = 0x6b;
	sendbuf[tx_len++] = 0x00;
	sendbuf[tx_len++] = 0x00;
        sendbuf[tx_len++] = 0x00;
	sendbuf[tx_len++] = 0x00; */


	/* Index of the network device */
	socket_address.sll_ifindex = if_idx.ifr_ifindex;
	/* Address length*/
	socket_address.sll_halen = ETH_ALEN;
	/* Destination MAC */
	socket_address.sll_addr[0] = MY_DEST_MAC0;
	socket_address.sll_addr[1] = MY_DEST_MAC1;
	socket_address.sll_addr[2] = MY_DEST_MAC2;
	socket_address.sll_addr[3] = MY_DEST_MAC3;
	socket_address.sll_addr[4] = MY_DEST_MAC4;
	socket_address.sll_addr[5] = MY_DEST_MAC5;

	/* Send packet */
	if (sendto(sockfd, sendbuf, tx_len, 0, (struct sockaddr*)&socket_address, sizeof(struct sockaddr_ll)) < 0)
	    printf("Send failed\n");

	return 0;
}

static void fill_query_bcast(struct query_bcast *q)
{
    q->type = QUERY_BROADCAST;
    snprintf(q->name, MAX_NAME_SIZE, "%s", MY_NAME);
    snprintf(q->surname, MAX_NAME_SIZE, "%s", MY_SURNAME);
}

static void fill_query_ucast(struct query_ucast *q)
{
    q->type = QUERY_UNICAST;
    snprintf(q->name, MAX_NAME_SIZE, "%s", MY_NAME);
    snprintf(q->surname, MAX_NAME_SIZE, "%s", MY_SURNAME);
    snprintf(q->target_name, MAX_NAME_SIZE, "%s", TARGET_NAME);
    snprintf(q->target_surname, MAX_NAME_SIZE, "%s", TARGET_SURNAME);
}

static void hex_print(void *pack, size_t len)
{
    int i = 0;
    for (i = 0; i < len; i++) {
        //fprintf(stdout, " %c", ((char *) pack)[i]);
    }
    //fprintf(stdout, "\n");
}


