/*
 * This file is part of the coreboot project.
 *
 * Copyright (C) 2000,2007 Ronald G. Minnich <rminnich@gmail.com>
 * Copyright (C) 2007-2008 coresystems GmbH
 * Copyright (C) 2013-2014 Sage Electronic Engineering, LLC.
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

#include <cpu/x86/stack.h>
#include <cpu/x86/mtrr.h>
#include <cpu/x86/cache.h>
#include <cpu/x86/post_code.h>
#include <cbmem.h>
#include <timestamp.h>

#include "microcode_size.h"

/* check the CONFIG_ defines */
#ifndef CONFIG_COLLECT_TIMESTAMPS
# error "CONFIG_COLLECT_TIMESTAMPS must be defined."
#endif

#ifndef CONFIG_SAVE_EARLY_TIMESTAMPS_TO_CMOS
# error "CONFIG_SAVE_EARLY_TIMESTAMPS_TO_CMOS must be defined."
#endif

#ifndef CONFIG_FSP_LOC
# error "CONFIG_FSP_LOC must be set."
#endif

#ifndef CONFIG_POST_IO
# error "CONFIG_POST_IO must be set."
#endif

#if CONFIG_POST_IO
# ifndef CONFIG_POST_IO_PORT
#  error "CONFIG_POST_IO_PORT must be set."
# endif
#endif

#ifndef CONFIG_CPU_MICROCODE_CBFS_LOC
# error "CONFIG_CPU_MICROCODE_CBFS_LOC must be set."
#endif

#define LHLT_DELAY	0x50000		/* I/O delay between post codes on failure */

	cmp   $0, %eax
	jne   bisthalt

cache_as_ram:
	post_code(0x20)

	/* save the pre-CAR TSC value to CMOS*/
#if CONFIG_COLLECT_TIMESTAMPS && CONFIG_SAVE_EARLY_TIMESTAMPS_TO_CMOS
	rdtsc							/* tsc goes int eax, edx */
	mov   %eax, %ebx				/* our value to write goes in ebx */
	mov   $CMOS_PRE_CAR_WRITE_ADDR, %eax	/* Set up the start address */
start_tsc_1:
	mov   $4, %ecx					/* initialize the loop counter to 4 loops through */
write_tsc_ebx_1:
	outb  %al, $TSC_CMOS_INDEX_VAL	/* write out the offset */
	xchg  %eax, %ebx				/* save address and get data to write in eax */
	outb  %al, $TSC_CMOS_INDEX_VAL + 1	/* write out the data */
	shr   $8, %eax					/* get eax ready to write the next byte */
	xchg  %eax, %ebx				/* save off data and fetch address for next write */
	inc   %eax						/* increment address for next byte to save */
	loop  write_tsc_ebx_1			/* continue writing the rest of the dword */
	mov   %edx, %ebx				/* get ready to save the upper tsc values */
	cmp   $CMOS_PRE_CAR_WRITE_ADDR + SIZE_OF_TSC, %eax	/* see if edx has already been written */
	jne   start_tsc_1				/* if it hasn't, loop back to write it */
#endif /* CONFIG_COLLECT_TIMESTAMPS && CONFIG_SAVE_EARLY_TIMESTAMPS_TO_CMOS */

	post_code(0x21)

	/*
	 * Find the FSP binary in cbfs.
	 * Make a fake stack that has the return value back to this code.
	 */
	lea   fake_fsp_stack, %esp
	jmp   find_fsp
find_fsp_ret:
	/* Save the FSP location */
	mov   %eax,   %ebp
	cmp   $CONFIG_FSP_LOC, %eax
	jb    halt1

	post_code(0x22)

	/* Calculate entry into FSP */
	mov   0x30(%ebp), %eax	/* Load TempRamInitEntry */
	add   0x1c(%ebp), %eax	/* add in the offset for the FSP base address */

	/*
	 * Pass early init variables on a fake stack (no memory yet)
	 * as well as the return location
	 */
	lea   CAR_init_stack, %esp

	/* call FSP binary to setup temporary stack */
	jmp   *%eax

