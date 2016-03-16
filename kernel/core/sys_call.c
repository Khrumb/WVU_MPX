#include "modules/mpx_supt.h"


#include <system.h>
#include <core/PCB.h>
#include <core/sys_call.h>

context* caller;

pcb* cop = NULL;

u32int* sys_call(context *registers)
{
	if(cop == NULL){
		caller = registers;
	} else {
		if(params.op_code == IDLE){
			cop->stack_top = (u32int*)registers;
			RemovePCB(cop);
			InsertPCB(cop);
		}
		if(params.op_code == EXIT){
			RemovePCB(cop);
			FreePCB(cop);
		}
	}

	if(ready->count != 0){
		cop=ready->head;
		return (u32int*)cop->stack_top;
	}

	return (u32int*)caller;
}
