#include <arpa/inet.h>
#include <linux/if_packet.h>
#include <linux/ip.h>
#include <linux/udp.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <net/if.h>
#include <netinet/ether.h>
#include <unistd.h>

#include "packets.h"

#define DEST_MAC0	0xff
#define DEST_MAC1	0xff
#define DEST_MAC2	0xff
#define DEST_MAC3	0xff
#define DEST_MAC4	0xff
#define DEST_MAC5	0xff

#define ETHER_TYPE	0x1234

#define DEFAULT_IF	"enp3s0"
#define BUF_SIZ		1024

#define MAX_MESSAGE_SIZE 1000

static uint8_t message[MAX_MESSAGE_SIZE];

char sourceMac[6];
char destMac[6];
char type[2];  //shows ether_type

static void decode_bcast();
static void decode_ucast();
static void decode_hello_response();
static void decode_chat();
static void decode_chat_ack();
static void decode_exiting();

int main(int argc, char *argv[])
{
	char sender[INET6_ADDRSTRLEN];
	int sockfd, ret, i;
	int sockopt;
	ssize_t numbytes;
	struct sockaddr_storage their_addr;
	uint8_t buf[BUF_SIZ];
	uint8_t ifName[IFNAMSIZ];
	
	
	/* Get interface name */
	if (argc > 1)
		strcpy(ifName, argv[1]);
	else
		strcpy(ifName, DEFAULT_IF);

	/* Header structures */
	struct ether_header *eh = (struct ether_header *) buf;
	struct iphdr *iph = (struct iphdr *) (buf + sizeof(struct ether_header));
	struct udphdr *udph = (struct udphdr *) (buf + sizeof(struct iphdr) + sizeof(struct ether_header));

	/* Open PF_PACKET socket, listening for EtherType ETHER_TYPE */
	if ((sockfd = socket(PF_PACKET, SOCK_RAW, htons(ETHER_TYPE))) == -1) {
		perror("listener: socket");	
		return -1;
	}


	/* Allow the socket to be reused - incase connection is closed prematurely */
	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &sockopt, sizeof sockopt) == -1) {
		perror("setsockopt");
		close(sockfd);
		exit(EXIT_FAILURE);
	}
	/* Bind to device */
	if (setsockopt(sockfd, SOL_SOCKET, SO_BINDTODEVICE, ifName, IFNAMSIZ-1) == -1)	{
		perror("SO_BINDTODEVICE");
		close(sockfd);
		exit(EXIT_FAILURE);
	}

repeat:	printf("listener: Waiting to recvfrom...\n");
	numbytes = recvfrom(sockfd, buf, BUF_SIZ, 0, NULL, NULL);
	printf("listener: got packet %lu bytes\n", numbytes);

	/* Check the packet is for me */
	if (eh->ether_dhost[0] == DEST_MAC0 &&
			eh->ether_dhost[1] == DEST_MAC1 &&
			eh->ether_dhost[2] == DEST_MAC2 &&
			eh->ether_dhost[3] == DEST_MAC3 &&
			eh->ether_dhost[4] == DEST_MAC4 &&
			eh->ether_dhost[5] == DEST_MAC5) {
		printf("Correct destination MAC address\n");
	} else {
		printf("Wrong destination MAC: %x:%x:%x:%x:%x:%x\n",
						eh->ether_dhost[0],
						eh->ether_dhost[1],
						eh->ether_dhost[2],
						eh->ether_dhost[3],
						eh->ether_dhost[4],
						eh->ether_dhost[5]);
		ret = -1;
		goto done;
	}


	
	/* Print packet */
	int d=0;	

 	printf("\nData:");
	for (i=0; i<numbytes; i++)
 	{
	  printf("%02x:",buf[i]);
  	}
        
        printf("\n");
 	
	printf("\nDestination Mac:");
	for (i=0; i<=5; i++)
        printf("%02x:",buf[i]);
	printf("\n");

 	printf("\nSource Mac:");
	for (i=6; i<=11; i++)
	printf("%02x:",buf[i]);
	printf("\n");

	printf("\nTYPE:");  //ether_type 0x1234
	for (i=12; i<=13; i++)
        printf("%02x:",buf[i]);
	printf("\n");

	//message is the remainder data, not a chat message.
	printf("\nMessage:");
        for (i=14; i<numbytes; i++)
        {
        message[d] = buf[i];
        printf("%02x:",buf[i]);
	d++;
	}


	
	uint8_t *type;
	type = &message[0];
        if (*type == QUERY_BROADCAST)
		decode_bcast();
        else if (*type == QUERY_UNICAST)
        	decode_ucast();
	else if (*type == HELLO_RESPONSE)
        	decode_hello_response();
	else if (*type == CHAT)
        	decode_chat();
	else if (*type == CHAT_ACK)
		decode_chat_ack();
	else if (*type == EXITING)
		decode_exiting();
	else 
		printf("undefined type!");
 

done:	goto repeat;

	close(sockfd);
	return ret;
}



//////////////////////////////////////////////////////////

static void decode_bcast()
{
    struct query_bcast *q;
    q = (struct query_bcast*) message;

    fprintf(stdout, "\n* decoding broadcast query *\n");
    fprintf(stdout, "q->type: %d\n", q->type);

    fprintf(stdout, "q->name: %s\n", q->name);
    fprintf(stdout, "q->surname: %s\n", q->surname);
}

static void decode_ucast()
{
    struct query_ucast *q;
    q = (struct query_ucast*) message;

    fprintf(stdout, "* decoding unicast query *\n");
    fprintf(stdout, "q->type: %d\n", q->type);

    fprintf(stdout, "q->name: %s\n", q->name);
    fprintf(stdout, "q->surname: %s\n", q->surname);

    fprintf(stdout, "q->target_name: %s\n", q->target_name);
    fprintf(stdout, "q->target_surname: %s\n", q->target_surname);
}

static void decode_hello_response()
{
    struct hello_response *q;
    q = (struct hello_response*) message;

    fprintf(stdout, "* decoding hello response *\n");
    fprintf(stdout, "q->type: %d\n", q->type);

    fprintf(stdout, "q->responder_name: %s\n", q->responder_name);
    fprintf(stdout, "q->responder_surname: %s\n", q->responder_surname);

    fprintf(stdout, "q->querier_name: %s\n", q->querier_name);
    fprintf(stdout, "q->querier_surname: %s\n", q->querier_surname);
}

static void decode_chat()
{
    struct chat *q;
    q = (struct chat*) message;

    fprintf(stdout, "* decoding chat *\n");
    fprintf(stdout, "q->type: %d\n", q->type);

    fprintf(stdout, "q->length: %s\n", q->length);

    fprintf(stdout, "q->packet_id: %d\n", q->packet_id);

    fprintf(stdout, "q->message: %s\n", q->message);
    
}

static void decode_chat_ack()
{
    struct chat_ack *q;
    q = (struct chat_ack*) message;

    fprintf(stdout, "* decoding chat ack *\n");
    fprintf(stdout, "q->type: %d\n", q->type);

    fprintf(stdout, "q->packet_id: %d\n", q->packet_id);
}

static void decode_exiting()
{
    struct exiting *q;
    q = (struct exiting*) message;

    fprintf(stdout, "* decoding exiting *\n");
    fprintf(stdout, "q->type: %d\n", q->type);

    fprintf(stdout, "q->name: %s\n", q->name);
    fprintf(stdout, "q->surname: %s\n", q->surname);
}








