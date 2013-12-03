// nfq-example.c
// (c) 2012 velocloud.net
// packet print code shamelessly stolen from tcpdump;

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <errno.h>

#include <netinet/in.h>
#include <linux/netfilter.h>

#include <libnetfilter_queue/libnetfilter_queue.h>

#include "netdissect.h"

// print interface;

netdissect_options Gndo;
netdissect_options *gndo = &Gndo;
char *program_name;
int32_t thiszone;

// global data;

typedef struct g {
	char *cmd;			// argv[0];
	struct nfq_handle *nfq_h;
	struct nfq_q_handle *ip_q;
} g_t;

g_t G;

// print a message;

void
Msg(char *fmt, ...)
{
	va_list ap;
	int abort;

	va_start(ap, fmt);
	abort = (fmt[0] == '@');
	if(abort)
		fmt++;
	printf("%s: ", G.cmd);
	vprintf(fmt, ap);
	va_end(ap);
	if(abort)
		exit(1);
}

// default print;

void
default_print(const u_char *bp, u_int length)
{
	hex_and_ascii_print("\n\t", bp, length); /* pass on lf and identation string */
}


// process an IP packet;

int
IpFilter(struct nfq_q_handle *q, struct nfgenmsg *msg, struct nfq_data *pkt, void *priv)
{
	g_t *g = priv;
	struct nfqnl_msg_packet_hdr *h;
	uint32_t id = 0;
	uint16_t hw = 0;
	uint32_t idev, odev, iphys, ophys;
	unsigned char *pkt_data;
	struct nfq_iphdr *ip_h;
	struct nfq_tcphdr *tcp_h;
	int len;

	h = nfq_get_msg_packet_hdr(pkt);
	if(h) {
		id = ntohl(h->packet_id);
		hw = ntohs(h->hw_protocol);
	}
	idev = nfq_get_indev(pkt);
	odev = nfq_get_outdev(pkt);
	iphys = nfq_get_physindev(pkt);
	ophys = nfq_get_physoutdev(pkt);
	len = nfq_get_payload(pkt, &pkt_data);
	printf(" 0x%04x #%d: %d/%d->%d/%d len=%d ",
		hw, id, idev, iphys, odev, ophys, len);

#ifdef DEBUG
{ int n;
	for(n = 0; n < 32; n++)
		printf(" %02x", pkt_data[n]);
	printf("\n");
}
#endif

	if(len > 0) {
		gndo->ndo_snapend = pkt_data + len;
		ip_print(gndo, pkt_data, len);
		printf("\n");
	}

	return(nfq_set_verdict(q, id, NF_ACCEPT, 0, NULL));
}

// main entry;

int
main(int argc, char **argv)
{
	g_t *g = &G;
	int c, fd, r;
	char buf[16*1024];

	g->cmd = argv[0];
	program_name = argv[0];
	Msg("libnetfilter from user space example app\n");
	thiszone = gmt2local(0);

	// get options;

	while((c = getopt(argc, argv, "v")) != EOF) {
		switch(c) {
		case 'v' :
			gndo->ndo_vflag++;
			break;
		default:
			Msg("@unknown option: -%c\n", c);
		}
	}

	// get nfq handle;

	g->nfq_h = nfq_open();
	if( !g->nfq_h)
		Msg("@cannot get nfq handle\n");

	// unbind existing nfq handler for PF_INET;
	// bind new nfq handler for PF_INET;

	if(nfq_unbind_pf(g->nfq_h, PF_INET) < 0)
		Msg("@cannot unbind PF_INET, %s\n", strerror(errno));
	if(nfq_bind_pf(g->nfq_h, PF_INET) < 0)
		Msg("@cannot bind PF_INET, %s\n", strerror(errno));

	// install a callback;

	g->ip_q = nfq_create_queue(g->nfq_h, 0, &IpFilter, g);
	if( !g->ip_q)
		Msg("@cannot create IP queue handler, %s\n", strerror(errno));

	// copy entire packet;
	// we want to know the performance impact;

	if(nfq_set_mode(g->ip_q, NFQNL_COPY_PACKET, sizeof(buf)) < 0)
		Msg("@cannot set NFQNL_COPY_PACKET, %s\n", strerror(errno));

	gndo->ndo_fflag = 1;
	gndo->ndo_nflag = 1;
	gndo->ndo_Xflag = 1;

	Msg("entering packet processing loop\n");

	fd = nfq_fd(g->nfq_h);
	while(1) {
		r = recv(fd, buf, sizeof(buf), 0);
		if(r == ENOBUFS)
			Msg("too slow to process all packets\n");
		else if(r < 0)
			Msg("@recv error: %s\n", strerror(errno));
		else
			nfq_handle_packet(g->nfq_h, buf, r);
	}
	return(0);
}

