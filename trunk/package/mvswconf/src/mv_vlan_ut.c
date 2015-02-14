#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <stdint.h>
#include <errno.h>

#include "mv_vlan.h"

#define IF_NAME "sw"

/* Marvell DSA Device private ioctl commands */
#define SIOCGMVVTU 0x89F8
#define SIOCSMVVTU 0x89F9
#define SIOCDMVVTU 0x89FA
#define SIOCFMVVTU 0x89FB

int main(int argc, char** argv) {

	//uint16_t vid = 0;
	/*
	uint16_t fid = 0;
	uint16_t sid = 0;
	uint16_t egress_flag = 0x0FFF;
	uint8_t pol_flag = 0;
	uint8_t priority = 0;
	uint32_t i = 11;

	for (i = 1; i < 100; i++) {
		if (mv_vtu_write(i, fid, pol_flag, sid, egress_flag, priority) < 0) {

			printf("MV VTU Write failed\n");
			exit(0);
		}
	}
*/
	mv_vtu_dump();

	/*
	mv_vtu_flush();

	vid = 10;
	if (mv_vtu_read(vid, fid, sid) < 0) {

		printf("MV VTU Read failed\n");
		exit(0);
	}
*/

	return 0;
}

