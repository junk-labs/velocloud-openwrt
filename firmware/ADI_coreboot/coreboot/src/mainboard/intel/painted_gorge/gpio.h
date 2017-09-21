/*
 * This file is part of the coreboot project.
 *
 * Copyright (C) 2011 The Chromium OS Authors. All rights reserved.
 * Copyright (C) 2013 Sage Electronic Engineering, LLC.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
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

#ifndef MAINBOARD_GPIO_H
#define MAINBOARD_GPIO_H

#include "southbridge/intel/fsp_rangeley/gpio.h"

/*
 * GPIO Configuration from PG_GPIO_REV1P2.xlsx
 * Unused GPIOS set to Output
 *
 * GPIO Signal | Default Signal Name | Default I/O | Painted Gorge Function | Painted Gorge I/O | Default - Pullup/Pulldown           | Notes
 * GPIOS_0     | NMI                 | I           |                        |                   | 1K PD On Board                      |
 * GPIOS_1     | ERROR2              | O           |                        |                   | None                                |
 * GPIOS_2     | ERROR1              | O           |                        |                   | None                                |
 * GPIOS_3     | ERROR0              | O           |                        |                   | None                                |
 * GPIOS_4     | IERR                | O           |                        |                   | None                                |
 * GPIOS_5     | MCERR               | O           |                        |                   | None                                |
 * GPIOS_6     | UART1_RXD           | I           | UART1_RXD              | I                 | None                                |
 * GPIOS_7     | UART1_TXD           | O           | UART1_TXD              | O                 | 10K PD On Board                     |
 * GPIOS_8     | SMB_CLK0            | I/O, OD     | SMB_CLK0               | I/O, OD           | 2.2K PU On Board                    |
 * GPIOS_9     | SMB_DATA0           | I/O, OD     | SMB_DATA0              | I/O, OD           | 2.2K PU On Board                    |
 * GPIOS_10    | SMBALERT_N0         | I/O, OD     | SMBALERT_N0            | I/O, OD           | 2.2K PU On Board                    |
 * GPIOS_11    | SMB_DATA1           | I/O, OD     | SMB_DATA1              | I/O, OD           | 2.2K PU On Board                    |
 * GPIOS_12    | SMB_CLK1            | I/O, OD     | SMB_CLK1               | I/O, OD           | 2.2K PU On Board                    |
 * GPIOS_13    | SMB_DATA2           | I/O, OD     | SMB_DATA2              | I/O, OD           | 2.2K PU On Board                    |
 * GPIOS_14    | SMB_CLK2            | I/O, OD     | SMB_CLK2               | I/O, OD           | 2.2K PU On Board                    |
 * GPIOS_15    | SATA_GP0            | I/O         |                        |                   | None                                |
 * GPIOS_16    | SATA_LEDN           | O, OD       |                        |                   | None                                |
 * GPIOS_17    | SATA3_GP0           | I/O         |                        |                   | None                                |
 * GPIOS_18    | SATA3_LEDN          | O, OD       |                        |                   | None                                |
 * GPIOS_19    | FLEX_CLK_SEL0       | I           | FLEX_CLK_SEL0          | I                 | 10K PU On board                     |
 * GPIOS_20    | FLEX_CLK_SEL1       | I           | FLEX_CLK_SEL1          | I                 | 10K PU On board                     |
 * GPIOS_21    | LPC_AD0             | O           | LPC_AD0                | O                 | None                                |
 * GPIOS_22    | LPC_AD1             | O           | LPC_AD1                | O                 | None                                |
 * GPIOS_23    | LPC_AD2             | O           | LPC_AD2                | O                 | None                                |
 * GPIOS_24    | LPC_AD3             | O           | LPC_AD3                | O                 | None                                |
 * GPIOS_25    | LPC_FRAME           | O           | LPC_FRAME              | O                 | None                                |
 * GPIOS_26    | LPC_CLKOUT0         | O           | LPC_CLKOUT0            | O                 | None                                |
 * GPIOS_27    | LPC_CLKOUT1         | O           |                        |                   | None                                |
 * GPIOS_28    | LPC_CLKRUNB         | I/O, OD     |                        |                   | 1K PU On Board                      |
 * GPIOS_29    | ILB_SEIRQ           | I/O         |                        |                   | None                                |
 * GPIOS_30    | PMU_RESETBUTTON_B   | I           | PMU_RESETBUTTON_B      | I                 | None                                |
 * GPIO_SUS0   | GPIO_SUS0           | I/O         | FP_GPIO9               | O                 | 1K PU On Board (BIOS Default High)  | Alta Boot mode 0 - from CPU, 1 - from SPI
 * GPIO_SUS1   | GPIO_SUS1           | I/O         | ALTA_VDD_OFF_N         | O                 | 1K PU On Board (BIOS Default High)  | Alta Power supply turn off from Avoton (active low)
 * GPIO_SUS2   | GPIO_SUS2           | I/O         | AVN_ALTA_RST_N         | O                 | 1K PD On Board (BIOS Default High)  | RESET Signal to Alta from Avoton (active low)
 * GPIO_SUS3   | CPU_RESET           | O           | CPU_RESET              | O                 | None                                |
 * GPIO_SUS4   | SUSPWRDNACK         | O           | VRHOT_ALERT_N          | I                 | 10K PU On board                     | IR3566-1 Regulator Over temp alert (active low)
 * GPIO_SUS5   | PMU_SUSCLK          | O           | SVRHOT_ALERT_N         | I                 | 10K PU On board                     | IR3566-2 Regulator Over temp alert (active low)
 * GPIO_SUS6   | PMU_SLP_DDRVTT_B    | O           | PMU_SLP_DDRVTT_B       | O                 | None                                |
 * GPIO_SUS7   | PMU_SLP_LAN_B       | O           | PMU_SLP_LAN_B          | O                 | 10K PU On board                     |
 * GPIO_SUS8   | PMU_WAKE_B          | I           | PMU_WAKE_B             | I                 | 10K PU On board                     |
 * GPIO_SUS9   | PMU_PWRBTN_B        | I           | PMU_PWRBTN_B           | I                 | None                                |
 * GPIO_SUS10  | SUS_STAT_B          | O           | ALTA_OT1_N             | I                 | 10K PU On board                     | Alta Thermal Die Over temp alert (active low)
 * GPIO_SUS11  | USB_OC0             | I           | USB_OC0                | I                 | 10K PU On board                     |
 * GPIO_SUS12  | SPI_CS1_B           | O           | SPI_CS1_B              | O                 | None                                |
 * GPIO_SUS13  | GBE_EE_DI           | I           | GBE_EE_DI              | I                 | None                                |
 * GPIO_SUS14  | GBE_EE_DO           | O           | GBE_EE_DO              | O                 | None                                |
 * GPIO_SUS15  | GBE_EE_SK           | O           | GBE_EE_SK              | O                 | None                                |
 * GPIO_SUS16  | GBE_EE_CS           | O           | GBE_EE_CS              | O                 | None                                |
 * GPIO_SUS17  | GBE_SDP0_0          | I/O         |                        |                   | None                                |
 * GPIO_SUS18  | GBE_SDP0_1          | I/O         |                        |                   | None                                |
 * GPIO_SUS19  | GBE_LED0            | O           | ENA_BP_AVN_N           | O                 | 10K PD On Board (BIOS Default Low)  | Bus Master for Thermal and VR components, 0 - Avoton, 1 -BP
 * GPIO_SUS20  | GBE_LED1            | O           | QSFP_SFP_I2C_RST_N     | O                 | 10K PD On Board (BIOS Default High) | QSFP+/SFP+ Sideband Signals Mux Reset
 * GPIO_SUS21  | GBE_LED2            | O           | SFP_I2C_RST_N          | O                 | 10K PD On Board (BIOS Default High) | SFP+ I2C Mux Reset
 * GPIO_SUS22  | GBE_LED3            | O           | QSFP_I2C_RST_N         | O                 | 10K PD On Board (BIOS Default High) | QSFP+ I2C Mux Reset
 * GPIO_SUS23  | GBE_WOL             | O           | GBE_WOL                | O                 | 10K PU On board                     |
 * GPIO_SUS24  | GBE_MDIO0_I2C_CLK   | I/O         | GBE_MDIO0_I2C_CLK      | I/O               | 10K PU On board                     |
 * GPIO_SUS25  | GBE_MDIO0_I2C_DATA  | I/O         | GBE_MDIO0_I2C_DATA     | I/O               | 10K PU On board                     |
 * GPIO_SUS26  | GBE_MDIO1_I2C_CLK   | I/O         |                        |                   | 10K PU On board                     |
 * GPIO_SUS27  | GBE_MDIO1_I2C_DATA  | I/O         |                        |                   | 10K PU On board                     |
 */


