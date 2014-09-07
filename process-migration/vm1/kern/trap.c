#include <inc/mmu.h>
#include <inc/x86.h>
#include <inc/assert.h>
#include <inc/string.h>
#include <kern/pmap.h>
#include <kern/trap.h>
#include <kern/console.h>
#include <kern/monitor.h>
#include <kern/env.h>
#include <kern/syscall.h>
#include <kern/sched.h>
#include <kern/kclock.h>
#include <kern/picirq.h>
#include <kern/time.h>

#include <kern/kdebug.h>
static struct Taskstate ts;

extern void trap_handle_divide();
extern void trap_handle_debug();
extern void trap_handle_nmi();
extern void trap_handle_brkpt();
extern void trap_handle_oflow();
extern void trap_handle_bound();
extern void trap_handle_illop();
extern void trap_handle_device();
extern void trap_handle_dblflt();
extern void trap_handle_tss();
extern void trap_handle_segnp();
extern void trap_handle_stack();
extern void trap_handle_gpflt();
extern void trap_handle_pgflt();
extern void trap_handle_fperr();
extern void trap_handle_align();
extern void trap_handle_mchk();
extern void trap_handle_simderr();
extern void trap_handle_syscall();


extern void irq_handler0();
extern void irq_handler1();
extern void irq_handler2();
extern void irq_handler3();
extern void irq_handler4();
extern void irq_handler5();
extern void irq_handler6();
extern void irq_handler7();
extern void irq_handler8();
extern void irq_handler9();
extern void irq_handler10();
extern void irq_handler11();
extern void irq_handler12();
extern void irq_handler13();
extern void irq_handler14();
extern void irq_handler15();

/* Interrupt descriptor table.  (Must be built at run time because
 * shifted function addresses can't be represented in relocation records.)
 */
struct Gatedesc idt[256] = { { 0 } };
struct Pseudodesc idt_pd = {
	sizeof(idt) - 1, (uint32_t) idt
};


static const char *trapname(int trapno)
{
	static const char * const excnames[] = {
		"Divide error",
		"Debug",
		"Non-Maskable Interrupt",
		"Breakpoint",
		"Overflow",
		"BOUND Range Exceeded",
		"Invalid Opcode",
		"Device Not Available",
		"Double Fault",
		"Coprocessor Segment Overrun",
		"Invalid TSS",
		"Segment Not Present",
		"Stack Fault",
		"General Protection",
		"Page Fault",
		"(unknown trap)",
		"x87 FPU Floating-Point Error",
		"Alignment Check",
		"Machine-Check",
		"SIMD Floating-Point Exception"
	};

	if (trapno < sizeof(excnames)/sizeof(excnames[0]))
		return excnames[trapno];
	if (trapno == T_SYSCALL)
		return "System call";
	if (trapno >= IRQ_OFFSET && trapno < IRQ_OFFSET + 16)
		return "Hardware Interrupt";
	return "(unknown trap)";
}


