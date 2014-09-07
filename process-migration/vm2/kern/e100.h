#ifndef JOS_KERN_E100_H
#define JOS_KERN_E100_H

#include <kern/pci.h>

#define TCB_MAXSIZE		1518
#define CBL_MAXSIZE		30
#define RFD_MAXSIZE     1518
#define RFA_MAXSIZE     30

struct tcb_special
{
	uint32_t tbd_array_addr;

	uint16_t tcb_byte_count;
	uint8_t tbb_thrs;
	uint8_t tbd_count;

	char data[TCB_MAXSIZE];
};

struct command_block 
{
	volatile uint16_t status;
	uint16_t control;
	uint32_t link;

	struct tcb_special tcb_info;
	struct command_block *prev_cb;
	struct command_block *next_cb;
	physaddr_t phy_addr;
};

struct command_block_list
{
	int available_command_block;
	int waiting_command_block;

	struct command_block *start;
	struct command_block *front;
	struct command_block *rear;
};

struct receive_frame_descriptor
{
	volatile uint16_t status;
	uint16_t control;
	uint32_t link;

	uint32_t rfd_reserved;
	uint16_t rfd_actual_count;
	uint16_t rfd_size;
	char data[RFD_MAXSIZE];

	struct receive_frame_descriptor *prev;
	struct receive_frame_descriptor *next;
	physaddr_t phy_addr;
};

struct receive_frame_area
{
	int available_receive_frame_descriptor;
	int waiting_receive_frame_descriptor;

	struct receive_frame_descriptor *start;
	struct receive_frame_descriptor *front;
	struct receive_frame_descriptor *rear;
};

struct nic
{
	uint32_t io_base;
	uint32_t io_size;

	struct command_block_list cb_list;
	struct receive_frame_area rf_area;
};

struct pci_func e100_instance;
struct nic nic_instance;

void init_cb_list();
void alloc_cb_list();
int append_nop_cb_list(uint16_t flag);
int append_transmit_cb_list(const char *data, uint16_t l, uint16_t flag);
void reclaim_cb_list();

void init_rf_area();
void alloc_rf_area();
void reclaim_rf_area();
int retrieve_data_rf_area(char *data);

int e100_attach_device(struct pci_func *pcif);
void e100_reset_device(struct pci_func e100);
void e100_run_command(int csr_comp, uint8_t cmd);
void e100_initialization();

int e100_send_msg(const char *data, uint16_t len);
int e100_recv_msg(char *data);

void delay();

#define E100_VENDOR_ID 0x8086
#define E100_DEVICE_ID 0x1209
	
#define E100_REG_IO     1
			 
#define CSR_SCB     0x0
#define CSR_STATUS  0x0
#define CSR_US      0x0
#define CSR_STATACK 0x1
#define CSR_COMMAND 0x2
#define CSR_UC      0x2
#define CSR_INT     0x3
#define CSR_GP      0x4 
#define CSR_PORT    0x8
					   
#define PORT_SOFT_RESET   0x0

#define CBF_EL	0x8000
#define CBF_S	0x4000
#define CBF_I	0x2000

#define CBC_NOP			0x0
#define CBC_TRANSMIT	0x4

#define CBS_F	0x0800
#define CBS_OK	0x2000
#define CBS_C	0x8000

#define CUC_START		0x10
#define CUC_RESUME		0x20

#define CUS_MASK		0xc0
#define CUS_IDLE		0x00
#define CUS_SUSPENDED	0x40

#define E_CBL_FULL	1
#define E_CBL_EMPTY	2
#define E_RFA_FULL	3
#define E_RFA_EMPTY	4

#define RFDF_EL		0x8000
#define RFDF_S		0x4000
#define RFDF_H		0x10
#define RFDF_SF		0x8

#define RFDS_C		0x8000
#define RFDS_OK		0x2000
#define RFDS_MASK	0x1fff

#define RFD_SIZE_MASK	0x3fff
#define RFD_AC_MASK		0x3fff
#define RFD_EOF			0x8000
#define RFD_F			0x4000

#define RUC_NOP			0x0
#define RUC_START		0x1
#define RUC_RESUME		0x2
#define RUC_REDIR		0x3
#define RUC_ABORT		0x4
#define RUC_LOADHDS		0x5
#define RUC_LOAD_BASE	0x6

#define RUS_MASK	0x3c
#define RUS_IDLE	0x0
#define RUS_SUSPEND	0x4
#define RUS_NORES	0x8
#define RUS_READY	0x10


#endif	// JOS_KERN_E100_H
