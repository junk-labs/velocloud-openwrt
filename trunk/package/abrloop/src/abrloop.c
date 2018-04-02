// abrloop.c Sandra Berndt
// marvell switch atu br loop handling;

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <assert.h>

// config;

#undef PRINT_PER_PORT_MACS
#undef PRINT_MERGED_MACS
#define BLOCK_TIME 10

// types;

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long int uint64_t;

// kernel interface;

#include "dsa_mv_ioctl.h"

// user space;

// switch data structrues;

struct sw_port {
	struct sw_port *next;		// next in list;
	struct sw_info *sw;		// ptr back to switch;
	uint8_t num;			// port number;
	uint8_t base;			// bit in merge buffer;
	uint16_t mask;			// port mask, 0=no port;
	uint16_t fid;			// filtering id;
	uint16_t nbuf;			// ent buffer allocated size;
	uint16_t nent;			// ent buffer # of entries;
	struct dsa_mv_atuent *ent;	// atu entries;
	struct dsa_mv_atuent *cur;	// current merge state;
	char name[IFNAMSIZ];		// interface name;
};

struct sw_info {
	struct sw_info *next;		// next switch;
	char *name;			// switch base name;
	uint8_t flags;			// flags;
	uint8_t nports;			// # of ports;
	uint8_t maxport;		// highest port number;
	uint8_t base;			// base vector in sw_mac;
	struct dsa_mv_atuinfo info;	// switch info;
	struct sw_port ports[16];	// port data;
};

// merged macs;

struct sw_mac {
	uint8_t mac[ETH_ALEN];	// mac;
	uint16_t state;		// mac state;
	uint32_t vec;		// merged vector;
};

struct sw_macs {
	uint32_t nbuf;		// size of allocated buffer;
	uint32_t nent;		// # of entries;
	struct sw_mac *macs;	// merged buffer;
};

struct sw_block {
	struct sw_mac mac;	// blocked mac;
	uint32_t timer;		// block timer;
	struct sw_block *next;	// next in list;
};

// init switch;

int
sw_init(int fd, char *name, int nmacspp, struct sw_info **out, struct sw_port **portq)
{
	struct sw_info *sw;
	struct sw_port *swp;
	int port, n, rv;
	uint16_t mask, fid;
	struct dsa_mv_atuent *ent;
	struct dsa_mv_reg *reg;
	struct ifreq ifr;

	// allocate switch info;

	sw = malloc(sizeof(*sw));
	*out = sw;
	if( !sw)
		return(-1);
	bzero(sw, sizeof(*sw));
	sw->name = name;

	// find out switch limits;

	bzero(&ifr, sizeof(ifr));
	snprintf(ifr.ifr_name, IFNAMSIZ-1, "%s0", name);
	ifr.ifr_data = (char *)&sw->info;
	rv = ioctl(fd, SIOCATUINFO, &ifr);
	if(rv < 0)
		return(-2);

	// use switch max if nmacspp is 0;

	if( !nmacspp)
		nmacspp = sw->info.nmacs;

	// init ports;
	// use xport addressing;

	snprintf(ifr.ifr_name, IFNAMSIZ-1, "%s0", sw->name);
	reg = (struct dsa_mv_reg *)&ifr.ifr_ifru;

	sw->nports = 0;
	sw->maxport = 0;
	for(port = 0; port < 16; port++) {
		mask = 1 << port;
		if( !(sw->info.ports & mask))
			continue;
		sw->nports++;
		sw->maxport = port;

		// allocate mac buffer;

		swp = sw->ports + port;
		swp->sw = sw;
		swp->num = port;
		swp->base = sw->base + port;
		swp->mask = mask;
		swp->fid = 0;
		swp->nbuf = nmacspp;

		swp->next = *portq;
		*portq = swp;

		snprintf(swp->name, IFNAMSIZ-1, "%s%d", name, port);
		swp->ent = malloc(nmacspp * sizeof(struct dsa_mv_atuent));
		if( !swp->ent)
			return(-4);

		// get current fid;

		reg->reg_type = MV_REG_XPORT;
		reg->reg_port = port;
		reg->reg_num = 0x05;
		rv = ioctl(fd, SIOCGMVREG, &ifr);
		if(rv < 0)
			return(-5);
		fid = (reg->reg_value & 0xff) << 4;
		reg->reg_num = 0x06;
		rv = ioctl(fd, SIOCGMVREG, &ifr);
		if(rv < 0)
			return(-5);
		fid |= ((reg->reg_value >> 12) & 0xf);
		swp->fid = fid;

		// set port SA filtering mode;
		// enable drop-on-unlock filtering;

		reg->reg_num = 0x04;
		rv = ioctl(fd, SIOCGMVREG, &ifr);
		if(rv < 0)
			return(-5);
		if((reg->reg_value & 0xc000) == 0xc000)
			continue;
		reg->reg_value &= ~0xc000;
		reg->reg_value |= 0x8000;
		rv = ioctl(fd, SIOCSMVREG, &ifr);
		if(rv < 0)
			return(-5);
	}
	return(0);
}

