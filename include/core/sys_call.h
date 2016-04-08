#ifndef _SYS_CALL_H
#define _SYS_CALL_H

#include <core/PCB.h>

typedef struct  context{
u32int gs , fs , es , ds ;
u32int edi , esi , ebp , esp , ebx , edx , ecx , eax ;
u32int eip , cs , eflags ;
} context;

extern pcb* cop;

/**
 * function name: sys_call
 * Description: prepares MPX for the next ready process to begin/resume execution
 * Parameters: context registers of the currently operating process
 * Return: u-32 int that denotes the top of the stack in memory
*/
u32int* sys_call(context *registers);

#endif
