/* For starting coreboot in protected mode */

#include <arch/rom_segs.h>
#include <cpu/x86/post_code.h>

	.code32
 
	/* This is the GDT for the ROM stage part of coreboot. It
	 * is different from the RAM stage GDT which is defined in
	 * c_start.S
	 */

	.align	4
.globl gdtptr
gdt:
gdtptr:
	.word	gdt_end - gdt -1 /* compute the table limit */
	.long	gdt		 /* we know the offset */
	.word	0

	/* selgdt 0x08, flat code segment */
	.word	0xffff, 0x0000
	.byte	0x00, 0x9b, 0xcf, 0x00 /* G=1 and 0x0f, So we get 4Gbytes for limit */

	/* selgdt 0x10,flat data segment */
	.word	0xffff, 0x0000
	.byte	0x00, 0x93, 0xcf, 0x00

gdt_end:


/*
 *	When we come here we are in protected mode. We expand
 *	the stack and copies the data segment from ROM to the
 *	memory.
 *
 *	After that, we call the chipset bootstrap routine that
 *	does what is left of the chipset initialization.
 *
 *	NOTE aligned to 4 so that we are sure that the prefetch
 *	cache will be reloaded.
 */
	.align	4
.globl protected_start
protected_start:

	movl	%eax, %ebp

	lgdt	%cs:gdtptr
	ljmp	$ROM_CODE_SEG, $__protected_start

__protected_start:
	/* Save the BIST value */
	movl	%ebp, %eax
	movl	%eax, %ebp

	post_code(POST_ENTER_PROTECTED_MODE)

	movw	$ROM_DATA_SEG, %ax
	movw	%ax, %ds
	movw	%ax, %es
	movw	%ax, %ss
	movw	%ax, %fs
	movw	%ax, %gs

/* VeloCloud TCO watchdog custom code. */

 movl $0x8000f840, %eax
 movw $0xcf8, %dx
 outl %eax, (%dx)
 movw $0x403, %ax  // map the io register address
 movw $0xcfc, %dx  // 0xcfc (data port).
 outw %ax, (%dx)   // write out 0x400
 inw  (%dx), %ax   // flush io register write

 xor  %ax, %ax     // clear ax register
 xor  %bx, %bx

 movw $0x472, %dx  // upper 16 bits of TCO_TMR
 movw $0x64, %ax   // raise the timeout value to 2min
 outw %ax, (%dx)   // write out the new timer value

 movw $0x466, %dx  // we already know we want bar 0x400 + TCO_STS (byte 3) (0x66)
 inw (%dx), %ax    // read register value
 movw %ax, %bx     // move the contents of reg ax to reg bx

 and $0x2, %al    // we want to know if the status bit is set
  
 cmp $0x2, %al	   // second timeout status bit 1?

 jne fsp_continue 
 
 movw %bx, %ax     // restore ax
 outw %ax, (%dx)
 inw (%dx), %ax    // flush result

 xor  %dx, %dx     // clear dx register
 movb $0x70, %dl   // select CMOS offset port
 movb $0x11, %al   // select offset 0x10 in cmos
 outb %al, (%dx)   // write out the offset

 xor  %dx, %dx     // clear the dx register
 movb $0x71, %dl   // select CMOS data port
 inb  (%dx), %al   // read CMOS offset into ax register
 
 add $0x1, %al     // add + 1 (to indicate watchdog reset) + n

 outb %al, (%dx)   // to the CMOS data port

fsp_continue: 

	/* Restore the BIST value to %eax */
	movl	%ebp, %eax

