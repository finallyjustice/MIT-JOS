#include <inc/lib.h>

void
sleep(int sec)
{
	unsigned now = sys_time_msec();
	unsigned end = now + sec * 1000;
			
	if ((int)now < 0 && (int)now >= -MAXERROR)
		panic("sys_time_msec: %e", (int)now);
	if (end < now)
		panic("sleep: wrap");
				
	while (sys_time_msec() < end)
		sys_yield();
}

void
umain(void)
{
	envid_t my_id=sys_getenvid();
	cprintf("Initialize Test Env: %d\n", my_id);
	uint32_t counter=0;
	while(1)
	{
		my_id=sys_getenvid();
		cprintf("Test Program,  Env ID: %d, Counter: %u\n$ ", my_id, counter);
		sleep(5);

		counter++;
		if(counter > 60000)
		{
			counter=0;
		}
	}
}
