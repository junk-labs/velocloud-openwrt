// igb-eeprom.c
// (c) 2012 velocloud.net
// tool to write IGB EEPROM;
// required patched kernel with 82580_VC, ethtool;

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

// manufacturing run data;

#define MAX_CHIPS 2

typedef struct mfg mfg_t;
struct mfg {
	char *name;				// name of run;
	char *board;			// name of board;
	char *ifs[MAX_CHIPS];	// interfaces of chips;
	char *tmp[MAX_CHIPS];	// template files;
	unsigned short pciid0[MAX_CHIPS];
	unsigned short pciid1[MAX_CHIPS];
	unsigned bnums[3];		// min/max/base;
	unsigned nmacs;			// # of macs;
	unsigned char mac_min[6];
	unsigned char mac_max[6];
};

// the first runs had a bug where the board number was added twice,
// once from the mac_min and via bnum;
// old ones are now commented out;
// if older boards have to be reflashed the full MAC has to be given;

mfg_t Mfg[] = {
#ifdef IGB_EEPROM_BUGGY_MACS
	{ "pw-pilot",
	  "edge500",
	  { "eth0", },
	  { "atom-c2000-igb-sgmii.bin", },
	  { 0x8086, },
	  { 0x1f41, },
	  { 1, 50 }, 4,
	  { 0xF0,0x8E,0xDB,0x01,0x00,0x00 }, { 0xF0,0x8E,0xDB,0x01,0x00,0xC7 }, },
	{ "pw-pilot2",
	  "edge500",
	  { "eth0", },
	  { "atom-c2000-igb-sgmii.bin", },
	  { 0x8086, },
	  { 0x1f41, },
	  { 51, 99 }, 4,
	  { 0xF0,0x8E,0xDB,0x01,0x00,0xC8 }, { 0xF0,0x8E,0xDB,0x01,0x01,0x8f }, },
	{ "pw-prod",
	  "edge500",
	  { "eth0", },
	  { "atom-c2000-igb-sgmii.bin", },
	  { 0x8086, },
	  { 0x1f41, },
	  { 100, 599 }, 4,
	  { 0xF0,0x8E,0xDB,0x01,0x01,0x90 }, { 0xF0,0x8E,0xDB,0x01,0x09,0x5f }, },
#endif // IGB_EEPROM_BUGGY_MACS

	// Range for miscellaneous new pilot projects: 00:01:00 to 00:ff:ff
	// Starting with Dolphin pilot

	// Dolphin pilot 1 - moved to 510-prod list below. Other new pilots
	// should be added below this one..
	/**
	{ "510-pilot",
	  "edge510",
	  { "eth0", },
	  { "atom-c2000-igb-sgmii.bin", },
	  { 0x8086, },
	  { 0x1f41, },
	  { 1, 959, 0 }, 4,
	  { 0xF0,0x8E,0xDB,0x00,0x01,0x00 }, { 0xF0,0x8E,0xDB,0x00,0x0f,0xff }, },
	  */


	// all above squeezed into 01:0x:xx 4k block;
	// start a new 4k block 01:1x:xx for pw-mp for units 200..2047;
	{ "pw-mp",
	  "edge500",
	  { "eth0", },
	  { "atom-c2000-igb-sgmii.bin", },
	  { 0x8086, },
	  { 0x1f41, },
	  { 200, 2047, 0 }, 4,
	  { 0xF0,0x8E,0xDB,0x01,0x10,0x00 }, { 0xF0,0x8E,0xDB,0x01,0x2f,0xff }, },

// 520/540 need 6 MACs: 4 (igb), 2 (i350);
	// adi/cm proto;
	{ "520-pilot",
	  "edge520",
	  { "eth0", "eth4", },
	  { "atom-c2000-igb-sgmii.bin", "i350-igb-sfp.bin", },
	  { 0x8086, 0x8086, },
	  { 0x1f41, 0x151f, },
	  { 1, 42, 0 }, 6,
	  { 0xF0,0x8E,0xDB,0x01,0x30,0x00 }, { 0xF0,0x8E,0xDB,0x01,0x3f,0xff }, },
	{ "540-pilot",
	  "edge540",
	  { "eth0", "eth4", },
	  { "atom-c2000-igb-sgmii.bin", "i350-igb-sfp.bin", },
	  { 0x8086, 0x8086, },
	  { 0x1f41, 0x151f, },
	  { 1, 42, 0 }, 6,
	  { 0xF0,0x8E,0xDB,0x01,0x40,0x00 }, { 0xF0,0x8E,0xDB,0x01,0x4f,0xff }, },
// 520/540 need 6 MACs: 4 (igb), 2 (i350);
	// adi/cm revB proto;
	{ "520b-pilot",
	  "edge520b",
	  { "eth0", "eth4", },
	  { "atom-c2000-igb-sgmii.bin", "i350-igb-sfp.bin", },
	  { 0x8086, 0x8086, },
	  { 0x1f41, 0x151f, },
	  { 1, 630, 0 }, 6,
	  { 0xF0,0x8E,0xDB,0x01,0x31,0x00 }, { 0xF0,0x8E,0xDB,0x01,0x3f,0xff }, },
	{ "540b-pilot",
	  "edge540b",
	  { "eth0", "eth4", },
	  { "atom-c2000-igb-sgmii.bin", "i350-igb-sfp.bin", },
	  { 0x8086, 0x8086, },
	  { 0x1f41, 0x151f, },
	  { 1, 630, 0 }, 6,
	  { 0xF0,0x8E,0xDB,0x01,0x41,0x00 }, { 0xF0,0x8E,0xDB,0x01,0x4f,0xff }, },

	// start a new 16k block 01:50:00..01:8f:ff for pw-mp2 for units 2500..6595;
	{ "pw-mp2",
	  "edge500",
	  { "eth0", },
	  { "atom-c2000-igb-sgmii.bin", },
	  { 0x8086, },
	  { 0x1f41, },
	  { 2500, 6595, 2500 }, 4,
	  { 0xF0,0x8E,0xDB,0x01,0x50,0x00 }, { 0xF0,0x8E,0xDB,0x01,0x8f,0xff }, },

// 520/540 need 6 MACs: 4 (igb), 2 (i350);
	// adi/cm revB production;
	{ "520b-prod",
	  "edge520b",
	  { "eth0", "eth4", },
	  { "atom-c2000-igb-sgmii.bin", "i350-igb-sfp.bin", },
	  { 0x8086, 0x8086, },
	  { 0x1f41, 0x151f, },
	  { 1001, 11922, 1001 }, 6,
	  { 0xF0,0x8E,0xDB,0x02,0x00,0x00 }, { 0xF0,0x8E,0xDB,0x02,0xff,0xff }, },
          // next batch starts at 11923 ^^
	{ "540b-prod",
	  "edge540b",
	  { "eth0", "eth4", },
	  { "atom-c2000-igb-sgmii.bin", "i350-igb-sfp.bin", },
	  { 0x8086, 0x8086, },
	  { 0x1f41, 0x151f, },
	  { 1001, 11922, 1001 }, 6,
	  { 0xF0,0x8E,0xDB,0x03,0x00,0x00 }, { 0xF0,0x8E,0xDB,0x03,0xff,0xff }, },
          // next batch starts at 11923 ^^
	// 2nd batch of 520b - 11923 to 33767:
	{ "520b-prod",
	  "edge520b",
	  { "eth0", "eth4", },
	  { "atom-c2000-igb-sgmii.bin", "i350-igb-sfp.bin", },
	  { 0x8086, 0x8086, },
	  { 0x1f41, 0x151f, },
	  { 11923, 33767, 11923 }, 6,
	  { 0xF0,0x8E,0xDB,0x04,0x00,0x00 }, { 0xF0,0x8E,0xDB,0x05,0xff,0xff }, },
          // next batch starts at 33768 ^^

	// Dolphin production
	// Also includes the pilot range from above, for ease of use
	// for ADI/SMTC.  You can program 1..959 using either manuf. run.
	{ "510-prod",
	  "edge510",
	  { "eth0", },
	  { "atom-c2000-igb-sgmii.bin", },
	  { 0x8086, },
	  { 0x1f41, },
	  { 1, 959, 0 }, 4,
	  { 0xF0,0x8E,0xDB,0x00,0x01,0x00 }, { 0xF0,0x8E,0xDB,0x00,0x0f,0xff }, },

	{ "510-prod",
	  "edge510",
	  { "eth0", },
	  { "atom-c2000-igb-sgmii.bin", },
	  { 0x8086, },
	  { 0x1f41, },
	  { 1001, 132072, 1001 }, 4,
	  { 0xF0,0x8E,0xDB,0x10,0x00,0x00 }, { 0xF0,0x8E,0xDB,0x17,0xff,0xff }, },

	{ 0 },
};

