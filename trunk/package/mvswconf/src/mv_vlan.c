/*
 * Jegadish D (jegadish@velocloud.net)
 * Copyright (C) 2013, Velocloud Inc.
 * 
 * Provides Marvell switch ioctl wrapper APIs for,
 *	- Setting up 802.1X mode
 *	- Flush all VTU entries 
 *	- Reading VTU entry from switch DB
 *	- Writing VTU entry to switch DB 
 *	- Deleting VTU enrty from switch DB 
 *	- Debug function to dump all VTU entries in switch DB
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

#include "mv_vlan.h"

#define MV_VTU_SW_PORT "sw0"

int32_t mv_vtu_read(uint16_t vid, uint16_t fid, uint16_t sid)
{
	struct ifreq if_reg;
	struct dsa_mv_vtu_opdesc* in_msg = 
		(struct dsa_mv_vtu_opdesc*)(&if_reg.ifr_ifru);
	struct dsa_mv_vtu_opdata* out_msg = 
		(struct dsa_mv_vtu_opdata*)(&if_reg.ifr_ifru);
	int32_t fd, ioctl_out;
	uint32_t if_name_len = strlen(MV_VTU_SW_PORT);

	memset(&if_reg, 0, sizeof(struct ifreq));
	strncpy(if_reg.ifr_name, MV_VTU_SW_PORT, if_name_len);
	fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (fd < 0) {
		perror("socket: ");
		return -1;
	}

	in_msg->vid = vid;
	in_msg->fid = fid;
	in_msg->sid = sid;
	printf("Read VID: %d\n", in_msg->vid);
	ioctl_out = ioctl(fd, SIOCGMVVTU, &if_reg);
	if (ioctl_out < 0) {
		fprintf(stderr, "SIOCGMVVTU on %s failed: %s\n", 
				if_reg.ifr_name, strerror(errno));
		return -1;
	}
	printf("ioctl reg out: %02x\n", out_msg->ops_reg); 
	printf("ioctl VID: %0X\n", out_msg->vid); 
	printf("ioctl Data out: %0X\n", out_msg->egress_flag); 
	close(fd);
	return out_msg->egress_flag;
}


int32_t mv_vtu_write(uint16_t vid, uint16_t fid, uint8_t pol_flag,
		uint16_t sid, uint16_t egress_flag, uint8_t priority)
{
	struct ifreq if_reg;
	struct dsa_mv_vtu_opdesc* in_msg = 
		(struct dsa_mv_vtu_opdesc*)(&if_reg.ifr_ifru);
	struct dsa_mv_vtu_opdata* out_msg = 
		(struct dsa_mv_vtu_opdata*)(&if_reg.ifr_ifru);
	int32_t fd, ioctl_out;
	uint32_t if_name_len = strlen(MV_VTU_SW_PORT);

	memset(&if_reg, 0, sizeof(struct ifreq));
	strncpy(if_reg.ifr_name, MV_VTU_SW_PORT, if_name_len);

	fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (fd < 0) {
		perror("socket: ");
		return -1;
	}
	in_msg->vid = vid;
	in_msg->fid = fid;
	in_msg->pol_flag = pol_flag;
	in_msg->sid = sid;
	in_msg->egress_flag = egress_flag;
	in_msg->priority = priority;

	ioctl_out = ioctl(fd, SIOCSMVVTU, &if_reg);
	if (ioctl_out < 0) {
		fprintf(stderr, "SIOCSMVVTU on %s failed: %s\n", 
				if_reg.ifr_name, strerror(errno));
		return -1;
	}
	printf("ioctl reg out: %02x\n", out_msg->ops_reg);
	close(fd);
	return ioctl_out;
}


int32_t mv_vtu_purge(uint16_t vid, uint16_t fid, uint16_t sid)
{
	struct ifreq if_reg;
	struct dsa_mv_vtu_opdesc* in_msg = 
		(struct dsa_mv_vtu_opdesc*)(&if_reg.ifr_ifru);
	struct dsa_mv_vtu_opdata* out_msg = 
		(struct dsa_mv_vtu_opdata*)(&if_reg.ifr_ifru);
	int32_t fd, ioctl_out;
	uint32_t if_name_len = strlen(MV_VTU_SW_PORT);

	memset(&if_reg, 0, sizeof(struct ifreq));
	strncpy(if_reg.ifr_name, MV_VTU_SW_PORT, if_name_len);

	fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (fd < 0) {
		perror("socket: ");
		return -1;
	}

	in_msg->vid = vid;
	in_msg->fid = fid;
	in_msg->sid = sid;
	ioctl_out = ioctl(fd, SIOCDMVVTU, &if_reg);
	if (ioctl_out < 0) {
		fprintf(stderr, "SIOCDMVVTU on %s failed: %s\n", 
				if_reg.ifr_name, strerror(errno));
		return -1;
	}
	printf("ioctl reg out: %02x\n", out_msg->ops_reg); 
	return ioctl_out;
}


int32_t mv_vtu_dump()
{
	struct ifreq if_reg;
	struct dsa_mv_vtu_opdesc* in_msg = 
		(struct dsa_mv_vtu_opdesc*)(&if_reg.ifr_ifru);
	struct dsa_mv_vtu_opdata* out_msg = 
		(struct dsa_mv_vtu_opdata*)(&if_reg.ifr_ifru);
	int32_t fd;
	uint32_t if_name_len = strlen(MV_VTU_SW_PORT);
	int ioctl_out = 0;

	memset(&if_reg, 0, sizeof(struct ifreq));
	strncpy(if_reg.ifr_name, MV_VTU_SW_PORT, if_name_len);

	fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (fd < 0) {
		perror("socket: ");
		return -1;
	}

	in_msg->vid = 0;
	while (1) {
		ioctl_out = ioctl(fd, SIOCGMVVTU, &if_reg);
		if (ioctl_out < 0) {
			fprintf(stderr, "SIOCGMVVTU on %s failed: %s\n", 
					if_reg.ifr_name, strerror(errno));
			return -1;
		}
		if (out_msg->vid == 0x0FFF)
			break;
		printf("ioctl reg out: %02x  %02X  %02X\n", out_msg->ops_reg, 
				out_msg->egress_flag, out_msg->vid);
		in_msg->vid = out_msg->vid;
	}
	close(fd);
	return ioctl_out;
}


int32_t mv_vtu_flush()
{
	struct ifreq if_reg;
	struct dsa_mv_vtu_opdata* out_msg = 
		(struct dsa_mv_vtu_opdata*)(&if_reg.ifr_ifru);
	int32_t fd, ioctl_out;
	uint32_t if_name_len = strlen(MV_VTU_SW_PORT);

	memset(&if_reg, 0, sizeof(struct ifreq));
	strncpy(if_reg.ifr_name, MV_VTU_SW_PORT, if_name_len);

	fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (fd < 0) {
		perror("socket: ");
		return -1;
	}

	ioctl_out = ioctl(fd, SIOCFMVVTU, &if_reg);
	if (ioctl_out < 0) {
		fprintf(stderr, "SIOCFMVVTU on %s failed: %s\n", 
				if_reg.ifr_name, strerror(errno));
		return -1;
	}
	printf("ioctl reg out: %02X\n", out_msg->ops_reg); 
	close(fd);
	return ioctl_out;
}

