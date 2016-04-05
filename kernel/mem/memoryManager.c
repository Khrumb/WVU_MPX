#include "modules/mpx_supt.h"

#include <string.h>

#include <system.h>
#include <core/serial.h>
#include <mem/heap.h>
#include <mem/memoryManager.h>

u32int mem_start;

struct list* mb_allocated;
struct list* mb_free;

char free_name[11] = "FREE BLOCK\0";

void InitializeHeap(int size){
  int eff_size = (size + sizeof(cmcb) + sizeof(lmcb));
  mem_start = kmalloc((u32int) eff_size);

  mb_free =(struct list*) kmalloc(sizeof(struct list));
  mb_allocated =(struct list*) kmalloc(sizeof(struct list));

  mb_allocated->head = NULL;

  mb_free->head = (struct cmcb*)mem_start;
  memset(mb_free->head, 0, sizeof(struct cmcb));
  mb_free->head->name = free_name;
  mb_free->head->beg_addr = (u32int*)(mb_free->head + sizeof(struct cmcb));
  mb_free->head->type = FREE;
  mb_free->head->size = size;
  mb_free->head->next = NULL;
  mb_free->head->prev = NULL;
  lmcb* limit = (struct lmcb*)(mem_start+size+sizeof(cmcb));
  memset(limit, 0, sizeof(struct lmcb));
  limit->size = size;
  limit->type = FREE;
}

void *AllocateMemory(int inc_size){
  if(mb_free != NULL && mb_free->head != NULL){
    //finding a free block
    unsigned int size = inc_size + sizeof(cmcb) + sizeof(lmcb);
    cmcb* current_free = mb_free->head;
    while(current_free->next != NULL && current_free->size >= size){
      current_free = current_free->next;
    }
    //making allocated block
    if(current_free->size >= size){

      if(mb_free->head == current_free){
        mb_free->head = NULL;
      }

      cmcb* current_alloc = current_free;

      //makes new free cmcb and makes it a deep copy.
      current_free = (struct cmcb*)(current_alloc+size);
      memset(current_free, 0, sizeof(cmcb));
      current_free->name = free_name;
      current_free->type = FREE;
      current_free->size = current_alloc->size - size;
      current_free->next = current_alloc->next;
      current_free->prev = current_alloc->prev;

      lmcb* tag = (struct lmcb*)(current_free+size+sizeof(cmcb));
      memset(tag, 0, sizeof(struct lmcb));
      tag->size = current_free->size;
      tag->type = FREE;

      if(mb_free->head == NULL){
        mb_free->head = current_free;
      } else {
        cmcb* itt_free = mb_free->head;
        while(itt_free->next != NULL){
          itt_free = itt_free->next;
        }
        itt_free->next = current_free;
        current_free->prev = itt_free;
      }

      //writes over old FREE cmcb with allocated cmcb
      memset(current_alloc, 0, size);
      memset(current_alloc, 0, sizeof(struct cmcb));
      current_alloc->type = ALLOCATED;
      current_alloc->size = inc_size;
      current_alloc->next = NULL;
      current_alloc->prev = NULL;

      //makes allocate lmcb
      lmcb* limit = (struct lmcb*)(current_alloc+inc_size+sizeof(cmcb));
      memset(limit, 0, sizeof(struct lmcb));
      limit->size = size;
      limit->type = ALLOCATED;

      //puts allocated in the allocated list
      if(mb_allocated->head == NULL){
        mb_allocated->head = current_alloc;
      } else {
        cmcb* itt = mb_allocated->head;
        while(itt->next != NULL){
          itt= itt->next;
        }
        itt->next = current_alloc;
        current_alloc->prev = itt;
      }
      return (void*)current_alloc+sizeof(struct cmcb);
    } else {
      serial_println("MASON! STOP TRYING TO BREAK OUR SHIT! (No more free memory.)");
    }
  } else {
    serial_println("NOPE. InitializeHeap pls.");
  }
  return NULL;
}

int freeMem(void *ptr){
  cmcb* current = mb_allocated->head;
  while(current->next != NULL){
    if(ptr == current+sizeof(struct cmcb)){
      current->type = FREE;
      if(current->prev != NULL){
        current->prev->next = current->next;
      }
      if(current->next != NULL){
        current->next->prev = current->prev;
      }
      cmcb* current_free = mb_free->head;
      while(current_free->next != NULL){
        current_free = current_free->next;
      }
      current_free->next = current;
      current->prev = current_free;
      //if()
      return 0;
    }
  }
  return -1;

}