// pair of mac address and string version of it;

typedef struct macns macns_t;
struct macns {
	unsigned char mac[6];	// address;
	char str[20];			// as string;
};

// global data;

typedef struct g g_t;
struct g {
	char *cmd;			// argv[0];
	char opt_oui;
	char opt_nic;
	char opt_mac;
	char opt_bid;
	char opt_pciid;
	char opt_x;
	char opt_dry;
	char opt_all;
	char nmacs;
	char *bid;
	char *board;
	char *umac;
	unsigned short pciid[2];
	unsigned char mac[6];
	macns_t macs[6];
	unsigned int bnum;
	char *ifname;
	char *tmp;
	void (*map)(g_t *g, unsigned char *eeprom, int size);
	void (*extra)(g_t *g, unsigned char *eeprom, int size);
};

g_t G;

// null MAC;

unsigned char mac_null[6];

// bitmap template;
// several things need to be filled in;
// and checksum recomputed;
// does not contain PXE firmware, and PXE disabled;

unsigned char eeprom[64*1024];

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

// edge500 board specifics;
// MDIO[0] shared among eth0 (88e1112) and eth2,eth3 (88e6320 single-chip addressing);
// MDIO[1] directly to 88e6176, single-chip addressing;

#define E500_PHY0_ADDR_6320 0	// shared MDIO[0];
#define E500_PHY1_ADDR_6176 0	// full bus decode, MDIO[1];
#define E500_PHY2_ADDR_6320 0	// shared MDIO[0];
#define E500_PHY3_ADDR_1112 0x7	// shared MDIO[0];

