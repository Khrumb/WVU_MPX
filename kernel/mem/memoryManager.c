#include "modules/mpx_supt.h"

#include <string.h>

#include <system.h>
#include <core/serial.h>
#include <mem/heap.h>
#include <mem/memoryManager.h>

u32int mem_start;

struct list* mb_allocated;
struct list* mb_free;

void InitializeHeap(int size){
  int eff_size = (size + sizeof(cmcb) + sizeof(lmcb));
  mem_start = kmalloc((u32int) eff_size);

  mb_free =(struct list*) kmalloc(sizeof(struct list));
  mb_allocated =(struct list*) kmalloc(sizeof(struct list));

  mb_allocated->head = NULL;

  mb_free->head = (struct cmcb*)mem_start;
  memset(mb_free->head, 0, sizeof(struct cmcb));
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
  if(mb_free->head != NULL){
    //finding a free block
    unsigned int size = inc_size + sizeof(cmcb) + sizeof(lmcb);
    cmcb* current_free = mb_free->head;
    while(current_free->size >= size && current_free->next != NULL){
      current_free = current_free->next;
    }

    //making allocated block
    if(current_free->size > size){
      cmcb* current_alloc = current_free;

      if(current_free == mb_free->head){
        //FIGURE OUT WHERE TO PUT THE NEW CMCB
      }

      //makes new free cmcb and makes it a deep copy.
      current_free = (struct cmcb*)(current_free+size);
      memset(current_free, 0, sizeof(cmcb));
      current_free->type = FREE
      current_free->size = current_alloc->size - inc_size;
      current_free->next = current_alloc->next;
      current_free->prev = current_alloc->prev;

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
      return (void*)current_alloc;
    } else {
      serial_println("MASON! STOP TRYING TO BREAK OUR SHIT! (No more free memory.)");
    }
  } else {
    serial_println("NOPE. InitializeHeap pls.");
  }
  return NULL;
}
