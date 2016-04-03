#ifndef _MEMORYMANAGER_H
#define _MEMORYMANAGER_H

#include <system.h>

#define FREE 0
#define ALLOCATED 1

extern u32int mem_start;

struct list{
  int count;
  struct cmcb *head;
  struct cmcb *tail;
};

typedef struct cmcb{

  unsigned int type; //0 = free, 1 = allocated

  u32int* beg_addr;
  unsigned int size;

  char* name;

  struct cmcb *next;
  struct cmcb *prev;
} cmcb;

typedef struct lmcb{
  unsigned int type; //0 = free, 1 = allocated
  unsigned int size;

} lmcb;

#endif
