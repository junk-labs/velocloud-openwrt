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

// global data;

typedef struct g g_t;
struct g {
	char *cmd;			// argv[0];
	char opt_oui;
	char opt_nic;
	char opt_mac;
	char opt_bid;
	char opt_pciid;
	char nmacs;
	char *bid;
	char *board;
	unsigned short pciid[2];
	unsigned char mac[6];
	unsigned char mac0[6];
	unsigned char mac1[6];
	unsigned char mac2[6];
	unsigned char mac3[6];
	char *ifname;
	char *tmp;
	void (*extra)(g_t *g, unsigned char *eeprom, int size);
};

g_t G;

// null MAC;

unsigned char null_mac[6];

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

#define PHY0_ADDR_6320 0	// shared MDIO[0];
#define PHY1_ADDR_6176 0	// full bus decode, MDIO[1];
#define PHY2_ADDR_6320 0	// shared MDIO[0];
#define PHY3_ADDR_1112 0x7	// shared MDIO[0];

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

// edge500 igb board specifics;

void
edge500_extra(g_t *g, unsigned char *eeprom, int size)
{
	// program OEM specific words;

	igb_word(eeprom, 0x06, 0x5663);	// "Vc";
	igb_word(eeprom, 0x07, 0x6535);	// "e5";

	// set phy mdio addresses;

	igb_ic4(eeprom + 0x000, PHY0_ADDR_6320);
	igb_ic4(eeprom + 0x100, PHY1_ADDR_6176);
	igb_ic4(eeprom + 0x180, PHY2_ADDR_6320);
	igb_ic4(eeprom + 0x200, PHY3_ADDR_1112);

	// set MDICNFG.COM_MDIO based on ports;

	igb_ic3(eeprom + 0x000, 1);
	igb_ic3(eeprom + 0x100, 0);
	igb_ic3(eeprom + 0x180, 1);
	igb_ic3(eeprom + 0x200, 1);
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

// main entry;

int
main(int argc, char **argv)
{
	g_t *g = &G;
	int c, fd, n, nb;
	unsigned short pciid;
	char mac0str[20];
	char mac1str[20];
	char mac2str[20];
	char mac3str[20];
	char magic[16];
	char bid[20];
	char file[50];
	char cmd[200];

	g->cmd = argv[0];
	g->ifname = "eth0";
	g->mac[0] = 'V';
	g->mac[1] = 'C';
	g->mac[2] = '1';
	g->pciid[0] = 0x8086;

	// get board name;

	g->board = get_board(file, sizeof(file));
	if( !g->board)
		Msg("@cannot determine board\n");
	Msg("found board '%s'\n", g->board);

	// get options;

	while((c = getopt(argc, argv, "o:n:m:B:b:p:i:t:h")) != EOF) {
		switch(c) {
		case 'o' :
			if( !get_num(optarg, &g->mac[0], 3))
				Msg("@bad OUI: %s\n", optarg);
			g->opt_oui = 1;
			break;
		case 'n' :
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
			if( !get_pciid(optarg, g->pciid))
				Msg("@bad PCI ID: %s\n", optarg);
			g->opt_pciid = 1;
			break;
		case 'i':
			g->ifname = optarg;
			break;
		case 't' :
			g->tmp = optarg;
			break;
		case 'h':
			Msg("help\n"
				"  %s [options]\n"
				"  [-o OUI]       (DEC, 0xHEX, ab:cd:ef)\n"
				"  [-n NIC]       (DEC, 0xHEX, ab:cd:ef)\n"
				"  [-m MAC]       (00:11:22:33:44:55)\n"
				"  [-B board]     (force board: ve1000, edge500)\n"
				"  [-b board-id]  (DEC, 0xHEX)\n"
				"  [-p pci-id]    (8086:1509)\n"
				"  [-i if]        (eth0)\n"
				"  [-t intel-tmp] (intel template file)\n"
				"  [-h]           (this help)\n",
				g->cmd);
			break;
		default:
			Msg("@unknown option: -%c\n", c);
		}
	}
	if(g->opt_mac & (g->opt_oui | g->opt_nic | g->opt_bid))
		Msg("@options -m and [-o,-n,-b] are mutually exclusive\n");
	if(g->opt_bid & g->opt_nic)
		Msg("@options -b and -n are mutually exclusive\n");
	if( !(g->opt_mac | g->opt_bid))
		Msg("@at least -b or -m options are required\n");
	if( !g->tmp)
		Msg("@need intel eeprom template file (-t)\n");

	// check board;

	if( !strcmp(g->board, "ve1000")) {
		pciid = 0x1509;
		g->nmacs = 2;
	} else if( !strcmp(g->board, "edge500")) {
		pciid = 0x1f41;
		g->nmacs = 4;
		g->extra = edge500_extra;
	} else
		Msg("@board '%s' not supported\n", g->board);
	if( !g->opt_pciid)
		g->pciid[1] = pciid;

	// make default macs;

	bcopy(g->mac, g->mac0, sizeof(g->mac));
	bcopy(g->mac, g->mac1, sizeof(g->mac));
	bcopy(g->mac, g->mac2, sizeof(g->mac));
	bcopy(g->mac, g->mac3, sizeof(g->mac));
	g->mac0[5] ^= 0x00;
	g->mac1[5] ^= 0x01;
	g->mac2[5] ^= 0x02;
	g->mac3[5] ^= 0x03;

	// report config;

	mac_str(mac0str, g->mac0);
	mac_str(mac1str, g->mac1);
	mac_str(mac2str, g->mac2);
	mac_str(mac3str, g->mac3);

	Msg("using interface %s\n", g->ifname);
	Msg("mac address 0: %s\n", mac0str);
	Msg("mac address 1: %s\n", mac1str);
	if(g->nmacs == 4) {
		Msg("mac address 2: %s\n", mac2str);
		Msg("mac address 3: %s\n", mac3str);
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

	// make EEPROM bitmap;

	bcopy(g->mac0, eeprom + 0x0, sizeof(g->mac0));
	bcopy(g->mac1, eeprom + 0x100, sizeof(g->mac1));
	if(g->nmacs == 4) {
		bcopy(g->mac2, eeprom + 0x180, sizeof(g->mac2));
		bcopy(g->mac3, eeprom + 0x200, sizeof(g->mac3));
	} else {
		bcopy(null_mac, eeprom + 0x180, sizeof(null_mac));
		bcopy(null_mac, eeprom + 0x200, sizeof(null_mac));
	}

	// call board specific checks and changes;

	if(g->extra)
		g->extra(g, eeprom, n);

	// write it out to bitmap;

	sprintf(file, "igb-eeprom-%s.bin", g->opt_mac? mac0str : g->bid);
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
	Msg("flashing: %s\n", cmd);
	n = system(cmd);
	Msg("flash status 0x%x: %s\n", n, n? "FAILED" : "success");

	return(0);
}

