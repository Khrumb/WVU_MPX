#ifndef _PCB_H
#define _PCB_H

#define READY 0
#define RUNNING 1
#define BLOCKED 2

#define SUSPENDED 0
#define NOT_SUSPENDED 1

typedef struct pcb_struct{
  char name[10]; //name length 9 + null term;
  unsigned int class; //nothing defined for values
  unsigned int priority; // 0-9, higher number, higher priority

  //state variables
  unsigned int running_state;
  unsigned int suspend_state;

  //stack area, min 1024bytes
  unsigned char* stack_top;
  unsigned char* stack_bottom;

  struct pcb *next;
  struct pcb *prev;
}
  __attribute__ ((packed)) pcb;

struct pcb* AllocatePCB();

int FreePCB(struct pcb* block);

struct pcb* SetupPCB(char name[10], unsigned int class, unsigned int priority);

struct pcb* FindPCB(char name[10]);

void InsertPCB(struct pcb* block);

int RemovePCB(struct pcb* block);

#endif