/* Core GPIO */
const struct soc_gpio soc_gpio_mode = {
//		.gpio0  = GPIO_MODE_GPIO, // Unused
//		.gpio1  = GPIO_MODE_GPIO, // Unused
//		.gpio2  = GPIO_MODE_GPIO, // Unused
//		.gpio3  = GPIO_MODE_GPIO, // Unused
//		.gpio4  = GPIO_MODE_GPIO, // Unused
//		.gpio5  = GPIO_MODE_GPIO, // Unused
//		.gpio15 = GPIO_MODE_GPIO, // Unused
//		.gpio16 = GPIO_MODE_GPIO, // Unused
//		.gpio17 = GPIO_MODE_GPIO, // Unused
//		.gpio18 = GPIO_MODE_GPIO, // Unused
//		.gpio27 = GPIO_MODE_GPIO, // Unused
//		.gpio28 = GPIO_MODE_GPIO, // Unused
//		.gpio29 = GPIO_MODE_GPIO, // Unused
};

const struct soc_gpio soc_gpio_direction = {
//		.gpio0  = GPIO_DIR_OUTPUT, // Unused
//		.gpio1  = GPIO_DIR_OUTPUT, // Unused
//		.gpio2  = GPIO_DIR_OUTPUT, // Unused
//		.gpio3  = GPIO_DIR_OUTPUT, // Unused
//		.gpio4  = GPIO_DIR_OUTPUT, // Unused
//		.gpio5  = GPIO_DIR_OUTPUT, // Unused
//		.gpio15 = GPIO_DIR_OUTPUT, // Unused
//		.gpio16 = GPIO_DIR_OUTPUT, // Unused
//		.gpio17 = GPIO_DIR_OUTPUT, // Unused
//		.gpio18 = GPIO_DIR_OUTPUT, // Unused
//		.gpio27 = GPIO_DIR_OUTPUT, // Unused
//		.gpio28 = GPIO_DIR_OUTPUT, // Unused
//		.gpio29 = GPIO_DIR_OUTPUT, // Unused
};

