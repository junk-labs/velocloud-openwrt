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
#ifndef MV_VLAN_H
#define MV_VLAN_H

#include <stdint.h>

/* Marvell DSA Device private ioctl commands */
#define SIOCGMVVTU 0x89F8
#define SIOCSMVVTU 0x89F9
#define SIOCDMVVTU 0x89FA
#define SIOCFMVVTU 0x89FB

/* VLAN */
struct dsa_mv_vtu_opdesc {

	uint16_t vid;
	uint16_t fid;
	uint8_t pol_flag;
	uint16_t sid;
	uint32_t egress_flag;
	uint8_t priority;
	uint8_t reserved[4];
};

struct dsa_mv_vtu_opdata {

	uint16_t ops_reg;
	uint16_t vid;
	uint16_t egress_flag;
	uint8_t valid_flag;
	uint8_t reserved[9];
};

struct dsa_mv_vtu_reg {
	union {
		struct dsa_mv_vtu_opdata out;
		struct dsa_mv_vtu_opdesc in;
	} vtu_op;
};

int32_t mv_vtu_read(uint16_t vid, uint16_t fid, uint16_t sid);

int32_t mv_vtu_write(uint16_t vid, uint16_t fid, uint8_t pol_flag,
		uint16_t sid, uint16_t egress_flag, uint8_t priority);

int32_t mv_vtu_purge(uint16_t vid, uint16_t fid, uint16_t sid);

int32_t mv_vtu_dump();

int32_t mv_vtu_flush();

#endif

