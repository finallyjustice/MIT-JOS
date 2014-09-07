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
	server.sin_port=htons(50002);          
		
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
				
	cprintf("Waiting for connections... (port 50002)\n");

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
	sys_my_env_establish(&new_env_id, ENV_TYPE_PGFAULT);

	cprintf("New Env Established: %d\n", new_env_id);
	sys_my_env_copy_trapframe(new_env_id, &(eh.env_tf));

	sys_my_env_set_ip_port(new_env_id, eh.env_ip_from, eh.env_port_from);

	struct Migration_Env_PGfault *me;
	me=(struct Migration_Env_PGfault *)malloc(sizeof(struct Migration_Env_PGfault));

	memset(buffer, 0, 1500);
	read(clientsock, buffer, sizeof(struct Migration_Env_PGfault));
	memcpy((void *)me, buffer, sizeof(struct Migration_Env_PGfault));

	int count;
	uint8_t *addr;

	for(count=0; count<me->m_total; count++)
	{
		addr=(uint8_t *)(me->m_env_vas[count]);
		sys_my_insert_pte_only(new_env_id, addr);
		cprintf("Recv Page Table Entry: 0x%08x\n", (uint32_t)addr);
	}

	close(clientsock);
	close(serversock);

	cprintf("Process Migration Finished, You can resume New Env: %d\n", new_env_id);

	free(me);

	return 0;
}
