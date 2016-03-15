#include <system.h>
#include "modules/mpx_supt.h"

#include <core/sys_call.h>

u32int ret;

u32int* sys_call(context *registers)
{
ret = registers->eax;
return &ret;
}
