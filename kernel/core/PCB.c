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
  sys_free_mem(block->name);
  return sys_free_mem(block);
}

struct pcb* SetupPCB(char* name, unsigned int class, unsigned int priority){
  if(priority > 9){
    return NULL;
  }
  int name_length = strlen(name);
  if(name_length <= 8){
    if(FindPCB(name) != NULL){
      return NULL;
    }
  } else {
    return NULL;
  }
  struct pcb* newBlock = AllocatePCB();

  int i;
  for(i = 0; i <= name_length; i ++){
    newBlock->name[i] = name[i];
  }
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
    if(strcmp(current_pcb->name, name) == 0){
      return current_pcb;
    }
  }
  if(blocked->head != NULL){
    current_pcb = blocked->head;
    while(current_pcb->next != NULL){
      if(strcmp(current_pcb->name, name) == 0)
        {return current_pcb;}
      current_pcb = current_pcb->next;
    }
    if(strcmp(current_pcb->name, name) == 0){
      return current_pcb;
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
      ready->count = ready->count + 1;
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
  switch(block->running_state){
    case 0:
      ready->count = ready->count - 1;
      break;
    case 2:
      blocked->count = blocked->count - 1;
      break;
  }
  if(block != NULL){
    if(blocked->tail == block){
      if(blocked->head == block){
        blocked->head = NULL;
      }
      blocked->tail = NULL;
    }else if(ready->tail == block){
      if(ready->head == block){
        ready->head = NULL;
      }
      ready->tail = NULL;
    } else {
      block->next->prev = block->prev;
      block->prev->next = block->next;
    }
    FreePCB(block);
    return 0;
  } else{
    return -1;
  }
}
