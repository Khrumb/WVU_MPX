#include <stdint.h>
#include <string.h>
#include <system.h>

#include <core/PCB.h>
#include <core/serial.h>

#include "modules/mpx_supt.h"

int init_queues(){
  int return_code = OK;

  ready = (struct queue*) sys_alloc_mem((size_t) sizeof(struct queue));
  if (ready == NULL) {
	return_code = ERROR;
  }
  else {
	ready->head = NULL;
	ready->tail = NULL;
	ready->count = 0;
  }

  blocked = (struct queue*) sys_alloc_mem((size_t) sizeof(struct queue));
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
  struct pcb *new_pcb;
  new_pcb = (struct pcb*) sys_alloc_mem((size_t) sizeof(struct pcb));
  new_pcb->stack_bottom = (unsigned char*) sys_alloc_mem(1024);
  new_pcb->stack_top = new_pcb->stack_bottom + 1024;
  return new_pcb;
}

int FreePCB(struct pcb* block){
  return sys_free_mem(block);
}

struct pcb* SetupPCB(char* name, unsigned int class, unsigned int priority){
  if(FindPCB(name) != NULL){
    return NULL;
  }
  if(priority > 9){
    return NULL;
  }
  if(FindPCB(name) != NULL){
    return NULL;
  }
  struct pcb* newBlock = AllocatePCB();
  newBlock->name = name;
  newBlock->priority = priority;
  newBlock->class = class;
  newBlock->running_state = READY;
  newBlock->suspended_state = NOT_SUSPENDED;
  return newBlock;
}

struct pcb* FindPCB(char* name){
  struct pcb* current_pcb;
  if(ready->head != NULL){
    current_pcb = ready->head;
    while(current_pcb->next != NULL){
      if(strcmp(current_pcb->name, name) == 0)
        {return current_pcb;}
      current_pcb = current_pcb->next;
    }
  }
  if(blocked->head != NULL){
    current_pcb = blocked->head;
    while(current_pcb->next != NULL){
      if(strcmp(current_pcb->name, name) == 0)
      {return current_pcb;}
      current_pcb = current_pcb->next;
    }
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
      ready->count = ready->count + 1;
    }
  } else if(block->running_state == BLOCKED){
    struct pcb* current_tail = blocked->tail;
    current_tail->next = block;
    block->prev = current_tail;
    blocked->tail = block;
    blocked->count = blocked->count + 1;
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
