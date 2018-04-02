// miitest.c v1 Sandra Berndt
// (c) Velocloud Inc 2016

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <assert.h>
#include <linux/mii.h>
#include <linux/sockios.h>
#include <net/if.h>

// globals;

typedef struct g g_t;
struct g {
	char *cmd;		// argv[0];
	char *ifname;		// interface name;
	char *test;		// test to run;
	int fd;			// netlink socket fd;
};

g_t G;

// mii data in struct ifreq;

static inline struct mii_ioctl_data *
if_mii(struct ifreq *rq)
{
	return((struct mii_ioctl_data *)&rq->ifr_ifru);
}

// usage help;

void
Usage(void)
{
	g_t *g = &G;

	fprintf(stderr, "usage: %s if testtype\n", g->cmd);
	exit(-1);
}

// read mii reg;

int
mii_read(g_t *g, unsigned reg)
{
	int rv;
	struct mii_ioctl_data *mid;
	struct ifreq ifr;

	bzero(&ifr, sizeof(ifr));
	strncpy(ifr.ifr_name, g->ifname, IFNAMSIZ-1);
	mid = if_mii(&ifr);
	mid->reg_num = reg;
	rv = ioctl(g->fd, SIOCGMIIREG, &ifr);
	if(rv < 0)
		return(-1);
	return(mid->val_out);
}

int
mii_read_err(g_t *g, unsigned reg)
{
	int rv;

	rv = mii_read(g, reg);
	if(rv < 0) {
		fprintf(stderr, "%s: cannot read mii reg %d\n", g->cmd, reg);
		exit(-3);
	}
	return(rv);
}

// write mii reg;

int
mii_write(g_t *g, unsigned reg, unsigned val)
{
	int rv;
	struct mii_ioctl_data *mid;
	struct ifreq ifr;

	bzero(&ifr, sizeof(ifr));
	strncpy(ifr.ifr_name, g->ifname, IFNAMSIZ-1);
	mid = if_mii(&ifr);
	mid->reg_num = reg & 0xff;
	mid->val_in = val;
	mid->val_out = reg >> 8;
	rv = ioctl(g->fd, SIOCSMIIREG, &ifr);
	return(rv);
}

void
mii_write_err(g_t *g, unsigned reg, unsigned val)
{
	int rv;

	rv = mii_write(g, reg, val);
	if(rv < 0) {
		fprintf(stderr, "%s: cannot write mii reg %d\n", g->cmd, reg);
		exit(-4);
	}
}

// report mii status;

void
mii_status(g_t *g)
{
	int rv;

	rv = mii_read_err(g, MII_BMSR);
	printf("current status:\n");
	printf(" link %s\n", (rv & BMSR_LSTATUS)? "up" : "down");
	printf(" aneg %scapable\n", (rv & BMSR_ANEGCAPABLE)? "" : "in");
	printf(" aneg %scomplete\n", (rv & BMSR_ANEGCOMPLETE)? "" : "in");
}

// 1000bt normal mode;

int
mii_normal(g_t *g)
{
	int rv;
	unsigned val;

	// unset test mode;
	// need phy reset to apply;
	// unblock MDIO writes by driver;

	printf("setting normal mode\n");
	val = 0;
	mii_write_err(g, 0x200 | MII_CTRL1000, val);
	val = BMCR_RESET;
	mii_write_err(g, MII_BMCR, val);

	return(0);
}

// 1000bt test mode 1;

int
mii_test1(g_t *g)
{
	int rv;
	unsigned val;

	printf("1000bt test mode 1\n");
	mii_status(g);

	// set test mode;
	// need phy reset to apply;
	// block further MDIO writes by driver;

	printf("setting test mode1\n");
	val = (1 << 13) | (1 << 12) | (1 << 11);
	mii_write_err(g, 0x100 | MII_CTRL1000, val);
/*XXX
	val =  BMCR_SPEED1000 | BMCR_RESET;
	mii_write_err(g, 0x100 | MII_BMCR, val);
*/

	return(0);
}

// main entry;

int
main(int argc, char **argv)
{
	int rv;
	g_t *g = &G;
	struct mii_ioctl_data *mid;
	struct ifreq ifr;

	// must have interface and test type;

	g->cmd = argv[0];
	if(argc < 3)
		Usage();
	g->ifname = argv[1];
	g->test = argv[2];

	// open netlink socket;

	g->fd = socket(AF_INET, SOCK_DGRAM, 0);
	if(g->fd < 0) {
		perror("socket");
		return(-1);
	}

	// find out phy info;

	bzero(&ifr, sizeof(ifr));
	strncpy(ifr.ifr_name, g->ifname, IFNAMSIZ-1);
	mid = if_mii(&ifr);
	rv = ioctl(g->fd, SIOCGMIIPHY, &ifr);
	if(rv < 0) {
		perror("phy addr");
		return(-2);
	}
	printf("%s: phy addr: 0x%02x\n", g->cmd, mid->phy_id);

	// branch to specific test;

	if( !strcmp(g->test, "normal"))
		rv = mii_normal(g);
	else if( !strcmp(g->test, "tm1"))
		rv = mii_test1(g);
	else {
		fprintf(stderr, "%s: '%s' no such test\n", g->cmd, g->test);
		rv = -3;
	}

	return(0);
}

