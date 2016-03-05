#include <stdint.h>
#include <string.h>
#include <system.h>

#include <core/PCB.h>
#include <core/serial.h>

#include "modules/mpx_supt.h"

/**
 * function name: init_queues
 * Description: calls sys_alloc_mem() to allocate  memory for both queues
 * Paramaters: None
 * Valid return: ok code
 * Invalid return: null (error)
*/
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

/**
 * function name: AllocatePCB
 * Description: calls sys_alloc_mem() to allocate  memory for process
 * Paramaters: None
 * Valid return: pcb pointer
 * Invalid return: null (error)
*/
struct pcb* AllocatePCB(){
  struct pcb *new_pcb;
  new_pcb = (struct pcb*) sys_alloc_mem((size_t) sizeof(struct pcb));
  new_pcb->stack_bottom = (unsigned char*) sys_alloc_mem(1024);
  new_pcb->stack_top = new_pcb->stack_bottom + 1024;
  return new_pcb;
}

/**
 * function name: FreePCB
 * Description: calls sys_free_mem() to free all memory associated with a given PCB
 * Parameters: pcb pointer
 * Valid return: ok code
 * Invalid return: error code
*/
int FreePCB(struct pcb* block){
  sys_free_mem(block->name);
  return sys_free_mem(block);
}

/**
 * function name: SetupPCB
 * Description: calls allocatePCB, initializes the a pcb, sets pcb to a ready state
 * Parameters: process name, class, and priority
 * Valid return: PCB pointer
 * Invalid return: null for error or invalid input message
*/
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
  newBlock->name[i+1] = '\0';
  newBlock->priority = priority;
  newBlock->class = class;
  newBlock->running_state = READY;
  newBlock->suspended_state = NOT_SUSPENDED;
  newBlock->next = NULL;
  newBlock->prev = NULL;
  return newBlock;
}

/**
 * function name: FindPCB
 * Description: searches both queues for a process with the given name
 * Parameters: process name
 * Valid return: PCB pointer
 * Invalid return: null if PCB does not exist
*/
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

/**
 * function name: InsertPCB
 * Description: puts PCB into correct queue
 * Returns: none
*/
void InsertPCB(struct pcb* block){
  if(block != NULL){
    switch(block->running_state){
      case 0:
        ready->count = ready->count + 1;
        insertIntoReady(block);
        break;
      case 2:
        blocked->count = blocked->count + 1;
        insertIntoBlocked(block);
        break;
    }
  }
}

void insertIntoReady(struct pcb* block){
  if(ready->head != NULL){
    struct pcb* current_block = ready->head;
    while(current_block->next != NULL && block->priority > current_block->priority){
      current_block = current_block->next;
    }
    if(block->priority > current_block->priority){
      if(current_block->next == NULL){
        block->next = NULL;
        ready->tail = block;
      } else {
        current_block->prev->next = block;
        current_block->next->prev = block;
        block->next = current_block->next;
      }
      current_block->next = block;
      block->prev = current_block;
    } else {
      if(current_block == ready->head){
         ready->head = block;
         block->prev = NULL;
         block->next = current_block;
     } else {
       block->next = current_block;
       block->prev = current_block->prev;
       current_block->prev->next = block;
       current_block->prev = block;
     }
    }
  } else {
    ready->head = block;
    ready->tail = block;
    block->next = NULL;
    block->prev = NULL;
  }
}

void insertIntoBlocked(struct pcb* block){
  if(blocked->tail == NULL){
    blocked->tail = block;
    if(blocked->head == NULL){
      blocked->head = block;
    }
  } else {
    block->prev = blocked->tail;
    block->next = NULL;
    blocked->tail->next = block;
    blocked->tail = block;
  }
}

/**
 * function name: RemovePCB
 * Description: removes PCB from queue it is currently stored in
 * Parameters: pointer to PCB
 * Valid return: confirmation message
 * Invalid return: null (PCB not found)
*/
int RemovePCB(struct pcb* block){
  if(block != NULL){
    switch(block->running_state){
      case 0:
        ready->count = ready->count - 1;
        removeFromReady(block);
        break;
      case 2:
        blocked->count = blocked->count - 1;
        removeFromBlocked(block);
        break;
    }
    FreePCB(block);
    return 0;
  } else{
    return -1;
  }
}

void removeFromReady(struct pcb* block){
  if(block->prev == NULL){
    ready->head = block->next;
  } else {
    block->prev->next = block->next;
  }
  if(block->next == NULL){
    ready->tail = block->prev;
  } else {
    block->next->prev = block->prev;
  }
}

void removeFromBlocked(struct pcb* block){
  if(block->prev == NULL){
    blocked->head = NULL;
    blocked->tail =NULL;
  } else {
    if(block->next == NULL){
      blocked->tail = block->prev;
    } else {
      block->next->prev = block->prev;
    }
    block->prev->next = block->next;
  }
}
