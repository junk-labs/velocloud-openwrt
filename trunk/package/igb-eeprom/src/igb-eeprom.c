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
	char *bid;
	unsigned short pciid[2];
	unsigned char mac[6];
	unsigned char mac0[6];
	unsigned char mac1[6];
	char *ifname;
	char *tmp;
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

// main entry;

int
main(int argc, char **argv)
{
	g_t *g = &G;
	int c, fd, n, nb;
	char mac0str[20];
	char mac1str[20];
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
	g->pciid[1] = 0x1509;

	// get options;

	while((c = getopt(argc, argv, "o:n:m:b:p:i:t:h")) != EOF) {
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
				"  [-b board-id]  (DEC, 0xHEX, ab:cd:ef)\n"
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

	bcopy(g->mac, g->mac0, sizeof(g->mac));
	bcopy(g->mac, g->mac1, sizeof(g->mac));
	g->mac1[5] ^= 1;

	// report config;

	mac_str(mac0str, g->mac0);
	mac_str(mac1str, g->mac1);
	Msg("using interface %s\n", g->ifname);
	Msg("mac address 0: %s\n", mac0str);
	Msg("mac address 1: %s\n", mac1str);

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
	bcopy(null_mac, eeprom + 0x180, sizeof(null_mac));
	bcopy(null_mac, eeprom + 0x200, sizeof(null_mac));

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

