/* See COPYRIGHT for copyright information. */

#include <inc/x86.h>
#include <inc/error.h>
#include <inc/string.h>
#include <inc/assert.h>

#include <kern/env.h>
#include <kern/pmap.h>
#include <kern/trap.h>
#include <kern/syscall.h>
#include <kern/console.h>
#include <kern/sched.h>
#include <kern/time.h>
#include <kern/e100.h>

// Print a string to the system console.
// The string is exactly 'len' characters long.
// Destroys the environment on memory errors.
static void
sys_cputs(const char *s, size_t len)
{
	// Check that the user has permission to read memory [s, s+len).
	// Destroy the environment if not.
	
	// LAB 3: Your code here.
	user_mem_assert(curenv, (void *)s, len, PTE_U);

	// Print the string supplied by the user.
	cprintf("%.*s", len, s);
}

// Read a character from the system console without blocking.
// Returns the character, or 0 if there is no input waiting.
static int
sys_cgetc(void)
{
	return cons_getc();
}

// Returns the current environment's envid.
static envid_t
sys_getenvid(void)
{
	return curenv->env_id;
}

// Destroy a given environment (possibly the currently running environment).
//
// Returns 0 on success, < 0 on error.  Errors are:
//	-E_BAD_ENV if environment envid doesn't currently exist,
//		or the caller doesn't have permission to change envid.
static int
sys_env_destroy(envid_t envid)
{
	int r;
	struct Env *e;

	if ((r = envid2env(envid, &e, 1)) < 0)
		return r;
	env_destroy(e);
	return 0;
}

static int
sys_my_env_destroy(envid_t envid)
{
	int r;
	struct Env *e;
	
	if ((r = my_envid2env(envid, &e)) < 0)
		return r;
	env_destroy(e);
		return 0;
}

// Deschedule current environment and pick a different one to run.
static void
sys_yield(void)
{
	sched_yield();
}

// Allocate a new environment.
// Returns envid of new environment, or < 0 on error.  Errors are:
//	-E_NO_FREE_ENV if no free environment is available.
//	-E_NO_MEM on memory exhaustion.
static envid_t
sys_exofork(void)
{
	// Create the new environment with env_alloc(), from kern/env.c.
	// It should be left as env_alloc created it, except that
	// status is set to ENV_NOT_RUNNABLE, and the register set is copied
	// from the current environment -- but tweaked so sys_exofork
	// will appear to return 0.

	// LAB 4: Your code here.
	//panic("sys_exofork not implemented");
	struct Env *env_instance;
	int result;
	
	result=env_alloc(&env_instance, sys_getenvid());
	if(result>=0)
	{
		env_instance->env_status=ENV_NOT_RUNNABLE;
		env_instance->env_tf=curenv->env_tf;
		env_instance->env_tf.tf_regs.reg_eax=0;
		//This is for priority from parent
		env_instance->env_priority=3;
		return env_instance->env_id;
	}
	else
	{
		return E_NO_FREE_ENV;
	}
}

// Set envid's env_status to status, which must be ENV_RUNNABLE
// or ENV_NOT_RUNNABLE.
//
// Returns 0 on success, < 0 on error.  Errors are:
//	-E_BAD_ENV if environment envid doesn't currently exist,
//		or the caller doesn't have permission to change envid.
//	-E_INVAL if status is not a valid status for an environment.
static int
sys_env_set_status(envid_t envid, int status)
{
	// Hint: Use the 'envid2env' function from kern/env.c to translate an
	// envid to a struct Env.
	// You should set envid2env's third argument to 1, which will
	// check whether the current environment has permission to set
	// envid's status.

	// LAB 4: Your code here.
	//panic("sys_env_set_status not implemented");
	struct Env *env_instance;
	int result;

	if(status != ENV_RUNNABLE && status != ENV_NOT_RUNNABLE)
	{
		cprintf("sys_set_env_status(): error with status\n");
		return -E_INVAL;
	}

	result=envid2env(envid, &env_instance, 1);
	if(result < 0)
	{
		cprintf("sys_set_env_status(): error with envid\n");
		return -E_BAD_ENV;
	}

	env_instance->env_status=status;

	return 0;
}

static int
sys_my_env_set_status(envid_t envid, int status)
{
	struct Env *env_instance;
	int result;

	if(status != ENV_RUNNABLE && status != ENV_NOT_RUNNABLE)
	{
		cprintf("sys_my_env_set_status(): error with status\n");
		return -E_INVAL;
	}
			
	result=my_envid2env(envid, &env_instance);
	if(result < 0)
	{
		cprintf("sys_my_env_set_status(): error with envid\n");
		return -E_BAD_ENV;
	}
					
	env_instance->env_status=status;
	return 0;
}

