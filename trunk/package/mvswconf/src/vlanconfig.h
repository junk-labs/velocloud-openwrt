/* 
 * K.Nandha Kumar (nandha@velocloud.net)
 * Copyright (C) 2013, Velocloud Inc.
 * 
 * Vlan features and configuration
 */

#ifndef VLAN_GLOBAL_H
#define VLAN_GLOBAL_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <linux/if_ether.h>
#include <stdint.h>
#include <errno.h>
#if 0
#include "vc_dbg.h"
#else
#include "syslog.h"

#define DBG_LOG(lvl,_,format, args...) syslog(LOG_USER|LOG_##lvl, format, ##args)
#define LOG_SEVERE LOG_ERR
#endif

#define IF_NAME "sw"
#define VLAN_CFG_FILE "/tmp/.vlanedge.info"
#define EDGE_CFG_FILE "/opt/vc/.edge.info"

#define VLAN_MAX 4096

#define LAN_TOT_PORTS 4
#define LANPORT1 0x1
#define LANPORT2 0x2
#define LANPORT3 0x4
#define LANPORT4 0x8

#define VLAN_SUCCESS 0
#define VLAN_FAILURE -1

enum port_modes {
        VLAN_ACCESS_MODE = 0,
        VLAN_TRUNK_MODE
};

typedef struct {
        int *iVlanIds;
        int iTotalVlans;
        int iDefaultVlanId;
        enum port_modes portMode;
        int iSwitchId;
        int iPort;  /* port index within the switch */
} eth_ports_s;

typedef struct _vlan_tbl_s {
	int lanPorts : LAN_TOT_PORTS;
} vlan_tbl_s;

int parse_edge_config();
void vlan_ids_free();

int vlan_tbl_build();
//void vlan_tbl_free();

void printRegisters();

void setVlanPorts(int port_num, int *iVlanIds, int iTotalVlans,
		char *portMode, char *intftype, int untaggedVlan);
#endif
