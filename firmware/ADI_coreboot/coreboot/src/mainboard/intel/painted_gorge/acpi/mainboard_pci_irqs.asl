/*
 * This file is part of the coreboot project.
 *
 * Copyright (C) 2012 The Chromium OS Authors. All rights reserved.
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

/* This is board specific information: IRQ routing */

// PCI Interrupt Routing
Method(_PRT)
{
	If (PICM) {
		Return (Package() {
			// PCIe Root Ports		0:1.x
			Package() { 0x0001ffff, 0, 0, 16 },
			Package() { 0x0001ffff, 1, 0, 17 },
			Package() { 0x0001ffff, 2, 0, 18 },
			Package() { 0x0001ffff, 3, 0, 19 },

			// PCIe Root Ports		0:2.x
			Package() { 0x0002ffff, 0, 0, 16 },
			Package() { 0x0002ffff, 1, 0, 17 },
			Package() { 0x0002ffff, 2, 0, 18 },
			Package() { 0x0002ffff, 3, 0, 19 },

			// PCIe Root Ports		0:3.x
			Package() { 0x0003ffff, 0, 0, 20 },
			Package() { 0x0003ffff, 1, 0, 21 },
			Package() { 0x0003ffff, 2, 0, 22 },
			Package() { 0x0003ffff, 3, 0, 23 },

			// PCIe Root Ports		0:4.x
			Package() { 0x0004ffff, 0, 0, 20 },
			Package() { 0x0004ffff, 1, 0, 21 },
			Package() { 0x0004ffff, 2, 0, 22 },
			Package() { 0x0004ffff, 3, 0, 23 },

			// ETH0		0:14.x
			Package() { 0x0014ffff, 0, 0, 20 },
			Package() { 0x0014ffff, 1, 0, 21 },
			Package() { 0x0014ffff, 2, 0, 22 },
			Package() { 0x0014ffff, 3, 0, 23 },

			// EHCI 	0:16.0
			Package() { 0x0016ffff, 0, 0, 23 },

			// SATA2	0:17.0
			Package() { 0x0017ffff, 0, 0, 19 },

			// SATA3	0:18.0
			Package() { 0x0018ffff, 0, 0, 19 },

			// PCU(LPC)	0:1f.0
			Package() { 0x001fffff, 0, 0, 18 },
		})
	} Else {
		Return (Package() {
			// PCIe Root Ports		0:1.x
			Package() { 0x0001ffff, 0, \_SB.PCI0.LPCB.LNKA, 0 },
			Package() { 0x0001ffff, 1, \_SB.PCI0.LPCB.LNKB, 0 },
			Package() { 0x0001ffff, 2, \_SB.PCI0.LPCB.LNKC, 0 },
			Package() { 0x0001ffff, 3, \_SB.PCI0.LPCB.LNKD, 0 },

			// PCIe Root Ports		0:2.x
			Package() { 0x0002ffff, 0, \_SB.PCI0.LPCB.LNKA, 0 },
			Package() { 0x0002ffff, 1, \_SB.PCI0.LPCB.LNKB, 0 },
			Package() { 0x0002ffff, 2, \_SB.PCI0.LPCB.LNKC, 0 },
			Package() { 0x0002ffff, 3, \_SB.PCI0.LPCB.LNKD, 0 },

			// PCIe Root Ports		0:3.x
			Package() { 0x0003ffff, 0, \_SB.PCI0.LPCB.LNKE, 0 },
			Package() { 0x0003ffff, 1, \_SB.PCI0.LPCB.LNKF, 0 },
			Package() { 0x0003ffff, 2, \_SB.PCI0.LPCB.LNKG, 0 },
			Package() { 0x0003ffff, 3, \_SB.PCI0.LPCB.LNKH, 0 },

			// PCIe Root Ports		0:4.x
			Package() { 0x0004ffff, 0, \_SB.PCI0.LPCB.LNKE, 0 },
			Package() { 0x0004ffff, 1, \_SB.PCI0.LPCB.LNKF, 0 },
			Package() { 0x0004ffff, 2, \_SB.PCI0.LPCB.LNKG, 0 },
			Package() { 0x0004ffff, 3, \_SB.PCI0.LPCB.LNKH, 0 },

			// ETH0		0:14.x
			Package() { 0x0014ffff, 0, \_SB.PCI0.LPCB.LNKE, 0 },
			Package() { 0x0014ffff, 1, \_SB.PCI0.LPCB.LNKF, 0 },
			Package() { 0x0014ffff, 2, \_SB.PCI0.LPCB.LNKG, 0 },
			Package() { 0x0014ffff, 3, \_SB.PCI0.LPCB.LNKH, 0 },

			// EHCI		0:16.0
			Package() { 0x0016ffff, 0, \_SB.PCI0.LPCB.LNKH, 0 },

			// SATA2	0:17.0
			Package() { 0x0017ffff, 0, \_SB.PCI0.LPCB.LNKD, 0 },

			// SATA3	0:18.0
			Package() { 0x0018ffff, 0, \_SB.PCI0.LPCB.LNKD, 0 },

			// PCU(LPC)	0:1f.0
			Package() { 0x001fffff, 0, \_SB.PCI0.LPCB.LNKC, 0 },
		})
	}
}
