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
	int sockfd;
	struct sockaddr_in servaddr;
	char buffer[1500];
	struct my_message_t msg;
	memset(&msg, 0, sizeof(struct my_message_t));
	msg.message_id=19;
	msg.message_rank=11;
	strcpy(msg.message, "I am message!");

	if((sockfd=socket(PF_INET,SOCK_STREAM, IPPROTO_UDP)) < 0)
	{
		cprintf("socket error\n");
		return -1;
	}

	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family=AF_INET;
	servaddr.sin_addr.s_addr=inet_addr("130.245.30.176");
	servaddr.sin_port=htons(26015);

	connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr));

	memset(buffer, 0, 1500);
	memcpy(buffer, (void *)&msg, sizeof(struct my_message_t));
	write(sockfd, buffer, 1500);

	return 0;
}
