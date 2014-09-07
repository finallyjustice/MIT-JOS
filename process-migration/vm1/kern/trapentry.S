/* See COPYRIGHT for copyright information. */

#include <inc/mmu.h>
#include <inc/memlayout.h>
#include <inc/trap.h>

#include <kern/picirq.h>


###################################################################
# exceptions/interrupts
###################################################################

/* The TRAPHANDLER macro defines a globally-visible function for handling
 * a trap.  It pushes a trap number onto the stack, then jumps to _alltraps.
 * Use TRAPHANDLER for traps where the CPU automatically pushes an error code.
 */ 
#define TRAPHANDLER(name, num)						\
.text;												\
	.globl name;		/* define global symbol for 'name' */	\
	.type name, @function;	/* symbol type is function */		\
	.align 2;		/* align function definition */		\
	name:			/* function starts here */		\
	pushl $(num);							\
	jmp _alltraps							

/* Use TRAPHANDLER_NOEC for traps where the CPU doesn't push an error code.
 * It pushes a 0 in place of the error code, so the trap frame has the same
 * format in either case.
 */
#define TRAPHANDLER_NOEC(name, num)					\
.text;												\
	.globl name;							\
	.type name, @function;						\
	.align 2;							\
	name:								\
	pushl $0;							\
	pushl $(num);							\
	jmp _alltraps
/*.text*/

/*
 * Lab 3: Your code here for generating entry points for the different traps.
 */
/*.data
	.global handler_entry
handler_entry:*/

.text

TRAPHANDLER_NOEC(trap_handle_divide, T_DIVIDE);
TRAPHANDLER_NOEC(trap_handle_debug, T_DEBUG);
TRAPHANDLER_NOEC(trap_handle_nmi, T_NMI);
TRAPHANDLER_NOEC(trap_handle_brkpt, T_BRKPT);
TRAPHANDLER_NOEC(trap_handle_oflow, T_OFLOW);
TRAPHANDLER_NOEC(trap_handle_bound, T_BOUND);
TRAPHANDLER_NOEC(trap_handle_illop, T_ILLOP);
TRAPHANDLER_NOEC(trap_handle_device, T_DEVICE);
TRAPHANDLER(trap_handle_dblflt, T_DBLFLT);
TRAPHANDLER(trap_handle_tss, T_TSS);
TRAPHANDLER(trap_handle_segnp, T_SEGNP);
TRAPHANDLER(trap_handle_stack, T_STACK);
TRAPHANDLER(trap_handle_gpflt, T_GPFLT);
TRAPHANDLER(trap_handle_pgflt, T_PGFLT);
TRAPHANDLER_NOEC(trap_handle_fperr, T_FPERR);
TRAPHANDLER(trap_handle_align, T_ALIGN);
TRAPHANDLER_NOEC(trap_handle_mchk, T_MCHK);
TRAPHANDLER_NOEC(trap_handle_simderr, T_SIMDERR);
TRAPHANDLER_NOEC(trap_handle_syscall, T_SYSCALL);

TRAPHANDLER_NOEC(irq_handler0, IRQ_OFFSET);
TRAPHANDLER_NOEC(irq_handler1, IRQ_OFFSET+1);
TRAPHANDLER_NOEC(irq_handler2, IRQ_OFFSET+2);
TRAPHANDLER_NOEC(irq_handler3, IRQ_OFFSET+3);
TRAPHANDLER_NOEC(irq_handler4, IRQ_OFFSET+4);
TRAPHANDLER_NOEC(irq_handler5, IRQ_OFFSET+5);
TRAPHANDLER_NOEC(irq_handler6, IRQ_OFFSET+6);
TRAPHANDLER_NOEC(irq_handler7, IRQ_OFFSET+7);
TRAPHANDLER_NOEC(irq_handler8, IRQ_OFFSET+8);
TRAPHANDLER_NOEC(irq_handler9, IRQ_OFFSET+9);
TRAPHANDLER_NOEC(irq_handler10, IRQ_OFFSET+10);
TRAPHANDLER_NOEC(irq_handler11, IRQ_OFFSET+11);
TRAPHANDLER_NOEC(irq_handler12, IRQ_OFFSET+12);
TRAPHANDLER_NOEC(irq_handler13, IRQ_OFFSET+13);
TRAPHANDLER_NOEC(irq_handler14, IRQ_OFFSET+14);
TRAPHANDLER_NOEC(irq_handler15, IRQ_OFFSET+15);

/*
 * Lab 3: Your code here for _alltraps
 */
.text
_alltraps:
	pushl %ds
	pushl %es
	pushal

	movl $GD_KD, %eax
	movw %ax, %ds
	movw %ax, %es
	pushl %esp
	movl $0x0, %ebp 

	call trap

	popal
	popl %es
	popl %ds  
	addl $8, %esp
	iret 