void
idt_init(void)
{
	extern struct Segdesc gdt[];
	
	// LAB 3: Your code here.
	//extern long handler_entry[][2];
	//int count;
	//for(count=0; handler_entry[count][1]; count++)  
	//{ 
	//	if(handler_entry[count][0]==T_SYSCALL)
	//	{
	//		SETGATE(idt[handler_entry[count][0]], 0, GD_KT, handler_entry[count][1], 3);
	//	}
	//	else
	//	{
	//		SETGATE(idt[handler_entry[count][0]], 0, GD_KT, handler_entry[count][1], 0);  
	//	}
	//}

	
	SETGATE(idt[T_DIVIDE], 0, GD_KT, trap_handle_divide, 0);
	SETGATE(idt[T_DEBUG], 0, GD_KT, trap_handle_debug, 0);
	SETGATE(idt[T_NMI], 0, GD_KT, trap_handle_nmi, 0);
	SETGATE(idt[T_BRKPT], 0, GD_KT, trap_handle_brkpt, 3);
	SETGATE(idt[T_OFLOW], 0, GD_KT, trap_handle_oflow, 0);
	SETGATE(idt[T_BOUND], 0, GD_KT, trap_handle_bound, 0);
	SETGATE(idt[T_ILLOP], 0, GD_KT, trap_handle_illop, 0);
	SETGATE(idt[T_DEVICE], 0, GD_KT, trap_handle_device, 0);
	SETGATE(idt[T_DBLFLT], 0, GD_KT, trap_handle_dblflt, 0);
	SETGATE(idt[T_TSS], 0, GD_KT, trap_handle_tss, 0);
	SETGATE(idt[T_SEGNP], 0, GD_KT, trap_handle_segnp, 0);
	SETGATE(idt[T_STACK], 0, GD_KT, trap_handle_stack, 0);
	SETGATE(idt[T_GPFLT], 0, GD_KT, trap_handle_gpflt, 0);
	SETGATE(idt[T_PGFLT], 0, GD_KT, trap_handle_pgflt, 0);
	SETGATE(idt[T_FPERR], 0, GD_KT, trap_handle_fperr, 0);
	SETGATE(idt[T_ALIGN], 0, GD_KT, trap_handle_align, 0);
	SETGATE(idt[T_MCHK], 0, GD_KT, trap_handle_mchk, 0);
	SETGATE(idt[T_SIMDERR], 0, GD_KT, trap_handle_simderr, 0);
	SETGATE(idt[T_SYSCALL], 0, GD_KT, trap_handle_syscall, 3);

	SETGATE(idt[IRQ_OFFSET], 0, GD_KT, irq_handler0, 0);
	SETGATE(idt[IRQ_OFFSET+1], 0, GD_KT, irq_handler1, 0);
	SETGATE(idt[IRQ_OFFSET+2], 0, GD_KT, irq_handler2, 0);
	SETGATE(idt[IRQ_OFFSET+3], 0, GD_KT, irq_handler3, 0);
	SETGATE(idt[IRQ_OFFSET+4], 0, GD_KT, irq_handler4, 0);
	SETGATE(idt[IRQ_OFFSET+5], 0, GD_KT, irq_handler5, 0);
	SETGATE(idt[IRQ_OFFSET+6], 0, GD_KT, irq_handler6, 0);
	SETGATE(idt[IRQ_OFFSET+7], 0, GD_KT, irq_handler7, 0);
	SETGATE(idt[IRQ_OFFSET+8], 0, GD_KT, irq_handler8, 0);
	SETGATE(idt[IRQ_OFFSET+9], 0, GD_KT, irq_handler9, 0);
	SETGATE(idt[IRQ_OFFSET+10], 0, GD_KT, irq_handler10, 0);
	SETGATE(idt[IRQ_OFFSET+11], 0, GD_KT, irq_handler11, 0);
	SETGATE(idt[IRQ_OFFSET+12], 0, GD_KT, irq_handler12, 0);
	SETGATE(idt[IRQ_OFFSET+13], 0, GD_KT, irq_handler13, 0);
	SETGATE(idt[IRQ_OFFSET+14], 0, GD_KT, irq_handler14, 0);
	SETGATE(idt[IRQ_OFFSET+15], 0, GD_KT, irq_handler15, 0);

	// Setup a TSS so that we get the right stack
	// when we trap to the kernel.
	ts.ts_esp0 = KSTACKTOP;
	ts.ts_ss0 = GD_KD;

	// Initialize the TSS field of the gdt.
	gdt[GD_TSS >> 3] = SEG16(STS_T32A, (uint32_t) (&ts),
					sizeof(struct Taskstate), 0);
	gdt[GD_TSS >> 3].sd_s = 0;

	// Load the TSS
	ltr(GD_TSS);

	// Load the IDT
	asm volatile("lidt idt_pd");
}

