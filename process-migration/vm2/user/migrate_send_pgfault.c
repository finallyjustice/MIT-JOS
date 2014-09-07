#include <inc/lib.h>
#include <lwip/sockets.h>
#include <lwip/inet.h>

int
umain(int argc, char **argv)
{
	if(argc != 6)
	{
		cprintf("migrate_send_pgfault: argument error!\n");
		return -1;
	}

	uint8_t *addr;
	int status;
	struct Migration_Env_PGfault *me=(struct Migration_Env_PGfault *)malloc(sizeof(struct Migration_Env_PGfault));

	int sockfd;
	struct sockaddr_in servaddr;
	char buffer[1500];
	struct Env_Header eh;
	int count;
	int src_env_index=0;

	void *temp_mem=malloc(PGSIZE);

	int serversock;
	int clientsock;
	struct sockaddr_in server;
	struct sockaddr_in client;

	envid_t src_envid=strtol(argv[1], 0, 0);

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

	for(count=0; count<NENV; count++)
	{
		if(envs[count].env_id == src_envid)
		{
			src_env_index=count;
			break;
		}
	}
	memset(&eh, 0, sizeof(struct Env_Header));
	strcpy(eh.env_ip_from, argv[4]);
	strcpy(eh.env_ip_to, argv[2]);
	eh.env_port_from=strtol(argv[5], 0, 0);;
	//eh.env_port_to=50001;
	eh.env_from_id=src_envid;
	eh.env_tf=envs[src_env_index].env_tf;

	memset(buffer, 0, 1500);
	memcpy(buffer, (void *)&eh, sizeof(struct Env_Header));
	write(sockfd, buffer, sizeof(struct Env_Header));

	memset(me, 0, sizeof(struct Migration_Env_PGfault));
	me->m_total=0;
	me->m_status=1;

	for(addr=(uint8_t*)UTEXT; addr<(uint8_t*)0xeebfe000; addr=addr+PGSIZE)
	{
		sys_my_check_page_simple(src_envid, addr, &status);
	
		if(status == 1)
		{
			me->m_env_vas[me->m_total]=(uint32_t)addr;
			cprintf("Send Page Table Entry: 0x%08x\n", me->m_env_vas[me->m_total]);
			me->m_total++;
		}
	}

	memset(buffer, 0, 1500);
	memcpy(buffer, (void *)me, sizeof(struct Migration_Env_PGfault));
	write(sockfd, buffer, sizeof(struct Migration_Env_PGfault));

	close(sockfd);
	
	sys_yield();
	sys_yield();
	sys_yield();
	sys_yield();
	sys_yield();
	sys_yield();

	if((serversock=socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
	{
		cprintf("Failed to create socket\n");
		return -1;
	}
				
	memset(&server, 0, sizeof(server));
	server.sin_family=AF_INET;
	server.sin_addr.s_addr=htonl(INADDR_ANY);
	server.sin_port=htons(50003);
					 
	if(bind(serversock, (struct sockaddr *)&server, sizeof(server)) < 0)
	{
		cprintf("Failed to bind the server socket\n");
		return -1;
	}
								
	if(listen(serversock, 10) < 0)
	{
		cprintf("Failed to listen on server socket\n");
		return -1;
	}
												
	cprintf("\nWaiting for connections... (port 50003)\n");
	cprintf("Waiting to handle page fault\n");

	unsigned int clientlen=sizeof(client);
	int received;

	struct Migration_Page_Request *mp_req=(struct Migration_Page_Request *)malloc(sizeof(struct Migration_Page_Request));
	struct Migration_Page_Respond *mp_rep=(struct Migration_Page_Respond *)malloc(sizeof(struct Migration_Page_Respond));

	while(1)
	{
		memset(&client, 0, sizeof(struct sockaddr_in));
		if((clientsock=accept(serversock, (struct sockaddr *)&client, &clientlen)) < 0)
		{
			cprintf("Failed to accept client connection\n");
			return -1;
		}
		
		memset(mp_req, 0, sizeof(struct Migration_Page_Request));
		memset(buffer, 0, 1500);

		read(clientsock, buffer, sizeof(struct Migration_Page_Request));
		memcpy(mp_req, buffer, sizeof(struct Migration_Page_Request));

		cprintf("\nRecv Request For Page: 0x%08x\n", mp_req->m_env_va);

		memset(temp_mem, 0, PGSIZE);
		status=0;
		sys_my_check_page(src_envid, (uint8_t *)(mp_req->m_env_va), temp_mem, &status);
		
		if(status == 0)
		{
			cprintf("Page Error!\n");
			return -1;
		}

		memset(buffer, 0, 1500);
		memset(mp_rep, 0, sizeof(struct Migration_Page_Respond));
		mp_rep->m_env_va=mp_req->m_env_va;
		mp_rep->m_env_sequence=1;
		memcpy(mp_rep->m_env_mem, temp_mem+1024*0, 1024);
		memcpy(buffer, mp_rep, sizeof(struct Migration_Page_Respond));
		write(clientsock, buffer, sizeof(struct Migration_Page_Respond));
		cprintf("Send Page: 0x%08x, Sequence: %d\n", mp_rep->m_env_va, mp_rep->m_env_sequence);	

		sys_yield();
		sys_yield();
		sys_yield();

		memset(buffer, 0, 1500);
		memset(mp_rep, 0, sizeof(struct Migration_Page_Respond));
		mp_rep->m_env_va=mp_req->m_env_va;
		mp_rep->m_env_sequence=2;
		memcpy(mp_rep->m_env_mem, temp_mem+1024*1, 1024);
		memcpy(buffer, mp_rep, sizeof(struct Migration_Page_Respond));
		write(clientsock, buffer, sizeof(struct Migration_Page_Respond));
		cprintf("Send Page: 0x%08x, Sequence: %d\n", mp_rep->m_env_va, mp_rep->m_env_sequence);

		sys_yield();
		sys_yield();
		sys_yield();

		memset(buffer, 0, 1500);
		memset(mp_rep, 0, sizeof(struct Migration_Page_Respond));
		mp_rep->m_env_va=mp_req->m_env_va;
		mp_rep->m_env_sequence=3;
		memcpy(mp_rep->m_env_mem, temp_mem+1024*2, 1024);
		memcpy(buffer, mp_rep, sizeof(struct Migration_Page_Respond));
		write(clientsock, buffer, sizeof(struct Migration_Page_Respond));
		cprintf("Send Page: 0x%08x, Sequence: %d\n", mp_rep->m_env_va, mp_rep->m_env_sequence);

		sys_yield();
		sys_yield();
		sys_yield();

		memset(buffer, 0, 1500);
		memset(mp_rep, 0, sizeof(struct Migration_Page_Respond));
		mp_rep->m_env_va=mp_req->m_env_va;
		mp_rep->m_env_sequence=4;
		memcpy(mp_rep->m_env_mem, temp_mem+1024*3, 1024);
		memcpy(buffer, mp_rep, sizeof(struct Migration_Page_Respond));
		write(clientsock, buffer, sizeof(struct Migration_Page_Respond));
		cprintf("Send Page: 0x%08x, Sequence: %d\n", mp_rep->m_env_va, mp_rep->m_env_sequence);

		close(clientsock);
	}
	close(serversock);

	return 0;
}
