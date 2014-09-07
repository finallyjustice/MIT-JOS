// User-level IPC library routines

#include <inc/lib.h>

// Receive a value via IPC and return it.
// If 'pg' is nonnull, then any page sent by the sender will be mapped at
//	that address.
// If 'from_env_store' is nonnull, then store the IPC sender's envid in
//	*from_env_store.
// If 'perm_store' is nonnull, then store the IPC sender's page permission
//	in *perm_store (this is nonzero iff a page was successfully
//	transferred to 'pg').
// If the system call fails, then store 0 in *fromenv and *perm (if
//	they're nonnull) and return the error.
// Otherwise, return the value sent by the sender
//
// Hint:
//   Use 'env' to discover the value and who sent it.
//   If 'pg' is null, pass sys_ipc_recv a value that it will understand
//   as meaning "no page".  (Zero is not the right value, since that's
//   a perfectly valid place to map a page.)
int32_t
ipc_recv(envid_t *from_env_store, void *pg, int *perm_store)
{
	// LAB 4: Your code here.
	//panic("ipc_recv not implemented");
	struct Env *env_instance;
	int result;
	
	if(pg != NULL)
	{
		result=sys_ipc_recv(pg);
	}
	else
	{
		result=sys_ipc_recv((void *)UTOP);
	}
	
	env_instance=(struct Env*)envs+ENVX(sys_getenvid());
	
	if(from_env_store != NULL)
	{
		if(result >= 0)
		{
			*from_env_store=env_instance->env_ipc_from;
		}
		else
		{
			*from_env_store=0;
		}
	}

	if(perm_store != NULL)
	{
		if(result >= 0)
		{
			*perm_store=env_instance->env_ipc_perm;
		}
		else
		{
			*perm_store=0;
		}
	}
	
	if(result < 0)
	{
		return result;
	}
	else
	{
		return env_instance->env_ipc_value;
	}
}

// Send 'val' (and 'pg' with 'perm', if 'pg' is nonnull) to 'toenv'.
// This function keeps trying until it succeeds.
// It should panic() on any error other than -E_IPC_NOT_RECV.
//
// Hint:
//   Use sys_yield() to be CPU-friendly.
//   If 'pg' is null, pass sys_ipc_recv a value that it will understand
//   as meaning "no page".  (Zero is not the right value.)
void
ipc_send(envid_t to_env, uint32_t val, void *pg, int perm)
{
	// LAB 4: Your code here.
	//panic("ipc_send not implemented");
	int result;

	if(pg == NULL)
	{
		result=sys_ipc_try_send(to_env, val, (void *)UTOP, perm);
	}
	else
	{
		result=sys_ipc_try_send(to_env, val, pg, perm);
	}
	
	while(result < 0)
	{
		if(result != -E_IPC_NOT_RECV)
		{
			panic ("ipc send error!");
		}
		
		sys_yield ();

		if(pg == NULL)
		{
			result=sys_ipc_try_send(to_env, val, (void *)UTOP, perm);
		}
		else
		{
			result=sys_ipc_try_send(to_env, val, pg, perm);
		}
	}
}
