#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <string.h>
#include <unistd.h>
#include <pcap.h>
#include <sys/ioctl.h>
//#include <net/if.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <linux/ethtool.h>

#include <linux/mii.h>

#include <ifaddrs.h>

#include <time.h>
#include <net/ethernet.h>
#include <net/if.h>
#include <net/if_arp.h>
#include <net/if_packet.h>
#include <netpacket/packet.h>
#include <sockios.h>

#include <CUnit/CUnit.h>
#define LOGFILE		"/tmp/network-test.log"
#define ETH_P_VC_CUSTOM		(1000)
#define TIMEOUT_SEC		(2)


static CU_pSuite pSuite = NULL;
char **portnames;
int num_ports = 0;

#define MAX_INTERFACES	(sizeof(portnames)/sizeof(char*))
#define CMDSIZE 100

struct port_def {
	char portname[16];
	char state;
	char if_addr[16];
	unsigned int ipaddr;
	unsigned int ifindex;

	struct sockaddr_ll sll;
};

static pcap_t *pcap = NULL;
static FILE *fplog = NULL;

struct port_def **port;


static char *print_mac(char *if_addr)
{
	char *macaddr = malloc(32);
	sprintf(macaddr, "%02x:%02x:%02x:%02x:%02x:%02x",
			if_addr[0] & 0xFF, if_addr[1] & 0xFF, if_addr[2] & 0xFF,
			if_addr[3] & 0xFF, if_addr[4] & 0xFF, if_addr[5] & 0xFF);
	return macaddr;
}


static int fill_data(char *buff, size_t n)
{
	int i;
	for (i = 0; i < n - 1; i++) {
		buff[i] = (48 + i % 75);
	}
	buff[i] = '\0';
	return n;
}

void dump_frame(char *p, size_t n)
{
	size_t i;

	for (i = 0; i < n; i++) {
		fprintf(fplog, "%02x ", p[i] & 0xFF);
		if (i != 0 && (i % 31==0)) {
			fprintf(fplog, "\n");
		}
	}

}

static int check_data(char *b1, char *b2, size_t n, int *pos)
{
	int i;
	for (i = 0; i < n; i++) {
		if (b1[i] != b2[i]) {
			*pos = i;
			fprintf(fplog, "Check_DATA failed @pos=%d\n", i);
			return -1;
		}
	}

	return 0;
}


static struct port_def *getport(int idx)
{
	return port[idx];
}

static int eth_self_test(char *interface)
{
	int fd, ret;
	int err;
	struct ifreq ifr;
	struct ethtool_drvinfo drvinfo;
	struct ethtool_test *test;
	struct ethtool_gstrings *strings;

	fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (fd <= 0) {
		fprintf(fplog, "Unable to open socket: %s\n", strerror(errno));
		return -1;
	}
	strncpy(ifr.ifr_name, interface, sizeof(ifr.ifr_name));

	drvinfo.cmd = ETHTOOL_GDRVINFO;
	ifr.ifr_data = (caddr_t) & drvinfo;
	err = ioctl(fd, SIOCETHTOOL, &ifr);
	if (err < 0) {
		perror("Cannot get driver information");
		return 72;
	}

	test = calloc(1, sizeof(*test) + drvinfo.testinfo_len * sizeof(long long));
	if (!test) {
		perror("Cannot allocate memory for test info");
		return 73;
	}
	memset(test->data, 0, drvinfo.testinfo_len * sizeof(long long));
	test->cmd = ETHTOOL_TEST;
	test->len = drvinfo.testinfo_len;
	test->flags = 0;
	ifr.ifr_data = (caddr_t) test;
	err = ioctl(fd, SIOCETHTOOL, &ifr);
	if (err < 0) {
		perror("Cannot test");
		free(test);
		return 74;
	}

	strings = calloc(1, sizeof(*strings) +
			 drvinfo.testinfo_len * ETH_GSTRING_LEN);
	if (!strings) {
		perror("Cannot allocate memory for strings");
		free(test);
		return 73;
	}
	memset(strings->data, 0, drvinfo.testinfo_len * ETH_GSTRING_LEN);
	strings->cmd = ETHTOOL_GSTRINGS;
	strings->string_set = ETH_SS_TEST;
	strings->len = drvinfo.testinfo_len;
	ifr.ifr_data = (caddr_t) strings;
	err = ioctl(fd, SIOCETHTOOL, &ifr);
	if (err < 0) {
		perror("Cannot get strings");
		free(test);
		free(strings);
		return 74;
	}

	if (test->flags & ETH_TEST_FL_FAILED) {
		fprintf(stderr, "ETh self test FAILED: %s\n", interface);
	}
	else 
		fprintf(stderr, "ETh self test PASSED: %s\n", interface);

	free(test);
	free(strings);

	close(fd);
	return 0;
}

