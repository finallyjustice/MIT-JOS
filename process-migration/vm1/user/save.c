#include <inc/lib.h>
#include <lwip/sockets.h>
#include <lwip/inet.h>

int
umain(int argc, char **argv)
{
	char buffer[1500];
	struct Env_Header eh;
	int count;
	int fd;

	envid_t src_envid=strtol(argv[1], 0, 0);
	int src_env_index=0;

	for(count=0; count<NENV; count++)
	{
		if(envs[count].env_id == src_envid)
		{
			src_env_index=count;
			break;
		}
	}
						     
	if(envs[count].env_status == ENV_RUNNABLE)
	{
		cprintf("You cannot create image for running process. Pause it first.\n");
		return -1;
	}

	fd=open(argv[2], O_RDWR|O_CREAT);

	memset(&eh, 0, sizeof(struct Env_Header));
	eh.env_from_id=src_envid;
	eh.env_tf=envs[src_env_index].env_tf;

	memset(buffer, 0, 1500);
	memcpy(buffer, (void *)&eh, sizeof(struct Env_Header));
	write(fd, buffer, sizeof(struct Env_Header));

	uint8_t *addr;
	void *temp_mem;
	temp_mem=malloc(PGSIZE);

	struct Migration_Env_Normal *me=(struct Migration_Env_Normal *)malloc(sizeof(struct Migration_Env_Normal));
	int status;

	for(addr=(uint8_t*)UTEXT; addr<(uint8_t*)0xeebfe000; addr=addr+PGSIZE)
	{
		memset(temp_mem, 0, PGSIZE);
		sys_my_check_page(src_envid, addr, temp_mem, &status);
		if(status == 1)
		{
			memset(me, 0, sizeof(struct Migration_Env_Normal));
			me->m_env_va=(uint32_t)addr;
			me->m_env_sequence=1;
			memmove(me->m_env_mem, (char *)(temp_mem+1024*0), 1024);
			me->m_status=1;
			memset(buffer, 0, 1500);
			memcpy(buffer, (void *)me, sizeof(struct Migration_Env_Normal));
			write(fd, buffer, sizeof(struct Migration_Env_Normal));
			cprintf("Save Page: 0x%08x, Sequence: %d\n", (uint32_t)addr, me->m_env_sequence);
			sys_yield();
			sys_yield();
			sys_yield();
			sys_yield();
			sys_yield();
			sys_yield();

			memset(me, 0, sizeof(struct Migration_Env_Normal));
			me->m_env_va=(uint32_t)addr;
			me->m_env_sequence=2;
			memmove(me->m_env_mem, (char *)(temp_mem+1024*1), 1024);
			me->m_status=1;
			memset(buffer, 0, 1500);
			memcpy(buffer, (void *)me, sizeof(struct Migration_Env_Normal));
			write(fd, buffer, sizeof(struct Migration_Env_Normal));
			cprintf("Save Page: 0x%08x, Sequence: %d\n", (uint32_t)addr, me->m_env_sequence);
			sys_yield();
			sys_yield();
			sys_yield();
			sys_yield();
			sys_yield();
			sys_yield();
			sys_yield();

			memset(me, 0, sizeof(struct Migration_Env_Normal));
			me->m_env_va=(uint32_t)addr;
			me->m_env_sequence=3;
			memmove(me->m_env_mem, (char *)(temp_mem+1024*2), 1024);
			me->m_status=1;
			memset(buffer, 0, 1500);
			memcpy(buffer, (void *)me, sizeof(struct Migration_Env_Normal));
			write(fd, buffer, sizeof(struct Migration_Env_Normal));
			cprintf("Save Page: 0x%08x, Sequence: %d\n", (uint32_t)addr, me->m_env_sequence);
			sys_yield();
			sys_yield();
			sys_yield();
			sys_yield();
			sys_yield();
			sys_yield();

			memset(me, 0, sizeof(struct Migration_Env_Normal));
			me->m_env_va=(uint32_t)addr;
			me->m_env_sequence=4;
			memmove(me->m_env_mem, (char *)(temp_mem+1024*3), 1024);
			me->m_status=1;
			memset(buffer, 0, 1500);
			memcpy(buffer, (void *)me, sizeof(struct Migration_Env_Normal));
			write(fd, buffer, sizeof(struct Migration_Env_Normal));
			cprintf("Save Page: 0x%08x, Sequence: %d\n", (uint32_t)addr, me->m_env_sequence);
			sys_yield();
			sys_yield();
			sys_yield();
			sys_yield();
			sys_yield();
			sys_yield();
		}
	}

	memset(me, 0, sizeof(struct Migration_Env_Normal));
	me->m_status=0;
	memset(buffer, 0, 1500);
	memcpy(buffer, (void *)me, sizeof(struct Migration_Env_Normal));
	write(fd, buffer, sizeof(struct Migration_Env_Normal));

	cprintf("You have successfully saved Env %d to %s\n", src_envid, argv[2]);

	close(fd);
	free(me);

	return 0;
}
