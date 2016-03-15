#ifndef _SYS_CALL_H
#define _SYS_CALL_H

typedef struct  {
u32int gs , fs , es , ds ;
u32int edi , esi , ebp , esp , ebx , edx , ecx , eax ;
u32int eip , cs , eflags ;
} context;

u32int* sys_call(context *registers);

#endif
