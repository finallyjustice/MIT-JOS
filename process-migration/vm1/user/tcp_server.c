#include <inc/lib.h>
#include <lwip/sockets.h>
#include <lwip/inet.h>

struct my_message_t{
	int message_id;
	int message_rank;
	char message[1024];
};

int
umain(void)
{
	int serversock;
	int clientsock;
	struct sockaddr_in server;
	struct sockaddr_in client;
	struct my_message_t msg;
	memset(&msg, 0, sizeof(struct my_message_t));

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
				
	cprintf("Waiting for connections...\n");

	unsigned int clientlen=sizeof(client);
	if((clientsock=accept(serversock, (struct sockaddr *)&client, &clientlen)) < 0)
	{
		cprintf("Failed to accept client connection\n");
		return -1;
	}
	
	int received;
	char buffer[1500];
	memset(buffer, 0, 1500);

	//while((received=read(clientsock, buffer, 1024)) > 0)
	//{
	//	cprintf("RECV: %s\n", buffer);
	//	memset(buffer, 0, 1024);
	//	close(clientsock);
	//}

	read(clientsock, buffer, 1500);
	memcpy((void *)&msg, buffer, sizeof(struct my_message_t));
	cprintf("ID		: %d\n", msg.message_id);
	cprintf("RANK	: %d\n", msg.message_rank);
	cprintf("Content: %s\n", msg.message);
	//cprintf("RECV: %s\n", buffer);
	//memset(buffer, 0, 1024);
	//read(clientsock, buffer, 1024);
	//cprintf("RECV: %s\n", buffer);
			
	close(serversock);

	return 0;
}
