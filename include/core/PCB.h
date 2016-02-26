#ifndef _PCB_H
#define _PCB_H

#define READY 0
#define RUNNING 1
#define BLOCKED 2

#define SUSPENDED 0
#define NOT_SUSPENDED 1

#define SYSTEM 0
#define APPLICATION 1

#define OK 0
#define ERROR 1

struct queue{
  int count;
  struct pcb *head;
  struct pcb *tail;
};

struct queue *ready;

struct queue *blocked;

typedef struct pcb{
  char name[9]; //name length 9 + null term;
  unsigned int class; //use defined shenanagains
  unsigned int priority; // 0-9, higher number, higher priority

  //state variables
  unsigned int running_state;
  unsigned int suspended_state;

  //stack area, min 1024bytes
  unsigned char* stack_top;
  unsigned char* stack_bottom;

  struct pcb *next;
  struct pcb *prev;
} pcb;

/**
 * function name: init_queues
 * Description: calls sys_alloc_mem() to allocate  memory for both queues
 * Paramaters: None
 * Valid return: ok code
 * Invalid return: null (error)
*/
int init_queues();

/**
 * function name: AllocatePCB
 * Description: calls sys_alloc_mem() to allocate  memory for process
 * Paramaters: None
 * Valid return: pcb pointer
 * Invalid return: null (error)
*/
struct pcb* AllocatePCB();

/**
 * function name: FreePCB
 * Description: calls sys_free_mem() to free all memory associated with a given PCB
 * Parameters: pcb pointer
 * Valid return: ok code
 * Invalid return: error code
*/
int FreePCB(struct pcb* block);

/**
 * function name: SetupPCB
 * Description: calls allocatePCB, initializes the a pcb, sets pcb to a ready state
 * Parameters: process name, class, and priority
 * Valid return: PCB pointer
 * Invalid return: null for error or invalid input message
*/
struct pcb* SetupPCB(char* name, unsigned int class, unsigned int priority);

/**
 * function name: FindPCB
 * Description: searches both queues for a process with the given name
 * Parameters: process name
 * Valid return: PCB pointer
 * Invalid return: null if PCB does not exist
*/
struct pcb* FindPCB(char* name);

/**
 * function name: InsertPCB
 * Description: puts PCB into correct queue
 * Returns: none
*/
void InsertPCB(struct pcb* block);

/**
 * function name: RemovePCB
 * Description: removes PCB from queue it is currently stored in
 * Parameters: pointer to PCB
 * Valid return: confirmation message
 * Invalid return: null (PCB not found)
*/
int RemovePCB(struct pcb* block);

#endif