static int
sys_my_env_copyfrom(envid_t src_envid, envid_t *dst_envid)
{
	if(env_copyfrom(src_envid, dst_envid) < 0)
	{
		return -1;
	}

	return 0;
}

static int
sys_my_env_set_ip_port(envid_t env_id, char *ip_addr, int port)
{
	struct Env *env_instance;
	int result;

	result=my_envid2env(env_id, &env_instance);
	if(result < 0)
	{
		cprintf("sys_my_env_set_ip_port(): error with env_id\n");
		return -E_BAD_ENV;
	}

	memmove(env_instance->env_ip_from, ip_addr, 16);
	env_instance->env_port_from=port;

	return 0;
}

static int
sys_my_insert_page(envid_t env_id, uint8_t *addr, void *temp_mem, int perm)
{
	struct Env *env_instance;
	struct Page *temp_page;
	int result;

	result=my_envid2env(env_id, &env_instance);
	if(result < 0)
	{
		panic("sys_my_insert_page(): error with envid\n");
		return -E_BAD_ENV;
	}

	if(page_alloc(&temp_page))
	{
		panic("sys_my_insert_page(): error to alloc page");
		return -1;
	}
	
	memmove((void *)page2kva(temp_page), (void *)temp_mem, PGSIZE);
	 
	if(page_insert(env_instance->env_pgdir, temp_page, (void *)addr, PTE_W|PTE_P|PTE_U) < 0)
	{
		panic("sys_my_insert_page(): page_insert failed!");
	}

	return 0;
}

static int 
sys_my_env_establish(envid_t *env_id, int env_type)
{
	if(env_establish(env_id, env_type) < 0)
	{
		panic("sys_my_env_establish(): no free env");
		return -1;
	}

	return 0;
}

static int
sys_my_env_copy_trapframe(envid_t env_id, struct Trapframe *tf)
{
	if(env_copy_trapframe(env_id, tf) < 0)
	{
		panic("sys_my_env_copy_trapframe(): error");
		return -1;
	}
	
	return 0;
}

static int 
sys_my_check_page(envid_t env_id, uint8_t *addr, void *temp_mem, int *status)
{
	*status=0;

	pte_t *pte;
	struct Env *env_instance;
	int result;

	result=my_envid2env(env_id, &env_instance);
	if(result < 0)
	{
		panic("sys_my_check_page(): error with env_id");
		*status=-1;
		return -1;
	}

	if((pte=pgdir_walk(env_instance->env_pgdir, addr, 0)) != NULL)
	{
		if((*pte)&PTE_P)
		{
			memmove((void *)temp_mem, (void *)KADDR(PTE_ADDR(*pte)), PGSIZE);
			*status=1;
			return 0;
		}
	}
	
	return 0;
}

static int 
sys_my_check_page_simple(envid_t env_id, uint8_t *addr, int *status)
{
	*status=0;

	pte_t *pte;
	struct Env *env_instance;
	int result;
	
	result=my_envid2env(env_id, &env_instance);
	if(result < 0)
	{
		panic("sys_my_check_page(): error with env_id");
		*status=-1;
		return -1;
	}
	
	if((pte=pgdir_walk(env_instance->env_pgdir, addr, 0)) != NULL)
	{
		if((*pte)&PTE_P)
		{
			*status=1;
			return 0;
		}
	}

	return 0;
}

static int
sys_my_insert_pte_only(envid_t env_id, uint8_t *addr)
{
	pte_t *pte;
	struct Env *env_instance;
	int result;

	result=my_envid2env(env_id, &env_instance);
	if(result < 0)
	{
		panic("sys_my_insert_pte_only(): error with env_id");
		return -1;
	}

	pte=pgdir_walk(env_instance->env_pgdir, addr, 1);
	*pte=*pte & PTE_P;

	return 0;
}

static int 
sys_my_check_env_pgfault(int *status, envid_t *env_id, uint32_t *addr)
{
	*status=0;
	if(epm.env_pg_status == 1)
	{
		*status=1;
		*env_id=epm.env_id;
		*addr=epm.env_fault_va;
	}

	return 0;
}

static int
sys_my_set_env_pgfault()
{
	if(epm.env_pg_status == 0)
	{
		panic("sys_my_set_env_pgfault(): error, already ready!");

		return -1;
	}

	epm.env_pg_status=0;

	return 0;
}

