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
#ifndef MV_DOT1X_H
#define MV_DOT1X_H

#include <stdint.h>

/* Marvell DSA Device private ioctl commands */
#define SIOCGMVREG 0x89F1
#define SIOCSMVREG 0x89F2
#define SIOCGMVMAC 0x89F3
#define SIOCSMVMAC 0x89F4
#define SIOCDMVMAC 0x89F5
#define SIOCFMVMAC 0x89F6
#define SIOCSMV1XM 0x89F7

/* Marvell DSA register types */
#define MV_REG_PORT 0
#define MV_REG_GLOBAL 1
#define MV_REG_GLOBAL2 2


struct dsa_mv_atu_opdata {

	uint16_t ops_reg;
	uint16_t data_reg;
	uint8_t mac_addr[6];
	uint8_t reserved[6];
};

struct dsa_mv_atu_opdesc {
	uint8_t mac_addr[6];
	uint16_t fid;
	uint8_t entry_state;
	uint8_t flush_all_flag;
	uint8_t reserved[6];
};

struct dsa_mv_atu_reg {
	union {
		struct dsa_mv_atu_opdata out;
		struct dsa_mv_atu_opdesc in;
	} atu_op;
};


int mv_mac_read(char const* if_name, uint8_t* mac_addr);

int mv_mac_write(char const* if_name, uint8_t entry_state, 
		uint8_t* mac_addr);

int mv_mac_purge(char const* if_name, uint8_t* mac_addr);

int mv_mac_dump(char const* if_name);

int mv_mac_flush(char const* if_name, uint8_t flush_all_flag);

int mv_setup_8021x(char const* if_name);


#endif

