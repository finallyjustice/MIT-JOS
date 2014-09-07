#include <inc/lib.h>
#include <inc/env.h>

void
umain(int argc, char **argv)
{
	envid_t src_envid=strtol(argv[1], 0, 0);
	envid_t dst_envid=0;
	
	if(sys_my_env_copyfrom(src_envid, &dst_envid) < 0)
	{
		cprintf("Clone New Env Failed\n");
		return;
	}

	cprintf("New Env Created: %d\n", dst_envid);
}
