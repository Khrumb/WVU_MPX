
#include <stdint.h>
#include <string.h>
#include <system.h>

#include <core/io.h>
#include <core/commandHandler.h>
#include <core/sys_call.h>
#include <core/PCB.h>
#include <core/serial.h>
#include <core/tables.h>
#include <core/interrupts.h>
#include <mem/heap.h>
#include <mem/memoryManager.h>
#include <mem/paging.h>

#include "modules/mpx_supt.h"


void kmain(void)
{
   extern uint32_t magic;
   // Uncomment if you want to access the multiboot header
   // extern void *mbd;
   // char *boot_loader_name = (char*)((long*)mbd)[16];

 /**
 * function name: serial import
 * Description: uses COM1 port to get input from basic IO
*/
   init_serial(COM1);
   set_serial_in(COM1);
   set_serial_out(COM1);
   mpx_init(MODULE_R4);
   klogv("Starting MPX boot sequence...");
   klogv("Initialized serial I/O on COM1 device...");
   // 1) Check that the boot was successful and correct when using grub
   // Comment this when booting the kernel directly using QEMU, etc.
   if ( magic != 0x2BADB002 ){
     //kpanic("Boot was not error free. Halting.");
   }
/**
  *initalize the Global Descriptor Table, initalise the programable interupt    controller, and initalize paging
*/

   init_gdt();
   init_idt();
   init_irq();
   irq_on();
   sti();
   init_paging();

   // 2) Descriptor Tables
   klogv("Initializing descriptor tables...");

   // 4) Virtual Memory
   klogv("Initializing virtual memory...");
   InitializeHeap(2700);

   klogv("Starting process manager..");
   init_queues();
   // 5) Call Commhand
   klogv("Transferring control to commhand...");
   char* name = "idle";
   pcb* new_pcb = SetupPCB(name, 0, 1);
   context* cp = (context*)(new_pcb->stack_top);
   memset(cp, 0, sizeof(context));
   cp->fs = 0x10;
   cp->gs = 0x10;
   cp->ds = 0x10;
   cp->es = 0x10;
   cp->cs = 0x8;
   cp->ebp = (u32int)(new_pcb->stack_bottom);
   cp->esp = (u32int)(new_pcb->stack_top);
   cp->eip = (u32int)(idle);
   cp->eflags = 0x202;
   InsertPCB(new_pcb);

   name = "commhand";
   new_pcb = SetupPCB(name, 0, 1);
   cp = (context*)(new_pcb->stack_top);
   memset(cp, 0, sizeof(context));
   cp->fs = 0x10;
   cp->gs = 0x10;
   cp->ds = 0x10;
   cp->es = 0x10;
   cp->cs = 0x8;
   cp->ebp = (u32int)(new_pcb->stack_bottom);
   cp->esp = (u32int)(new_pcb->stack_top);
   cp->eip = (u32int)(commandHandler);
   cp->eflags = 0x202;
   InsertPCB(new_pcb);
   asm volatile("int $60");

  //klogv(buffer);


   // 11) System Shutdown
   klogv("Starting system shutdown procedure...");

   /* Shutdown Procedure */
   klogv("Shutdown complete. You may now turn off the machine. (QEMU: C-a x)");
   hlt();
}

//getdate set dae get time setTimeout shutdown help version
