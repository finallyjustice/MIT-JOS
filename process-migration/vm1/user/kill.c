#include <inc/lib.h>
#include <inc/env.h>

void
umain(int argc, char **argv)
{
	envid_t env_id=strtol(argv[1], 0, 0);
	
	sys_my_env_destroy(env_id);
	cprintf("Process %d is killed.\n", env_id);
}
