/* See COPYRIGHT for copyright information. */

#ifndef JOS_INC_ENV_H
#define JOS_INC_ENV_H

#include <inc/types.h>
#include <inc/queue.h>
#include <inc/trap.h>
#include <inc/memlayout.h>

typedef int32_t envid_t;

// An environment ID 'envid_t' has three parts:
//
// +1+---------------21-----------------+--------10--------+
// |0|          Uniqueifier             |   Environment    |
// | |                                  |      Index       |
// +------------------------------------+------------------+
//                                       \--- ENVX(eid) --/
//
// The environment index ENVX(eid) equals the environment's offset in the
// 'envs[]' array.  The uniqueifier distinguishes environments that were
// created at different times, but share the same environment index.
//
// All real environments are greater than 0 (so the sign bit is zero).
// envid_ts less than 0 signify errors.  The envid_t == 0 is special, and
// stands for the current environment.

#define LOG2NENV		10
#define NENV			(1 << LOG2NENV)
#define ENVX(envid)		((envid) & (NENV - 1))

// Values of env_status in struct Env
#define ENV_FREE		0
#define ENV_RUNNABLE		1
#define ENV_NOT_RUNNABLE	2

#define ENV_TYPE_NORMAL		0
#define ENV_TYPE_PGFAULT	5

struct Env {
	struct Trapframe env_tf;	// Saved registers
	LIST_ENTRY(Env) env_link;	// Free list link pointers
	envid_t env_id;			// Unique environment identifier
	envid_t env_parent_id;		// env_id of this env's parent
	unsigned env_status;		// Status of the environment
	uint32_t env_runs;		// Number of times environment has run

	// Address space
	pde_t *env_pgdir;		// Kernel virtual address of page dir
	physaddr_t env_cr3;		// Physical address of page dir

	// Exception handling
	void *env_pgfault_upcall;	// page fault upcall entry point

	// Lab 4 IPC
	bool env_ipc_recving;		// env is blocked receiving
	void *env_ipc_dstva;		// va at which to map received page
	uint32_t env_ipc_value;		// data value sent to us 
	envid_t env_ipc_from;		// envid of the sender	
	int env_ipc_perm;		// perm of page mapping received

	int env_type;
	char env_ip_from[16];
	int env_port_from;

	uint32_t env_priority;
};

//used by normal migration
struct Migration_Env_Normal {
	uint32_t m_env_va;
	int m_env_sequence;
	char m_env_mem[1024];
	int m_status;
};

//used by pgfault migration
struct Migration_Env_PGfault {
	int m_total;
	uint32_t m_env_vas[256];
	int m_status;
};

struct Migration_Page_Request {
	uint32_t m_env_va;
};

struct Migration_Page_Respond {
	uint32_t m_env_va;
	int m_env_sequence;
	char m_env_mem[1024];
};

struct Env_Header {
	char env_ip_from[16];
	char env_ip_to[16];
	int env_port_from;
	int env_port_to;
	int env_from_id;
	struct Trapframe env_tf;
};

struct Env_PGfault_Msg {
	envid_t env_id;
	uint32_t env_fault_va;
	int env_pg_status;
};

#endif // !JOS_INC_ENV_H