// Set envid's trap frame to 'tf'.
// tf is modified to make sure that user environments always run at code
// protection level 3 (CPL 3) with interrupts enabled.
//
// Returns 0 on success, < 0 on error.  Errors are:
//	-E_BAD_ENV if environment envid doesn't currently exist,
//		or the caller doesn't have permission to change envid.
static int
sys_env_set_trapframe(envid_t envid, struct Trapframe *tf)
{
	// LAB 5: Your code here.
	// Remember to check whether the user has supplied us with a good
	// address!
	//panic("sys_env_set_trapframe not implemented");
	struct Env *env_instance;

	if(envid2env(envid, &env_instance, 1) < 0)
	{
		cprintf("sys_env_set_trapframe() error: bad env id\n");
		return -E_BAD_ENV;
	}

	env_instance->env_tf=*tf;
	env_instance->env_tf.tf_eflags=env_instance->env_tf.tf_eflags | FL_IF;
	env_instance->env_tf.tf_cs=GD_UT | 3;
	
	return 0;
}

// Set the page fault upcall for 'envid' by modifying the corresponding struct
// Env's 'env_pgfault_upcall' field.  When 'envid' causes a page fault, the
// kernel will push a fault record onto the exception stack, then branch to
// 'func'.
//
// Returns 0 on success, < 0 on error.  Errors are:
//	-E_BAD_ENV if environment envid doesn't currently exist,
//		or the caller doesn't have permission to change envid.
static int
sys_env_set_pgfault_upcall(envid_t envid, void *func)
{
	// LAB 4: Your code here.
	struct Env *env_instance;

	if(envid2env(envid, &env_instance, 1) < 0)
	{
		return -E_BAD_ENV;
	}
	
	env_instance->env_pgfault_upcall=func;
	
	return 0;
	//panic("sys_env_set_pgfault_upcall not implemented");
}

// Allocate a page of memory and map it at 'va' with permission
// 'perm' in the address space of 'envid'.
// The page's contents are set to 0.
// If a page is already mapped at 'va', that page is unmapped as a
// side effect.
//
// perm -- PTE_U | PTE_P must be set, PTE_AVAIL | PTE_W may or may not be set,
//         but no other bits may be set.  See PTE_USER in inc/mmu.h.
//
// Return 0 on success, < 0 on error.  Errors are:
//	-E_BAD_ENV if environment envid doesn't currently exist,
//		or the caller doesn't have permission to change envid.
//	-E_INVAL if va >= UTOP, or va is not page-aligned.
//	-E_INVAL if perm is inappropriate (see above).
//	-E_NO_MEM if there's no memory to allocate the new page,
//		or to allocate any necessary page tables.
static int
sys_page_alloc(envid_t envid, void *va, int perm)
{
	// Hint: This function is a wrapper around page_alloc() and
	//   page_insert() from kern/pmap.c.
	//   Most of the new code you write should be to check the
	//   parameters for correctness.
	//   If page_insert() fails, remember to free the page you
	//   allocated!

	// LAB 4: Your code here.
	//panic("sys_page_alloc not implemented");
	struct Env *env_instance;
	struct Page *page_instance;

	if(envid2env(envid, &env_instance, 1) < 0)
	{
		return -E_BAD_ENV;
	}
	
	if(va >= (void *)UTOP)
	{
		return -E_INVAL;
	}

	if((perm&PTE_U) == 0)
	{
		return -E_INVAL;
	}
	if((perm&PTE_P) == 0)
	{
		return -E_INVAL;
	}
	
	if((perm&~PTE_USER) > 0)
	{
		return -E_INVAL;
	}
	
	if(page_alloc(&page_instance) < 0)
	{
		return -E_NO_MEM;
	}
	
	if(page_insert(env_instance->env_pgdir, page_instance, va, perm) < 0) 
	{
		page_free(page_instance);

		return -E_NO_MEM;
	}
		
	memset(page2kva(page_instance), 0, PGSIZE);
		
	return 0;
}

