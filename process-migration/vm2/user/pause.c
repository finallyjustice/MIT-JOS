#include <inc/lib.h>
#include <inc/env.h>

void
umain(int argc, char **argv)
{
	envid_t env_id=strtol(argv[1], 0, 0);
	
	sys_my_env_set_status(env_id, ENV_NOT_RUNNABLE);
	cprintf("Process %d is paused.\n", env_id);
}