// edge520/540 board specifics;
// MDIO[0] eth0 to 88e6176 switch, single-chip addressing);
// MDIO[1] eth1 to 88e6176 switch, single-chip addressing);
// MDIO bitbang to eth2,eth3 88e1514 phys;

#define E5X0_PHY0_ADDR_6176 0	// full bus decode, MDIO[0];
#define E5X0_PHY1_ADDR_6176 0	// full bus decode, MDIO[1];
#define E5X0_PHY2_ADDR_1514 0	// mdio bitbang, not assigned here;
#define E5X0_PHY3_ADDR_1514 0	// mdio bitbang, not assigned here;

// edge510 board specifics;
// MDIO[0] eth0 to 88e1543 quad-phy

#define E510_PHY0_ADDR_1543 0	// full bus decode, MDIO[0];
#define E510_PHY1_ADDR_1543 1	// full bus decode, MDIO[1];
#define E510_PHY2_ADDR_1543 2	// full bus decode, MDIO[2];
#define E510_PHY3_ADDR_1543 3	// full bus decode, MDIO[3];

// initialization control 4;
// set phy address;

void
igb_ic4(unsigned char *p, int addr)
{
	unsigned short *w = ((unsigned short *)p) + 0x13;
	unsigned short old, new;

	old = *w;
	new = 
	new = (old & ~0x2e) | (addr << 1);
	*w = new;
	Msg("initialization control 4: 0x%x -> 0x%x\n", old, new);
}