// Map the page of memory at 'srcva' in srcenvid's address space
// at 'dstva' in dstenvid's address space with permission 'perm'.
// Perm has the same restrictions as in sys_page_alloc, except
// that it also must not grant write access to a read-only
// page.
//
// Return 0 on success, < 0 on error.  Errors are:
//	-E_BAD_ENV if srcenvid and/or dstenvid doesn't currently exist,
//		or the caller doesn't have permission to change one of them.
//	-E_INVAL if srcva >= UTOP or srcva is not page-aligned,
//		or dstva >= UTOP or dstva is not page-aligned.
//	-E_INVAL is srcva is not mapped in srcenvid's address space.
//	-E_INVAL if perm is inappropriate (see sys_page_alloc).
//	-E_INVAL if (perm & PTE_W), but srcva is read-only in srcenvid's
//		address space.
//	-E_NO_MEM if there's no memory to allocate any necessary page tables.
static int
sys_page_map(envid_t srcenvid, void *srcva,
	     envid_t dstenvid, void *dstva, int perm)
{
	// Hint: This function is a wrapper around page_lookup() and
	//   page_insert() from kern/pmap.c.
	//   Again, most of the new code you write should be to check the
	//   parameters for correctness.
	//   Use the third argument to page_lookup() to
	//   check the current permissions on the page.

	// LAB 4: Your code here.
	//panic("sys_page_map not implemented");
	struct Env *env_src;
	struct Env *env_dst;
	struct Page *p;
	pte_t *pte;
	
	if(envid2env(srcenvid, &env_src, 1)<0)
	{
		cprintf("Bad Env for Source!\n");
		return -E_BAD_ENV;
	}
	if(envid2env(dstenvid, &env_dst, 1)<0)
	{
		cprintf("Bad Env For Destination with ID: %08x!\n", dstenvid);
		return -E_BAD_ENV;
	}

	if((srcva>=(void *)UTOP) || (srcva!=ROUNDUP(srcva, PGSIZE)))
	{
		return -E_INVAL;
	}
	
	if((dstva>=(void *)UTOP) || (dstva!=ROUNDUP(dstva, PGSIZE)))
	{
		return -E_INVAL;
	}
	
	if((perm&PTE_U)==0 || (perm&PTE_P)==0)
	{
		return -E_INVAL;
	}
	
	if((perm&~PTE_USER) > 0)
	{
		return -E_INVAL;
	}
	
	p=page_lookup(env_src->env_pgdir, srcva, &pte);
	
	if(p == NULL)
	{
		return -E_INVAL;
	}

	if((perm & PTE_W) > 0 && (*pte & PTE_W) == 0)
	{
		return -E_INVAL;
	}
	
	if(page_insert(env_dst->env_pgdir, p, dstva, perm) < 0)
	{
		return -E_NO_MEM;
	}
	
	return 0;
}

// Unmap the page of memory at 'va' in the address space of 'envid'.
// If no page is mapped, the function silently succeeds.
//
// Return 0 on success, < 0 on error.  Errors are:
//	-E_BAD_ENV if environment envid doesn't currently exist,
//		or the caller doesn't have permission to change envid.
//	-E_INVAL if va >= UTOP, or va is not page-aligned.
static int
sys_page_unmap(envid_t envid, void *va)
{
	// Hint: This function is a wrapper around page_remove().

	// LAB 4: Your code here.
	//panic("sys_page_unmap not implemented");
	struct Env *env_instance;
	 
	if(envid2env(envid, &env_instance, 1) < 0)
	{
		return -E_BAD_ENV;
	}

	if(va >= (void *)UTOP)
	{
		return -E_INVAL;
	}

	if(va != ROUNDUP(va, PGSIZE))
	{
		return -E_INVAL;
	}
	
	page_remove(env_instance->env_pgdir, va);
	
	return 0;
}