const struct soc_gpio soc_gpio_level = {
};

const struct soc_gpio soc_gpio_tpe = {
};

const struct soc_gpio soc_gpio_tne = {
};

const struct soc_gpio soc_gpio_ts = {
};

/* Keep the CFIO struct in register order, not gpio order. */
const struct soc_cfio soc_cfio_core[] = {
	{ 0x8000, 0x0000, 0x0004, 0x040c },  /* CFIO gpios_28 */
	{ 0x8000, 0x0000, 0x0002, 0x040c },  /* CFIO gpios_27 */
	{ 0x8500, 0x0000, 0x0002, 0x040c },  /* CFIO gpios_26 */
	{ 0x8480, 0x0000, 0x0002, 0x040c },  /* CFIO gpios_21 */
	{ 0x8480, 0x0000, 0x0002, 0x040c },  /* CFIO gpios_22 */
	{ 0x8480, 0x0000, 0x0002, 0x040c },  /* CFIO gpios_23 */
	{ 0x8000, 0x0000, 0x0004, 0x040c },  /* CFIO gpios_25 */
	{ 0x8480, 0x0000, 0x0002, 0x040c },  /* CFIO gpios_24 */
	{ 0x80c028, 0x30003, 0x0004, 0x040c },  /* CFIO gpios_19 */
	{ 0x80c028, 0x30003, 0x0004, 0x040c },  /* CFIO gpios_20 */
	{ 0xc028, 0x30003, 0x0004, 0x040c },  /* CFIO gpios_18 */
	{ 0x04a9, 0x30003, 0x0002, 0x040c },  /* CFIO gpios_17 */
	{ 0x80c028, 0x30003, 0x0004, 0x040c },  /* CFIO gpios_7 */
	{ 0xc028, 0x30003, 0x0004, 0x040c },  /* CFIO gpios_4 */
	{ 0xc028, 0x30003, 0x0004, 0x040c },  /* CFIO gpios_5 */
	{ 0xc528, 0x30003, 0x0002, 0x040c },  /* CFIO gpios_6 */
	{ 0xc028, 0x30003, 0x0004, 0x040c },  /* CFIO gpios_1 */
	{ 0xc028, 0x20002, 0x0004, 0x040c },  /* CFIO gpios_2 */
	{ 0xc028, 0x20002, 0x0004, 0x040c },  /* CFIO gpios_3 */
	{ 0xc528, 0x30003, 0x0002, 0x040c },  /* CFIO gpios_0 */
	{ 0x0000, 0x0000, 0x0000, 0x0000 },  /* CFIO Reserved */
	{ 0xc4a8, 0x30003, 0x0002, 0x040c },  /* CFIO gpios_10 */
	{ 0xc4a8, 0x30003, 0x0002, 0x040c },  /* CFIO gpios_13 */
	{ 0xc4a8, 0x30003, 0x0000, 0x040c },  /* CFIO gpios_14 */
	{ 0xc4a8, 0x30003, 0x0002, 0x040c },  /* CFIO gpios_11 */
	{ 0xc4a8, 0x30003, 0x0000, 0x040c },  /* CFIO gpios_8 */
	{ 0xc4a8, 0x30003, 0x0002, 0x040c },  /* CFIO gpios_9 */
	{ 0xc4a8, 0x30003, 0x0000, 0x040c },  /* CFIO gpios_12 */
	{ 0xc4a8, 0x30003, 0x0002, 0x040c },  /* CFIO gpios_29 */
	{ 0xc4a8, 0x30003, 0x0002, 0x040c },  /* CFIO gpios_30 */
	{ 0x04a9, 0x30003, 0x0002, 0x040c },  /* CFIO gpios_15 */
	{ 0xc028, 0x30003, 0x0004, 0x040c },  /* CFIO gpios_16 */
};