static int set_speed(int speed, char *interface)
{
	int fd, ret;
	struct ethtool_cmd edata;
	struct ifreq ifr;

	fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (fd <= 0) {
		fprintf(fplog, "Unable to open socket: %s\n", strerror(errno));
		return -1;
	}

	strncpy(ifr.ifr_name, interface, sizeof(ifr.ifr_name));
	ifr.ifr_data = &edata;

	bzero(&edata, sizeof(struct ethtool_cmd));

	edata.cmd = ETHTOOL_GSET;

	ret = ioctl(fd, SIOCETHTOOL, &ifr);
	if (ret < 0) {
		fprintf(fplog, "SIOCETHTOOL error: (%s:%d) %s\n", __FUNCTION__, __LINE__, strerror(errno));
		close(fd);
		return -1;
	}

	edata.cmd = ETHTOOL_SSET;
	ethtool_cmd_speed_set(&edata, speed); // internal API to set speed

	ret = ioctl(fd, SIOCETHTOOL, &ifr);
	if (ret < 0) {
		fprintf(fplog, "SIOCETHTOOL error: %s\n", strerror(errno));
		return -1;
	}

	/* Allow interface to [possibly] settle */
	fprintf(fplog, "waiting for interface to apply settings\n");
	sleep(2);

	close(fd);
	return 0;
}

