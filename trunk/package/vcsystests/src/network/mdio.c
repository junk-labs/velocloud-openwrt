#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <time.h>
#include <syslog.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#ifndef __GLIBC__
#include <linux/if_arp.h>
#include <linux/if_ether.h>
#endif
#include <linux/mii.h>
#include <sockios.h>






/*--------------------------------------------------------------------*/

static int mdio_read(int skfd, int location, struct ifreq ifr)
{
	struct mii_ioctl_data *mii = (struct mii_ioctl_data *)&ifr.ifr_data;
	mii->reg_num = location;
	if (ioctl(skfd, SIOCGMIIREG, &ifr) < 0) {
		fprintf(stderr, "SIOCGMIIREG on %s failed: %s\n", ifr.ifr_name,
			strerror(errno));
		return -1;
	}
	return mii->val_out;
}

static void mdio_write(int skfd, int location, int value, struct ifreq ifr)
{
	struct mii_ioctl_data *mii = (struct mii_ioctl_data *)&ifr.ifr_data;
	mii->reg_num = location;
	mii->val_in = value;
	if (ioctl(skfd, SIOCSMIIREG, &ifr) < 0) {
		fprintf(stderr, "SIOCSMIIREG on %s failed: %s\n", ifr.ifr_name,
			strerror(errno));
	}
}

/*--------------------------------------------------------------------*/
