// LAB 6: Your driver code here

#include <inc/x86.h>
#include <inc/stdio.h>
#include <kern/e100.h>
#include <kern/pmap.h>	   
#include <inc/string.h>

int e100_attach_device(struct pci_func *pcif_instance)
{
	pci_func_enable(pcif_instance);

	e100_instance.bus=pcif_instance->bus;
	e100_instance.dev_id=pcif_instance->dev_id;
	e100_instance.dev_class=pcif_instance->dev_class;
			

	int count;
	for(count=0; count<6; count++)
	{
		e100_instance.reg_base[count]=pcif_instance->reg_base[count];
		e100_instance.reg_size[count]=pcif_instance->reg_size[count];
	}						 

	nic_instance.io_base=pcif_instance->reg_base[E100_REG_IO];
	nic_instance.io_size=pcif_instance->reg_size[E100_REG_IO];
	e100_instance.irq_line=pcif_instance->irq_line;

	e100_reset_device(e100_instance);
	e100_run_command(CSR_INT, 1);

	init_cb_list();
	init_rf_area();

	return 0;
}

void e100_reset_device(struct pci_func e100_instance)
{
	outl(e100_instance.reg_base[E100_REG_IO]+CSR_PORT, PORT_SOFT_RESET);
	
	delay();
	delay();
}

void alloc_cb_list()
{
	int result;
	struct Page *page;
	struct command_block *prev_cb=NULL;
	struct command_block *curr_cb=NULL;

	if(page_alloc(&page) != 0)
	{
		panic("alloc_cb_list: run out of memory!");
	}
			
	memset(page2kva(page), 0, PGSIZE);
	page->pp_ref++;
				
	curr_cb=(struct command_block *)page2kva(page);
	curr_cb->phy_addr=page2pa(page);
	nic_instance.cb_list.start=curr_cb;
	prev_cb=curr_cb;

	int count;
	for(count=1; count<CBL_MAXSIZE; count++)
	{
		if(page_alloc(&page) != 0)
		{
			panic("alloc_cb_list: run out of memory!");
		}

		memset(page2kva(page), 0, PGSIZE);
		page->pp_ref++;

		curr_cb=(struct command_block *)page2kva(page);
		curr_cb->phy_addr=page2pa(page);

		prev_cb->link=curr_cb->phy_addr;
		prev_cb->next_cb=curr_cb;
		curr_cb->prev_cb=prev_cb;

		prev_cb=curr_cb;
	}

	prev_cb->link=nic_instance.cb_list.start->phy_addr;
	nic_instance.cb_list.start->prev_cb=prev_cb;
	prev_cb->next_cb=nic_instance.cb_list.start;
	
	nic_instance.cb_list.available_command_block=CBL_MAXSIZE;
	nic_instance.cb_list.waiting_command_block=0;

	nic_instance.cb_list.front=nic_instance.cb_list.start;
	nic_instance.cb_list.rear=nic_instance.cb_list.start->prev_cb;
}

void e100_run_command(int csr_comp, uint8_t cmd)
{
	int command;

	outb(nic_instance.io_base+csr_comp, cmd);

	command=inb(nic_instance.io_base+CSR_COMMAND);
	while(command != 0)
	{
		command=inb(nic_instance.io_base+CSR_COMMAND);
	}
}

void e100_initialization()
{
	e100_reset_device(e100_instance);

	e100_run_command(CSR_INT, 1);

	init_cb_list();
	init_rf_area();
}

int append_nop_cb_list(uint16_t flag)
{
	if(nic_instance.cb_list.available_command_block == 0)
	{
		return -E_CBL_FULL;
	}

	nic_instance.cb_list.waiting_command_block++;
	nic_instance.cb_list.available_command_block--;

	nic_instance.cb_list.rear=nic_instance.cb_list.rear->next_cb;
	nic_instance.cb_list.rear->control=CBC_NOP | flag;
	nic_instance.cb_list.rear->status=0;

	return 0;
}

void init_cb_list()
{
	alloc_cb_list();

	append_nop_cb_list(CBF_S);

	outl(nic_instance.io_base+CSR_GP, nic_instance.cb_list.front->phy_addr);
	e100_run_command(CSR_COMMAND, CUC_START);
}

int append_transmit_cb_list(const char *message, uint16_t length, uint16_t flag)
{
	if(nic_instance.cb_list.available_command_block == 0)
	{
		return -E_CBL_FULL;
	}

	nic_instance.cb_list.waiting_command_block++;
	nic_instance.cb_list.available_command_block--;

	nic_instance.cb_list.rear=nic_instance.cb_list.rear->next_cb;

	nic_instance.cb_list.rear->control=CBC_TRANSMIT | flag;
	nic_instance.cb_list.rear->status=0;

	nic_instance.cb_list.rear->tcb_info.tbd_array_addr=0xFFFFFFFF;
	nic_instance.cb_list.rear->tcb_info.tbb_thrs=0xE0;
	nic_instance.cb_list.rear->tcb_info.tbd_count=0;
	nic_instance.cb_list.rear->tcb_info.tcb_byte_count=length;

	memmove(nic_instance.cb_list.rear->tcb_info.data, (void *)message, length);

	return 0;
}

