#ifndef _MEMORYMANAGER_H
#define _MEMORYMANAGER_H

#include <system.h>

#define FREE 5
#define ALLOCATED 55

extern u32int*  mem_start;

struct list{
  int count;
  struct cmcb *head;
  struct cmcb *tail;
};

typedef struct cmcb{

  unsigned int type; //0 = free, 1 = allocated

  void* beg_addr;
  unsigned int size;

  char* name;

  struct cmcb *next;
  struct cmcb *prev;
} cmcb;

typedef struct lmcb{
  unsigned int type; //0 = free, 1 = allocated
  unsigned int size;

} lmcb;

struct list* mb_allocated;
struct list* mb_free;

void InitializeHeap(int size);

void *AllocateMemory(int inc_size);

int freeMem(void *ptr);

#endif
