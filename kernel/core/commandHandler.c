#include <stdint.h>
#include <string.h>
#include <system.h>

#include <core/io.h>
#include <core/serial.h>
#include <core/interrupts.h>

int shutdown = 0;
int a =0;
int hour;
int min;
int sec;


void version(){
  serial_println("Version: MODULE_R1");
}

void turnOff(){
  serial_print("Are you sure you want to shutdown (y/n):");
  char c[2];
  c[0] = 0;
  c[1] = '\0';
  while(1){
   if (inb(COM1+5)&1){
     c[0] = inb(COM1);
     if(c[0] == 121){
       shutdown = 1;
     }
     serial_println(c);
     break;
   }
  }

}

void help(){
  serial_println("Command List:");
  serial_println(" getdate - displays the current system date.");
  serial_println(" setdate - sets current system date (dd/mm/yyyy).");
  serial_println(" settime - displays the current system time.");
  serial_println(" gettime - sets current system date (hh:mm:ss).");
  serial_println(" version - displays version information.");
  serial_println(" shutdown - shuts down the OS.");
  serial_println(" help - displays a list of commands and their uses.");
}

int BCDtoD(int bcd){
  int sum = 0;
  int factor = 1;
  do{
    sum += (bcd%10)*factor;
    factor = factor *2;
    bcd = bcd/10;
  }while(bcd > 0);
  return sum;
}

void itos(int num, char *number){

  int i = 1;
  //+48
  do{
    number[i] = (num%10)+48;
    i--;
    num = num/10;
  }while(num > 0);
  number[2] = '\0';
}

void getTime(){
  outb(0x70, 0x04);
  hour = inb(0x71);
  outb(0x70, 0x02);
  min = inb(0x71);
  outb(0x70, 0x00);
  sec = inb(0x71);
  hour = BCDtoD(hour);
  min = BCDtoD(min);
  sec = BCDtoD(sec);
  char number[3];
  itos(hour, number);
  serial_println(number);
}



void test(){
  outb(0x70, 0x04);
  a = inb(0x71);
  //char number[3];
  //itos(temp, number);
  //serial_println(number);
}

void parseCommand(char* command){
  char* com1 = strtok(command, " ");
  if(!strcmp(com1, "shutdown\0")){
    turnOff();
  } else if(!strcmp(com1, "version\0")){
    version();
  } else if(!strcmp(com1, "help\0")){
    help();
  } else if(!strcmp(com1, "gettime\0")){
    getTime();
  } else if(!strcmp(com1, "settime\0")){
    help();
  } else if(!strcmp(com1, "getdate\0")){
    help();
  } else if(!strcmp(com1, "setdate\0")){
    help();
  } else if(!strcmp(com1, "test\0")){
    test();
  } else {
    serial_println("Invalid command. User 'help' to get a complete list." );
  }
}

void commandHandler(){
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
         parseCommand(buffer);
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
}