void reclaim_cb_list()
{
	while((nic_instance.cb_list.front->status & CBS_C)!=0 && nic_instance.cb_list.waiting_command_block>0)
	{
		nic_instance.cb_list.front=nic_instance.cb_list.front->next_cb;

		nic_instance.cb_list.waiting_command_block--;
		nic_instance.cb_list.available_command_block++;
	}
}

int e100_send_msg(const char *message, uint16_t length)
{
	int status;

	reclaim_cb_list();

	if(nic_instance.cb_list.available_command_block == 0)
	{
		return -E_CBL_FULL;
	}

	nic_instance.cb_list.rear->control=nic_instance.cb_list.rear->control &  ~CBF_S;	
	append_transmit_cb_list(message, length, CBF_S);

	status=inb(nic_instance.io_base+CSR_STATUS);
	if((status & CUS_MASK) == CUS_SUSPENDED)
	{
		e100_run_command(CSR_COMMAND, CUC_RESUME);
	}

	return 0;
}

void alloc_rf_area()
{
	struct Page *page;
	int result;

	struct receive_frame_descriptor *prev_rfd=NULL;
	struct receive_frame_descriptor *curr_rfd=NULL;

	if(page_alloc(&page) != 0)
	{
		panic("alloc_rf_area: run out of memory");
	}

	memset(page2kva(page), 0, PGSIZE);
	page->pp_ref++;
			
	curr_rfd=(struct receive_frame_descriptor *)page2kva(page);
	curr_rfd->phy_addr=page2pa(page);
	curr_rfd->control=0;
	curr_rfd->status=0;
	curr_rfd->rfd_size=RFD_MAXSIZE;
	nic_instance.rf_area.start=curr_rfd;
	prev_rfd=curr_rfd;
	
	int count;
	for(count=1; count<RFA_MAXSIZE; count++)
	{
		if(page_alloc(&page) != 0)
		{
			panic("alloc_rf_area: run out of memory");
		}

		memset(page2kva(page), 0, PGSIZE);
		page->pp_ref++;

		curr_rfd=(struct receive_frame_descriptor *)page2kva(page);
		curr_rfd->phy_addr=page2pa(page);
		curr_rfd->control=0;
		curr_rfd->status=0;
		curr_rfd->rfd_size=RFD_MAXSIZE;

		if(count == 0)
		{
			nic_instance.rf_area.start=curr_rfd;
		}
		else
		{
			prev_rfd->link=curr_rfd->phy_addr;
			prev_rfd->next=curr_rfd;
			curr_rfd->prev=prev_rfd;
		}

		prev_rfd=curr_rfd;
	}

	prev_rfd->link=nic_instance.rf_area.start->phy_addr;
	nic_instance.rf_area.start->prev=prev_rfd;
	prev_rfd->next=nic_instance.rf_area.start;

	nic_instance.rf_area.available_receive_frame_descriptor=RFA_MAXSIZE;
	nic_instance.rf_area.waiting_receive_frame_descriptor=0;

	nic_instance.rf_area.front=nic_instance.rf_area.start;
	nic_instance.rf_area.rear=nic_instance.rf_area.start->prev;
	nic_instance.rf_area.rear->control=nic_instance.rf_area.rear->control | RFDF_S;
}

void reclaim_rf_area()
{
	while((nic_instance.rf_area.rear->next->status & RFDS_C) != 0 && nic_instance.rf_area.available_receive_frame_descriptor > 0)
	{
		nic_instance.rf_area.rear=nic_instance.rf_area.rear->next;

		nic_instance.rf_area.waiting_receive_frame_descriptor++;
		nic_instance.rf_area.available_receive_frame_descriptor--;

	}
}

int retrieve_data_rf_area(char *buffer)
{
	int result;

	if(nic_instance.rf_area.waiting_receive_frame_descriptor == 0)
	{
		return -E_RFA_EMPTY;
	}

	nic_instance.rf_area.waiting_receive_frame_descriptor--;
	nic_instance.rf_area.available_receive_frame_descriptor++;

	nic_instance.rf_area.front->prev->control=nic_instance.rf_area.front->prev->control & ~RFDF_S;
	nic_instance.rf_area.front->control=RFDF_S;
	nic_instance.rf_area.front->status=0;

	result=nic_instance.rf_area.front->rfd_actual_count & RFD_AC_MASK;
	memmove(buffer, nic_instance.rf_area.front->data, result);
	nic_instance.rf_area.front=nic_instance.rf_area.front->next;

	return result;
}

void init_rf_area()
{
	alloc_rf_area();
	outl(nic_instance.io_base + CSR_GP, nic_instance.rf_area.front->phy_addr);
	e100_run_command(CSR_COMMAND, RUC_START);
}

int e100_recv_msg(char *buffer)
{
	int result;
	int status;
	reclaim_rf_area();

	if(nic_instance.rf_area.waiting_receive_frame_descriptor == 0)
	{
		return -E_RFA_EMPTY;
	}

	result=retrieve_data_rf_area(buffer);

	status=inb(nic_instance.io_base + CSR_STATUS);

	if((status&RUS_MASK) == RUS_SUSPEND)
	{
		e100_run_command(CSR_COMMAND, RUC_RESUME);
	}

	return result;
}

void delay(void)
{
	inb(0x84);
	inb(0x84);
	inb(0x84);
	inb(0x84);
}