// Try to send 'value' to the target env 'envid'.
// If srcva < UTOP, then also send page currently mapped at 'srcva',
// so that receiver gets a duplicate mapping of the same page.
//
// The send fails with a return value of -E_IPC_NOT_RECV if the
// target is not blocked, waiting for an IPC.
//
// The send also can fail for the other reasons listed below.
//
// Otherwise, the send succeeds, and the target's ipc fields are
// updated as follows:
//    env_ipc_recving is set to 0 to block future sends;
//    env_ipc_from is set to the sending envid;
//    env_ipc_value is set to the 'value' parameter;
//    env_ipc_perm is set to 'perm' if a page was transferred, 0 otherwise.
// The target environment is marked runnable again, returning 0
// from the paused sys_ipc_recv system call.  (Hint: does the
// sys_ipc_recv function ever actually return?)
//
// If the sender wants to send a page but the receiver isn't asking for one,
// then no page mapping is transferred, but no error occurs.
// The ipc only happens when no errors occur.
//
// Returns 0 on success, < 0 on error.
// Errors are:
//	-E_BAD_ENV if environment envid doesn't currently exist.
//		(No need to check permissions.)
//	-E_IPC_NOT_RECV if envid is not currently blocked in sys_ipc_recv,
//		or another environment managed to send first.
//	-E_INVAL if srcva < UTOP but srcva is not page-aligned.
//	-E_INVAL if srcva < UTOP and perm is inappropriate
//		(see sys_page_alloc).
//	-E_INVAL if srcva < UTOP but srcva is not mapped in the caller's
//		address space.
//	-E_INVAL if (perm & PTE_W), but srcva is read-only in the
//		current environment's address space.
//	-E_NO_MEM if there's not enough memory to map srcva in envid's
//		address space.
static int
sys_ipc_try_send(envid_t envid, uint32_t value, void *srcva, unsigned perm)
{
	// LAB 4: Your code here.
	//panic("sys_ipc_try_send not implemented");
	struct Env *env_instance;
	struct Page *p;
	pte_t *pte;

	if(envid2env(envid, &env_instance, 0) < 0)
	{
		return -E_BAD_ENV;
	}

	if(!env_instance->env_ipc_recving || env_instance->env_ipc_from!=0)
	{
		return -E_IPC_NOT_RECV;
	}

	if(srcva<(void *)UTOP && srcva!=ROUNDDOWN(srcva, PGSIZE))
	{
		return -E_INVAL;
	}

	if(srcva < (void *)UTOP)
	{
		if((perm&PTE_U)==0 || (perm&PTE_P)==0)
		{
			return -E_INVAL;
		}

		if((perm&~PTE_USER) > 0)
		{
			return -E_INVAL;
		}
	}
	
	if(srcva<(void *)UTOP && (p=page_lookup(curenv->env_pgdir, srcva, &pte))==NULL)
	{
		return -E_INVAL;
	}
	
	if(srcva<(void *)UTOP && (*pte&PTE_W)==0 && (perm&PTE_W)>0)
	{
		return -E_INVAL;
	}
	
	if(srcva<(void *)UTOP && env_instance->env_ipc_dstva!=0) 
	{
		if(page_insert(env_instance->env_pgdir, p, env_instance->env_ipc_dstva, perm) < 0)
		{
			return -E_NO_MEM;
		}
		
		env_instance->env_ipc_perm=perm;
	}
	
	env_instance->env_ipc_from=curenv->env_id;
	env_instance->env_ipc_value=value;
	env_instance->env_ipc_recving=0;
	env_instance->env_tf.tf_regs.reg_eax=0;	
	env_instance->env_status=ENV_RUNNABLE;
		
	return 0;
}

// Block until a value is ready.  Record that you want to receive
// using the env_ipc_recving and env_ipc_dstva fields of struct Env,
// mark yourself not runnable, and then give up the CPU.
//
// If 'dstva' is < UTOP, then you are willing to receive a page of data.
// 'dstva' is the virtual address at which the sent page should be mapped.
//
// This function only returns on error, but the system call will eventually
// return 0 on success.
// Return < 0 on error.  Errors are:
//	-E_INVAL if dstva < UTOP but dstva is not page-aligned.
static int
sys_ipc_recv(void *dstva)
{
	// LAB 4: Your code here.
	//panic("sys_ipc_recv not implemented");
	if(dstva<(void *)UTOP && ROUNDDOWN(dstva, PGSIZE)!=dstva)
	{
		return -E_INVAL;
	}
	else
	{	
		curenv->env_ipc_dstva=dstva;
		curenv->env_ipc_from=0;
		curenv->env_ipc_recving=1;
		curenv->env_status=ENV_NOT_RUNNABLE;

		sys_yield ();

		return 0;
	}
}


static int
sys_env_set_priority(envid_t env_id, uint32_t env_priority)
{
	struct Env *env_instance;
	
	if(env_priority > 5 || env_priority < 1)
	{
		return -E_INVAL;
	}
				
	if(envid2env(env_id, &env_instance, 1) < 0)
	{
		return -E_BAD_ENV;
	}

	env_instance->env_priority=env_priority;
						
	return 0;
}

// Return the current time.
static int
sys_time_msec(void) 
{
	// LAB 6: Your code here.
	return time_msec();
}

static int 
sys_net_try_send(const char *data, uint32_t len)
{
	return e100_send_msg(data, len);
}

static int
sys_net_try_recv(char *data)
{
	return e100_recv_msg(data);
}