CAR_init_done:
	addl  $4, %esp
	cmp   $0, %eax
	jne   halt2

	/* Save FSP_INFO_HEADER location in ebx */
	mov    %ebp, %ebx

	/*
	 * set up bootloader stack
	 * ecx:  stack base
	 * edx:  stack top
	 */
	lea   -4(%edx), %esp
	movl  %esp, %ebp
	pushl %ebx

	/* save the post-CAR TSC value */
#if CONFIG_COLLECT_TIMESTAMPS && CONFIG_SAVE_EARLY_TIMESTAMPS_TO_CMOS
	rdtsc							/* tsc goes int eax, edx */
	mov   %eax, %ebx				/* our value to write goes in ebx */
	mov   $CMOS_POST_CAR_WRITE_ADDR, %eax	/* Set up the start address */
start_tsc_2:
	mov   $4, %ecx					/* initialize the loop counter to 4 loops through */
write_tsc_ebx_2:
	outb  %al, $TSC_CMOS_INDEX_VAL	/* write out the offset */
	xchg  %eax, %ebx				/* save address and get data to write in eax */
	outb  %al, $TSC_CMOS_INDEX_VAL + 1	/* write out the data */
	shr   $8, %eax					/* get eax ready to write the next byte */
	xchg  %eax, %ebx				/* save off data and fetch address for next write */
	inc   %eax						/* increment address for next byte to save */
	loop  write_tsc_ebx_2			/* continue writing the rest of the dword */
	mov   %edx, %ebx				/* get ready to save the upper tsc values */
	cmp   $CMOS_POST_CAR_WRITE_ADDR + SIZE_OF_TSC, %eax	/* see if edx has already been written */
	jne   start_tsc_2				/* if it hasn't, loop back to write it */
#endif /* CONFIG_COLLECT_TIMESTAMPS && CONFIG_SAVE_EARLY_TIMESTAMPS_TO_CMOS */

before_romstage:
	post_code(0x23)

	/* Call romstage.c main function. */
	call  main /* does not return */
	movb  $0xB8, %ah
	jmp   .Lhlt

bisthalt:
	movb  $0xB9, %ah
	jmp   .Lhlt

halt1:
	/*
	 * Failures for postcode 0xBA - failed in find_fsp()
	 *
	 * Values are:
	 * 0x01 - FV signature, "_FVH" not present
	 * 0x02 - FFS GUID not present
	 * 0x03 - FSP INFO Header not found
	 * 0x04 - ImageBase does not equal CONFIG_FSP_LOC - Is the FSP rebased to
	 *        a different location, or does it need to be?
	 * 0x05 - FSP INFO Header signature "FSPH" not found
	 * 0x06 - FSP Image ID is not the expected ID.
	 */
	movb  $0xBA, %ah
	jmp   .Lhlt

halt2:
	/*
	 * Failures for postcode 0xBB - failed in the FSP:
	 *
	 * 0x00 - FSP_SUCCESS: Temp RAM was initialized successfully.
	 * 0x02 - FSP_INVALID_PARAMETER: Input parameters are invalid.
	 * 0x0E - FSP_NOT_FOUND: No valid microcode was found in the microcode region.
	 * 0x03 - FSP_UNSUPPORTED: The FSP calling conditions were not met.
	 * 0x07 - FSP_DEVICE_ERROR: Temp RAM initialization failed
	 * 0x14 - FSP_ALREADY_STARTED: Temp RAM initialization has been invoked
	 */
	movb  $0xBB, %ah

.Lhlt:
	xchg  %al, %ah
#if CONFIG_POST_IO
	outb  %al, $CONFIG_POST_IO_PORT
#else
	post_code(POST_DEAD_CODE)
#endif
	movl  $LHLT_DELAY, %ecx
.Lhlt_Delay:
	outb  %al, $0xED
	loop  .Lhlt_Delay
	jmp   .Lhlt

/*
 * esp is set to this location so that the call into and return from the FSP
 * in find_fsp will work.
 */
	.align 4
fake_fsp_stack:
	.long  find_fsp_ret

CAR_init_params:
	.long  CONFIG_CPU_MICROCODE_CBFS_LOC
	.long  MICROCODE_LENGTH
	.long  0xFFFFFFFF - CACHE_ROM_SIZE + 1	/* Firmware Location */
	.long  CACHE_ROM_SIZE	/* Total Firmware Length */

CAR_init_stack:
	.long  CAR_init_done
	.long  CAR_init_params

