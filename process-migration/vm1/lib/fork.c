// implement fork from user space

#include <inc/string.h>
#include <inc/lib.h>

// PTE_COW marks copy-on-write page table entries.
// It is one of the bits explicitly allocated to user processes (PTE_AVAIL).
#define PTE_COW		0x800

//
// Custom page fault handler - if faulting page is copy-on-write,
// map in our own private writable copy.
//
static void
pgfault(struct UTrapframe *utf)
{
	void *addr = (void *) utf->utf_fault_va;
	uint32_t err = utf->utf_err;
	int r;

	// Check that the faulting access was (1) a write, and (2) to a
	// copy-on-write page.  If not, panic.
	// Hint:
	//   Use the read-only page table mappings at vpt
	//   (see <inc/memlayout.h>).

	// LAB 4: Your code here.
	
	if((vpd[VPD(addr)]&PTE_P)==0 || (vpt[VPN(addr)]&PTE_COW)==0 || (err&FEC_WR)==0)
	{
		panic("not write or cow");
	}

	// Allocate a new page, map it at a temporary location (PFTEMP),
	// copy the data from the old page to the new page, then move the new
	// page to the old page's address.
	// Hint:
	//   You should make three system calls.
	//   No need to explicitly delete the old page's mapping.

	// LAB 4: Your code here.
	if(sys_page_alloc(0, (void *)PFTEMP, PTE_P|PTE_U|PTE_W) < 0)
	{
		panic("page allocation error!");
	}
	
	addr=ROUNDDOWN(addr, PGSIZE);
	
	memmove(PFTEMP, addr, PGSIZE);
	
	if(sys_page_map(0, PFTEMP, 0, addr, PTE_P|PTE_U|PTE_W) < 0)
	{
		panic("page mapping error!");
	}

	//panic("pgfault not implemented");
}

//
// Map our virtual page pn (address pn*PGSIZE) into the target envid
// at the same virtual address.  If the page is writable or copy-on-write,
// the new mapping must be created copy-on-write, and then our mapping must be
// marked copy-on-write as well.  (Exercise: Why do we need to mark ours
// copy-on-write again if it was already copy-on-write at the beginning of
// this function?)
//
// Returns: 0 on success, < 0 on error.
// It is also OK to panic on error.
// 
static int
duppage(envid_t envid, unsigned pn)
{
	int r;

	// LAB 4: Your code here.
	pte_t pte;
	void *frame_addr;

	frame_addr=(void *)((uint32_t)pn*PGSIZE);
	pte=vpt[VPN(frame_addr)];

	if((pte & PTE_SHARE) != 0)
	{
		if((r=sys_page_map(0, frame_addr, envid, frame_addr, PTE_USER)) < 0)
		{
			panic("duppage(): Page Mapping Error : %e", r);
		}
	}
	else
	{
		if((pte&PTE_COW)>0 || (pte&PTE_W)>0) 
		{
			if((r=sys_page_map(0, frame_addr, envid, frame_addr, PTE_P|PTE_U|PTE_COW)) < 0)
			{
				panic("Page Mapping Error : %e", r);
			}
			
			if(sys_page_map(0, frame_addr, 0, frame_addr, PTE_P|PTE_U|PTE_COW) < 0)
			{
				panic("Page Mapping Error");
			}	
		}	 
		else 
		{
			if(sys_page_map(0, frame_addr, envid, frame_addr, PTE_P|PTE_U) < 0)
			{
				panic("Page Mapping Error");
			}
		}
	}

	return 0;
}

//
// User-level fork with copy-on-write.
// Set up our page fault handler appropriately.
// Create a child.
// Copy our address space and page fault handler setup to the child.
// Then mark the child as runnable and return.
//
// Returns: child's envid to the parent, 0 to the child, < 0 on error.
// It is also OK to panic on error.
//
// Hint:
//   Use vpd, vpt, and duppage.
//   Remember to fix "env" in the child process.
//   Neither user exception stack should ever be marked copy-on-write,
//   so you must allocate a new page for the child's user exception stack.
//
envid_t
fork(void)
{
	// LAB 4: Your code here.
	//panic("fork not implemented");
	extern void _pgfault_upcall(void);
	set_pgfault_handler(pgfault);

	envid_t envid;
	uint32_t addr;
	
	envid=sys_exofork();
	
	if(envid < 0)
	{
		panic("sys_exofork error!");
	}
	
	if(envid == 0) 
	{
		env=&envs[ENVX(sys_getenvid())];
		return 0;
	}
	
	addr=UTEXT;
	while(addr<UXSTACKTOP-PGSIZE)
	{
		if((vpd[VPD(addr)]&PTE_P)>0 && (vpt[VPN(addr)]&PTE_P)>0 && (vpt[VPN(addr)]&PTE_U)>0)
		{   
			duppage(envid, VPN(addr));
		}

		addr=addr+PGSIZE;
	}
				
	if(sys_page_alloc(envid, (void *)(UXSTACKTOP-PGSIZE), PTE_U|PTE_W|PTE_P) < 0)
	{
		panic("page allocation error");
	}
			
	sys_env_set_pgfault_upcall(envid, _pgfault_upcall);
				
	if(sys_env_set_status(envid, ENV_RUNNABLE) < 0)
	{
		panic("page allocation error");
	}
					
	return envid;
}

// Challenge!
int
sfork(void)
{
	panic("sfork not implemented");
	return -E_INVAL;
}