/* SUS GPIO */
const struct soc_gpio soc_gpio_sus_mode  = {
	.gpio0  = GPIO_MODE_GPIO,// GPIO_SUS0	FP_GPIO9			O
	.gpio1  = GPIO_MODE_GPIO,// GPIO_SUS1	ALTA_VDD_OFF_N		O
	.gpio2  = GPIO_MODE_GPIO,// GPIO_SUS2	AVN_ALTA_RST_N		O
	.gpio4  = GPIO_MODE_GPIO,// GPIO_SUS4	VRHOT_ALERT_N		I
	.gpio5  = GPIO_MODE_GPIO,// GPIO_SUS5	SVRHOT_ALERT_N		I
	.gpio10 = GPIO_MODE_GPIO,// GPIO_SUS10	ALTA_OT1_N			I
//	.gpio17 = GPIO_MODE_GPIO,// Unused
//	.gpio18 = GPIO_MODE_GPIO,// Unused
	.gpio19 = GPIO_MODE_GPIO,// GPIO_SUS19	ENA_BP_AVN_N		O
	.gpio20 = GPIO_MODE_GPIO,// GPIO_SUS20	QSFP_SFP_I2C_RST_N	O
	.gpio21 = GPIO_MODE_GPIO,// GPIO_SUS21	SFP_I2C_RST_N		O
	.gpio22 = GPIO_MODE_GPIO,// GPIO_SUS22	QSFP_I2C_RST_N		O
//	.gpio26 = GPIO_MODE_GPIO,// Unused
//	.gpio27 = GPIO_MODE_GPIO,// Unused
};

