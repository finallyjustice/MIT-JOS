// Simple command-line kernel monitor useful for
// controlling the kernel and exploring the system interactively.

#include <inc/stdio.h>
#include <inc/string.h>
#include <inc/memlayout.h>
#include <inc/assert.h>
#include <inc/x86.h>

#include <kern/console.h>
#include <kern/monitor.h>
#include <kern/kdebug.h>
#include <kern/trap.h>
#include <kern/pmap.h>

#define CMDBUF_SIZE	80	// enough for one VGA text line


struct Command {
	const char *name;
	const char *desc;
	// return -1 to force monitor to exit
	int (*func)(int argc, char** argv, struct Trapframe* tf);
};

static struct Command commands[] = {
	{ "help", "Display this list of commands", mon_help },
	{ "kerninfo", "Display information about the kernel", mon_kerninfo },
	{ "backtrace", "Display information about backtrace", mon_backtrace},
	{ "showmappings", "Display page mapping information", mon_showmappings},
	{ "setmappingperm", "Change or set page mapping permission", mon_setmappingperm},
	{ "dumpmeminfo", "Dump memory information", mon_dumpmeminfo},
	{ "alloc_page", "Allocate a new page", mon_alloc_page},
	{ "page_status", "Return status of page", mon_page_status},
	{ "free_page", "Free a page", mon_free_page},
};
#define NCOMMANDS (sizeof(commands)/sizeof(commands[0]))

unsigned read_eip();

/***** Implementations of basic kernel monitor commands *****/

int
mon_help(int argc, char **argv, struct Trapframe *tf)
{
	int i;

	for (i = 0; i < NCOMMANDS; i++)
		cprintf("%s - %s\n", commands[i].name, commands[i].desc);
	return 0;
}

int
mon_kerninfo(int argc, char **argv, struct Trapframe *tf)
{
	extern char _start[], etext[], edata[], end[];

	cprintf("Special kernel symbols:\n");
	cprintf("  _start %08x (virt)  %08x (phys)\n", _start, _start - KERNBASE);
	cprintf("  etext  %08x (virt)  %08x (phys)\n", etext, etext - KERNBASE);
	cprintf("  edata  %08x (virt)  %08x (phys)\n", edata, edata - KERNBASE);
	cprintf("  end    %08x (virt)  %08x (phys)\n", end, end - KERNBASE);
	cprintf("Kernel executable memory footprint: %dKB\n",
		(end-_start+1023)/1024);
	return 0;
}

int
mon_backtrace(int argc, char **argv, struct Trapframe *tf)
{
	cprintf("Stack backtrace:\n");
	// Your code here.
	uint32_t ebp;
	uint32_t eip;
	uint32_t arg0;
	uint32_t arg1;
	uint32_t arg2;
	uint32_t arg3;
	uint32_t arg4;
	int count;
	
	ebp = read_ebp();
	
	eip = *((uint32_t*)ebp+1);
	arg0 = *((uint32_t*)ebp+2);
	arg1 = *((uint32_t*)ebp+3);
	arg2 = *((uint32_t*)ebp+4);
	arg3 = *((uint32_t*)ebp+5);
	arg4 = *((uint32_t*)ebp+6);

	do{
		cprintf("   ebp %08x  eip %08x  args %08x %08x %08x %08x %08x\n", ebp, eip, arg0, arg1, arg2, arg3, arg4);
		
		struct Eipdebuginfo info;
		debuginfo_eip(eip, &info);
		     
		cprintf("      %s:%d:  ", info.eip_file, info.eip_line);
		for(count=0;count<info.eip_fn_namelen;count++)
		{
			cprintf("%c", info.eip_fn_name[count]);
		}
		cprintf("+%x\n", eip-info.eip_fn_addr);
		
		ebp = *(uint32_t *)ebp;
		
		if (ebp != 0)
		{
			eip = *((uint32_t*)ebp+1);
			arg0 = *((uint32_t*)ebp+2);
			arg1 = *((uint32_t*)ebp+3);
			arg2 = *((uint32_t*)ebp+4);
			arg3 = *((uint32_t*)ebp+5);
			arg4 = *((uint32_t*)ebp+6);
		}
	}while(ebp != 0);
										   
	return 0;
}

int
mon_showmappings(int argc, char **argv, struct Trapframe *tf)
{
	if(argc != 3)
	{
		cprintf("Usage: showmappings start_va end_va\n");
		return 1;
	}

	uint32_t start_va;
	uint32_t end_va;

	start_va=strtol(argv[1], 0, 0);
	end_va=strtol(argv[2], 0, 0);

	showmappings(start_va, end_va);

	return 0;
}