void
print_trapframe(struct Trapframe *tf)
{
	cprintf("TRAP frame at %p\n", tf);
	print_regs(&tf->tf_regs);
	cprintf("  es   0x----%04x\n", tf->tf_es);
	cprintf("  ds   0x----%04x\n", tf->tf_ds);
	cprintf("  trap 0x%08x %s\n", tf->tf_trapno, trapname(tf->tf_trapno));
	cprintf("  err  0x%08x\n", tf->tf_err);
	cprintf("  eip  0x%08x\n", tf->tf_eip);
	cprintf("  cs   0x----%04x\n", tf->tf_cs);
	cprintf("  flag 0x%08x\n", tf->tf_eflags);
	cprintf("  esp  0x%08x\n", tf->tf_esp);
	cprintf("  ss   0x----%04x\n", tf->tf_ss);
}

void
print_regs(struct PushRegs *regs)
{
	cprintf("  edi  0x%08x\n", regs->reg_edi);
	cprintf("  esi  0x%08x\n", regs->reg_esi);
	cprintf("  ebp  0x%08x\n", regs->reg_ebp);
	cprintf("  oesp 0x%08x\n", regs->reg_oesp);
	cprintf("  ebx  0x%08x\n", regs->reg_ebx);
	cprintf("  edx  0x%08x\n", regs->reg_edx);
	cprintf("  ecx  0x%08x\n", regs->reg_ecx);
	cprintf("  eax  0x%08x\n", regs->reg_eax);
}

static void
trap_dispatch(struct Trapframe *tf)
{
	// Handle processor exceptions.
	// LAB 3: Your code here.
	if(tf->tf_trapno == T_PGFLT)
	{
		page_fault_handler(tf);
	}
	
	if(tf->tf_trapno == T_BRKPT)
	{
		monitor(tf);
	}
			
	if(tf->tf_trapno == T_SYSCALL)
	{
		tf->tf_regs.reg_eax=syscall(tf->tf_regs.reg_eax, tf->tf_regs.reg_edx, tf->tf_regs.reg_ecx, tf->tf_regs.reg_ebx, tf->tf_regs.reg_edi, tf->tf_regs.reg_esi);
	
		return;
	}

	// Handle clock interrupts.
	// LAB 4: Your code here.
	if(tf->tf_trapno==(IRQ_OFFSET+IRQ_TIMER))
	{
		time_tick();
		sched_yield();
		return;
	}

	// Add time tick increment to clock interrupts.
	// LAB 6: Your code here.

	// Handle spurious interrupts
	// The hardware sometimes raises these because of noise on the
	// IRQ line or other reasons. We don't care.
	if (tf->tf_trapno == IRQ_OFFSET + IRQ_SPURIOUS) {
		cprintf("Spurious interrupt on irq 7\n");
		print_trapframe(tf);
		return;
	}


	// Handle keyboard and serial interrupts.
	// LAB 7: Your code here.
	if(tf->tf_trapno == IRQ_OFFSET+IRQ_KBD)
	{
		kbd_intr();
		return;
	}

	if(tf->tf_trapno == IRQ_OFFSET+IRQ_SERIAL)
	{
		serial_intr();
		return;
	}

	// Unexpected trap: The user process or the kernel has a bug.
	print_trapframe(tf);
	if (tf->tf_cs == GD_KT)
		panic("unhandled trap in kernel");
	else {
		env_destroy(curenv);
		return;
	}
}

void
trap(struct Trapframe *tf)
{
	// The environment may have set DF and some versions
	// of GCC rely on DF being clear
	asm volatile("cld" ::: "cc");

	// Check that interrupts are disabled.  If this assertion
	// fails, DO NOT be tempted to fix it by inserting a "cli" in
	// the interrupt path.
	assert(!(read_eflags() & FL_IF));

	if ((tf->tf_cs & 3) == 3) {
		// Trapped from user mode.
		// Copy trap frame (which is currently on the stack)
		// into 'curenv->env_tf', so that running the environment
		// will restart at the trap point.
		assert(curenv);
		curenv->env_tf = *tf;
		// The trapframe on the stack should be ignored from here on.
		tf = &curenv->env_tf;
	}
	
	// Dispatch based on what type of trap occurred
	trap_dispatch(tf);

	// If we made it to this point, then no other environment was
	// scheduled, so we should return to the current environment
	// if doing so makes sense.
	if (curenv && curenv->env_status == ENV_RUNNABLE)
		env_run(curenv);
	else
		sched_yield();
}


