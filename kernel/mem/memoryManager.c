#include "modules/mpx_supt.h"

#include <string.h>

#include <system.h>
#include <core/serial.h>
#include <mem/heap.h>
#include <mem/memoryManager.h>
#include <core/commandHandler.h>
#include <core/sys_call.h>
#include <core/PCB.h>




u32int* mem_start;
u32int* mem_end;

struct list* mb_allocated;
struct list* mb_free;

char free_name[11] = "FREE BLOCK\0";
char alloc_name[11] = "SYSTEM\0";


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
  if(mb_free == NULL && mb_allocated == NULL){
    int eff_size = (size + sizeof(struct cmcb) + sizeof(struct lmcb));
    mem_start = (u32int *)kmalloc(eff_size);
    mem_end = mem_start+eff_size;
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
    lmcb* limit = (struct lmcb*)(mb_free->head+size+sizeof(struct cmcb));
    memset(limit, 0, sizeof(struct lmcb));
    limit->size = size;
    limit->type = FREE;
  } else {
    serial_println("Heap is already initalized.");
  }
}

void *AllocateMemory(int inc_size){
  if(mb_free != NULL && mb_free->head != NULL){
    //finding a free block
    unsigned int size = inc_size + sizeof(struct cmcb) + sizeof(struct lmcb);
    cmcb* current_free = mb_free->head;
    while(current_free->next != NULL){
      if(current_free->size > (unsigned int)size || current_free->size-size == 0){
          break;
      } else {
        current_free = current_free->next;
      }
    }
    //making allocated block
    if(current_free->size > (unsigned int)size || current_free->size-inc_size == 0){
      if(mb_free->head == current_free){
        mb_free->head = NULL;
      }
      cmcb* current_alloc = current_free;
      //makes new free cmcb and makes it a deep copy.
      if(current_alloc->size - inc_size != 0 ){
        current_free = (struct cmcb*)(current_alloc+size);
        if(current_free->type != ALLOCATED){
          memset(current_free, 0, sizeof(cmcb));
          current_free->name = free_name;
          current_free->type = FREE;
          current_free->beg_addr = (u32int*)(current_free + sizeof(struct cmcb));
          current_free->size = current_alloc->size - size;
          current_free->next = current_alloc->next;
          current_free->prev = current_alloc->prev;
          lmcb* limit = (struct lmcb*)(current_free+current_free->size+sizeof(struct cmcb));
          memset(limit, 0, sizeof(struct lmcb));
          limit->size = current_free->size;
          limit->type = FREE;

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
        } else {
          serial_println("not Writing freeBlock head");
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
      //memset(current_alloc, 0, size);
      memset(current_alloc, 0, sizeof(struct cmcb));
      if(cop != NULL){
        current_alloc->name = cop->name; //placeholder
      } else {
        current_alloc->name = alloc_name; //placeholder
      }
      current_alloc->type = ALLOCATED;
      current_alloc->beg_addr = (u32int*)(current_alloc + sizeof(struct cmcb));
      current_alloc->size = inc_size;
      current_alloc->next = NULL;
      current_alloc->prev = NULL;

      //makes allocate lmcb
      lmcb* limit = (struct lmcb*)(current_alloc+inc_size+sizeof(struct cmcb));
      memset(limit, 0, sizeof(struct lmcb));
      limit->size = current_alloc->size;
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
      serial_println("OUT_OF_MEMORY_ERROR\n(THERE YOU GO AGAIN MASON! BREAKING OUR SHIT'N SHIT!.)");
    }
  } else {
    serial_println("NOPE. InitializeHeap pls.");
  }
  return NULL;
}

void mergeFree(cmcb* current){
  lmcb* limit = (struct lmcb*)(current-sizeof(struct lmcb));
  cmcb* adjacent = (struct cmcb*)(current-(limit->size+sizeof(struct lmcb)+sizeof(struct cmcb)));
  if(adjacent->type == FREE){
    adjacent->size = adjacent->size + current->size+ sizeof(struct cmcb)+sizeof(struct lmcb);
    limit = (struct lmcb*)(current+adjacent->size+sizeof(struct cmcb));
    limit->size = adjacent->size;
    current->prev->next = current->next;
    current->next->prev = current->prev;
    current = adjacent;
  }
  adjacent = (struct cmcb*)(current+current->size+sizeof(struct lmcb)+sizeof(struct cmcb));
  if((u32int*)adjacent <= mem_end && adjacent->type == FREE){
    current->size = adjacent->size + current->size + sizeof(struct cmcb)+sizeof(struct lmcb);
    limit = (struct lmcb*)(current+current->size+sizeof(struct cmcb));
    limit->size = current->size;
    adjacent->prev->next = adjacent->next;
    adjacent->next->prev = adjacent->prev;
    if(adjacent == mb_free->head){
      mb_free->head = mb_free->head->next;
    }
  }
  //mergeFree(current);
}

int freeMem(void *ptr){
  cmcb* current = mb_allocated->head;
  while(current != NULL){
    if((void *)ptr == current->beg_addr){
      if(current == mb_allocated->head){
        mb_allocated->head = current->next;
      }
      current->name=free_name;
      current->type = FREE;
      if(current->prev != NULL){
        current->prev->next = current->next;
      }
      if(current->next != NULL){
        current->next->prev = current->prev;
      }

      //inserts and the end of the list
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
