/******************************************************************************
 * Copyright (c) 2004, 2008 IBM Corporation
 * Copyright (c) 2009 Pattrick Hueper <phueper@hueper.net>
 * All rights reserved.
 * This program and the accompanying materials
 * are made available under the terms of the BSD License
 * which accompanies this distribution, and is available at
 * http://www.opensource.org/licenses/bsd-license.php
 *
 * Contributors:
 *     IBM Corporation - initial implementation
 *****************************************************************************/

#include <string.h>
#include <types.h>
#if CONFIG_FRAMEBUFFER_SET_VESA_MODE
#include <boot/coreboot_tables.h>
#endif

#include <arch/byteorder.h>

#include "debug.h"

#include <x86emu/x86emu.h>
#include <x86emu/regs.h>
#include "../x86emu/prim_ops.h"

#include "biosemu.h"
#include "io.h"
#include "mem.h"
#include "interrupt.h"
#include "device.h"

#include <cbfs.h>

#include <delay.h>
#include "../../src/lib/jpeg.h"

#include <vbe.h>

// pointer to VBEInfoBuffer, set by vbe_prepare
u8 *vbe_info_buffer = 0;

// virtual BIOS Memory
u8 *biosmem;
u32 biosmem_size;

#if CONFIG_FRAMEBUFFER_SET_VESA_MODE
static inline u8
vbe_prepare(void)
{
	vbe_info_buffer = biosmem + (VBE_SEGMENT << 4);	// segment:offset off VBE Data Area
	//clear buffer
	memset(vbe_info_buffer, 0, 512);
	//set VbeSignature to "VBE2" to indicate VBE 2.0+ request
	vbe_info_buffer[0] = 'V';
	vbe_info_buffer[0] = 'B';
	vbe_info_buffer[0] = 'E';
	vbe_info_buffer[0] = '2';
	// ES:DI store pointer to buffer in virtual mem see vbe_info_buffer above...
	M.x86.R_EDI = 0x0;
	M.x86.R_ES = VBE_SEGMENT;

	return 0;		// successfull init
}

// VBE Function 00h
static u8
vbe_info(vbe_info_t * info)
{
	vbe_prepare();
	// call VBE function 00h (Info Function)
	M.x86.R_EAX = 0x4f00;

	// enable trace
	CHECK_DBG(DEBUG_TRACE_X86EMU) {
		X86EMU_trace_on();
	}
	// run VESA Interrupt
	runInt10();

	if (M.x86.R_AL != 0x4f) {
		DEBUG_PRINTF_VBE("%s: VBE Info Function NOT supported! AL=%x\n",
				 __func__, M.x86.R_AL);
		return -1;
	}

	if (M.x86.R_AH != 0x0) {
		DEBUG_PRINTF_VBE
		    ("%s: VBE Info Function Return Code NOT OK! AH=%x\n",
		     __func__, M.x86.R_AH);
		return M.x86.R_AH;
	}
	//printf("VBE Info Dump:");
	//dump(vbe_info_buffer, 64);

	//offset 0: signature
	info->signature[0] = vbe_info_buffer[0];
	info->signature[1] = vbe_info_buffer[1];
	info->signature[2] = vbe_info_buffer[2];
	info->signature[3] = vbe_info_buffer[3];

	// offset 4: 16bit le containing VbeVersion
	info->version = in16le(vbe_info_buffer + 4);

	// offset 6: 32bit le containing segment:offset of OEM String in virtual Mem.
	info->oem_string_ptr =
	    biosmem + ((in16le(vbe_info_buffer + 8) << 4) +
		       in16le(vbe_info_buffer + 6));

	// offset 10: 32bit le capabilities
	info->capabilities = in32le(vbe_info_buffer + 10);

	// offset 14: 32 bit le containing segment:offset of supported video mode table
	u16 *video_mode_ptr;
	video_mode_ptr =
	    (u16 *) (biosmem +
			  ((in16le(vbe_info_buffer + 16) << 4) +
			   in16le(vbe_info_buffer + 14)));
	u32 i = 0;
	do {
		info->video_mode_list[i] = in16le(video_mode_ptr + i);
		i++;
	}
	while ((i <
		(sizeof(info->video_mode_list) /
		 sizeof(info->video_mode_list[0])))
	       && (info->video_mode_list[i - 1] != 0xFFFF));

	//offset 18: 16bit le total memory in 64KB blocks
	info->total_memory = in16le(vbe_info_buffer + 18);

	return 0;
}

static int mode_info_valid;

int vbe_mode_info_valid(void)
{
	return mode_info_valid;
}

// VBE Function 01h
static u8
vbe_get_mode_info(vbe_mode_info_t * mode_info)
{
	vbe_prepare();
	// call VBE function 01h (Return VBE Mode Info Function)
	M.x86.R_EAX = 0x4f01;
	M.x86.R_CX = mode_info->video_mode;

	// enable trace
	CHECK_DBG(DEBUG_TRACE_X86EMU) {
		X86EMU_trace_on();
	}
	// run VESA Interrupt
	runInt10();

	if (M.x86.R_AL != 0x4f) {
		DEBUG_PRINTF_VBE
		    ("%s: VBE Return Mode Info Function NOT supported! AL=%x\n",
		     __func__, M.x86.R_AL);
		return -1;
	}

	if (M.x86.R_AH != 0x0) {
		DEBUG_PRINTF_VBE
		    ("%s: VBE Return Mode Info (mode: %04x) Function Return Code NOT OK! AH=%02x\n",
		     __func__, mode_info->video_mode, M.x86.R_AH);
		return M.x86.R_AH;
	}

	//pointer to mode_info_block is in ES:DI
	memcpy(mode_info->mode_info_block,
	       biosmem + ((M.x86.R_ES << 4) + M.x86.R_DI),
	       sizeof(mode_info->mode_info_block));
	mode_info_valid = 1;

	//printf("Mode Info Dump:");
	//dump(mode_info_block, 64);

	return 0;
}