static int test_check_speed_data(int speed, int if_send, int if_recv, int size, unsigned long long numberofpackets)
{
	int txfd = 0, rxfd = 0;
	char *txframe = NULL;
	char *rxframe = NULL;
	struct port_def *psend = NULL;
	struct port_def *precv = NULL;
	struct sockaddr_ll sll;
	char *p = NULL;
	struct ether_header *eh;
	int n, ntemp;
	int nc = 0;
	struct timeval tv = {0, 0};
	socklen_t addrlen;
	int err;
	struct sockaddr_ll rll;

	/* Speed test is needed only when the number of bytes is greater than a GB 
	   as we check with micro second calculation */
	 
        unsigned long long numberofbytes = numberofpackets * size;
        unsigned long long mulfactor, oneGBtobesent, calcMbps;
	char speedunit;

        /*stuff for time measuring: */
        struct timeval begin;
        struct timeval end;
        struct timeval result;
        unsigned long long allovertime;
	
	/* Get Interface properties */
	/* Set the interface speed */
	
	psend = getport(if_send);
	precv = getport(if_recv);

	if(speed)
	{
		fprintf(fplog, "Setting the speed to %d in interface %s, %s\n", speed, psend->portname, precv->portname);
		set_speed(speed, psend->portname);
		set_speed(speed, precv->portname);
	}

	if (!psend || !precv) {
		err = -EINVAL;
		fprintf(fplog, "!psend(%d) || !precv (%d)\n", if_send, if_recv);
		goto bail;
	}

	txframe = calloc(1, size + ETHER_HDR_LEN); // buffer to be filled to send data
	rxframe = calloc(1, size + ETHER_HDR_LEN); // empty buffer to hold recieved data

	/* Create a transmit socket. */
	txfd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
	if (txfd < 0) {
		err = -errno;
		fprintf(fplog, "txsocket: (%d): %s\n", -errno, strerror(errno));
		goto bail;
	}

	/* Create a receive socket */
	rxfd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
	if (rxfd < 0) {
		err = -errno;
		fprintf(fplog, "rxsocket: (%d): %s\n", -errno, strerror(errno));
		goto bail;
	}

	err = setsockopt(txfd, SOL_SOCKET, SO_BINDTODEVICE,
			 psend->portname, strlen(psend->portname) + 1);
	if (err != 0) {
		err = -errno;
		fprintf(fplog, "txsetsockopt: (%d): %s\n", -errno, strerror(errno));
		goto bail;
	}
	err = setsockopt(rxfd, SOL_SOCKET, SO_BINDTODEVICE,
			 precv->portname, strlen(precv->portname) + 1);
	if (err != 0) {
		err = -errno;
		fprintf(fplog, "rxsetsockopt-bind2device: (%d): %s\n", -errno, strerror(errno));
		goto bail;
	}

	/* Timeout if we dont receive a frame within TIMEOUT seconds*/
	tv.tv_sec = TIMEOUT_SEC;
	tv.tv_usec = 0;

	if (setsockopt(rxfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(struct timeval)) < 0) {
		err = -errno;
		fprintf(fplog, "rxsetsockopt-timeout: (%d): %s\n", -errno, strerror(errno));
		goto bail;
	}


	//err = bind(rxfd, (struct sockaddr *)(&precv->sll), sizeof(struct sockaddr_ll));
	//if (err != 0) {
		//return -103;
	//}
	//if (!psend->state || !precv->state) {
		//size = -4;
		//goto bail;
	//}

	/* Fill a tx-ll sockaddr */
	bzero(&sll, sizeof(struct sockaddr_ll));
	memcpy(&sll, &(psend->sll), sizeof(struct sockaddr_ll));

	/* Fill a rx-ll sockaddr */
	bzero(&rll, sizeof(struct sockaddr_ll));
	memcpy(&rll, &(psend->sll), sizeof(struct sockaddr_ll));
	addrlen = sizeof(struct sockaddr_ll);

	/* ------------------------------------------------------------------
	 * Prepare to craft the frame
	 * ----------------------------------------------------------------*/

	/* Step 1: Set the ethernet frame */
	eh = (struct ether_header *) txframe;
	/* Step 1a: destination mac is the address of interface that
	 * the caller requested in if_recv. */
	memcpy(&eh->ether_dhost, precv->if_addr, ETHER_ADDR_LEN);

	/* Step 1b: source mac is the address of interface that
	 * the caller requested in if_send. */
	memcpy(&eh->ether_shost, psend->if_addr, ETHER_ADDR_LEN);

	/* Step 2: We use a type of 1000 - so that we can quickly
	 * identify if this is the packet we sent out or not.
	 * Note: if_ether.h give a list of ETH_P_xxx types that
	 * are numbers assigned by IANA. Since this is for
	 * internal use, we simply use one that is [in]-convenient
	 */
	eh->ether_type = htons(ETH_P_VC_CUSTOM);

	/* Step 3: Frame header is filled. No pack the payload
	 * with bytes that are readable. */
	p = txframe + ETHER_HDR_LEN;
	n = fill_data(p, size);

	if(speed)
	{
		/* time begin and end is cleared */
		allovertime = 0;

		/*clear the timers:*/
		timerclear(&begin);
		timerclear(&end);

		/*get time before sending.....*/
		gettimeofday(&begin,NULL);
	}

	/* Step 4: send the "packet" */
	fprintf(fplog, "==> numberofpackets to be sent =%d\n", numberofpackets);
	while(numberofpackets--)
	{
		ntemp = n;
resend:
		n = sendto(txfd, txframe, n + ETHER_HDR_LEN, MSG_DONTWAIT,
			   (struct sockaddr *) &sll, sizeof(struct sockaddr_ll));

		if(errno == EAGAIN) goto resend;
		if(errno == EMSGSIZE) { 
			printf("MSGSIZE: Number of bytes sent: %d", n); 
			goto bail;
		}

		if (n != (ETHER_HDR_LEN + size)) {
			err = -errno;
			fprintf(fplog, "sendto: (%d): %s numberofpackets=%d, bytes sent=%d\n", -errno, strerror(errno), numberofpackets, n);
			goto bail;
		}

		/* Read in a loop till we get a packet of interest */
		do {
			n = 0;
			if (nc++ >= 30)
				break;
			n = recvfrom(rxfd, rxframe, size + ETHER_HDR_LEN, 0,
				   (struct sockaddr *) &rll, &addrlen);
			if (n <=0 && errno == EAGAIN) {
				continue;
			}

			eh = (struct ether_header *)(rxframe);
		} while (eh->ether_type != htons(ETH_P_VC_CUSTOM));
		nc = 0; /* Got the correct packet */
		
		if (n != (ETHER_HDR_LEN + size)) {
			err = -EAGAIN;
			fprintf(fplog, "...... seems like we waited too long to recvfrom\n");
			goto bail;
		}

		/* 
		   Note: We are calculating for loop back tests, assuming no other packet flows
		         between the ethernet ports
                */
		/* Check data received is correct */
		/*
		int pos;
		if (check_data(txframe + ETHER_HDR_LEN, rxframe + ETHER_HDR_LEN, size, &pos) != 0) {
			fprintf(fplog, "data check failed on %s -> %s, l = %d, pos = %d\n",
			psend->portname, precv->portname, size, pos);
			err = -EINVAL;
			goto bail;
		}
		*/
		n = ntemp;

	}
	if(speed)
	{
		gettimeofday(&end,NULL);
		timersub(&end,&begin,&result);

		allovertime += ((result.tv_sec * 1000000) + result.tv_usec );
		numberofbytes = numberofbytes * (1000 * 1000);
		fprintf(fplog,"Recieved %lld bytes takes %lf seconds \n", numberofbytes, (double)allovertime / (1000 * 1000));

		oneGBtobesent = numberofbytes / allovertime;

		if(oneGBtobesent >= (1000 * 1000))
		{
			mulfactor = 1000 * 1000;
			speedunit = 'G';
		}
		else if(oneGBtobesent >= 1000)
		{
			mulfactor = 1000;
			speedunit = 'M';
		}
		else if(oneGBtobesent >= 1)
		{
			mulfactor = 1;
			speedunit = 'K';
		}

		calcMbps = (numberofbytes) / (mulfactor * allovertime);

		fprintf(fplog, "Speed = %lld%cbps\n", calcMbps * 8, speedunit);
	}

	//fprintf(fplog, "TXFRAME......................................\n");
	//dump_frame(txframe, size + ETHER_HDR_LEN);

	//fprintf(fplog, "RXFRAME......................................\n");
	//dump_frame(rxframe, n);

bail:
	//fprintf(fplog, "ret = %d\n", size);
	if(speed)
	{
		fprintf(fplog, "Resetting the speed to 1000 in interface %s, %s\n", psend->portname, precv->portname);
		set_speed(1000, psend->portname); /* Reset to default speed */
		set_speed(1000, precv->portname);
	}

	if (txframe)
		free(txframe);
	if (rxframe)
		free(rxframe);

	if (txfd > 0)
		close(txfd);
	if (rxfd > 0)
		close(rxfd);
	return err;
}