void
page_fault_handler(struct Trapframe *tf)
{
	uint32_t fault_va;

	// Read processor's CR2 register to find the faulting address
	fault_va = rcr2();

	// Handle kernel-mode page faults.
	
	// LAB 3: Your code here.
	if((tf->tf_cs&3) == 0)
	{
		panic("handle kernel-mode page faults: 0x%08x", fault_va);
	}

	// We've already handled kernel-mode exceptions, so if we get here,
	// the page fault happened in user mode.

	// Call the environment's page fault upcall, if one exists.  Set up a
	// page fault stack frame on the user exception stack (below
	// UXSTACKTOP), then branch to curenv->env_pgfault_upcall.
	//
	// The page fault upcall might cause another page fault, in which case
	// we branch to the page fault upcall recursively, pushing another
	// page fault stack frame on top of the user exception stack.
	//
	// The trap handler needs one word of scratch space at the top of the
	// trap-time stack in order to return.  In the non-recursive case, we
	// don't have to worry about this because the top of the regular user
	// stack is free.  In the recursive case, this means we have to leave
	// an extra word between the current top of the exception stack and
	// the new stack frame because the exception stack _is_ the trap-time
	// stack.
	//
	// If there's no page fault upcall, the environment didn't allocate a
	// page for its exception stack or can't write to it, or the exception
	// stack overflows, then destroy the environment that caused the fault.
	// Note that the grade script assumes you will first check for the page
	// fault upcall and print the "user fault va" message below if there is
	// none.  The remaining three checks can be combined into a single test.
	//
	// Hints:
	//   user_mem_assert() and env_run() are useful here.
	//   To change what the user environment runs, modify 'curenv->env_tf'
	//   (the 'tf' variable points at 'curenv->env_tf').

	// LAB 4: Your code here.
	
	if(curenv->env_type == ENV_TYPE_PGFAULT)
	{
		my_env_handle_pgfault(curenv->env_id, fault_va);
		//cprintf("I have page error: 0x%08x\n", fault_va);
		//env_run(curenv);
	}

	if(curenv->env_pgfault_upcall != NULL) 
	{
		struct UTrapframe *utf;
	
		if((tf->tf_esp >= UXSTACKTOP-PGSIZE) && (tf->tf_esp <= UXSTACKTOP-1))
		{
			utf=(struct UTrapframe *)(tf->tf_esp-4-sizeof(struct UTrapframe));
		}
		else
		{
			utf=(struct UTrapframe *)(UXSTACKTOP-sizeof(struct UTrapframe));
		}
		
		user_mem_assert(curenv, utf, sizeof(struct UTrapframe), PTE_W);

		utf->utf_esp=tf->tf_esp;
		utf->utf_eflags=tf->tf_eflags;
		utf->utf_eip=tf->tf_eip;
		utf->utf_regs=tf->tf_regs;
		utf->utf_err=tf->tf_err;
		utf->utf_fault_va=fault_va;
		
		curenv->env_tf.tf_esp=(uint32_t)utf;
		curenv->env_tf.tf_eip=(uint32_t)curenv->env_pgfault_upcall;

		env_run(curenv);
	}
	else
	{

	}
		
	// Destroy the environment that caused the fault.
	cprintf("[%08x] user fault va %08x ip %08x\n",
		curenv->env_id, fault_va, tf->tf_eip);
	print_trapframe(tf);
	env_destroy(curenv);
}

