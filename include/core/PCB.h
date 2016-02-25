#ifndef _PCB_H
#define _PCB_H

#define READY 0
#define RUNNING 1
#define BLOCKED 2

#define SUSPENDED 0
#define NOT_SUSPENDED 1

#define SYSTEM 0
#define APPLICATION 1

#define OK 0
#define ERROR 1

struct queue{
  int count;
  struct pcb *head;
  struct pcb *tail;
};

struct queue *ready;

struct queue *blocked;

typedef struct pcb{
  char* name; //name length 9 + null term;
  unsigned int class; //use defined shenanagains
  unsigned int priority; // 0-9, higher number, higher priority

  //state variables
  unsigned int running_state;
  unsigned int suspended_state;

  //stack area, min 1024bytes
  unsigned char* stack_top;
  unsigned char* stack_bottom;

  struct pcb *next;
  struct pcb *prev;
} pcb;

int init_queues();

struct pcb* AllocatePCB();

int FreePCB(struct pcb* block);

struct pcb* SetupPCB(char* name, unsigned int class, unsigned int priority);

struct pcb* FindPCB(char* name);

void InsertPCB(struct pcb* block);

int RemovePCB(struct pcb* block);

#endif