static unsigned long getpps_forspeed(int speed)
{
/*
        * Packet format:
        * Interface Gap | Preamble | DST MAC | SRC MAC | Type | Data 	| CRC
        *    12	        |    8	   |    6    |    6    |   2  | 46(min) |  4

	* Total = 84 Bytes minimum
	* 1Gbps = 1,000,000,000 bits/s = (1,000,000,000 bits/s) / (8 bits/byte)= 125,000,000 bytes/s
	* PPS = (125,000,000 bytes/s) / (84 bytes/packet) = 1,488,095 pps.
	*
 	* same calculation to different port speed:

	* TABULAR SHEET REFERENCE 
	* Speed		bits/sec	maxPPS
	* 10Mbps 	10000000	14881
	* 100Mbps	100000000	148810
	* 1Gbps 	1000000000	1488095
	* 10Gbps	10000000000	14880952
*/
	return ((speed * 1000000 ) / ( 8 * 84));
}

static int test_check_data(int if_send, int if_recv, int size, unsigned long long numberofpackets)
{
	return test_check_speed_data(0, if_send, if_recv, size, numberofpackets);
}

static int test_check_speed(int speed, int if_send, int if_recv)
{
	int getppsvalue = getpps_forspeed(speed);
	return test_check_speed_data(speed, if_send, if_recv, 1, getppsvalue); /* 84 --> total eframe packet */
}

static int test_tcp_check_data(int if_send, int if_recv, int size)
{
	return size;
}

#if 0
static void test_check_data_port_0_1(void)
{

}

static void test_check_data_port_0_2(void)
{

	CU_ASSERT_EQUAL(test_check_speed(10, 0, 2), 0); 
	CU_ASSERT_EQUAL(test_check_speed(100, 0, 2), 0); 
	CU_ASSERT_EQUAL(test_check_speed(1000, 0, 2), 0); 
}


static void test_check_data_port_0_3(void)
{
	CU_ASSERT_EQUAL(test_check_data(0, 3, 32, 1), 0);
	CU_ASSERT_EQUAL(test_check_data(0, 3, 64, 1), 0);
	CU_ASSERT_EQUAL(test_check_data(0, 3, 128, 1), 0);
	CU_ASSERT_EQUAL(test_check_data(0, 3, 256, 1), 0);
	CU_ASSERT_EQUAL(test_check_data(0, 3, 512, 1), 0);
	CU_ASSERT_EQUAL(test_check_data(0, 3, 1024, 1), 0);
	CU_ASSERT_EQUAL(test_check_data(0, 3, 255, 1), 0);

	CU_ASSERT_EQUAL(test_check_speed(10, 0, 3), 0); 
	CU_ASSERT_EQUAL(test_check_speed(100, 0, 3), 0); 
	CU_ASSERT_EQUAL(test_check_speed(1000, 0, 3), 0); 
}

/*--------------------------------------------------------------------------*/
static void test_check_data_port_1_0(void)
{
	CU_ASSERT_EQUAL(test_check_data(1, 0, 32, 1), 0);
	CU_ASSERT_EQUAL(test_check_data(1, 0, 64, 1), 0);
	CU_ASSERT_EQUAL(test_check_data(1, 0, 128, 1), 0);
	CU_ASSERT_EQUAL(test_check_data(1, 0, 256, 1), 0);
	CU_ASSERT_EQUAL(test_check_data(1, 0, 512, 1), 0);
	CU_ASSERT_EQUAL(test_check_data(1, 0, 1024, 1), 0);
	CU_ASSERT_EQUAL(test_check_data(1, 0, 255, 1), 0);

	CU_ASSERT_EQUAL(test_check_speed(10, 1, 0), 0); 
	CU_ASSERT_EQUAL(test_check_speed(100, 1, 0), 0); 
	CU_ASSERT_EQUAL(test_check_speed(1000, 1, 0), 0); 
	return;
}

static void test_check_data_port_1_2(void)
{
	CU_ASSERT_EQUAL(test_check_data(1, 2, 32, 1), 32);
	CU_ASSERT_EQUAL(test_check_data(1, 2, 64, 1), 64);
	CU_ASSERT_EQUAL(test_check_data(1, 2, 128, 1), 128);
	CU_ASSERT_EQUAL(test_check_data(1, 2, 256, 1), 256);
	CU_ASSERT_EQUAL(test_check_data(1, 2, 512, 1), 512);
	CU_ASSERT_EQUAL(test_check_data(1, 2, 1024, 1), 1024);
	CU_ASSERT_EQUAL(test_check_data(1, 2, 255, 1), 255);

	CU_ASSERT_EQUAL(test_check_speed(10, 1, 2), 0); 
	CU_ASSERT_EQUAL(test_check_speed(100, 1, 2), 0); 
	CU_ASSERT_EQUAL(test_check_speed(1000, 1, 2), 0); 
	return;
}