// Dispatches to the correct kernel function, passing the arguments.
int32_t
syscall(uint32_t syscallno, uint32_t a1, uint32_t a2, uint32_t a3, uint32_t a4, uint32_t a5)
{
	// Call the function corresponding to the 'syscallno' parameter.
	// Return any appropriate return value.
	// LAB 3: Your code here.
	int32_t result;

	if(syscallno == SYS_cputs)
	{
		sys_cputs((const char*)a1, (size_t)a2);
		result=0;
	}
	else if(syscallno == SYS_cgetc)
	{
		result=sys_cgetc();
	}
	else if(syscallno == SYS_getenvid)
	{
		result=sys_getenvid();
	}
	else if(syscallno == SYS_env_destroy)
	{
		result=sys_env_destroy((envid_t)a1);
	}
	else if(syscallno == SYS_yield)
	{
		sys_yield();
		result=0;
	}
	else if(syscallno == SYS_exofork)
	{
		result=sys_exofork();
	}
	else if(syscallno == SYS_env_set_status)
	{
		result=sys_env_set_status(a1, a2);
	}
	else if(syscallno == SYS_env_set_pgfault_upcall)
	{
		result=sys_env_set_pgfault_upcall(a1, (void *)a2);
	}
	else if(syscallno == SYS_page_alloc)
	{
		result=sys_page_alloc(a1, (void *)a2, a3);
	}
	else if(syscallno == SYS_page_map)
	{
		result=sys_page_map(a1, (void *)a2, a3, (void *)a4, a5);
	}
	else if(syscallno == SYS_page_unmap)
	{
		result=sys_page_unmap(a1, (void *)a2);
	}
	else if(syscallno == SYS_env_set_priority)
	{
		result=sys_env_set_priority(a1, a2);
	}
	else if(syscallno == SYS_ipc_try_send)
	{
		result=sys_ipc_try_send(a1, a2, (void *) a3, a4);
	}
	else if(syscallno == SYS_ipc_recv)
	{
		result=sys_ipc_recv((void *) a1);
	}
	else if(syscallno == SYS_env_set_trapframe)
	{
		result=sys_env_set_trapframe(a1, (struct Trapframe *)a2);
	}
	else if(syscallno == SYS_time_msec)
	{
		result=sys_time_msec();
	}
	else if(syscallno == SYS_net_try_send)
	{
		result=sys_net_try_send((char *)a1, (uint32_t)a2);
	}
	else if(syscallno == SYS_net_try_recv)
	{
		result=sys_net_try_recv((char *)a1);
	}
	else if(syscallno == SYS_my_env_set_status)
	{
		result=sys_my_env_set_status(a1, a2);
	}
	else if(syscallno == SYS_my_env_copyfrom)
	{
		result=sys_my_env_copyfrom(a1, (envid_t *)a2);
	}
	else if(syscallno == SYS_my_env_establish)
	{
		result=sys_my_env_establish((envid_t *)a1, (int)a2);
	}
	else if(syscallno == SYS_my_env_copy_trapframe)
	{
		result=sys_my_env_copy_trapframe(a1, (struct Trapframe*)a2);
	}
	else if(syscallno == SYS_my_check_page)
	{
		result=sys_my_check_page(a1, (uint8_t *)a2, (void *)a3, (int *)a4);
	}
	else if(syscallno == SYS_my_insert_page)
	{
		result=sys_my_insert_page(a1, (uint8_t *)a2, (void *)a3, (int)a4);
	}
	else if(syscallno == SYS_my_check_page_simple)
	{
		result=sys_my_check_page_simple(a1, (uint8_t *)a2, (int *)a3);
	}
	else if(syscallno == SYS_my_insert_pte_only)
	{
		result=sys_my_insert_pte_only(a1, (uint8_t *)a2);
	}
	else if(syscallno == SYS_my_check_env_pgfault)
	{
		result=sys_my_check_env_pgfault((int *)a1, (envid_t *)a2, (uint32_t *)a3);
	}
	else if(syscallno == SYS_my_env_set_ip_port)
	{
		result=sys_my_env_set_ip_port(a1, (char *)a2, (int)a3);
	}
	else if(syscallno == SYS_my_set_env_pgfault)
	{
		result=sys_my_set_env_pgfault();
	}
	else if(syscallno == SYS_my_env_destroy)
	{
		result=sys_my_env_destroy(a1);
	}
	else
	{
		result=-E_INVAL;
	}
	
	return result;
	//panic("syscall not implemented");
}

