#include <inc/lib.h>
#include <lwip/sockets.h>
#include <lwip/inet.h>

int
umain(int argc, char **argv)
{
	if(argc != 5)
	{
		cprintf("migrate_send_normal: argument error!\n");
		cprintf("migrate_send_normal \"env_id\" \"destination_ip\" \"destination_port\" \"local_ip\"\n");
		return -1;
	}

	int sockfd;
	struct sockaddr_in servaddr;

	char buffer[1500];
	struct Env_Header eh;
	int count;

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
		cprintf("You cannot migrate running process. Pause it first.\n");
		return -1;
	}

	if((sockfd=socket(PF_INET,SOCK_STREAM, IPPROTO_UDP)) < 0)
	{
		cprintf("socket error\n");
		return -1;
	}

	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family=AF_INET;
	servaddr.sin_addr.s_addr=inet_addr(argv[2]);
	servaddr.sin_port=htons(strtol(argv[3], 0, 0));

	if(connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
	{
		cprintf("connect error\n");
		return -1;
	}

	memset(&eh, 0, sizeof(struct Env_Header));
	strcpy(eh.env_ip_from, argv[4]);
	strcpy(eh.env_ip_to, argv[2]);
	//eh.env_port_from=50002;
	//eh.env_port_to=50001;
	eh.env_from_id=src_envid;
	eh.env_tf=envs[src_env_index].env_tf;

	memset(buffer, 0, 1500);
	memcpy(buffer, (void *)&eh, sizeof(struct Env_Header));
	write(sockfd, buffer, sizeof(struct Env_Header));

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
			write(sockfd, buffer, sizeof(struct Migration_Env_Normal));
			cprintf("Send Page: 0x%08x, Sequence: %d\n", (uint32_t)addr, me->m_env_sequence);
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
			write(sockfd, buffer, sizeof(struct Migration_Env_Normal));
			cprintf("Send Page: 0x%08x, Sequence: %d\n", (uint32_t)addr, me->m_env_sequence);
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
			write(sockfd, buffer, sizeof(struct Migration_Env_Normal));
			cprintf("Send Page: 0x%08x, Sequence: %d\n", (uint32_t)addr, me->m_env_sequence);
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
			write(sockfd, buffer, sizeof(struct Migration_Env_Normal));
			cprintf("Send Page: 0x%08x, Sequence: %d\n", (uint32_t)addr, me->m_env_sequence);
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
	write(sockfd, buffer, sizeof(struct Migration_Env_Normal));

	memset(buffer, 0, 1500);
	read(sockfd, buffer, 1500);
	cprintf("Feedback: %s\n", buffer);
	cprintf("You have successfully moved Env %d to %s\n", src_envid, argv[2]);
	
	close(sockfd);
	free(me);

	return 0;
}