static void test_check_data_port_1_3(void)
{
	CU_ASSERT_EQUAL(test_check_data(1, 3, 32, 1), 32);
	CU_ASSERT_EQUAL(test_check_data(1, 3, 64, 1), 64);
	CU_ASSERT_EQUAL(test_check_data(1, 3, 128, 1), 128);
	CU_ASSERT_EQUAL(test_check_data(1, 3, 256, 1), 256);
	CU_ASSERT_EQUAL(test_check_data(1, 3, 512, 1), 512);
	CU_ASSERT_EQUAL(test_check_data(1, 3, 1024, 1), 1024);
	CU_ASSERT_EQUAL(test_check_data(1, 3, 255, 1), 255);

	CU_ASSERT_EQUAL(test_check_speed(10, 1, 3), 0); 
	CU_ASSERT_EQUAL(test_check_speed(100, 1, 3), 0); 
	CU_ASSERT_EQUAL(test_check_speed(1000, 1, 3), 0); 
	return;
}
/*--------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------*/
static void test_check_data_port_2_0(void)
{
	CU_ASSERT_EQUAL(test_check_data(2, 0, 32, 1), 32);
	CU_ASSERT_EQUAL(test_check_data(2, 0, 64, 1), 64);
	CU_ASSERT_EQUAL(test_check_data(2, 0, 128, 1), 128);
	CU_ASSERT_EQUAL(test_check_data(2, 0, 256, 1), 256);
	CU_ASSERT_EQUAL(test_check_data(2, 0, 512, 1), 512);
	CU_ASSERT_EQUAL(test_check_data(2, 0, 1024, 1), 1024);
	CU_ASSERT_EQUAL(test_check_data(2, 0, 255, 1), 255);

	CU_ASSERT_EQUAL(test_check_speed(10, 2, 0), 0); 
	CU_ASSERT_EQUAL(test_check_speed(100, 2, 0), 0); 
	CU_ASSERT_EQUAL(test_check_speed(1000, 2, 0), 0); 
	return;
}

static void test_check_data_port_2_1(void)
{
	CU_ASSERT_EQUAL(test_check_data(2, 1, 32, 1), 32);
	CU_ASSERT_EQUAL(test_check_data(2, 1, 64, 1), 64);
	CU_ASSERT_EQUAL(test_check_data(2, 1, 128, 1), 128);
	CU_ASSERT_EQUAL(test_check_data(2, 1, 256, 1), 256);
	CU_ASSERT_EQUAL(test_check_data(2, 1, 512, 1), 512);
	CU_ASSERT_EQUAL(test_check_data(2, 1, 1024, 1), 1024);
	CU_ASSERT_EQUAL(test_check_data(2, 1, 255, 1), 255);

	CU_ASSERT_EQUAL(test_check_speed(10, 2, 1), 0); 
	CU_ASSERT_EQUAL(test_check_speed(100, 2, 1), 0); 
	CU_ASSERT_EQUAL(test_check_speed(1000, 2, 1), 0); 
	return;
}

static void test_check_data_port_2_3(void)
{
	CU_ASSERT_EQUAL(test_check_data(2, 3, 32, 1), 32);
	CU_ASSERT_EQUAL(test_check_data(2, 3, 64, 1), 64);
	CU_ASSERT_EQUAL(test_check_data(2, 3, 128, 1), 128);
	CU_ASSERT_EQUAL(test_check_data(2, 3, 256, 1), 256);
	CU_ASSERT_EQUAL(test_check_data(2, 3, 512, 1), 512);
	CU_ASSERT_EQUAL(test_check_data(2, 3, 1024, 1), 1024);
	CU_ASSERT_EQUAL(test_check_data(2, 3, 255, 1), 255);

	CU_ASSERT_EQUAL(test_check_speed(10, 2, 3), 0); 
	CU_ASSERT_EQUAL(test_check_speed(100, 2, 3), 0); 
	CU_ASSERT_EQUAL(test_check_speed(1000, 2, 3), 0); 
	return;
}
/*--------------------------------------------------------------------------*/


/*--------------------------------------------------------------------------*/
static void test_check_data_port_3_0(void)
{
	CU_ASSERT_EQUAL(test_check_data(3, 0, 32, 1), 32);
	CU_ASSERT_EQUAL(test_check_data(3, 0, 64, 1), 64);
	CU_ASSERT_EQUAL(test_check_data(3, 0, 128, 1), 128);
	CU_ASSERT_EQUAL(test_check_data(3, 0, 256, 1), 256);
	CU_ASSERT_EQUAL(test_check_data(3, 0, 512, 1), 512);
	CU_ASSERT_EQUAL(test_check_data(3, 0, 1024, 1), 1024);
	CU_ASSERT_EQUAL(test_check_data(3, 0, 255, 1), 255);

	CU_ASSERT_EQUAL(test_check_speed(10, 3, 0), 0); 
	CU_ASSERT_EQUAL(test_check_speed(100, 3, 0), 0); 
	CU_ASSERT_EQUAL(test_check_speed(1000, 3, 0), 0); 
	return;
}

static void test_check_data_port_3_1(void)
{
	CU_ASSERT_EQUAL(test_check_data(3, 1, 32, 1), 32);
	CU_ASSERT_EQUAL(test_check_data(3, 1, 64, 1), 64);
	CU_ASSERT_EQUAL(test_check_data(3, 1, 128, 1), 128);
	CU_ASSERT_EQUAL(test_check_data(3, 1, 256, 1), 256);
	CU_ASSERT_EQUAL(test_check_data(3, 1, 512, 1), 512);
	CU_ASSERT_EQUAL(test_check_data(3, 1, 1024, 1), 1024);
	CU_ASSERT_EQUAL(test_check_data(3, 1, 255, 1), 255);

	CU_ASSERT_EQUAL(test_check_speed(10, 3, 1), 0); 
	CU_ASSERT_EQUAL(test_check_speed(100, 3, 1), 0); 
	CU_ASSERT_EQUAL(test_check_speed(1000, 3, 1), 0); 
	return;
}

