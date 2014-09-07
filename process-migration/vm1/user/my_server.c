#include <inc/lib.h>

void
umain(int argc, char **argv)
{
	int sockfd;
	int n;
	struct sockaddr_in servaddr;
	struct sockaddr_in cliaddr;
	socklen_t len;
	char buffer[1024];
	memset(buffer, 0, 1024);

	sockfd=socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);	
	if(sockfd <=  0)
	{
		cprintf("socket error!\n");
		return;
	}
	cprintf("USER: %d\n", sockfd);

	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family=AF_INET;
	servaddr.sin_addr.s_addr=htonl(INADDR_ANY);
	servaddr.sin_port=htons(50001);
	if(bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
	{
		cprintf("bind error!\n");
		return;
	}

	while((n=recvfrom(sockfd, buffer, 1024, 0, (struct sockaddr *)&cliaddr, &len)) > 0)
		cprintf("RECV %s, %d\n", buffer, n);
}
