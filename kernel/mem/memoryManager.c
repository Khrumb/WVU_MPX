#include "modules/mpx_supt.h"

#include <string.h>

#include <system.h>
#include <core/serial.h>
#include <mem/heap.h>
#include <mem/memoryManager.h>
#include <core/commandHandler.h>


u32int* mem_start;

struct list* mb_allocated;
struct list* mb_free;

char free_name[11] = "FREE BLOCK\0";
char alloc_name[11] = "<PCB NAME>\0";


void reorderList(struct list* unordered){
  cmcb* current = unordered->head;
  while(current != NULL){
    if(current->next != NULL){
      if((unsigned int)current > (unsigned int)current->next){
        if(current == unordered->head){
          unordered->head =current->next;
        }
        cmcb* temp = current->next;
        current->next = temp->next;
        temp->prev = current->prev;
        current->prev->next = temp;
        current->prev = temp;
        temp->next = current;
        current = current->prev;
      }
    }
    current = current->next;
  }

}

void InitializeHeap(int size){
  int eff_size = (size + sizeof(cmcb) + sizeof(lmcb));
  mem_start = (u32int *)kmalloc(eff_size);

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
  lmcb* limit = (struct lmcb*)(mem_start+size+sizeof(struct cmcb));
  memset(limit, 0, sizeof(struct lmcb));
  limit->size = size;
  limit->type = FREE;
}

void *AllocateMemory(int inc_size){
  if(mb_free != NULL && mb_free->head != NULL){
    //finding a free block
    unsigned int size = inc_size + sizeof(struct cmcb) + sizeof(struct lmcb);
    cmcb* current_free = mb_free->head;
    while(current_free->next != NULL){
      if(current_free->size >= (unsigned int)inc_size+sizeof(struct lmcb)){
          break;
      } else {
        current_free = current_free->next;
      }
    }
    //making allocated block
    if(current_free->size >= (unsigned int)inc_size){
      if(mb_free->head == current_free){
        mb_free->head = NULL;
      }
      cmcb* current_alloc = current_free;
      //makes new free cmcb and makes it a deep copy.
      if(current_alloc->size - size != 0 ){
        current_free = (struct cmcb*)(current_alloc+size);
        if(current_free->type != ALLOCATED){
          memset(current_free, 0, sizeof(cmcb));
          current_free->name = free_name;
          current_free->type = FREE;
          current_free->beg_addr = (u32int*)(current_free + sizeof(struct cmcb));
          current_free->size = current_alloc->size - size;
          current_free->next = current_alloc->next;
          current_free->prev = current_alloc->prev;
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
          mb_free->count++;
        }
      } else {
        if(mb_free->head == NULL && current_free->next != NULL){
          mb_free->head = current_free->next;
        }
        current_free->prev = NULL;
        current_free->next = NULL;
        current_free = NULL;
      }
      //writes over old FREE cmcb with allocated cmcb
      memset(current_alloc, 0, size);
      memset(current_alloc, 0, sizeof(struct cmcb));

      current_alloc->name = alloc_name; //placeholder
      current_alloc->type = ALLOCATED;
      current_alloc->beg_addr = (u32int*)(current_alloc + sizeof(struct cmcb));
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
      mb_allocated->count++;
      current_alloc->beg_addr = (u32int*)(current_alloc+sizeof(struct cmcb));
      reorderList(mb_allocated);
      return (void*)current_alloc+sizeof(struct cmcb);
    } else {
      serial_println("MASON! STOP TRYING TO BREAK OUR SHIT! (Not Enough free memory.)");
    }
  } else {
    serial_println("NOPE. InitializeHeap pls.");
  }
  return NULL;
}

void mergeFree(cmcb* current){
  cmcb* current_free = mb_free->head;
  while(current_free != NULL){
    unsigned int size = current_free->size + sizeof(struct cmcb) + sizeof(struct lmcb);
    cmcb* finder = (struct cmcb*)(current_free+size);
    if(current == finder){
      current_free->size = current->size+current_free->size + sizeof(struct cmcb) + sizeof(struct lmcb);
      lmcb* limit = (struct lmcb*)(current_free+current_free->size+sizeof(struct cmcb));
      memset(limit, 0, sizeof(struct lmcb));
      limit->size = size;
      limit->type = FREE;
      current->prev->next = current->next;
      current->next->prev = current->prev;
      mergeFree(current_free->next);
      break;
    }
    current_free = current_free->next;
  }
}

int freeMem(void *ptr){
  cmcb* current = mb_allocated->head;
  while(current->next != NULL){
    if((u32int *)ptr == current->beg_addr){
      current->name=free_name;
      current->type = FREE;
      if(current->prev != NULL){
        current->prev->next = current->next;
      }
      if(current->next != NULL){
        current->next->prev = current->prev;
      }

      cmcb* current_free = mb_free->head;
      if(current_free != NULL){

        while(current_free->next != NULL){
          current_free = current_free->next;
        }
        current_free->next = current;
        current->prev = current_free;
        current->next = NULL;
      } else {
        mb_free->head = current;
        current->next = NULL;
        current->prev = NULL;
      }
      mergeFree(current);
      reorderList(mb_free);
      return 0;
    } else {
      current = current->next;
    }
  }
  return -1;
}