static void test_check_data_port_3_2(void)
{
	CU_ASSERT_EQUAL(test_check_data(3, 2, 32, 1), 32);
	CU_ASSERT_EQUAL(test_check_data(3, 2, 64, 1), 64);
	CU_ASSERT_EQUAL(test_check_data(3, 2, 128, 1), 128);
	CU_ASSERT_EQUAL(test_check_data(3, 2, 256, 1), 256);
	CU_ASSERT_EQUAL(test_check_data(3, 2, 512, 1), 512);
	CU_ASSERT_EQUAL(test_check_data(3, 2, 1024, 1), 1024);
	CU_ASSERT_EQUAL(test_check_data(3, 2, 255, 1), 255);

	CU_ASSERT_EQUAL(test_check_speed(10, 3, 2), 0); 
	CU_ASSERT_EQUAL(test_check_speed(100, 3, 2), 0); 
	CU_ASSERT_EQUAL(test_check_speed(1000, 3, 2), 0); 
	return;
}
#endif
/*--------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------*/
static void test_tcp_send_0_1(void)
{
	CU_ASSERT_EQUAL(test_tcp_check_data(0, 1, 1024), 1024);
}
/*--------------------------------------------------------------------------*/

static void test_ping(void)
{
	int ret = system("ping -c9 -w5 www.google.com > /tmp/nw-ping-test 2>&1");

	return;
}

int test_port_state(int port_index)
{
	if (port[port_index]->state != 1) {
		fprintf(fplog, "Port [%s] is DOWN.\n", port[port_index]->portname);
	} else {

		fprintf(fplog, "Port [%s] is UP.\n", port[port_index]->portname);
	}
	return port[port_index]->state;
}

/*--------------------------------------------------------------------------*/
void test_port_0(void)
{
	/*
	 * test_port_state
	 * test_port_0_1
	 * test_port_0_2
	 * test_speed_10
	 * test_speed_100
	 * test_speed_1000
	 */
	CU_ASSERT_EQUAL(test_port_state(0), 1);
	CU_ASSERT_EQUAL(test_port_state(1), 1);
	CU_ASSERT_EQUAL(test_port_state(2), 1);

	CU_ASSERT_EQUAL(test_check_data(0, 1, 32, 1000), 0);
	CU_ASSERT_EQUAL(test_check_data(0, 1, 64, 1000), 0);
	CU_ASSERT_EQUAL(test_check_data(0, 1, 128, 1000), 0);
	CU_ASSERT_EQUAL(test_check_data(0, 1, 256, 1000), 0);
	CU_ASSERT_EQUAL(test_check_data(0, 1, 512, 1000), 0);
	CU_ASSERT_EQUAL(test_check_data(0, 1, 1024, 1000), 0);
	CU_ASSERT_EQUAL(test_check_data(0, 1, 255, 1000), 0);

	CU_ASSERT_EQUAL(test_check_data(0, 2, 32, 1), 0);
	CU_ASSERT_EQUAL(test_check_data(0, 2, 64, 1), 0);
	CU_ASSERT_EQUAL(test_check_data(0, 2, 128, 1), 0);
	CU_ASSERT_EQUAL(test_check_data(0, 2, 256, 1), 0);
	CU_ASSERT_EQUAL(test_check_data(0, 2, 512, 1), 0);
	CU_ASSERT_EQUAL(test_check_data(0, 2, 1024, 1), 0);
	CU_ASSERT_EQUAL(test_check_data(0, 2, 255, 1), 0);

	CU_ASSERT_EQUAL(test_check_speed(10, 0, 1), 0); 
	CU_ASSERT_EQUAL(test_check_speed(100, 0, 1), 0); 
	CU_ASSERT_EQUAL(test_check_speed(1000, 0, 1), 0); 
	
	CU_ASSERT_EQUAL(test_check_speed(10, 0, 2), 0); 
	CU_ASSERT_EQUAL(test_check_speed(100, 0, 2), 0); 
	CU_ASSERT_EQUAL(test_check_speed(1000, 0, 2), 0); 
}