int 
mon_setmappingperm(int argc, char **argv, struct Trapframe *tf)
{
	if(argc < 3)
	{
		cprintf("Usage: setmappingperm virtaddr change|clear [KRO|KRW|URO|URW]\n");
		cprintf("KRO - Kernel Level Read Only\n");
		cprintf("KRW - Kernel Level Read Write\n");
		cprintf("URO - User Level Read Only\n");
		cprintf("URW - User Level Read Write\n");
	}
	else
	{
		uint32_t va=strtol(argv[1], 0, 0);	

		if(strncmp(argv[2], "change", 6) == 0)
		{
			if(argc != 4)
			{
				cprintf("Usage: setmappingperm virtaddr change [KRO|KRW|URO|URW]\n");
				return 0;
			}

			cprintf("######Before Change:\n");
			showmappings(va, va+1);
			       
			if(strncmp(argv[3], "KRO", 3) == 0)
			{
				setmappingperm(va, 0x0);
			}
			else if(strncmp(argv[3], "KRW", 3) == 0)
			{
				setmappingperm(va, 0x2);
			}
			else if(strncmp(argv[3], "URO", 3) == 0)
			{
				setmappingperm(va, 0x4);
			}
			else if(strncmp(argv[3], "URW", 3) == 0)
			{
				setmappingperm(va, 0x6);
			}
			else
			{
				cprintf("Usage: setmappingperm virtaddr change [KRO|KRW|URO|URW]\n");
				return 0;
			}
			      
			cprintf("######After Change:\n");
			showmappings(va, va+1);
		}
		else if(strncmp(argv[2], "clear", 5) == 0)
		{
			cprintf("######Before Clear:\n");
			showmappings(va, va+1);

			setmappingperm(va, 0x4);

			cprintf("######After Clear:\n");
			showmappings(va, va+1);
		}
		else
		{
			cprintf("Usage: setmappingperm virtaddr change|clear [KRO|KRW|URO|URW]\n");
		}
	}
	return 0;
}

int
mon_dumpmeminfo(int argc, char **argv, struct Trapframe *tf)
{
	uint32_t start_va;
	uint32_t end_va;
	if(argc != 4) 
	{
		cprintf("Usage: dumpmem v|p addr num_words\n");
		cprintf("v: virtual address\n");
		cprintf("p: physical address\n");

		return 0;
	}

	if(argv[1][0]!='v' && argv[1][0]!='p')
	{
		cprintf("Usage: dumpmem v|p addr num_words\n");
		cprintf("v: virtual address\n");
		cprintf("p: physical address\n");
		
		return 0;
	}

	start_va=strtol(argv[2], 0, 0);
	end_va=start_va+4*strtol(argv[3], 0, 0);
	
	if(start_va > end_va)
	{
		cprintf("start address is larger than end address");

		return 0;
	}

	if(start_va!=ROUNDUP(start_va, 4) || end_va!=ROUNDUP(end_va, 4)) 
	{
		cprintf ("Address is not multiple of 4!\n");

		return 0;
	}
			
					
	if(argv[1][0] == 'p') 
	{
		start_va=start_va+ KERNBASE;
		end_va=end_va+KERNBASE;
	}
							
	dumpmeminfo(start_va, end_va);
						
	return 0;
}

int
mon_alloc_page(int argc, char **argv, struct Trapframe *tf)
{
	struct Page *temp;
	
	if(page_alloc(&temp) == 0) 
	{
		temp->pp_ref=temp->pp_ref+1;

		cprintf ("  0x%08x\n", page2pa(temp));
	} 
	else 
	{
		cprintf("   Cannot allocate a new page for you\n");
	}
	return 0;
}

int
mon_page_status(int argc, char **argv, struct Trapframe *tf)
{
	if(argc != 2) 
	{
		cprintf("Usage: page_status address\n");
		return 0;
	}
	
	uint32_t pa;
	struct Page *temp;

	pa=strtol(argv[1], 0, 0);
	temp=pa2page(pa);
		
	if(temp->pp_ref == 0) 
	{
		cprintf("  free\n");
	} 
	else 
	{
		cprintf("  allocated\n");
	}

	return 0;
}

int
mon_free_page(int argc, char **argv, struct Trapframe *tf)
{
	if(argc != 2)
	{
		cprintf("Usage: free_page address\n");
		return 0;
	}

	uint32_t pa;
	struct Page *temp;

	pa=strtol(argv[1], 0, 0);
	temp=pa2page(pa);

	if(temp->pp_ref == 0) 
	{
		cprintf("  Free page failed\n");
	}
	else
	{
		temp->pp_ref--;

		if(temp->pp_ref == 0)
		{
			page_free(temp);
		}
		cprintf("  Free page successfully\n");
	}

	return 0;
}

/***** Kernel monitor command interpreter *****/

#define WHITESPACE "\t\r\n "
#define MAXARGS 16

static int
runcmd(char *buf, struct Trapframe *tf)
{
	int argc;
	char *argv[MAXARGS];
	int i;

	// Parse the command buffer into whitespace-separated arguments
	argc = 0;
	argv[argc] = 0;
	while (1) {
		// gobble whitespace
		while (*buf && strchr(WHITESPACE, *buf))
			*buf++ = 0;
		if (*buf == 0)
			break;

		// save and scan past next arg
		if (argc == MAXARGS-1) {
			cprintf("Too many arguments (max %d)\n", MAXARGS);
			return 0;
		}
		argv[argc++] = buf;
		while (*buf && !strchr(WHITESPACE, *buf))
			buf++;
	}
	argv[argc] = 0;

	// Lookup and invoke the command
	if (argc == 0)
		return 0;
	for (i = 0; i < NCOMMANDS; i++) {
		if (strcmp(argv[0], commands[i].name) == 0)
			return commands[i].func(argc, argv, tf);
	}
	cprintf("Unknown command '%s'\n", argv[0]);
	return 0;
}

void
monitor(struct Trapframe *tf)
{
	char *buf;

	cprintf("Welcome to the JOS kernel monitor!\n");
	cprintf("Type 'help' for a list of commands.\n");

	if (tf != NULL)
		print_trapframe(tf);

	while (1) {
		buf = readline("K> ");
		if (buf != NULL)
			if (runcmd(buf, tf) < 0)
				break;
	}
}

// return EIP of caller.
// does not work if inlined.
// putting at the end of the file seems to prevent inlining.
unsigned
read_eip()
{
	uint32_t callerpc;
	__asm __volatile("movl 4(%%ebp), %0" : "=r" (callerpc));
	return callerpc;
}