// VBE Function 02h
static u8
vbe_set_mode(vbe_mode_info_t * mode_info)
{
	vbe_prepare();
	// call VBE function 02h (Set VBE Mode Function)
	M.x86.R_EAX = 0x4f02;
	M.x86.R_BX = mode_info->video_mode;
	M.x86.R_BX |= 0x4000;	// set bit 14 to request linear framebuffer mode
	M.x86.R_BX &= 0x7FFF;	// clear bit 15 to request clearing of framebuffer

	DEBUG_PRINTF_VBE("%s: setting mode: 0x%04x\n", __func__,
			 M.x86.R_BX);

	// enable trace
	CHECK_DBG(DEBUG_TRACE_X86EMU) {
		X86EMU_trace_on();
	}
	// run VESA Interrupt
	runInt10();

	if (M.x86.R_AL != 0x4f) {
		DEBUG_PRINTF_VBE
		    ("%s: VBE Set Mode Function NOT supported! AL=%x\n",
		     __func__, M.x86.R_AL);
		return -1;
	}

	if (M.x86.R_AH != 0x0) {
		DEBUG_PRINTF_VBE
		    ("%s: mode: %x VBE Set Mode Function Return Code NOT OK! AH=%x\n",
		     __func__, mode_info->video_mode, M.x86.R_AH);
		return M.x86.R_AH;
	}
	return 0;
}

vbe_mode_info_t mode_info;

void vbe_set_graphics(void)
{
	u8 rval;

	vbe_info_t info;
	rval = vbe_info(&info);
	if (rval != 0)
		return;

	DEBUG_PRINTF_VBE("VbeSignature: %s\n", info.signature);
	DEBUG_PRINTF_VBE("VbeVersion: 0x%04x\n", info.version);
	DEBUG_PRINTF_VBE("OemString: %s\n", info.oem_string_ptr);
	DEBUG_PRINTF_VBE("Capabilities:\n");
	DEBUG_PRINTF_VBE("\tDAC: %s\n",
			 (info.capabilities & 0x1) ==
			 0 ? "fixed 6bit" : "switchable 6/8bit");
	DEBUG_PRINTF_VBE("\tVGA: %s\n",
			 (info.capabilities & 0x2) ==
			 0 ? "compatible" : "not compatible");
	DEBUG_PRINTF_VBE("\tRAMDAC: %s\n",
			 (info.capabilities & 0x4) ==
			 0 ? "normal" : "use blank bit in Function 09h");

	mode_info.video_mode = (1 << 14) | CONFIG_FRAMEBUFFER_VESA_MODE;
	vbe_get_mode_info(&mode_info);
	vbe_set_mode(&mode_info);

#if CONFIG_BOOTSPLASH
	unsigned char *framebuffer =
		(unsigned char *) le32_to_cpu(mode_info.vesa.phys_base_ptr);
	DEBUG_PRINTF_VBE("FRAMEBUFFER: 0x%p\n", framebuffer);

	struct jpeg_decdata *decdata;
	decdata = malloc(sizeof(*decdata));

	/* Switching Intel IGD to 1MB video memory will break this. Who
	 * cares. */
	// int imagesize = 1024*768*2;

	unsigned char *jpeg = cbfs_get_file_content(CBFS_DEFAULT_MEDIA,
						    "bootsplash.jpg",
						    CBFS_TYPE_BOOTSPLASH,
						    NULL);
	if (!jpeg) {
		DEBUG_PRINTF_VBE("Could not find bootsplash.jpg\n");
		return;
	}
	DEBUG_PRINTF_VBE("Splash at %p ...\n", jpeg);
	dump(jpeg, 64);

	int ret = 0;
	DEBUG_PRINTF_VBE("Decompressing boot splash screen...\n");
	ret = jpeg_decode(jpeg, framebuffer, 1024, 768, 16, decdata);
	DEBUG_PRINTF_VBE("returns %x\n", ret);
#endif
}

void fill_lb_framebuffer(struct lb_framebuffer *framebuffer)
{
	framebuffer->physical_address = le32_to_cpu(mode_info.vesa.phys_base_ptr);

	framebuffer->x_resolution = le16_to_cpu(mode_info.vesa.x_resolution);
	framebuffer->y_resolution = le16_to_cpu(mode_info.vesa.y_resolution);
	framebuffer->bytes_per_line = le16_to_cpu(mode_info.vesa.bytes_per_scanline);
	framebuffer->bits_per_pixel = mode_info.vesa.bits_per_pixel;

	framebuffer->red_mask_pos = mode_info.vesa.red_mask_pos;
	framebuffer->red_mask_size = mode_info.vesa.red_mask_size;

	framebuffer->green_mask_pos = mode_info.vesa.green_mask_pos;
	framebuffer->green_mask_size = mode_info.vesa.green_mask_size;

	framebuffer->blue_mask_pos = mode_info.vesa.blue_mask_pos;
	framebuffer->blue_mask_size = mode_info.vesa.blue_mask_size;

	framebuffer->reserved_mask_pos = mode_info.vesa.reserved_mask_pos;
	framebuffer->reserved_mask_size = mode_info.vesa.reserved_mask_size;
}

void vbe_textmode_console(void)
{
	/* Wait, just a little bit more, pleeeease ;-) */
	delay(2);

	M.x86.R_EAX = 0x0003;
	runInt10();
}

#endif