void test_port_1(void)
{
	/*
	 * test_port_state
	 * test_port_1_0
	 * test_port_1_2
	 * test_speed_10
	 * test_speed_100
	 * test_speed_1000
	 */
	CU_ASSERT_EQUAL(test_port_state(0), 1);
	CU_ASSERT_EQUAL(test_port_state(1), 1);
	CU_ASSERT_EQUAL(test_port_state(2), 1);

	CU_ASSERT_EQUAL(test_check_data(1, 0, 32, 1000), 0);
	CU_ASSERT_EQUAL(test_check_data(1, 0, 64, 1000), 0);
	CU_ASSERT_EQUAL(test_check_data(1, 0, 128, 1000), 0);
	CU_ASSERT_EQUAL(test_check_data(1, 0, 256, 1000), 0);
	CU_ASSERT_EQUAL(test_check_data(1, 0, 512, 1000), 0);
	CU_ASSERT_EQUAL(test_check_data(1, 0, 1024, 1000), 0);
	CU_ASSERT_EQUAL(test_check_data(1, 0, 255, 1000), 0);

	CU_ASSERT_EQUAL(test_check_data(1, 2, 32, 1), 0);
	CU_ASSERT_EQUAL(test_check_data(1, 2, 64, 1), 0);
	CU_ASSERT_EQUAL(test_check_data(1, 2, 128, 1), 0);
	CU_ASSERT_EQUAL(test_check_data(1, 2, 256, 1), 0);
	CU_ASSERT_EQUAL(test_check_data(1, 2, 512, 1), 0);
	CU_ASSERT_EQUAL(test_check_data(1, 2, 1024, 1), 0);
	CU_ASSERT_EQUAL(test_check_data(1, 2, 255, 1), 0);

	CU_ASSERT_EQUAL(test_check_speed(10, 1, 0), 0); 
	CU_ASSERT_EQUAL(test_check_speed(100, 1, 0), 0); 
	CU_ASSERT_EQUAL(test_check_speed(1000, 1, 0), 0); 

	CU_ASSERT_EQUAL(test_check_speed(10, 1, 2), 0); 
	CU_ASSERT_EQUAL(test_check_speed(100, 1, 2), 0); 
	CU_ASSERT_EQUAL(test_check_speed(1000, 1, 2), 0); 
}

void test_port_2(void)
{

	eth_self_test("eth2");
	/*
	 * test_port_state
	 * test_port_0_1
	 * test_port_0_2
	 * test_speed_10
	 * test_speed_100
	 * test_speed_1000
	 */
	CU_ASSERT_EQUAL(test_port_state(0), 1);
	CU_ASSERT_EQUAL(test_port_state(1), 1);
	CU_ASSERT_EQUAL(test_port_state(2), 1);

	CU_ASSERT_EQUAL(test_check_data(2, 0, 32, 1000), 0);
	CU_ASSERT_EQUAL(test_check_data(2, 0, 64, 1000), 0);
	CU_ASSERT_EQUAL(test_check_data(2, 0, 128, 1000), 0);
	CU_ASSERT_EQUAL(test_check_data(2, 0, 256, 1000), 0);
	CU_ASSERT_EQUAL(test_check_data(2, 0, 512, 1000), 0);
	CU_ASSERT_EQUAL(test_check_data(2, 0, 1024, 1000), 0);
	CU_ASSERT_EQUAL(test_check_data(2, 0, 255, 1000), 0);

	CU_ASSERT_EQUAL(test_check_data(2, 1, 32, 1), 0);
	CU_ASSERT_EQUAL(test_check_data(2, 1, 64, 1), 0);
	CU_ASSERT_EQUAL(test_check_data(2, 1, 128, 1), 0);
	CU_ASSERT_EQUAL(test_check_data(2, 1, 256, 1), 0);
	CU_ASSERT_EQUAL(test_check_data(2, 1, 512, 1), 0);
	CU_ASSERT_EQUAL(test_check_data(2, 1, 1024, 1), 0);
	CU_ASSERT_EQUAL(test_check_data(2, 1, 255, 1), 0);

	CU_ASSERT_EQUAL(test_check_speed(10, 2, 0), 0); 
	CU_ASSERT_EQUAL(test_check_speed(100, 2, 0), 0); 
	CU_ASSERT_EQUAL(test_check_speed(1000, 2, 0), 0); 

	CU_ASSERT_EQUAL(test_check_speed(10, 2, 1), 0); 
	CU_ASSERT_EQUAL(test_check_speed(100, 2, 1), 0); 
	CU_ASSERT_EQUAL(test_check_speed(1000, 2, 1), 0); 
}
/*--------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------*/
static int suite_init(void)
{
	return 0;
}

static int suite_cleanup(void)
{
	usleep(1000); /* Ethernet has to come up */
	free(port);

	if (fplog) {
		fclose(fplog);
		fplog = NULL;
	}
	return 0;
}