// initialization control 3;
// set mdio, sgmmi link mode, com_mdio, ext_mdio;

void
igb_ic3(unsigned char *p, int com)
{
	unsigned short *w = ((unsigned short *)p) + 0x24;
	unsigned short old, new;

	old = *w;
	new = (old & ~0x8) | 0x4004;
	if(com)
		new |= 0x8;
	*w = new;
	Msg("initialization control 3: 0x%x -> 0x%x\n", old, new);
}

// set a specific word;

void
igb_word(unsigned char *p, unsigned int word, unsigned short data)
{
	unsigned short *w = ((unsigned short *)p) + word;
	unsigned short old;

	old = *w;
	*w = data;
	Msg("word %d: 0x%x -> 0x%x\n", word, old, data);
}

// map edge500 macs;

void
igb_c2000_map(g_t *g, unsigned char *eeprom, int size)
{
	bcopy(g->macs[0].mac, eeprom + 0x0, 6);
	bcopy(g->macs[1].mac, eeprom + 0x100, 6);
	bcopy(g->macs[2].mac, eeprom + 0x180, 6);
	bcopy(g->macs[3].mac, eeprom + 0x200, 6);
}

// map ve1000 i350 macs;

void
igb_i350_ve_map(g_t *g, unsigned char *eeprom, int size)
{
	bcopy(g->macs[0].mac, eeprom + 0x0, 6);
	bcopy(g->macs[1].mac, eeprom + 0x100, 6);
	bcopy(mac_null, eeprom + 0x180, 6);
	bcopy(mac_null, eeprom + 0x200, 6);
}

// map 520/540 i350 macs;

void
igb_i350_5x_map(g_t *g, unsigned char *eeprom, int size)
{
	bcopy(g->macs[4].mac, eeprom + 0x0, 6);
	bcopy(g->macs[5].mac, eeprom + 0x100, 6);
	bcopy(mac_null, eeprom + 0x180, 6);
	bcopy(mac_null, eeprom + 0x200, 6);
}

// map edge520/540;

void
edge5x0_map(g_t *g, unsigned char *eeprom, int size)
{
	switch(g->opt_x) {
	case 0:
		igb_c2000_map(g, eeprom, size);
		break;
	case 1:
		igb_i350_5x_map(g, eeprom, size);
		break;
	}
}

// edge500 igb board specifics;

void
edge500_extra(g_t *g, unsigned char *eeprom, int size)
{
	// program OEM specific words;

	igb_word(eeprom, 0x06, 0x5663);	// "Vc";
	igb_word(eeprom, 0x07, 0x6535);	// "e5";

	// set phy mdio addresses;

	igb_ic4(eeprom + 0x000, E500_PHY0_ADDR_6320);
	igb_ic4(eeprom + 0x100, E500_PHY1_ADDR_6176);
	igb_ic4(eeprom + 0x180, E500_PHY2_ADDR_6320);
	igb_ic4(eeprom + 0x200, E500_PHY3_ADDR_1112);

	// set MDICNFG.COM_MDIO based on ports;

	igb_ic3(eeprom + 0x000, 1);
	igb_ic3(eeprom + 0x100, 0);
	igb_ic3(eeprom + 0x180, 1);
	igb_ic3(eeprom + 0x200, 1);
}

// edge510 igb board specifics;

