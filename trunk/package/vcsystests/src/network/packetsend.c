#include "packetsend.h"
#include "util.h"

#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/time.h>

#include <asm/types.h>

#include <math.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

#include <linux/if_packet.h>
#include <linux/if_ether.h>
#include <linux/if_arp.h>

#define BUF_SIZE ETH_FRAME_TOTALLEN

int s = 0; /*Socketdescriptor*/
void* buffer = NULL;
long total_sent_packets = 0;
char *devicename1 = 0;
char *devicename2 = 0;

int getifindex(struct ifreq *ifr, int s);
void getifmac(struct ifreq *ifr, int s);

int test(int argc, char *argv[]) {
	buffer = (void*)malloc(BUF_SIZE); 	/*Buffer for ethernet frame*/
	unsigned char* etherhead = buffer;	/*Pointer to ethenet header*/
	unsigned char* data = buffer + 14;	/*Userdata in ethernet frame*/
	struct ethhdr *eh = (struct ethhdr *)etherhead; /*Another pointer to ethernet header*/

	unsigned char src_mac[6];		/*our MAC address*/
	unsigned char dest_mac[6];              /*destination MAC address*/

	struct ifreq ifr[2];
	struct sockaddr_ll socket_address;
	int ifindex = 0;			/*Ethernet Interface index*/
	int i,j;
	unsigned long long k;
	int p;
	int length;				/*length of received packet*/
	int sent;				/*length of sent packet*/

	unsigned long long numberofbytes = 1024 * 1024 * 1024;
	unsigned long long oneGBtobesent, calcMbps;

	/*stuff for time measuring: */
	struct timeval begin;
        struct timeval end;
        struct timeval result;
        unsigned long long allovertime;

	if(argc != 3)
	{
		printf("Usage: ./packetsend eth0 eth1\n");
		exit(0);
	}

	devicename1 = (char *) malloc (strlen(argv[1]));
	strcpy(devicename1, argv[1]);
	strncpy(ifr[0].ifr_name, devicename1, IFNAMSIZ);

	devicename2 = (char *) malloc (strlen(argv[2]));
	strcpy(devicename2, argv[2]);
	strncpy(ifr[1].ifr_name, devicename2, IFNAMSIZ);

	printf("Client started, entering initialiation phase...\n");

	/*open socket*/
	s = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
	if (s == -1) {
		perror("socket():");
	        exit(1);
	}
	printf("Successfully opened socket: %i\n", s);
	
	ifindex = getifindex(&ifr[0], s);

	ifindex = ifr[0].ifr_ifindex;
	printf("Successfully got interface index: %i %i\n", ifindex, ifr[0].ifr_ifindex);
	
	/*retrieve corresponding MAC*/
	getifmac(&ifr[0], s);
	getifmac(&ifr[1], s);

        for (i = 0; i < 6; i++) {
		src_mac[i] = ifr[0].ifr_hwaddr.sa_data[i];
		dest_mac[i] = ifr[1].ifr_hwaddr.sa_data[i];
	}
	printf("Successfully got our Sender MAC address: %02X:%02X:%02X:%02X:%02X:%02X\n", 
			src_mac[0],src_mac[1],src_mac[2],src_mac[3],src_mac[4],src_mac[5]);
	printf("Successfully got our Destination MAC address: %02X:%02X:%02X:%02X:%02X:%02X\n", 
			dest_mac[0],dest_mac[1],dest_mac[2],dest_mac[3],dest_mac[4],dest_mac[5]);

	/*prepare sockaddr_ll*/
	socket_address.sll_family   = PF_PACKET;
	socket_address.sll_protocol = htons(ETH_P_IP);
	socket_address.sll_ifindex  = ifindex;
	socket_address.sll_hatype   = ARPHRD_ETHER;
	socket_address.sll_pkttype  = PACKET_OTHERHOST;
	socket_address.sll_halen    = ETH_ALEN;

	socket_address.sll_addr[0]  = dest_mac[0];
	socket_address.sll_addr[1]  = dest_mac[1];
        socket_address.sll_addr[2]  = dest_mac[2];
        socket_address.sll_addr[3]  = dest_mac[3];
        socket_address.sll_addr[4]  = dest_mac[4];
        socket_address.sll_addr[5]  = dest_mac[5];

	socket_address.sll_addr[6]  = 0x00; 
	socket_address.sll_addr[7]  = 0x00;
	
	/*establish signal handler*/
	signal(SIGINT, sigint);
	printf("Successfully established signal handler for SIGINT\n");

	printf("We are in production state, sending packets....\n");

	//for (i = 50; i <= 1500; i += 50) 

	allovertime = 0;
	i = 1500;	
	/*clear the timers:*/
	timerclear(&begin);
	timerclear(&end);

	/*get time before sending.....*/
	gettimeofday(&begin,NULL);
	for (k = 1; k <= numberofbytes; k += i) {
		/*prepare buffer*/
		memcpy((void*)buffer, (void*)dest_mac, ETH_MAC_LEN);
		memcpy((void*)(buffer+ETH_MAC_LEN), (void*)src_mac, ETH_MAC_LEN);
		eh->h_proto = ETH_P_NULL;
		/*fill it with random data....*/
		for (j = 0; j < i - 1; j++) {
			data[j] = (j % 75 + 48);
		}
		data[j] = '\0';

		/*send packet*/
		sent = sendto(s, buffer, i+ETH_HEADER_LEN, 0, (struct sockaddr*)&socket_address, sizeof(socket_address));
		if (sent == -1) {
			perror("sendto():");
			exit(1);
		}

		total_sent_packets++;

	}
	/*get time after sending.....*/

	gettimeofday(&end,NULL);
	timersub(&end,&begin,&result); 

	allovertime += ((result.tv_sec * 1000000 ) + result.tv_usec );
	printf("Sending %lld bytes takes %lld seconds \n", k -1, (long long)(numberofbytes/allovertime));

	oneGBtobesent = (long long)(numberofbytes/allovertime);
	calcMbps = 1024 * 8 / oneGBtobesent;
	printf("Speed = %lldMbps or %lldMBps\n", calcMbps, calcMbps / 8);
	sigint(0); // Print the data

}
void sigint(int signum) {
	/*Clean up.......*/

	struct ifreq ifr;

        if (s == -1)
        	return;

	strncpy(ifr.ifr_name, devicename1, IFNAMSIZ);
	
	ioctl(s, SIOCGIFFLAGS, &ifr);
	ifr.ifr_flags &= ~IFF_PROMISC;
	ioctl(s, SIOCSIFFLAGS, &ifr);
	close(s);
	
	free(devicename1);
	free(devicename2);
	free(buffer);
	
	printf("Sender Application terminating....\n");

	printf("Totally sent: %d packets\n", total_sent_packets);
	exit(0);
}
