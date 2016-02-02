/*
  ----- kmain.c -----

  Description..: Kernel main. The first function called after
      the bootloader. Initialization of hardware, system
      structures, devices, and initial processes happens here.
*/

#include <stdint.h>
#include <string.h>
#include <system.h>

#include <core/io.h>
#include <core/serial.h>
#include <core/tables.h>
#include <core/interrupts.h>
#include <mem/heap.h>
#include <mem/paging.h>

#include "modules/mpx_supt.h"

int shutdown = 0;

void turnOff(){
  shutdown = 1;
}

void commandHandler(char* command){
  serial_println(strtok(command, " "));
  turnOff();
}



void kmain(void)
{
   extern uint32_t magic;
   // Uncomment if you want to access the multiboot header
   // extern void *mbd;
   // char *boot_loader_name = (char*)((long*)mbd)[16];


   // 0) Initialize Serial I/O and call mpx_init
   init_serial(COM1);
   set_serial_in(COM1);
   set_serial_out(COM1);
   mpx_init(MODULE_R1);
   klogv("Starting MPX boot sequence...");
   klogv("Initialized serial I/O on COM1 device...");
   // 1) Check that the boot was successful and correct when using grub
   // Comment this when booting the kernel directly using QEMU, etc.
   if ( magic != 0x2BADB002 ){
     //kpanic("Boot was not error free. Halting.");
   }

   init_gdt();
   init_idt();
   init_irq();
   irq_on();
   init_paging();

   // 2) Descriptor Tables
   klogv("Initializing descriptor tables...");

   // 4) Virtual Memory
   klogv("Initializing virtual memory...");

   // 5) Call Commhand
   klogv("Transferring control to commhand...");
   char buffer[400];
   char c[2];
   c[0] = 0;
   c[1] = '\0';
   int index = 0;
   serial_print("> ");
   while(shutdown != 1){
    if (inb(COM1+5)&1){
      c[0] = inb(COM1);
      switch(c[0]){
        case 13:
          serial_println("");
          //insert command handler here
          commandHandler(buffer);
          serial_print("> ");
          do{
            buffer[index] = '\0';
          }while(index-- > 0);
          index = 0;
          break;
        case 127:
          if(index != 0){
            buffer[index] = '\0';
            serial_print("\033[D ");
            serial_print("\033[D");
            index--;
          }
          break;
        case 27:
            c[0] = inb(COM1);
            c[0] = inb(COM1);
            switch(c[0]){
              case 'C':
                if(buffer[index] != '\0'){
                  serial_print("\033[C");
                  index++;
                }
                break;
              case 'D':
                if(index != 0){
                  serial_print("\033[D");
                  index--;
                }
              }
            //index++;
          break;
        default:
          buffer[index++] = c[0];
          serial_print(c);
        }
      }
    }
    //klogv(buffer);


   // 11) System Shutdown
   klogv("Starting system shutdown procedure...");

   /* Shutdown Procedure */
   klogv("Shutdown complete. You may now turn off the machine. (QEMU: C-a x)");
   hlt();
}

//getdate set dae get time setTimeout shutdown help version
