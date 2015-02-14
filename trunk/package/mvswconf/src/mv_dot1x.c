/*
 * Jegadish D (jegadish@velocloud.net)
 * Copyright (C) 2013, Velocloud Inc.
 * 
 * Provides Marvell switch ioctl wrapper APIs for,
 *	- Setting up 802.1X mode
 *	- Flush all MAC entries on a particular port
 *	- Reading MAC address from switch DB
 *	- Writing MAC address to switch DB 
 *	- Deleting MAC address from switch DB 
 *	- Debug function to dump all learned MAC entries in switch DB
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

#include "mv_dot1x.h"

uint8_t base_mac_addr[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

uint8_t eap_mcast_addr[] = {0x01, 0x80, 0xC2, 0x00, 0x00, 0x03};

int32_t mv_mac_read(char const* if_name, uint8_t* mac_addr)
{
	struct ifreq if_reg;
	struct dsa_mv_atu_opdesc* in_msg = 
		(struct dsa_mv_atu_opdesc*)(&if_reg.ifr_ifru);
	struct dsa_mv_atu_opdata* out_msg = 
		(struct dsa_mv_atu_opdata*)(&if_reg.ifr_ifru);
	int32_t fd, ioctl_out;
	uint32_t if_name_len = strlen(if_name);

	memset(&if_reg, 0, sizeof(struct ifreq));
	strncpy(if_reg.ifr_name, if_name, if_name_len);
	fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (fd < 0) {
		perror("socket: ");
		return -1;
	}

	memcpy(in_msg->mac_addr, mac_addr, 6);
	ioctl_out = ioctl(fd, SIOCGMVMAC, &if_reg);
	if (ioctl_out < 0) {
		fprintf(stderr, "SIOCGMVMAC on %s failed: %s\n", if_reg.ifr_name,
				strerror(errno));
		return -1;
	}
	printf("ioctl reg out: %02x  %02X\n", out_msg->ops_reg, out_msg->data_reg);
	printf("ioctl MAC out: %0x:%0X:%0X:%0X:%0X:%0X\n", 
			out_msg->mac_addr[0],
			out_msg->mac_addr[1], out_msg->mac_addr[2],
			out_msg->mac_addr[3],
			out_msg->mac_addr[4], out_msg->mac_addr[5]);
	close(fd);
	return ioctl_out;
}

int mv_mac_write(char const* if_name, uint8_t entry_state, 
		uint8_t* mac_addr)
{
	struct ifreq if_reg;
	struct dsa_mv_atu_opdesc* in_msg = 
		(struct dsa_mv_atu_opdesc*)(&if_reg.ifr_ifru);
	struct dsa_mv_atu_opdata* out_msg = 
		(struct dsa_mv_atu_opdata*)(&if_reg.ifr_ifru);
	int32_t fd, ioctl_out;
	uint32_t if_name_len = strlen(if_name);

	memset(&if_reg, 0, sizeof(struct ifreq));
	strncpy(if_reg.ifr_name, if_name, if_name_len);

	fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (fd < 0) {
		perror("socket: ");
		return -1;
	}

	memcpy(in_msg->mac_addr, mac_addr, 6);
	in_msg->entry_state = entry_state;
	ioctl_out = ioctl(fd, SIOCSMVMAC, &if_reg);
	if (ioctl_out < 0) {
		fprintf(stderr, "SIOCSMVMAC on %s failed: %s\n", if_reg.ifr_name,
				strerror(errno));
		return -1;
	}
	printf("ioctl reg out: %02x  %02X\n", out_msg->ops_reg, out_msg->data_reg);
	printf("ioctl MAC out: %0x:%0X:%0X:%0X:%0X:%0X\n", 
			out_msg->mac_addr[0],
			out_msg->mac_addr[1], out_msg->mac_addr[2],
			out_msg->mac_addr[3],
			out_msg->mac_addr[4], out_msg->mac_addr[5]);
	close(fd);
	return ioctl_out;
}

int mv_mac_purge(char const* if_name, uint8_t* mac_addr)
{
	struct ifreq if_reg;
	struct dsa_mv_atu_opdesc* in_msg = 
		(struct dsa_mv_atu_opdesc*)(&if_reg.ifr_ifru);
	struct dsa_mv_atu_opdata* out_msg = 
		(struct dsa_mv_atu_opdata*)(&if_reg.ifr_ifru);
	int32_t fd, ioctl_out;
	uint32_t if_name_len = strlen(if_name);

	memset(&if_reg, 0, sizeof(struct ifreq));
	strncpy(if_reg.ifr_name, if_name, if_name_len);

	fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (fd < 0) {
		perror("socket: ");
		return -1;
	}

	memcpy(in_msg->mac_addr, mac_addr, 6);
	ioctl_out = ioctl(fd, SIOCDMVMAC, &if_reg);
	if (ioctl_out < 0) {
		fprintf(stderr, "SIOCDMVMAC on %s failed: %s\n", if_reg.ifr_name,
				strerror(errno));
		return -1;
	}
	printf("ioctl reg out: %02x  %02X\n", out_msg->ops_reg, out_msg->data_reg);
	printf("ioctl MAC out: %0x:%0X:%0X:%0X:%0X:%0X\n", 
			out_msg->mac_addr[0],
			out_msg->mac_addr[1], out_msg->mac_addr[2],
			out_msg->mac_addr[3],
			out_msg->mac_addr[4], out_msg->mac_addr[5]);
	return ioctl_out;
}

int mv_mac_dump(char const* if_name)
{
	struct ifreq if_reg;
	struct dsa_mv_atu_opdesc* in_msg = 
		(struct dsa_mv_atu_opdesc*)(&if_reg.ifr_ifru);
	struct dsa_mv_atu_opdata* out_msg = 
		(struct dsa_mv_atu_opdata*)(&if_reg.ifr_ifru);
	int32_t fd;
	uint32_t if_name_len = strlen(if_name);
	uint8_t t_addr[6];
	uint8_t mac_addr[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
	int ioctl_out = 0;

	memset(&if_reg, 0, sizeof(struct ifreq));
	strncpy(if_reg.ifr_name, if_name, if_name_len);

	fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (fd < 0) {
		perror("socket: ");
		return -1;
	}

	memcpy(in_msg->mac_addr, mac_addr, 6);

	while (1) {
		ioctl_out = ioctl(fd, SIOCGMVMAC, &if_reg);
		if (ioctl_out < 0) {
			fprintf(stderr, "SIOCGMVMAC on %s failed: %s\n", if_reg.ifr_name,
					strerror(errno));
			return -1;
		}
		printf("ioctl reg out: %02X  %02X\n", out_msg->ops_reg, out_msg->data_reg);
		printf("ioctl MAC out: %0X:%0X:%0X:%0X:%0X:%0X\n", 
				out_msg->mac_addr[0],
				out_msg->mac_addr[1], out_msg->mac_addr[2],
				out_msg->mac_addr[3],
				out_msg->mac_addr[4], out_msg->mac_addr[5]);
		memcpy(t_addr, out_msg->mac_addr, 6);
		memset(out_msg, 0, sizeof(struct dsa_mv_atu_opdata));
		memcpy(in_msg->mac_addr, t_addr, 6);
		if ((in_msg->mac_addr[0] == 0xFF) && 
				(in_msg->mac_addr[1] == 0xFF) &&
				(in_msg->mac_addr[2] == 0xFF) &&
				(in_msg->mac_addr[3] == 0xFF) &&
				(in_msg->mac_addr[4] == 0xFF) &&
				(in_msg->mac_addr[5] == 0xFF)) {
			break;
		}	
	}
	close(fd);
	return ioctl_out;
}


int mv_mac_flush(char const* if_name, uint8_t flush_all_flag)
{
	struct ifreq if_reg;
	struct dsa_mv_atu_opdesc* in_msg = 
		(struct dsa_mv_atu_opdesc*)(&if_reg.ifr_ifru);
	struct dsa_mv_atu_opdata* out_msg = 
		(struct dsa_mv_atu_opdata*)(&if_reg.ifr_ifru);
	int32_t fd, ioctl_out;
	uint32_t if_name_len = strlen(if_name);

	memset(&if_reg, 0, sizeof(struct ifreq));
	strncpy(if_reg.ifr_name, if_name, if_name_len);

	fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (fd < 0) {
		perror("socket: ");
		return -1;
	}

	in_msg->flush_all_flag = flush_all_flag;
	ioctl_out = ioctl(fd, SIOCFMVMAC, &if_reg);
	if (ioctl_out < 0) {
		fprintf(stderr, "SIOCFMVMAC on %s failed: %s\n", if_reg.ifr_name,
				strerror(errno));
		return -1;
	}
	printf("ioctl reg out: %02x  %02X\n", out_msg->ops_reg, out_msg->data_reg);
	printf("ioctl MAC out: %0x:%0X:%0X:%0X:%0X:%0X\n", 
			out_msg->mac_addr[0],
			out_msg->mac_addr[1], out_msg->mac_addr[2],
			out_msg->mac_addr[3],
			out_msg->mac_addr[4], out_msg->mac_addr[5]);
	close(fd);
	return ioctl_out;

}

int mv_setup_8021x(char const* if_name)
{
	int32_t rc = 0, fd;
	uint32_t if_name_len = strlen(if_name);
	struct ifreq if_reg;

	memset(&if_reg, 0, sizeof(struct ifreq));
	strncpy(if_reg.ifr_name, if_name, if_name_len);

	fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (fd < 0) {
		perror("socket: ");
		rc = -1;
		goto error_return;
	}

	rc = ioctl(fd, SIOCSMV1XM, &if_reg);
	if (rc < 0) {
		fprintf(stderr, "SIOCSMV1XM on %s failed: %s\n", if_reg.ifr_name,
				strerror(errno));
		rc = -2;
	}
	close(fd);
error_return:
	return rc;
}