void
edge510_extra(g_t *g, unsigned char *eeprom, int size)
{
	// program OEM specific words;

	igb_word(eeprom, 0x06, 0x5663);	// "Vc";
	igb_word(eeprom, 0x07, 0x3531);	// "51";

	// set phy mdio addresses;

	igb_ic4(eeprom + 0x000, E510_PHY0_ADDR_1543);
	igb_ic4(eeprom + 0x100, E510_PHY1_ADDR_1543);
	igb_ic4(eeprom + 0x180, E510_PHY2_ADDR_1543);
	igb_ic4(eeprom + 0x200, E510_PHY3_ADDR_1543);

	// set MDICNFG.COM_MDIO based on ports;

	igb_ic3(eeprom + 0x000, 1);
	igb_ic3(eeprom + 0x100, 1);
	igb_ic3(eeprom + 0x180, 1);
	igb_ic3(eeprom + 0x200, 1);
}

// edge520/540 igb board specifics;

void
edge5x0_extra_c2000(g_t *g, unsigned char *eeprom, int size)
{
	// program OEM specific words;

	igb_word(eeprom, 0x06, 0x5663);	// "Vc";
	igb_word(eeprom, 0x07, 0x3558);	// "5X";

	// set phy mdio addresses;

	igb_ic4(eeprom + 0x000, E5X0_PHY0_ADDR_6176);
	igb_ic4(eeprom + 0x100, E5X0_PHY1_ADDR_6176);
	igb_ic4(eeprom + 0x180, E5X0_PHY2_ADDR_1514);
	igb_ic4(eeprom + 0x200, E5X0_PHY3_ADDR_1514);

	// set MDICNFG.COM_MDIO based on ports;

	igb_ic3(eeprom + 0x000, 1);
	igb_ic3(eeprom + 0x100, 0);
	igb_ic3(eeprom + 0x180, 1);
	igb_ic3(eeprom + 0x200, 1);
}

// edge520/540 igb specifics;
// opt_x indexes template to use;

void
edge5x0_extra(g_t *g, unsigned char *eeprom, int size)
{
	// no extra changes for sfp i350;

	if(g->opt_x == 0)
		edge5x0_extra_c2000(g, eeprom, size);
}

// get a number of given size;
// either 0xHEX, or XX:XX:XX...;

int
get_num(char *s, unsigned char *dst, int n)
{
	char *ep;
	unsigned int v;

	if((n != 3) && (n != 6))
		return(0);

	if(strchr(s, ':')) {
		while(n--) {
			v = strtoul(s, &ep, 16);
			if(ep <= s)
				return(0);		// no number;
			if(n && (*ep != ':'))
				return(0);		// bad format;
			if(v > 0xff)
				return(0);		// byte too large;
			*dst++ = v;
			s = ep + 1;
		}
	} else {
		v = strtoul(s, &ep, 0);
		if(ep <= s)
			return(0);			// no number;
		if(n != 3)
			return(0);			// only do dec/hex for oui/nic;
		if(v >= (1 << 24))
			return(0);			// number too big;
		dst[0] = (v >> 16) & 0xff;
		dst[1] = (v >> 8) & 0xff;
		dst[2] = (v >> 0) & 0xff;
	}
	return(1);
}

// get board id for NIC part;

int
get_bid(char *s, unsigned char *dst)
{
	char *ep;
	unsigned int v;

	v = strtoul(s, &ep, 0);
	if(ep <= s)
		return(0);			// no number;
	if(v >= (1 << 20))
		return(0);			// number too big;
	v <<= 4;
	dst[0] = (v >> 16) & 0xff;
	dst[1] = (v >> 8) & 0xff;
	dst[2] = (v >> 0) & 0xff;
	return(1);
}

// pci id for ethtool magic;
// 8086:1509

int
get_pciid(char *s, unsigned short *dst)
{
	char *ep;
	unsigned int v, d;

	v = strtoul(s, &ep, 16);
	if(ep <= s)
		return(0);			// no number;
	if(v >= (1 << 16))
		return(0);			// number too big;
	if(*ep != ':')
		return(0);			// device id missing;

	s = ep + 1;
	d = strtoul(s, &ep, 16);
	if(ep <= s)
		return(0);			// no number;
	if(d >= (1 << 16))
		return(0);			// number too big;

	dst[0] = v;
	dst[1] = d;
	return(1);
}

// make printable mac string;

