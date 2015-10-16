/* Insufferable and horrendous pattern matching aside, this small utility
 * demonstrates how to register for a pwr fail signal event from the 
 * VeloCloud NMI handler.  This is applicable only for the Edge5x0 right now
 * but I could see this used also for other things, such as IPMI related
 * NMIs for enterprise systems.
 */


#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h> 
#include <limits.h>
#include <string.h>
#include <errno.h>

#define DEV_BUF 1024

char proc_devbuf[DEV_BUF];

void nmi_pwr_loss_handler(int sig)
{
	if (sig == SIGPWR) {
		fprintf(stdout, "Application caught power loss signal, power loss imminent\n");
		fflush(stdout);
	}
}

int main(void)
{
	int rc, fd, major;
	char *devptr, *mptr;
	dev_t nmi_dev;

	if (access("/dev/nmi-vc", F_OK)) {
		
		fd = open("/proc/devices", O_RDONLY);

		if (fd > -1) {

			if (read(fd, proc_devbuf, DEV_BUF) <= 0) {
				rc = errno;
				fprintf(stderr, "Problem accessing /proc/devices\n");
				return (rc);
			}

			devptr = strstr(proc_devbuf, "nmi-vc");

			if (! devptr) {
				fprintf(stderr, "Cannot find nmi-vc, driver loaded?\n");
				return (-ENODEV);
			}
	
			devptr--;
			devptr--;

			while (isdigit(*devptr) && isdigit(*(devptr-1))) { --devptr; }

			major = strtol(devptr, &mptr, 10);
	
			if (major == LONG_MIN || major == LONG_MAX) {
				fprintf(stderr, "Invalid major number\n");
				return (-ENXIO);
			}

			close(fd);
	
			nmi_dev = makedev(major, 0);

			if (mknod("dev/nmi-vc", S_IFCHR, nmi_dev) < 0) {
				fprintf(stderr, "Cannot create device file\n");
				return (-EINVAL);
			}

		} else {
			rc = errno;
			fprintf(stderr, "Configuration problem, check linux .config\n");
			return (rc);	
		}
	}

	fd = open("/dev/nmi-vc", 0);

	signal(SIGPWR, nmi_pwr_loss_handler);

	while (1) {
		sleep (1);
	}
}
