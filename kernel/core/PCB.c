#include <stdint.h>
#include <string.h>
#include <system.h>

#include <core/PCB.h>

struct queue{
  int count;
  pcb *head;
  pcb *tail;
};

struct queue *ready;

struct queue *blocked;

struct pcb* AllocatePCB(){
  
}

int FreePCB(struct pcb* block){
  
}

struct pcb* SetupPCB(char* name, unsigned int class, unsigned int priority){
  
}

struct pcb* FindPCB(char* name){
  
}

void InsertPCB(struct pcb* block){
  
}

int RemovePCB(struct pcb* block){
  
}
