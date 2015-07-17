/*
 * This file is part of the coreboot project.
 *
 * Copyright (C) 2011 Advanced Micro Devices, Inc.
 * Copyright (c) 2014 Sage Electronic Engineering, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
 */

#ifndef SUPERIO_NUVOTON_NCT6776F_NCT6776F_H
#define SUPERIO_NUVOTON_NCT6776F_NCT6776F_H

/* Logical Device Numbers (LDN). */
#define NCT6776F_FDC        0x00  /* FDC - not pinned out */
#define NCT6776F_LPT        0x01
#define NCT6776F_SP1        0x02  /* UARTA */
#define NCT6776F_SP2        0x03  /* UARTB - IR*/
#define NCT6776F_KBD        0x05
#define NCT6776F_CIR        0x06
#define NCT6776F_GPIO6789   0x07  /* GPIO 6,7,8,9 Interface */
#define NCT6776F_GPIO_WDT   0x08  /* GPIO 0,1,A & WDT Interface */
#define NCT6776F_GPIO2345   0x09  /* GPIO 2,3,4,5 Interface */
#define NCT6776F_ACPI       0x0A  /* ACPI */
#define NCT6776F_HWMON      0x0B  /* Hardware Monitor / Front panel LED */
#define NCT6776F_VID        0x0D
#define NCT6776F_CIR_WAKE   0x0E
#define NCT6776F_GPIO_PP_OD 0x0F  /* GPIO Push-Pull / Open drain select  */
#define NCT6776F_SVID       0x14
#define NCT6776F_SLEEP      0x16
#define NCT6776F_GPIOA      0x17

/* Virtual Logical Device Numbers (LDN) */
#define NCT6776F_GPIO_V 0x07 /* GPIO - 0,1,6 Interface */

/* SIO global configuration */
#define IRQ_TYPE_SEL_CR10   0x10     /* UARTA,UARTB */

#define GLOBAL_OPTION_CR26  0x26
#define CR26_LOCK_REG       (1 << 4) /* required to access CR10/CR11 */
#define GLOBAL_OPTION_CR2A  0x2a

/* CR10 Bit definitions */
#define FDC_IRQ_TYPE_SELECT_BIT		7
#define LPT_IRQ_TYPE_SELECT_BIT		6
#define SP1_IRQ_TYPE_SELECT_BIT		5
#define SP2_IRQ_TYPE_SELECT_BIT		4
#define KBC_IRQ_TYPE_SELECT_BIT		3
#define MOUSE_IRQ_TYPE_SELECT_BIT	2
#define CIR_IRQ_TYPE_SELECT_BIT		1
#define CIR_WAKE_IRQ_TYPE_SELECT_BIT	0


/* Virtual devices sharing the enables are encoded as follows:
	VLDN = baseLDN[7:0] | [10:8] bitpos of enable in 0x30 of baseLDN
*/
#define NCT6776F_GPIO0 ((0 << 8) | NCT6776F_GPIO_V)
#define NCT6776F_GPIO1 ((1 << 8) | NCT6776F_GPIO_V)
#define NCT6776F_GPIO6 ((6 << 8) | NCT6776F_GPIO_V)

void nct6776f_enable_serial(device_t dev, u16 iobase);

#endif /* SUPERIO_NUVOTON_NCT6776F_NCT6776F_H */
