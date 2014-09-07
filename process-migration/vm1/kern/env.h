/* See COPYRIGHT for copyright information. */

#ifndef JOS_KERN_ENV_H
#define JOS_KERN_ENV_H

#include <inc/env.h>

#ifndef JOS_MULTIENV
// Change this value to 1 once you're allowing multiple environments
// (for UCLA: Lab 3, Part 3; for MIT: Lab 4).
#define JOS_MULTIENV 0
#endif

extern struct Env *envs;		// All environments
extern struct Env *curenv;		// Current environment

extern struct Env_PGfault_Msg epm;

LIST_HEAD(Env_list, Env);		// Declares 'struct Env_list'

void	env_init(void);
int	env_alloc(struct Env **e, envid_t parent_id);
void	env_free(struct Env *e);
void	env_create(uint8_t *binary, size_t size);
void	env_destroy(struct Env *e);	// Does not return if e == curenv

int	envid2env(envid_t envid, struct Env **env_store, bool checkperm);
int my_envid2env(envid_t envid, struct Env **env_store);
// The following two functions do not return
void	env_run(struct Env *e) __attribute__((noreturn));
void	env_pop_tf(struct Trapframe *tf) __attribute__((noreturn));

int env_copyfrom(envid_t src_envid, envid_t *dst_envid);
int env_establish(envid_t *env_id, int env_type);
int env_copy_trapframe(envid_t env_id, struct Trapframe *tf);
void my_env_handle_pgfault(envid_t env_id, uint32_t fault_va);

// For the grading script
#define ENV_CREATE2(start, size)	{		\
	extern uint8_t start[], size[];			\
	env_create(start, (int)size);			\
}

#define ENV_CREATE(x)			{		\
	extern uint8_t _binary_obj_##x##_start[],	\
		_binary_obj_##x##_size[];		\
	env_create(_binary_obj_##x##_start,		\
		(int)_binary_obj_##x##_size);		\
}

#endif // !JOS_KERN_ENV_H