const struct soc_gpio soc_gpio_sus_direction = {
	.gpio0  = GPIO_DIR_OUTPUT,// GPIO_SUS0	FP_GPIO9			O
	.gpio1  = GPIO_DIR_OUTPUT,// GPIO_SUS1	ALTA_VDD_OFF_N		O
	.gpio2  = GPIO_DIR_OUTPUT,// GPIO_SUS2	AVN_ALTA_RST_N		O
	.gpio4  = GPIO_DIR_INPUT, // GPIO_SUS4	VRHOT_ALERT_N		I
	.gpio5  = GPIO_DIR_INPUT, // GPIO_SUS5	SVRHOT_ALERT_N		I
	.gpio10 = GPIO_DIR_INPUT, // GPIO_SUS10	ALTA_OT1_N			I
//	.gpio17 = GPIO_DIR_OUTPUT,// Unused
//	.gpio18 = GPIO_DIR_OUTPUT,// Unused
	.gpio19 = GPIO_DIR_OUTPUT,// GPIO_SUS19	ENA_BP_AVN_N		O
	.gpio20 = GPIO_DIR_OUTPUT,// GPIO_SUS20	QSFP_SFP_I2C_RST_N	O
	.gpio21 = GPIO_DIR_OUTPUT,// GPIO_SUS21	SFP_I2C_RST_N		O
	.gpio22 = GPIO_DIR_OUTPUT,// GPIO_SUS22	QSFP_I2C_RST_N		O
//	.gpio26 = GPIO_DIR_OUTPUT,// Unused
//	.gpio27 = GPIO_DIR_OUTPUT,// Unused
};

const struct soc_gpio soc_gpio_sus_level = {
	.gpio0  = GPIO_LEVEL_HIGH,// GPIO_SUS0	(BIOS Default High)
	.gpio1  = GPIO_LEVEL_HIGH,// GPIO_SUS1	(BIOS Default High)
	.gpio2  = GPIO_LEVEL_HIGH,// GPIO_SUS2	(BIOS Default High)
	.gpio19 = GPIO_LEVEL_LOW, // GPIO_SUS19	(BIOS Default Low)
	.gpio20 = GPIO_LEVEL_HIGH,// GPIO_SUS20	(BIOS Default High)
	.gpio21 = GPIO_LEVEL_HIGH,// GPIO_SUS21	(BIOS Default High)
	.gpio22 = GPIO_LEVEL_HIGH,// GPIO_SUS22	(BIOS Default High)

};

const struct soc_gpio soc_gpio_sus_tpe = {
};

const struct soc_gpio soc_gpio_sus_tne = {
};

const struct soc_gpio soc_gpio_sus_ts = {
};

const struct soc_gpio soc_gpio_sus_we = {
};


