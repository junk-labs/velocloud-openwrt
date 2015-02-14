/*
 * Jegadish D (jegadish@velocloud.net)
 * Copyright (C) 2013, Velocloud Inc.
 * 
 * Provides Marvell switch ioctl wrapper APIs for,
 *	- Reading a switch register (both port and global) 
 *	- Writing a value into switch register (both port and global) 
 */
#ifndef MV_IOCTL_H
#define MV_IOCTL_H

#include <stdint.h>

int32_t mv_reg_read(int32_t fd, uint16_t port_num, 
		uint16_t reg_type, uint16_t location);

int32_t mv_reg_write(int32_t fd, uint16_t port_num, uint16_t reg_type, 
		uint16_t location, uint16_t value);


#endif

