#include <inc/lib.h>
#include <inc/env.h>

int 
umain(void)
{
	int count;

	for(count=0; count<NENV; count++)
	{
		if(envs[count].env_status != ENV_FREE)
		{
			cprintf("Process ID: %d   Parent ID: %d   ", envs[count].env_id, envs[count].env_parent_id, envs[count].env_status);
			cprintf("Status: ");

			if(envs[count].env_status == ENV_RUNNABLE)
			{
				cprintf("Running\n");
			}
			if(envs[count].env_status == ENV_NOT_RUNNABLE)
			{
				cprintf("Sleeping\n");
			}
		}
	}

	return 0;
}
