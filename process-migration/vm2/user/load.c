#include <inc/lib.h>
#include <lwip/sockets.h>

int
umain(int argc, char **argv)
{

	struct Env_Header eh;
	memset(&eh, 0, sizeof(struct Env_Header));

	char buffer[1500];
	memset(buffer, 0, 1500);

	int fd;

	fd=open(argv[1], O_RDWR);

	read(fd, buffer, sizeof(struct Env_Header));
	memcpy((void *)&eh, buffer, sizeof(struct Env_Header));

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
		read(fd, buffer, sizeof(struct Migration_Env_Normal));

		memset(&me, 0, sizeof(struct Migration_Env_Normal));	
		memcpy(&me, (struct Migration_Env_Normal *)buffer, sizeof(struct Migration_Env_Normal));
		        
		if(me.m_status == 0)
		{   
			break;
		}
		
		memcpy(temp_mem, (void *)(me.m_env_mem), 1024);
		cprintf("Load Page: 0x%08x, sequence: %d \n", me.m_env_va, me.m_env_sequence);

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
		read(fd, buffer, sizeof(struct Migration_Env_Normal));
		         
		memset(&me, 0, sizeof(struct Migration_Env_Normal));   
		memcpy(&me, (struct Migration_Env_Normal *)buffer, sizeof(struct Migration_Env_Normal));
		                 
		memcpy(temp_mem+1024, (void *)(me.m_env_mem), 1024);
		cprintf("Load Page: 0x%08x, sequence: %d \n", me.m_env_va, me.m_env_sequence);				    

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
		read(fd, buffer, sizeof(struct Migration_Env_Normal));
		         
		memset(&me, 0, sizeof(struct Migration_Env_Normal));   
		memcpy(&me, (struct Migration_Env_Normal *)buffer, sizeof(struct Migration_Env_Normal));
						       
		memcpy(temp_mem+1024*2, (void *)(me.m_env_mem), 1024);
		cprintf("Load Page: 0x%08x, sequence: %d \n", me.m_env_va, me.m_env_sequence);				      

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
		read(fd, buffer, sizeof(struct Migration_Env_Normal));
		          
		memset(&me, 0, sizeof(struct Migration_Env_Normal));   
		memcpy(&me, (struct Migration_Env_Normal *)buffer, sizeof(struct Migration_Env_Normal));
						      
		memcpy(temp_mem+1024*3, (void *)(me.m_env_mem), 1024);
		cprintf("Load Page: 0x%08x, sequence: %d \n", me.m_env_va, me.m_env_sequence);				      

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


	close(fd);
	
	cprintf("Load Image Successful, You can resume New Env: %d\n", new_env_id);

	return 0;
}
