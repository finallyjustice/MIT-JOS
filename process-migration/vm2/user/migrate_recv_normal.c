#include <inc/lib.h>
#include <lwip/sockets.h>
#include <lwip/inet.h>

int
umain(void)
{
	int serversock;
	int clientsock;
	struct sockaddr_in server;
	struct sockaddr_in client;

	struct Env_Header eh;
	memset(&eh, 0, sizeof(struct Env_Header));

	if((serversock=socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
	{
		cprintf("Failed to create socket\n");
		return -1;
	}
		 
	memset(&server, 0, sizeof(server));     
	server.sin_family=AF_INET;            
	server.sin_addr.s_addr=htonl(INADDR_ANY); 
	server.sin_port=htons(50001);          
		
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
				
	cprintf("Waiting for connections... (port 50001)\n");

	unsigned int clientlen=sizeof(client);
	if((clientsock=accept(serversock, (struct sockaddr *)&client, &clientlen)) < 0)
	{
		cprintf("Failed to accept client connection\n");
		return -1;
	}
	
	int received;
	char buffer[1500];
	memset(buffer, 0, 1500);

	read(clientsock, buffer, sizeof(struct Env_Header));
	memcpy((void *)&eh, buffer, sizeof(struct Env_Header));

	cprintf("Migration:\n");
	cprintf("   From:   %s\n", eh.env_ip_from);
	cprintf("   To:     %s\n", eh.env_ip_to);
	cprintf("   Env ID: %d\n", eh.env_from_id);

	envid_t new_env_id;
	sys_my_env_establish(&new_env_id, ENV_TYPE_NORMAL);

	cprintf("New Env Established: %d\n", new_env_id);
	sys_my_env_copy_trapframe(new_env_id, &(eh.env_tf));

	void *temp_mem;
	temp_mem=malloc(PGSIZE);

	uint8_t *addr;
	   
	struct Migration_Env_Normal me;

	while(1)
	{
		memset(temp_mem, 0, PGSIZE);

		memset(buffer, 0, 1500);
		read(clientsock, buffer, sizeof(struct Migration_Env_Normal));

		memset(&me, 0, sizeof(struct Migration_Env_Normal));	
		memcpy(&me, (struct Migration_Env_Normal *)buffer, sizeof(struct Migration_Env_Normal));
		        
		if(me.m_status == 0)
		{   
			break;
		}
		
		memcpy(temp_mem, (void *)(me.m_env_mem), 1024);
		cprintf("Recv Page: 0x%08x, sequence: %d \n", me.m_env_va, me.m_env_sequence);

		sys_yield();
		sys_yield();
		sys_yield();
		sys_yield();
		sys_yield();
		sys_yield();
		sys_yield();
		sys_yield();
		sys_yield();
		sys_yield();
		sys_yield();
		sys_yield();
		sys_yield();

		memset(buffer, 0, 1500);
		read(clientsock, buffer, sizeof(struct Migration_Env_Normal));
		         
		memset(&me, 0, sizeof(struct Migration_Env_Normal));   
		memcpy(&me, (struct Migration_Env_Normal *)buffer, sizeof(struct Migration_Env_Normal));
		                 
		memcpy(temp_mem+1024, (void *)(me.m_env_mem), 1024);
		cprintf("Recv Page: 0x%08x, sequence: %d \n", me.m_env_va, me.m_env_sequence);				    

		sys_yield();
		sys_yield();
		sys_yield();
		sys_yield();
		sys_yield();
		sys_yield();
		sys_yield();
		sys_yield();
		sys_yield();
		sys_yield();
		sys_yield();
		sys_yield();
		sys_yield();
		sys_yield();
		sys_yield();

		memset(buffer, 0, 1500);
		read(clientsock, buffer, sizeof(struct Migration_Env_Normal));
		         
		memset(&me, 0, sizeof(struct Migration_Env_Normal));   
		memcpy(&me, (struct Migration_Env_Normal *)buffer, sizeof(struct Migration_Env_Normal));
						       
		memcpy(temp_mem+1024*2, (void *)(me.m_env_mem), 1024);
		cprintf("Recv Page: 0x%08x, sequence: %d \n", me.m_env_va, me.m_env_sequence);				      

		sys_yield();
		sys_yield();
		sys_yield();
		sys_yield();
		sys_yield();
		sys_yield();
		sys_yield();
		sys_yield();
		sys_yield();
		sys_yield();
		sys_yield();
		sys_yield();
		sys_yield();
		sys_yield();
		sys_yield();

		memset(buffer, 0, 1500);
		read(clientsock, buffer, sizeof(struct Migration_Env_Normal));
		          
		memset(&me, 0, sizeof(struct Migration_Env_Normal));   
		memcpy(&me, (struct Migration_Env_Normal *)buffer, sizeof(struct Migration_Env_Normal));
						      
		memcpy(temp_mem+1024*3, (void *)(me.m_env_mem), 1024);
		cprintf("Recv Page: 0x%08x, sequence: %d \n", me.m_env_va, me.m_env_sequence);				      

		sys_yield();
		sys_yield();
		sys_yield();
		sys_yield();
		sys_yield();
		sys_yield();
		sys_yield();
		sys_yield();
		sys_yield();
		sys_yield();
		sys_yield();
		sys_yield();
		sys_yield();
		sys_yield();
		sys_yield();

		addr=(uint8_t *)(me.m_env_va);
		sys_my_insert_page(new_env_id, addr, (void *)temp_mem, PTE_W|PTE_P|PTE_U);
		
	}

	memset(buffer, 0, 1500);
	strcpy(buffer, "SUCCESS");
	write(clientsock, buffer, 1500);

	close(serversock);
	
	cprintf("Process Migration Finished, You can resume New Env: %d\n", new_env_id);

	return 0;
}
