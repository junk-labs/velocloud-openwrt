/*
 * Jegadish D (jegadish@velocloud.net)
 * Copyright (C) 2013, Velocloud Inc.
 * 
 * Provides Marvell switch ioctl wrapper APIs for,
 *	- Reading a switch register (both port and global) 
 *	- Writing a value into switch register (both port and global) 
 */
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

#define IF_NAME "sw"

/* Marvell DSA Device private ioctl commands */
#define SIOCGMVREG 0x89F1
#define SIOCSMVREG 0x89F2

/* Marvell DSA register types */
#define MV_REG_PORT 0
#define MV_REG_GLOBAL 1
#define MV_REG_GLOBAL2 2
#define MV_REG_DIRECT 3

/* Marvell DSA Device private ioctl data */
struct dsa_mv_reg {
	uint16_t reg_type;
	uint16_t reserved;
	uint16_t reg_num;
	uint16_t out_reg_value;
};


int32_t mv_reg_read(int32_t fd, uint16_t port_num, 
		uint16_t reg_type, uint16_t location)
{
	struct ifreq if_reg;
	struct dsa_mv_reg* msg = (struct dsa_mv_reg*)(&if_reg.ifr_ifru);
	memset(&if_reg, 0, sizeof(struct ifreq));
	snprintf(if_reg.ifr_name, IFNAMSIZ, IF_NAME"%d", port_num);

	msg->reg_type = reg_type;
	msg->reg_num = location;

	int ioctl_out = ioctl(fd, SIOCGMVREG, &if_reg);
	if (ioctl_out < 0) {
		fprintf(stderr, "SIOCGMVREG on %s failed: %s\n", if_reg.ifr_name,
				strerror(errno));
		return -1;
	}
	/*
	printf("ioctl out: %u  Read value: %u  %02X\n", ioctl_out, 
			msg->out_reg_value, msg->out_reg_value);
			*/
	return msg->out_reg_value;
}

int32_t mv_reg_write(int32_t fd, uint16_t port_num, uint16_t reg_type, 
		uint16_t location, uint16_t value)
{
	struct ifreq if_reg;
	struct dsa_mv_reg* msg = (struct dsa_mv_reg*)(&if_reg.ifr_ifru);
	memset(&if_reg, 0, sizeof(struct ifreq));
	snprintf(if_reg.ifr_name, IFNAMSIZ, IF_NAME"%d", port_num);

	msg->reg_type = reg_type;
	msg->reg_num = location;
	msg->out_reg_value = value;

	int ioctl_out = ioctl(fd, SIOCSMVREG, &if_reg);
	if (ioctl_out < 0) {
		fprintf(stderr, "SIOCSMVREG on %s failed: %s\n", if_reg.ifr_name,
				strerror(errno));
		return -1;
	}
	//printf("ioctl out: %u\n", ioctl_out);
	return ioctl_out;
}