// make sure ports have separate address learning databases;
// kernel module doesn't let us talk to cpu port, argh!!
// so port 4 (cpu) has the default FID of 0;
// make all the LAN ports 1..4, cross 5..6;
// assume the upper 8 bits in reg 0x5 are all 0s;
// returns 1 if switch ports need to be updated;

int
sw_fid_check(struct sw_info *sw)
{
	unsigned port;
	struct sw_port *swp;
	int update = 0;
	unsigned fid;

	// this is switch centric view,
	// cross -switch base is not a concern;

	for(port = 0; port <= sw->maxport; port++) {
		swp = sw->ports + port;
		if( !(swp->mask & sw->info.ports))
			continue;

		fid = (swp->mask & sw->info.cpu)? 0 : (port + 1);
		if(swp->fid != fid) {
			update++;
			swp->fid = fid;
		}
	}
	return(update);
}

// update switch FIDs;
// returns 0 if ok, or <0 for error;

int
sw_fid_update(int fd, struct sw_info *sw)
{
	unsigned port;
	struct sw_port *swp;
	unsigned fid;
	struct dsa_mv_reg *reg;
	int rv;
	struct ifreq ifr;

	printf("%s: updating per-port FID tags\n", sw->name);
	bzero(&ifr, sizeof(ifr));
	snprintf(ifr.ifr_name, IFNAMSIZ-1, "%s0", sw->name);
	reg = (struct dsa_mv_reg *)&ifr.ifr_ifru;

	for(port = 0; port <= sw->maxport; port++) {
		swp = sw->ports + port;

		reg->reg_type = MV_REG_XPORT;
		reg->reg_port = port;
		reg->reg_num = 0x06;

		// only use lowest 4 bits of fid right now;
		// otherwise, must read/write reg 0x05 as well;

		rv = ioctl(fd, SIOCGMVREG, &ifr);
		if(rv < 0)
			return(rv);
		fid = (reg->reg_value >> 12) & 0xf;
		if(fid == (swp->fid & 0xf))
			continue;

		reg->reg_value = ((swp->fid & 0xf) << 12) | (reg->reg_value & 0xff);

		rv = ioctl(fd, SIOCSMVREG, &ifr);
		if(rv < 0)
			return(rv);
		//printf("%s: fixed FID=%d\n", swp->name, swp->fid);
	}
	return(0);
}

// allocate merge buffer;
// size must be the sum of all switch max'es;

struct sw_macs *
sw_mbuf_alloc(uint32_t nmacs)
{
	struct sw_macs *sm;

	sm = malloc(sizeof(*sm) + (nmacs * sizeof(struct sw_mac)));
	if(sm) {
		sm->nbuf = nmacs;
		sm->nent = 0;
		sm->macs = (struct sw_mac *)(sm + 1);
	}
	return(sm);
}

// print an atu mac list;

void
print_atu_macs(struct sw_port *swp, unsigned port)
{
	struct dsa_mv_atuent *ent;
	int i;

	printf("%s: %d macs\n", swp->name, swp->nent);
	ent = swp->ent;
	for(i = 0; i < swp->nent; i++, ent++) {
		printf(" %02x:%02x:%02x:%02x:%02x:%02x vec 0x%04x state 0x%x: %s\n",
			ent->mac[0], ent->mac[1], ent->mac[2],
			ent->mac[3], ent->mac[4], ent->mac[5],
			ent->portvec, ent->state,
			ent->portvec? "new" : "blocked");
	}
}