int enumerate_devices(void)
{
	int i = 0, j;
	struct port_def *p;
	struct ifreq ifr;
	int cfd = -1;
	int err = 0;
	struct ifaddrs *ifaddr, *ifa;
	int family;
	int nport = 0;

	portnames = calloc(16, sizeof(char *));

	/* Find out how many interfaces are there */
	err = getifaddrs(&ifaddr);

	for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
		if (!strstr(ifa->ifa_name, "eth"))
			continue;
		family = ifa->ifa_addr->sa_family;
		if (family != AF_PACKET)
			continue;

		portnames[nport] = strdup(ifa->ifa_name);
		nport++;

		fprintf(fplog, "Interface : %s address family: %d%s\n", ifa->ifa_name, family,
			(family == AF_PACKET) ? " (AF_PACKET)" :
			(family == AF_INET) ?   " (AF_INET)" :
			(family == AF_INET6) ?  " (AF_INET6)" : "");

	}


	freeifaddrs(ifaddr);

	port = calloc(nport, sizeof(struct portdef *));
	/* Read all needed parameters for each of the interfaces */

	cfd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
	if (cfd == -1) {
		fclose(fplog);
		return -1;
	}

	for(i = 0; i < nport; i++) {

		if (portnames[i] == NULL)
			continue;

		p = port[i] = malloc(sizeof(struct port_def));
		//p = (port[i]);
		sprintf(p->portname, "%s", portnames[i]);
		p->state = 0;
		p->ipaddr = 0xFFFFFFFF;

		bzero(&ifr, sizeof(struct ifreq));
		snprintf(ifr.ifr_name, IFNAMSIZ, "%s", p->portname);

		err = ioctl(cfd, SIOGIFINDEX, &ifr);
		if (err < 0) {
			perror("SIOGIFINDEX");
			return -1;
		}

		p->ifindex = ifr.ifr_ifindex;

		err = ioctl(cfd, SIOCGIFHWADDR, &ifr);
		if (err < 0) {
			perror("SIOCGIFHWADDR");
			return -1;
		}

		memcpy(&p->if_addr, ifr.ifr_hwaddr.sa_data, 6);


		/* Get interface state */
		err = ioctl(cfd, SIOCGIFFLAGS, &ifr);
		if (err < 0) {
			perror("SIOCGIFFLAGS");
			return -1;
		}
		if ((ifr.ifr_flags & IFF_UP) == IFF_UP) {
			p->state = 1;
		}

		p->sll.sll_family = PF_PACKET;
		p->sll.sll_protocol = htons(ETH_P_VC_CUSTOM);
		p->sll.sll_ifindex = p->ifindex;
		p->sll.sll_hatype = ARPHRD_ETHER;
		p->sll.sll_halen = ETH_ALEN;
		p->sll.sll_pkttype = PACKET_OTHERHOST;
		p->sll.sll_addr[0] = p->if_addr[0];
		p->sll.sll_addr[1] = p->if_addr[1];
		p->sll.sll_addr[2] = p->if_addr[2];
		p->sll.sll_addr[3] = p->if_addr[3];
		p->sll.sll_addr[4] = p->if_addr[4];
		p->sll.sll_addr[5] = p->if_addr[5];
		p->sll.sll_addr[6] = 0;
		p->sll.sll_addr[7] = 0;

	}

	/* Get IP Addr of interface */
	struct ifconf ifc;
	struct ifreq nifr[2*nport];
	int sfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sfd < 0) {
		perror("socket");
		return -1;
	}

	ifc.ifc_buf = (char *) nifr;
	ifc.ifc_len= sizeof(nifr);

	err = ioctl(sfd, SIOCGIFCONF, &ifc);
	if (err < 0) {
		perror("SIOCGIFCONF");
		return -1;
	}

	for (i = 0; i < nport; i++) {
		p = port[i];
		for (j = 0; j < ifc.ifc_len/sizeof(nifr[0]); j++) {
			if (strcmp(nifr[j].ifr_name, p->portname) == 0) {
				char *mac;
				struct sockaddr_in *sa = &nifr[j].ifr_addr;
				p->ipaddr = sa->sin_addr.s_addr;
				mac = print_mac(p->if_addr);
				fprintf(fplog, "INTERFACE: %s ---> %s\t\t\t[%-20s] [%-5s]\n",
						p->portname, inet_ntoa(sa->sin_addr),
						mac,
						(p->state) ? "UP" : "DOWN");
				free(mac);
			}
		}
	}

	/* Get interface speed */


	close(sfd);
	close(cfd);

	return nport;
}

int init_network_suite(void)
{
	char *curr, *other;
	char name[64];
	int i, j;
	if (!fplog)
		fplog = fopen(LOGFILE, "w+");

	pSuite = CU_add_suite("network", suite_init, suite_cleanup);
	if (pSuite == NULL)
		return -1;

	num_ports = enumerate_devices();

	for (i = 0; i < num_ports; i++) {
		curr = portnames[i];

		sprintf(name, "test_%s", curr);
		if (strcmp(curr, "eth0") == 0) {
			if (NULL == CU_add_test(pSuite, name, test_port_0)) {
				return -1;
			}
		} else if (strcmp(curr, "eth1") == 0) {
			if (NULL == CU_add_test(pSuite, name, test_port_1)) {
				return -1;
			}
		} else if (strcmp(curr, "eth2") == 0) {
			if (NULL == CU_add_test(pSuite, name, test_port_2)) {
				return -1;
			}
		}
	}

	return 0;
}



#if 0
/*---------------------------------------------------------*/
static CU_TestInfo tests[] = {
	{"CheckDataPort_0_1", test_check_data_port_0_1},
	{"CheckDataPort_0_2", test_check_data_port_0_2},
	{"CheckDataPort_0_3", test_check_data_port_0_3},

	{"CheckDataPort_1_0", test_check_data_port_1_0},
	{"CheckDataPort_1_2", test_check_data_port_1_2},
	{"CheckDataPort_1_3", test_check_data_port_1_3},

	{"CheckDataPort_2_0", test_check_data_port_2_0},
	{"CheckDataPort_2_1", test_check_data_port_2_1},
	{"CheckDataPort_2_3", test_check_data_port_2_3},

	{"CheckDataPort_3_0", test_check_data_port_3_0},
	{"CheckDataPort_3_1", test_check_data_port_3_1},
	{"CheckDataPort_3_2", test_check_data_port_3_2},


//	{"PingTest", test_ping},
	//{"TCPSend_0_1", test_tcp_send_0_1},

	CU_TEST_INFO_NULL
};

const CU_SuiteInfo suite_network = {
	"network", suite_init, suite_cleanup, tests,
};
#endif
