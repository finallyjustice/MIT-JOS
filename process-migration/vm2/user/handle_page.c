// hello, world
#include <inc/lib.h>

int
umain(void)
{
	cprintf("Initialize Page Fault Handler Process!\n");

	int status=0;
	envid_t env_id;
	uint32_t addr;

	int count;
	int env_index=0;

	char *buffer=(char *)malloc(1500);
	
	int sockfd;
	struct sockaddr_in servaddr;

	void *temp_mem=malloc(PGSIZE);

	struct Migration_Page_Request *mp_req=(struct Migration_Page_Request *)malloc(sizeof(struct Migration_Page_Request));
	struct Migration_Page_Respond *mp_rep=(struct Migration_Page_Respond *)malloc(sizeof(struct Migration_Page_Respond));

	while(1)
	{
		status=0;
		sys_my_check_env_pgfault(&status, &env_id, &addr);

		if(status == 1)
		{
			for(count=0; count<NENV; count++)
			{
				if(envs[count].env_id == env_id)
				{
					env_index=count;
					break;
				}
			}

			cprintf("Remote Host: %s:%d, Env: %d\n", envs[env_index].env_ip_from, envs[env_index].env_port_from, env_id);

			if((sockfd=socket(PF_INET,SOCK_STREAM, IPPROTO_UDP)) < 0)
			{
				cprintf("socket error\n");
				return -1;
			}
					                        
			memset(&servaddr, 0, sizeof(servaddr));
			servaddr.sin_family=AF_INET;
			servaddr.sin_addr.s_addr=inet_addr((char *)(envs[env_index].env_ip_from));
			servaddr.sin_port=htons(envs[env_index].env_port_from);
				                          
			if(connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
			{
				cprintf("connect error\n");
				return -1;
			}

			memset(mp_req, 0, sizeof(struct Migration_Page_Request));
			memset(buffer, 0, 1500);

			mp_req->m_env_va=addr;

			memcpy(buffer, mp_req, sizeof(struct Migration_Page_Request));
			write(sockfd, buffer, sizeof(struct Migration_Page_Request));
			
			memset(buffer, 0, 1500);
			memset(mp_rep, 0, sizeof(struct Migration_Page_Respond));
			read(sockfd, buffer, sizeof(struct Migration_Page_Respond));
			memcpy(mp_rep, buffer, sizeof(struct Migration_Page_Respond));
			memcpy(temp_mem+1024*0, mp_rep->m_env_mem, 1024);
			cprintf("Recv Page: 0x%08x, Sequence: %d\n", mp_rep->m_env_va, mp_rep->m_env_sequence);

			sys_yield();
			sys_yield();
			sys_yield();

			memset(buffer, 0, 1500);
			memset(mp_rep, 0, sizeof(struct Migration_Page_Respond));
			read(sockfd, buffer, sizeof(struct Migration_Page_Respond));
			memcpy(mp_rep, buffer, sizeof(struct Migration_Page_Respond));
			memcpy(temp_mem+1024*1, mp_rep->m_env_mem, 1024);
			cprintf("Recv Page: 0x%08x, Sequence: %d\n", mp_rep->m_env_va, mp_rep->m_env_sequence);

			sys_yield();
			sys_yield();
			sys_yield();

			memset(buffer, 0, 1500);
			memset(mp_rep, 0, sizeof(struct Migration_Page_Respond));
			read(sockfd, buffer, sizeof(struct Migration_Page_Respond));
			memcpy(mp_rep, buffer, sizeof(struct Migration_Page_Respond));
			memcpy(temp_mem+1024*2, mp_rep->m_env_mem, 1024);
			cprintf("Recv Page: 0x%08x, Sequence: %d\n", mp_rep->m_env_va, mp_rep->m_env_sequence);

			sys_yield();
			sys_yield();
			sys_yield();

			memset(buffer, 0, 1500);
			memset(mp_rep, 0, sizeof(struct Migration_Page_Respond));
			read(sockfd, buffer, sizeof(struct Migration_Page_Respond));
			memcpy(mp_rep, buffer, sizeof(struct Migration_Page_Respond));
			memcpy(temp_mem+1024*3, mp_rep->m_env_mem, 1024);
			cprintf("Recv Page: 0x%08x, Sequence: %d\n", mp_rep->m_env_va, mp_rep->m_env_sequence);

			sys_yield();
			sys_yield();
			sys_yield();

			close(sockfd);

			sys_my_insert_page(env_id, (uint8_t *)mp_rep->m_env_va, temp_mem, PTE_W|PTE_U|PTE_P);

			sys_my_set_env_pgfault();
			sys_my_env_set_status(env_id, ENV_RUNNABLE);
		}

		sys_yield();
	}

	return 0;
}
