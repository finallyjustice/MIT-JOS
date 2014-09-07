// Ping-pong a counter between two processes.
// Only need to start one of these -- splits into two, crudely.

#include <inc/string.h>
#include <inc/lib.h>

envid_t dumbfork(void);

void
umain(void)
{
	envid_t who;
	int i;

	// fork a child process
	who = dumbfork();

	if(who == 0)
	{
		sys_env_set_priority(sys_getenvid(), 5);
	}
	else
	{
		who =dumbfork();

		if(who == 0)
		{
			sys_env_set_priority(sys_getenvid(), 4);
		}
		else
		{
			who = dumbfork();
	
			if(who == 0)
			{	
				sys_env_set_priority(sys_getenvid(), 3);
			}
			else
			{
				who = dumbfork();
	 
				if(who == 0)
				{
					sys_env_set_priority(sys_getenvid(), 2);
				}
				else
				{
					who = dumbfork();
	
					if(who == 0)
					{
						sys_env_set_priority(sys_getenvid(), 1);
					}
				}
			}
		}
	}

	struct Env *env_instance;
	env_instance=(struct Env*)&envs[ENVX(sys_getenvid())];
		
	int count;
	for(count=0; count<10; count++)
	{
		cprintf("I am process %08x, level %u\n", sys_getenvid(), env_instance->env_priority);
		sys_yield();
	}
}

void
duppage(envid_t dstenv, void *addr)
{
	int r;

	// This is NOT what you should do in your fork.
	if ((r = sys_page_alloc(dstenv, addr, PTE_P|PTE_U|PTE_W)) < 0)
		panic("sys_page_alloc: %e", r);
	if ((r = sys_page_map(dstenv, addr, 0, UTEMP, PTE_P|PTE_U|PTE_W)) < 0)
		panic("sys_page_map: %e", r);
	memmove(UTEMP, addr, PGSIZE);
	if ((r = sys_page_unmap(0, UTEMP)) < 0)
		panic("sys_page_unmap: %e", r);
}

envid_t
dumbfork(void)
{
	envid_t envid;
	uint8_t *addr;
	int r;
	extern unsigned char end[];

	// Allocate a new child environment.
	// The kernel will initialize it with a copy of our register state,
	// so that the child will appear to have called sys_exofork() too -
	// except that in the child, this "fake" call to sys_exofork()
	// will return 0 instead of the envid of the child.
	envid = sys_exofork();
	if (envid < 0)
		panic("sys_exofork: %e", envid);
	if (envid == 0) {
		// We're the child.
		// The copied value of the global variable 'env'
		// is no longer valid (it refers to the parent!).
		// Fix it and return 0.
		env = &envs[ENVX(sys_getenvid())];
		return 0;
	}

	// We're the parent.
	// Eagerly copy our entire address space into the child.
	// This is NOT what you should do in your fork implementation.
	for (addr = (uint8_t*) UTEXT; addr < end; addr += PGSIZE)
		duppage(envid, addr);

	// Also copy the stack we are currently running on.
	duppage(envid, ROUNDDOWN(&addr, PGSIZE));

	// Start the child environment running
	if ((r = sys_env_set_status(envid, ENV_RUNNABLE)) < 0)
		panic("sys_env_set_status: %e", r);

	return envid;
}

