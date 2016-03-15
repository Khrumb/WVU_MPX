#include <system.h>
#include "modules/mpx_supt.h"

#include <core/sys_call.h>
#include <core/PCB.h>

context* caller;

pcb* cop;

u32int* sys_call(context *registers)
{
	if(cop == NULL){
		caller = registers;
	} else {
		if(params->op_code == IDLE){
			//reassign stacktop of COP
			caller = registers;

		}
		if(params->op_code == EXIT){
			RemovePCB(cop);
		}
	}
	if(ready->count != 0){
		cop=ready->head;
		return (u32int*)cop->stack_top;
	}
	return (u32int*)caller;
}
