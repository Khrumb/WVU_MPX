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

struct pcb* SetupPCB(char* name, unsigned int class, unsigned int priority){
  struce pcb* newBlock = AllocatePCB();
  newBlock->name = name;
  newBlock->priority = priority;
  newBlock->class = class;
  newBlock->running_state = READY;
  newBlock->suspended_state = NOT_SUSPENDED;
  InsertPCB(newBlock);
}

struct pcb* FindPCB(char* name){
`struct pcb* current_pcb = ready->head;
  while(current_pcb->next != NULL){
    if(strcmp(current_pcb->name, name) == 0)
      {return current_pcb;}
    current_pcb = current_pcb->next;
  }
  current_pcb = blocked->head;
  while(current_pcb->next != NULL){
    if(strcmp(current_pcb->name, name) == 0)
      {return current_pcb;}
    current_pcb = current_pcb->next;
  }
  return NULL;
}


void InsertPCB(struct pcb* block){
  if(block->running_state == READY){
    if(ready->head != NULL){
      struct pcb* current_block = ready->head;
      while(current_block->next != NULL && current_block->priority <= block->priority){
        current_block = current_block->next;
      }
      if(current_block->next == NULL){
        ready->tail = block;
      } else {
        current_block->next->prev = block;
      }
      block->next = current_block->next;
      block->prev = current_block;
      current_block->next = block;
    } else {
      ready->head = block;
      block->prev = NULL;
      block->next = NULL;
      ready->tail = block;
    }
  } else if(block->running_state == BLOCKED){
    struct pcb* current_tail = blocked->tail;
    current_tail->next = block;
    block->prev = current_tail;
    blocked->tail = block;
  }
}

int RemovePCB(struct pcb* block){
  if(block != NULL){
    block->prev->next = block->next;
    block->next->prev = block->prev;
    return FreePCB(block);
  } else{
    return -1;
  }


}
