#include "ns.h"

extern union Nsipc nsipcbuf;

void
output(envid_t ns_envid)
{
	binaryname = "ns_output";

	// LAB 6: Your code here:
	// 	- read a packet from the network server
	//	- send the packet to the device driver

	uint32_t request;
	uint32_t recv_env_id;
	int perm;
	int result;

	while(1)
	{
		perm=0;
		request=ipc_recv((int32_t *)&recv_env_id, &nsipcbuf, &perm);

		if(!(perm&PTE_P))
		{
			panic("output(): Invalid request");
		}

		if(request != NSREQ_OUTPUT)
		{
			panic("output(): Invalid IPC Request");
		}

		while((result=sys_net_try_send(nsipcbuf.pkt.jp_data, nsipcbuf.pkt.jp_len)) != 0)
		{
			sys_page_unmap(0, &nsipcbuf);
		}
	}
}