void
mac_str(char *d, unsigned char *mac)
{
	snprintf(d, 20, "%02x:%02x:%02x:%02x:%02x:%02x",
		mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
}

// add to mac address;
// does work in place;

void
mac_add(unsigned char *dst, unsigned char *src, unsigned add)
{
	unsigned long long mac;

	mac = ((unsigned long long)src[0] << 40)
		| ((unsigned long long)src[1] << 32)
		| ((unsigned long long)src[2] << 24)
		| ((unsigned long long)src[3] << 16)
		| ((unsigned long long)src[4] << 8)
		| ((unsigned long long)src[5] << 0);
	mac += add;
	dst[0] = mac >> 40;
	dst[1] = mac >> 32;
	dst[2] = mac >> 24;
	dst[3] = mac >> 16;
	dst[4] = mac >> 8;
	dst[5] = mac >> 0;
}

// get baord name;

char *
get_board(char *name, int size)
{
	int fd, n;
	char *p;

	fd = open("/sys/devices/platform/vc/board", O_RDONLY);
	if(fd < 0)
		return("ve1000");
	n = read(fd, name, size);
	close(fd);
	if(n <= 0)
		return(NULL);
	name[size-1] = 0;
	p = strchr(name, '\n');
	if(p)
		*p = 0;
	return(name);
}

// print manufacturing lots;

void
mfg_list(g_t *g)
{
	mfg_t *mfg;
	char *p;

	Msg("list of mfg runs (for -u):\n");
	for(mfg = Mfg; p = mfg->name; mfg++) {
		if( !g->opt_all && strcmp(g->board, mfg->board))
			continue;
		Msg(" %s board ids %d..%d base %02x:%02x:%02x:%02x:%02x:%02x macs %u\n",
			p, mfg->bnums[0], mfg->bnums[1],
			mfg->mac_min[0], mfg->mac_min[1], mfg->mac_min[2],
			mfg->mac_min[3], mfg->mac_min[4], mfg->mac_min[5],
			mfg->nmacs);
	}
}

// manufacturing support;

char *
mfg_init(g_t *g)
{
	char *p, *ep;
	int len;
	mfg_t *mfg;
	unsigned int bnum;
	unsigned long long mac;

	// check manufacturing run string;

	p = strchr(g->umac, ':');
	if( !p)
		return("missing board number");
	if(p <= g->umac)
		return("missing manufacturer run");
	len = p - g->umac;
	bnum = strtoul(++p, &ep, 10);
	if(ep <= p)
		return("missing/illegal board number");

	// lookup run;

	for(mfg = Mfg; p = mfg->name; mfg++) {
		if( !strncmp(g->umac, p, len) &&
	            (bnum >= mfg->bnums[0]) &&
                    (bnum <= mfg->bnums[1]))
			break;
	}
	if( !p)
		return("unknown manufacturer run");
	if(strcmp(mfg->board, g->board))
		return("unsupported board type");
	Msg("mfg: %s [%d..%d], board: %d\n",
		mfg->name, mfg->bnums[0], mfg->bnums[1], bnum);

	// init MAC addresses;

	bnum -= mfg->bnums[2];
	g->nmacs = mfg->nmacs;
	mac_add(g->mac, mfg->mac_min, bnum * g->nmacs);

	// init interface and template file;
	// opt_x indexes template to use;

	g->ifname = mfg->ifs[g->opt_x];
	g->tmp = mfg->tmp[g->opt_x];

	// init pci ids;

	g->pciid[0] = mfg->pciid0[g->opt_x];
	g->pciid[1] = mfg->pciid1[g->opt_x];

	return(0);
}

// main entry;

int
main(int argc, char **argv)
{
	g_t *g = &G;
	int c, fd, n, nb;
	unsigned short pciid[2];
	char *msg;
	macns_t *macns;
	char magic[16];
	char bid[20];
	char file[50];
	char cmd[200];
	char *override_ifname = NULL;

	g->cmd = argv[0];
	g->ifname = "eth0";
	g->bid = "image";
	g->mac[0] = 'V';
	g->mac[1] = 'C';
	g->mac[2] = '1';

	// get board name;

	g->board = get_board(file, sizeof(file));
	if( !g->board)
		Msg("@cannot determine board\n");
	Msg("found board '%s'\n", g->board);

	// get options;

	while((c = getopt(argc, argv, "o:N:m:B:b:p:i:t:u:xnah")) != EOF) {
		switch(c) {
		case 'o' :
			if( !get_num(optarg, &g->mac[0], 3))
				Msg("@bad OUI: %s\n", optarg);
			g->opt_oui = 1;
			break;
		case 'N' :
			if( !get_num(optarg, &g->mac[3], 3))
				Msg("@bad NIC: %s\n", optarg);
			g->opt_nic = 1;
			break;
		case 'm' :
			if( !get_num(optarg, &g->mac[0], 6))
				Msg("@bad MAC address: %s\n", optarg);
			g->opt_mac = 1;
			break;
		case 'B' :
			g->board = optarg;
			Msg("overwriting board to '%s'\n", g->board);
			break;
		case 'b' :
			if( !get_bid(optarg, &g->mac[3]))
				Msg("@bad board id: %s\n", optarg);
			g->opt_bid = 1;
			g->bid = optarg;
			break;
		case 'p' :
			if( !get_pciid(optarg, pciid))
				Msg("@bad PCI ID: %s\n", optarg);
			g->opt_pciid = 1;
			break;
		case 'i':
			override_ifname = optarg;
			break;
		case 't' :
			g->tmp = optarg;
			break;
		case 'u' :
			g->umac = optarg;
			break;
		case 'x' :
			g->opt_x = 1;
			break;
		case 'n' :
			g->opt_dry = 1;
			break;
		case 'a' :
			g->opt_all = 1;
			break;
		case 'h':
			Msg("help\n"
				"  %s [options]\n"
				"  [-o OUI]       (DEC, 0xHEX, ab:cd:ef)\n"
				"  [-N NIC]       (DEC, 0xHEX, ab:cd:ef)\n"
				"  [-m MAC]       (00:11:22:33:44:55)\n"
				"  [-B board]     (force board: ve1000, edge500, ...)\n"
				"  [-b board-id]  (DEC, 0xHEX)\n"
				"  [-p pci-id]    (8086:1509)\n"
				"  [-i if]        (eth0)\n"
				"  [-t intel-tmp] (intel template file)\n"
				"  [-u mfg:id]    (unique MAC from pool with board id)\n"
				"  [-x]           (program i350-sfp on edge520/540)\n"
				"  [-n]           (dry run, do not program)\n"
				"  [-a]           (list all mfg runs)\n"
				"  [-h]           (this help)\n",
				g->cmd);
			mfg_list(g);
			exit(0);
		default:
			Msg("@unknown option: -%c\n", c);
		}
	}
	if(g->opt_mac & (g->opt_oui | g->opt_nic | g->opt_bid))
		Msg("@options -m and [-o,-n,-b] are mutually exclusive\n");
	if(g->opt_bid & g->opt_nic)
		Msg("@options -b and -n are mutually exclusive\n");
	if( !(g->opt_mac || g->opt_bid || g->umac))
		Msg("@at least one of the -b,-m or -u options are required\n");

	// check board;

	if( !strcmp(g->board, "ve1000")) {
		g->pciid[0] = 0x8086;
		g->pciid[1] = 0x1509;
		g->nmacs = 2;
		g->map = igb_i350_ve_map;
	} else if( !strcmp(g->board, "edge500")) {
		g->pciid[0] = 0x8086;
		g->pciid[1] = 0x1f41;
		g->nmacs = 4;
		g->map = igb_c2000_map;
		g->extra = edge500_extra;
	} else if( !strncmp(g->board, "edge520", 7)) {
		g->pciid[0] = 0x8086;
		g->pciid[1] = 0x1f41;
		g->nmacs = 6;
		g->map = edge5x0_map;
		g->extra = edge5x0_extra;
	} else if( !strncmp(g->board, "edge540", 7)) {
		g->pciid[0] = 0x8086;
		g->pciid[1] = 0x1f41;
		g->nmacs = 6;
		g->map = edge5x0_map;
		g->extra = edge5x0_extra;
	} else if( !strncmp(g->board, "edge510", 7)) {
		g->pciid[0] = 0x8086;
		g->pciid[1] = 0x1f41;
		g->nmacs = 4;
		g->map = igb_c2000_map;
		g->extra = edge510_extra;
	} else
		Msg("@board '%s' not supported\n", g->board);

	// setup defaults for manufacturing;

	if(g->umac) {
		if(g->opt_oui | g->opt_nic | g->opt_mac | g->opt_bid)
			Msg("@options -u and [-o,-n,-m,-b] are mutually exclusive\n");
		msg = mfg_init(g);
		if(msg)
			Msg("@%s: %s\n", g->umac, msg);
	}
	if(override_ifname) {
		g->ifname = override_ifname;
	}

	// allow pciid overwrite;
	// for eeproms that change pciid after flashing;

	if(g->opt_pciid) {
		g->pciid[0] = pciid[0];
		g->pciid[1] = pciid[1];
	}

	// require template file;

	if( !g->tmp)
		Msg("@need intel eeprom template file (-t)\n");

	// make default macs;

	for(n = 0; n < g->nmacs; n++) {
		macns = g->macs + n;
		mac_add(macns->mac, g->mac, n);
		mac_str(macns->str, macns->mac);
	}

	// report config;

	Msg("using interface %s\n", g->ifname);
	for(n = 0; n < g->nmacs; n++) {
		macns = g->macs + n;
		Msg("mac address %d: %s\n", n, macns->str);
	}
	sprintf(magic, "0x%04x%04x", g->pciid[1], g->pciid[0]);
	Msg("using magic %s\n", magic);

	// read the intel template;
	// must have at least the LAN0..3 sections;

	Msg("reading intel eeprom template %s\n", g->tmp);
	fd = open(g->tmp, O_RDONLY);
	if(fd < 0)
		Msg("@cannot open intel template '%s'\n", g->tmp);
	n = read(fd, eeprom, sizeof(eeprom));
	if(n < 0)
		Msg("@cannot read intel template '%s'\n", g->tmp);
	if(n < 0x280)
		Msg("@intel template '%s' too small (%d)\n", g->tmp, n);
	Msg("read %d bytes from intel template\n", n);
	close(fd);

	// make EEPROM bitmap mods;
	// map mac addresses to chips;
	// call board specific checks and changes;

	if(g->map)
		g->map(g, eeprom, n);
	if(g->extra)
		g->extra(g, eeprom, n);

	// write it out to bitmap;

	macns = g->macs;
	sprintf(file, "/tmp/igb-eeprom-%d-%s.bin", g->opt_x, g->opt_mac? macns->str : g->bid);
	fd = open(file, O_RDWR | O_CREAT | O_TRUNC, 0600);
	if(fd < 0)
		Msg("@cannot open bitmap file '%s'\n", file);
	nb = write(fd, eeprom, n);
	if(nb != n)
		Msg("@cannot write bitmap file '%s'\n", file);
	close(fd);

	// flash the bits;

	sprintf(cmd, "ethtool -E %s magic %s offset 0 < %s",
		g->ifname, magic, file);
	Msg("%sflashing: %s\n", g->opt_dry? "dry run, not " : "", cmd);
	if( !g->opt_dry) {
		n = system(cmd);
		Msg("flash status 0x%x: %s\n", n, n? "FAILED" : "success");
		if(n)
			return(n);
	}

	// remove the temp file above
	unlink(file);

	return(0);
}