// get atu entries on all switch ports;

int
sw_get_atu(int fd, struct sw_info *sw)
{
	uint32_t port, got;
	struct sw_port *swp;
	int rv;
	struct dsa_mv_atuop atu;
	struct ifreq ifr;

	ifr.ifr_data = (char *)&atu;
	snprintf(ifr.ifr_name, IFNAMSIZ-1, "%s0", sw->name);

	got = 0;
	for(port = 0; port < sw->nports; port++) {
		swp = sw->ports + port;
		swp->nent = 0;
		if( !swp->mask)
			continue;

		atu.nent = swp->nbuf;
		atu.fid = swp->fid;
		atu.ent = swp->ent;
		rv = ioctl(fd, SIOCGETATU, &ifr);
		if(rv < 0)
			break;
		swp->nent = rv;
		got |= swp->mask;

#ifdef PRINT_PER_PORT_MACS
		print_atu_macs(swp, port);
#endif // PRINT_PER_PORT_MACS
	}
	return(got);
}

// count # of port vevtor bits;

uint8_t port_cnt[16] = { 0,1,1,2,1,2,2,3,1,2,2,3,2,3,3,4 };

unsigned
cnt_ports(uint32_t vec)
{
	unsigned cnt, b;

	cnt = 0;
	for(b = 0; b < (sizeof(vec) * 8); b += 4) {
		cnt += port_cnt[vec & 0xf];
		vec >>= 4;
	}
	return(cnt);
}

// add mac to block list;
// XXX this should pull from a free list;

struct sw_block *
bl_alloc(struct sw_block **q, struct sw_mac *mac)
{
	struct sw_block *sb;

	sb = malloc(sizeof(struct sw_block));
	if(sb) {
		memcpy(&sb->mac, mac, sizeof(*mac));
		sb->timer = BLOCK_TIME;
		sb->next = *q;
		*q = sb;
	}
	return(sb);
}

// remove mac from block list;
// XXX this should return to a free list;

void
bl_free(struct sw_block **bl, struct sw_block *sb)
{

	*bl = sb->next;
	free(sb);
}

// check if mac is in mac list;
// list is sorted, so can do binary search;
// XXX could hash this;

struct sw_block *
bl_lookup(struct sw_block *sb, struct sw_mac *mac)
{
	for(; sb; sb = sb->next) {
		if( !memcmp(sb->mac.mac, mac->mac, sizeof(mac->mac)))
			break;
	}
	return(sb);
}

// block/unblock mac;
// block by adding atu entry with port vector 0;

int
mac_block(int fd, struct sw_port *swp, struct sw_block *sb, int block)
{
	uint32_t vec, mask, invec;
	struct sw_info *sw;
	int rv;
	struct dsa_mv_atuset *set;
	struct ifreq ifr;

	printf(" %02x:%02x:%02x:%02x:%02x:%02x %s\n",
		sb->mac.mac[0], sb->mac.mac[1], sb->mac.mac[2],
		sb->mac.mac[3], sb->mac.mac[4], sb->mac.mac[5],
		block? "block" : "unblock");

	// on all switches that are in vector;
	// but not on the cpu ports, since we want other ports to not block;
	// ignore vector if mac in blocked state;

	set = (struct dsa_mv_atuset *)&ifr.ifr_ifru;
	for(vec = sb->mac.vec; swp; swp = swp->next) {
		sw = swp->sw;
		if(sw->info.cpu & (1 << swp->num))
			continue;
		invec = vec & (1 << swp->base);
		if( !invec && !sb->mac.state)
			continue;

		// purge or set static entry;

		set->fid = swp->fid;
		set->ent.portvec = 0;
		set->ent.state = block? 0xe : 0x0;
		bcopy(sb->mac.mac, set->ent.mac, sizeof(set->ent.mac));

		snprintf(ifr.ifr_name, IFNAMSIZ-1, "%s0", sw->name);
		rv = ioctl(fd, SIOCSETATU, &ifr);
		if(rv < 0)
			return(-6);
	}
	return(0);
}

// handle loop;