/* Keep the CFIO struct in register order, not gpio order. */
const struct soc_cfio soc_cfio_sus[] = {
	{ 0xc028, 0x30003, 0x0004, 0x040c },  /* CFIO SUS gpios_21 */
	{ 0xc028, 0x30003, 0x0004, 0x040c },  /* CFIO SUS gpios_20 */
	{ 0xc028, 0x30003, 0x0004, 0x040c },  /* CFIO SUS gpios_19 */
	{ 0xc028, 0x30003, 0x0004, 0x040c },  /* CFIO SUS gpios_22 */
	{ 0xc4a8, 0x30003, 0x0002, 0x040c },  /* CFIO SUS gpios_17 */
	{ 0xc4a8, 0x30003, 0x0002, 0x040c },  /* CFIO SUS gpios_18 */
	{ 0x0000, 0x0000, 0x0000, 0x0000 },  /* CFIO Reserved */
	{ 0x0000, 0x0000, 0x0000, 0x0000 },  /* CFIO Reserved */
	{ 0x0000, 0x0000, 0x0000, 0x0000 },  /* CFIO Reserved */
	{ 0xc4a8, 0x30003, 0x0002, 0x040c },  /* CFIO SUS gpios_14 */
	{ 0xc028, 0x30003, 0x0004, 0x040c },  /* CFIO SUS gpios_13 */
	{ 0xc028, 0x30003, 0x0004, 0x040c },  /* CFIO SUS gpios_15 */
	{ 0xc028, 0x30003, 0x0004, 0x040c },  /* CFIO SUS gpios_16 */
	{ 0xc4a8, 0x30003, 0x0002, 0x040c },  /* CFIO SUS gpios_25 */
	{ 0xc4a8, 0x30003, 0x0002, 0x040c },  /* CFIO SUS gpios_24 */
	{ 0xc4a8, 0x30003, 0x0002, 0x040c },  /* CFIO SUS gpios_26 */
	{ 0xc4a8, 0x30003, 0x0002, 0x040c },  /* CFIO SUS gpios_27 */
	{ 0xc4a8, 0x30003, 0x0002, 0x040c },  /* CFIO SUS gpios_23 */
	{ 0xc4a8, 0x30003, 0x0003, 0x040c },  /* CFIO SUS gpios_2 */
	{ 0xc4a8, 0x30003, 0x0003, 0x040c },  /* CFIO SUS gpios_1 */
	{ 0x8050, 0x0000, 0x0004, 0x040c },  /* CFIO SUS gpios_7 */
	{ 0x0000, 0x0000, 0x0000, 0x0000 },  /* CFIO Reserved */
	{ 0x0000, 0x0000, 0x0000, 0x0000 },  /* CFIO Reserved */
	{ 0xc028, 0x30003, 0x0004, 0x040c },  /* CFIO SUS gpios_3 */
	{ 0xc4a8, 0x30003, 0x0003, 0x040c },  /* CFIO SUS gpios_0 */
	{ 0x0000, 0x0000, 0x0000, 0x0000 },  /* CFIO Reserved */
	{ 0x0000, 0x0000, 0x0000, 0x0000 },  /* CFIO Reserved */
	{ 0x8000, 0x0000, 0x0004, 0x040c },  /* CFIO SUS gpios_12 */
	{ 0x8050, 0x0000, 0x0004, 0x040c },  /* CFIO SUS gpios_6 */
	{ 0xc028, 0x30003, 0x0004, 0x040c },  /* CFIO SUS gpios_10 */
	{ 0xc4a8, 0x30003, 0x0002, 0x040c },  /* CFIO SUS gpios_9 */
	{ 0xc4a8, 0x30003, 0x0002, 0x040c },  /* CFIO SUS gpios_8 */
	{ 0x0000, 0x0000, 0x0000, 0x0000 },  /* CFIO Reserved */
	{ 0x0000, 0x0000, 0x0000, 0x0000 },  /* CFIO Reserved */
	{ 0x8050, 0x0000, 0x0004, 0x040c },  /* CFIO SUS gpios_4 */
	{ 0xc4a8, 0x30003, 0x0002, 0x040c },  /* CFIO SUS gpios_11 */
	{ 0x0000, 0x0000, 0x0000, 0x0000 },  /* CFIO Reserved */
	{ 0x0000, 0x0000, 0x0000, 0x0000 },  /* CFIO Reserved */
	{ 0x0000, 0x0000, 0x0000, 0x0000 },  /* CFIO Reserved */
	{ 0x0000, 0x0000, 0x0000, 0x0000 },  /* CFIO Reserved */
	{ 0xc028, 0x30003, 0x0004, 0x040c },  /* CFIO SUS gpios_5 */
};

const struct soc_gpio_map gpio_map = {
	.core = {
		.mode      = &soc_gpio_mode,
		.direction = &soc_gpio_direction,
		.level     = &soc_gpio_level,
		.tpe       = &soc_gpio_tpe,
		.tne       = &soc_gpio_tne,
		.ts        = &soc_gpio_ts,
		.cfio_init = &soc_cfio_core[0],
		.cfio_entrynum = sizeof(soc_cfio_core) / sizeof(struct soc_cfio),
	},
	.sus = {
		.mode      = &soc_gpio_sus_mode,
		.direction = &soc_gpio_sus_direction,
		.level     = &soc_gpio_sus_level,
		.tpe       = &soc_gpio_sus_tpe,
		.tne       = &soc_gpio_sus_tne,
		.ts        = &soc_gpio_sus_ts,
		.we        = &soc_gpio_sus_we,
		.cfio_init = &soc_cfio_sus[0],
		.cfio_entrynum = sizeof(soc_cfio_sus) / sizeof(struct soc_cfio),
	},
};

#endif
