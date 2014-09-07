// System call stubs.

#include <inc/syscall.h>
#include <inc/lib.h>

static inline int32_t
syscall(int num, int check, uint32_t a1, uint32_t a2, uint32_t a3, uint32_t a4, uint32_t a5)
{
	int32_t ret;

	// Generic system call: pass system call number in AX,
	// up to five parameters in DX, CX, BX, DI, SI.
	// Interrupt kernel with T_SYSCALL.
	//
	// The "volatile" tells the assembler not to optimize
	// this instruction away just because we don't use the
	// return value.
	// 
	// The last clause tells the assembler that this can
	// potentially change the condition codes and arbitrary
	// memory locations.

	asm volatile("int %1\n"
		: "=a" (ret)
		: "i" (T_SYSCALL),
		  "a" (num),
		  "d" (a1),
		  "c" (a2),
		  "b" (a3),
		  "D" (a4),
		  "S" (a5)
		: "cc", "memory");
	
	if(check && ret > 0)
		panic("syscall %d returned %d (> 0)", num, ret);

	return ret;
}

void
sys_cputs(const char *s, size_t len)
{
	syscall(SYS_cputs, 0, (uint32_t)s, len, 0, 0, 0);
}

int
sys_cgetc(void)
{
	return syscall(SYS_cgetc, 0, 0, 0, 0, 0, 0);
}

int
sys_env_destroy(envid_t envid)
{
	return syscall(SYS_env_destroy, 1, envid, 0, 0, 0, 0);
}

envid_t
sys_getenvid(void)
{
	 return syscall(SYS_getenvid, 0, 0, 0, 0, 0, 0);
}

void
sys_yield(void)
{
	syscall(SYS_yield, 0, 0, 0, 0, 0, 0);
}

int
sys_page_alloc(envid_t envid, void *va, int perm)
{
	return syscall(SYS_page_alloc, 1, envid, (uint32_t) va, perm, 0, 0);
}

int
sys_page_map(envid_t srcenv, void *srcva, envid_t dstenv, void *dstva, int perm)
{
	return syscall(SYS_page_map, 1, srcenv, (uint32_t) srcva, dstenv, (uint32_t) dstva, perm);
}

int
sys_page_unmap(envid_t envid, void *va)
{
	return syscall(SYS_page_unmap, 1, envid, (uint32_t) va, 0, 0, 0);
}

// sys_exofork is inlined in lib.h

int
sys_env_set_status(envid_t envid, int status)
{
	return syscall(SYS_env_set_status, 1, envid, status, 0, 0, 0);
}

int
sys_env_set_trapframe(envid_t envid, struct Trapframe *tf)
{
	return syscall(SYS_env_set_trapframe, 1, envid, (uint32_t) tf, 0, 0, 0);
}

int
sys_env_set_pgfault_upcall(envid_t envid, void *upcall)
{
	return syscall(SYS_env_set_pgfault_upcall, 1, envid, (uint32_t) upcall, 0, 0, 0);
}

int
sys_ipc_try_send(envid_t envid, uint32_t value, void *srcva, int perm)
{
	return syscall(SYS_ipc_try_send, 0, envid, value, (uint32_t) srcva, perm, 0);
}

int
sys_ipc_recv(void *dstva)
{
	return syscall(SYS_ipc_recv, 1, (uint32_t)dstva, 0, 0, 0, 0);
}

unsigned int
sys_time_msec(void)
{
	return (unsigned int) syscall(SYS_time_msec, 0, 0, 0, 0, 0, 0);
}

int
sys_env_set_priority(envid_t envid, uint32_t priority)
{
	return syscall(SYS_env_set_priority, 0, envid, priority, 0, 0, 0);
}

int 
sys_net_try_send(char *data, uint32_t len)
{
	//cprintf("TEST: %s, %08x, %d\n", data, data, len);
	return syscall(SYS_net_try_send, 0, (uint32_t)data, (uint32_t)len, 0, 0, 0);
}

int 
sys_net_try_recv(char *data)
{
	return syscall(SYS_net_try_recv, 0, (uint32_t)data, 0, 0, 0, 0);
}

int
sys_my_env_set_status(envid_t envid, int status)
{
	return syscall(SYS_my_env_set_status, 1, envid, status, 0, 0, 0);
}

int
sys_my_env_copyfrom(envid_t src_envid, envid_t *dst_envid)
{
	return syscall(SYS_my_env_copyfrom, 1, src_envid, (uint32_t)dst_envid, 0, 0, 0);
}

int 
sys_my_env_establish(envid_t *env_id, int env_type)
{
	return syscall(SYS_my_env_establish, 1, (uint32_t)env_id, (uint32_t)env_type, 0, 0, 0);
}

int
sys_my_env_copy_trapframe(envid_t env_id, struct Trapframe *tf)
{
	return syscall(SYS_my_env_copy_trapframe, 1, (uint32_t)env_id, (uint32_t)tf, 0, 0, 0);
}

int
sys_my_check_page(envid_t env_id, uint8_t *addr, void *temp_mem, int *status)
{
	return syscall(SYS_my_check_page, 1, (uint32_t)env_id, (uint32_t)addr, (uint32_t)temp_mem, (uint32_t)status, 0);
}

int 
sys_my_insert_page(envid_t env_id, uint8_t *addr, void *temp_mem, int perm)
{
	return syscall(SYS_my_insert_page, 1, (uint32_t)env_id, (uint32_t)addr, (uint32_t)temp_mem, (uint32_t)perm, 0);
}

int
sys_my_check_page_simple(envid_t env_id, uint8_t *addr, int *status)
{
	return syscall(SYS_my_check_page_simple, 1, (uint32_t)env_id, (uint32_t)addr, (uint32_t)status, 0, 0);
}

int
sys_my_insert_pte_only(envid_t env_id, uint8_t *addr)
{
	return syscall(SYS_my_insert_pte_only, 1, (uint32_t)env_id, (uint32_t)addr, 0, 0, 0);
}

int
sys_my_check_env_pgfault(int *status, envid_t *env_id, uint32_t *addr)
{
	return syscall(SYS_my_check_env_pgfault, 1, (uint32_t)status, (uint32_t)env_id, (uint32_t)addr, 0, 0);
}

int
sys_my_env_set_ip_port(envid_t env_id, char *ip_addr, int port)
{
	return syscall(SYS_my_env_set_ip_port, 1, (uint32_t)env_id, (uint32_t)ip_addr, (uint32_t)port, 0, 0);
}

int
sys_my_set_env_pgfault()
{
	return syscall(SYS_my_set_env_pgfault, 1, 0, 0, 0, 0, 0);
}

int
sys_my_env_destroy(envid_t env_id)
{
	return syscall(SYS_my_env_destroy, 1, env_id, 0, 0, 0, 0);
}
