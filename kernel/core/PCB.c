#include <stdint.h>
#include <string.h>
#include <system.h>

#include <core/PCB.h>

struct queue{
  int count;
  pcb *head;
  pcb *tail;
};

struct queue *ready;

struct queue *blocked;

int init_queues(){
  int return_code = OK;
	
  ready = (queue*) sys_alloc_mem((size_t) sizeof(queue));
  if (ready == NULL) {
	return_code = ERROR;
  }
  else {
	ready->head = NULL;
	ready->tail = NULL;
	ready->count = 0;
  }
	
  blocked = (queue*) sys_alloc_mem((size_t) sizeof(queue));
  if (blocked == NULL) {
	return_code = ERROR;
  }
  else {
	blocked->head = NULL;
	blocked->tail = NULL;
	blocked->count = 0;
  }
  return return_code;
}


struct pcb* AllocatePCB(){
  pcb_struct *new_pcb;
  new_pcb = (pcb_struct*) sys_alloc_mem((size_t) sizeof(pcb_struct));	
  new_pcb->stack_bottom = (unsigned char*) sys_alloc_mem(PCB_stack_size);	
  new_pcb->stack_top = new_pcb->stack_bottom + PCB_stack_size;
  return new_pcb;
}

int FreePCB(struct pcb* block){
  sys_free_mem(block->stack_bottom);
  sys_free_mem(block->stack_top);
  sys_free_mem(block->name);
  sys_free_mem(block->priority);
  sys_free_mem(block->class);
  sys_free_mem(block->running_state);
  sys_free_mem(block->suspended_state);
  sys_free_mem(block->next);
  sys_free_mem(block->prev);
  sys_free_mem(block);
  return OK;
}
/*
struct pcb* SetupPCB(char* name, unsigned int class, unsigned int priority){

}

struct pcb* FindPCB(char* name){

}

void InsertPCB(struct pcb* block){

}

int RemovePCB(struct pcb* block){

}
*/