void
handle_mac(int fd, struct sw_block **bl, struct sw_port *portq, struct sw_mac *mac)
{
	struct sw_block *sb;
	unsigned cnt_old, cnt_new;

	// print mac;

	printf(" %02x:%02x:%02x:%02x:%02x:%02x ports 0x%x state 0x%x\n",
		mac->mac[0], mac->mac[1], mac->mac[2],
		mac->mac[3], mac->mac[4], mac->mac[5],
		mac->vec, mac->state);

	// see if mac is blocked already;
	// if not, then add it;

	sb = bl_lookup(*bl, mac);
	if( !sb) {
		sb = bl_alloc(bl, mac);
		if( !sb)
			return;	// XXX just quit for now;
		mac_block(fd, portq, sb, 1);
	}

	// update port vector and timer;
	// restart timer if new port showed up,
	// but don't if older ports disappeared;

	cnt_old = cnt_ports(sb->mac.vec);
	cnt_new = cnt_ports(mac->vec);
	if(cnt_new > cnt_old) {
		sb->mac.vec = mac->vec;
		sb->mac.state = mac->state;
		sb->timer = BLOCK_TIME;
		mac_block(fd, portq, sb, 1);
	} 
}

// process reported macs;

void
proc_macs(int fd, struct sw_port *portq, struct sw_macs *sm, struct sw_block **bl)
{
	uint32_t n, cnt;
	struct sw_mac *mac;
	struct sw_block *sb;

	// handle all new MACs from merge list;

	mac = sm->macs;
	for(n = 0; n < sm->nent; n++, mac++) {
		cnt = cnt_ports(mac->vec);
		if((cnt > 1) | mac->state)
			handle_mac(fd, bl, portq, mac);
	}

	// handle all already blocked MACs;
	// let them time out, then unblock them;
	// let's not wait for switch age time to expire,
	// instead, if loop still exists it will be found again;

	while(sb = *bl) {
		if(--sb->timer == 0) {
			mac_block(fd, portq, sb, 0);
			bl_free(bl, sb);
		} else
			bl = &sb->next;
	}
}

// N-way merge of cpu and LAN ports;
// kernel returns sorted list, so merge is linear in time;
// mbuf <- min_mac(p[]);
// returns # of merged MACs;

int
sw_merge(struct sw_macs *sm, struct sw_port *ports)
{
	struct sw_port *p, *pmin;
	struct sw_mac *m, *me;
	uint64_t mmac, nmac, mac;
	struct dsa_mv_atuent *ent;

	// init merge state;

	for(p = ports; p; p = p->next)
		p->cur = p->ent;

	// merge from all ports;

	mmac = 0;
	m = sm->macs - 1;
	me = m + sm->nbuf;
	sm->nent = 0;
	do {
		// find next min mac on any of the ports;

		nmac = 0xffffffffffff;
		pmin = NULL;
		for(p = ports; p; p = p->next) {
			ent = p->cur;
			if(ent >= (p->ent + p->nent))
				continue;

			mac = (uint64_t)(ent->mac[0]) << 40;
			mac |= (uint64_t)(ent->mac[1]) << 32;
			mac |= (uint64_t)(ent->mac[2]) << 24;
			mac |= (uint64_t)(ent->mac[3]) << 16;
			mac |= (uint64_t)(ent->mac[4]) << 8;
			mac |= (uint64_t)(ent->mac[5]) << 0;
			assert(mac >= mmac);

			if(mac == mmac) {
				m->vec |= ent->portvec;
				m->state |= (ent->portvec == 0);
				p->cur = ++ent;
				pmin = NULL;
				continue;
			}
			if(mac < nmac) {
				nmac = mac;
				pmin = p;
			}
		}

		// stop if no more per-port macs;
		// if the same mac, it already got added to port vector;

		if(nmac == 0xffffffffffff)
			break;
		if( !pmin)
			continue;

		// new MAC, add new entry to mbuf;

		m++;
		sm->nent++;

		ent = pmin->cur;
		m->vec = ent->portvec;
		m->state = (ent->portvec == 0);
		pmin->cur = ++ent;

		mmac = nmac;
		m->mac[5] = nmac; nmac >>= 8;
		m->mac[4] = nmac; nmac >>= 8;
		m->mac[3] = nmac; nmac >>= 8;
		m->mac[2] = nmac; nmac >>= 8;
		m->mac[1] = nmac; nmac >>= 8;
		m->mac[0] = nmac; nmac >>= 8;

	} while(sm->nent < sm->nbuf);
	return(sm->nent);
}

