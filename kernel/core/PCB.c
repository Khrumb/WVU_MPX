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
