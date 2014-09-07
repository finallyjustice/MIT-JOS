#ifndef JOS_INC_SYSCALL_H
#define JOS_INC_SYSCALL_H

/* system call numbers */
enum
{
	SYS_cputs = 0,
	SYS_cgetc,
	SYS_getenvid,
	SYS_env_destroy,
	SYS_page_alloc,
	SYS_page_map,
	SYS_page_unmap,
	SYS_exofork,
	SYS_env_set_status,
	SYS_my_env_set_status,
	SYS_env_set_trapframe,
	SYS_env_set_pgfault_upcall,
	SYS_yield,
	SYS_ipc_try_send,
	SYS_ipc_recv,
	SYS_time_msec,
	SYS_env_set_priority,
	SYS_net_try_send,
	SYS_net_try_recv,
	SYS_my_env_copyfrom,
	SYS_my_env_establish,
	SYS_my_env_copy_trapframe,
	SYS_my_check_page,
	SYS_my_insert_page,
	SYS_my_check_page_simple,
	SYS_my_insert_pte_only,
	SYS_my_check_env_pgfault,
	SYS_my_env_set_ip_port,
	SYS_my_set_env_pgfault,
	SYS_my_env_destroy,
	NSYSCALLS
};

#endif /* !JOS_INC_SYSCALL_H */