// print an merged mac list;

void
print_macs(struct sw_macs *sm)
{
	int i;
	struct sw_mac *mac;

	printf("%d merged MACs\n", sm->nent);
	mac = sm->macs;
	for(i = 0; i < sm->nent; i++, mac++) {
		printf(" %02x:%02x:%02x:%02x:%02x:%02x vec 0x%04x state 0x%x: %s\n",
			mac->mac[0], mac->mac[1], mac->mac[2],
			mac->mac[3], mac->mac[4], mac->mac[5],
			mac->vec, mac->state,
			mac->vec? "new" : "blocked");
	}
}

// switch names;

char *edge500[] = { "sw", NULL };
char *edge5x0[] = { "sw0p", "sw1p", NULL };

// main entry;
// sw0..sw3 are the interfaces of the physical switch;

int
main(int argc, char **argv)
{
	char **names, *name;
	int fd, rv, n, port;
	struct sw_info *sw, *switches, **swq;
	struct sw_port *portq;
	struct sw_macs *sm;
	struct sw_block *bl;
	uint8_t base;
	uint32_t got;

	// check system;
	// XXX should read from sysfs;

	name = argv[1];
	if( !name) {
		fprintf(stderr, "%s: need board name\n", argv[0]);
		return(-10);
	}
	if( !strcmp(name, "edge500"))
		names = edge500;
	else if( !strcmp(name, "edge5x0"))
		names = edge5x0;
	else {
		fprintf(stderr, "%s: invalid board name\n", name);
		return(-10);
	}

	// open netlink socket;

	fd = socket(AF_INET, SOCK_DGRAM, 0);
	if(fd < 0) {
		perror("socket: ");
		return(-1);
	}

	// init switch(es);
	// force FID bases 8 apart for easy looking at merge buffer hex;

	portq = NULL;
	base = 0;
	for(swq = &switches; name = *names++; swq = &sw->next) {
		rv = sw_init(fd, name, 0, &sw, &portq);
		if(rv) {
			fprintf(stderr, "%s%d: switch init failed: %d\n", name, 0, rv);
			return(-1);
		}
		sw->base = base;
		base += 8;
		*swq = sw;

		printf("%s limits: macs=%d fid=%d cpu=0x%x cross=0x%x lan=0x%0x\n",
			sw->name,
			sw->info.nmacs, sw->info.fid,
			sw->info.cpu, sw->info.cross, sw->info.lan);

		// check and fix atu FIDs;

		if(sw_fid_check(sw))
			sw_fid_update(fd, sw);
	}

	// allocate merge buffer;
	// theorectically, needs to be the sum of max macs for all switches;

#ifndef BE_CORRECT
	n = 0;
	for(sw = switches; sw; sw = sw->next)
		n += sw->info.nmacs;
#else // BE_CORRECT
	n = 1024;
#endif // BE_CORRECT

	sm = sw_mbuf_alloc(n);
	if( !sm) {
		fprintf(stderr, "error: mbuf alloc\n");
		return(-1);
	}

	// for all switches;
	// - read atu per fid (per port);
	// - kernel returns sorted list in ascending MAC order;
	// - do N-way merge into sw_macs;
	// - bridge loop when SA with multiple port vector bits on;

	// check every second;

	bl = NULL;
	while(1) {
		sleep(1);

		// get all per-port databases;
		// fid: 0=cpu 1..4=ports, 5/6=cross;

		for(sw = switches; sw; sw = sw->next) {
			got = sw_get_atu(fd, sw);
			if(got != sw->info.ports) {
				fprintf(stderr, "%s: did not get all atu, 0x%x/0x%x\n",
					sw->name, got, sw->info.ports);
				return(-1);
			}
		}

		sw_merge(sm, portq);

#ifdef PRINT_MERGED_MACS
		print_macs(sm);
#endif // PRINT_MERGED_MACS

		// process macs involved in bridge loop;

		proc_macs(fd, portq, sm, &bl);
	}
	close(fd);
	return(rv);
}

