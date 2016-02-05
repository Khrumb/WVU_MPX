#include <stdint.h>
#include <string.h>
#include <system.h>

#include <core/io.h>
#include <core/serial.h>
#include <core/interrupts.h>

int shutdown = 0;
int a =0;

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

char* itoa(int val, int base){
  static char buf[32] = {0};
	int i = 30;
	for(; val && i ; --i, val /= base)
		buf[i] = "0123456789abcdef"[val % base];
	return &buf[i+1];
}

char BCDtoDec(char bcd){
  return (((bcd & 0xF0) >> 4) * 10) + (bcd & 0x0F);
}

char DectoBCD(int dec){
  return ((dec / 10) << 4) + (dec % 10);
}

char *formatNum(char *num){
  char* num1 = strtok(num, " ");
  if(!strcmp(num, "\0")){
    return "00";
  } else if(!strcmp(num1, "1\0")){
    return "01";
  } else if(!strcmp(num1, "2\0")){
    return "02";
  } else if(!strcmp(num1, "3\0")){
    return "03";
  } else if(!strcmp(num1, "4\0")){
    return "04";
  } else if(!strcmp(num, "5\0")){
    return "05";
  } else if(!strcmp(num1, "6\0")){
    return "06";
  } else if(!strcmp(num1, "7\0")){
    return "07";
  } else if(!strcmp(num1, "8\0")){
    return "08";
  } else if(!strcmp(num1, "9\0")){
    return "09";
  } else
    return num;
}

void getTime(){
  outb(0x70, 0x04);
  char hour = inb(0x71);
  outb(0x70, 0x02);
  char min = inb(0x71);
  outb(0x70, 0x00);
  char sec = inb(0x71);
  hour = BCDtoDec(hour);
  min = BCDtoDec(min);  
  sec = BCDtoDec(sec);
  char *h, *numHours;
  h = itoa(hour, 10);
  numHours = formatNum(h);
  serial_print(numHours);
  serial_print(":");
  char *m, *numMins;
  m = itoa(min, 10);
  numMins = formatNum(m);
  serial_print(numMins);
  serial_print(":");
  char *s, *numSecs;
  s = itoa(sec, 10);
  numSecs = formatNum(s);
  serial_print(numSecs);
  serial_println("");
}

int asciiToDec(char num){
  switch(num){
    case 48:
	return 0;
	break;
    case 49:
	return 1;
	break;
    case 50:
	return 2;
	break;
    case 51:
	return 3;
	break;
    case 52:
	return 4;
	break;
    case 53:
	return 5;
	break;
    case 54:
	return 6;
	break;
    case 55:
	return 7;
	break;
    case 56:
	return 8;
	break;
    case 57:
	return 9;
	break;
  }
  return -1; 
}


int parseTime(char buffer[]){
  int tens = asciiToDec(buffer[0]);
  int ones = asciiToDec(buffer[1]);
  int h = tens * 10 + ones;
  if(h<0 || h>12)
    return 1;
  char hour = DectoBCD(h);
  tens = asciiToDec(buffer[3]);
  ones = asciiToDec(buffer[4]);
  int m = tens * 10 + ones;
  if(m<0 || m>60)
    return 1;
  char min = DectoBCD(m);
  tens = asciiToDec(buffer[6]);
  ones = asciiToDec(buffer[7]);
  int s = tens * 10 + ones;
  if(s<0 || s>60)
    return 1;
  char sec = DectoBCD(s);
  cli();
  outb(0x70, 0x04);
  outb(0x71, hour);
  outb(0x70, 0x02);
  outb(0x71, min);
  outb(0x70, 0x00);
  outb(0x71, sec);
  sti();
  return 0;
}

void setTime(){
  serial_println("Enter the time in the format 00:00:00");
  char buffer[400];
  char c[2];
  c[0] = 0;
  c[1] = '\0';
  int index = 0;
  int input = 1;
  serial_print("> ");
  while(input==1){
   if (inb(COM1+5)&1){
     c[0] = inb(COM1);
     if(c[0]==13){
         serial_println("");
	 if(parseTime(buffer)==0){
	   input=0;
	 }
	 else{
	   serial_println("Error invalid time. Please enter a valid time in the format 00:00:00");
	   serial_print("> ");
	 }
         do{
           buffer[index] = '\0';
         }while(index-- > 0);
         index = 0;
     }
     else{
	buffer[index++] = c[0];
        serial_print(c);
     }
   }
  }
}

void getDate(){
  outb(0x70, 0x07);
  char day = inb(0x71);
  outb(0x70, 0x08);
  char month = inb(0x71);
  outb(0x70, 0x09);
  char year = inb(0x71);
  day = BCDtoDec(day);
  month = BCDtoDec(month);  
  year = BCDtoDec(year);
  char *m, *numMonths;
  m = itoa(month, 10);
  numMonths = formatNum(m);
  serial_print(numMonths);
  serial_print("/");
  char *d, *numDays;
  d = itoa(day, 10);
  numDays = formatNum(d);
  serial_print(numDays);
  serial_print("/");
  char *y, *numYears;
  y = itoa(year, 10);
  numYears = formatNum(y);
  serial_print(numYears);
  serial_println("");
}

int parseDate(char buffer[]){
  int tens = asciiToDec(buffer[0]);
  int ones = asciiToDec(buffer[1]);
  int m = tens * 10 + ones;
  if(m<1 || m>12)
    return 1;
  char month = DectoBCD(m);
  tens = asciiToDec(buffer[3]);
  ones = asciiToDec(buffer[4]);
  int d = tens * 10 + ones;
  if(d<1 || d>31)
    return 1;
  char day = DectoBCD(d);
  tens = asciiToDec(buffer[6]);
  ones = asciiToDec(buffer[7]);
  int y = tens * 10 + ones;
  char year = DectoBCD(y);
  cli();
  outb(0x70, 0x07);
  outb(0x71, day);
  outb(0x70, 0x08);
  outb(0x71, month);
  outb(0x70, 0x09);
  outb(0x71, year);
  sti();
  return 0;
}


void setDate(){
  serial_println("Enter the date in the format 01/01/01");
  char buffer[400];
  char c[2];
  c[0] = 0;
  c[1] = '\0';
  int index = 0;
  int input = 1;
  serial_print("> ");
  while(input==1){
   if (inb(COM1+5)&1){
     c[0] = inb(COM1);
     if(c[0]==13){
         serial_println("");
	 if(parseDate(buffer)==0){
	   input=0;
	 }
	 else{
	   serial_println("Error invalid date. Please enter a valid date in the format 00/00/00");
	   serial_print("> ");
	 }
         do{
           buffer[index] = '\0';
         }while(index-- > 0);
         index = 0;
     }
     else{
	buffer[index++] = c[0];
        serial_print(c);
     }
   }
  }
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
    setTime();
  } else if(!strcmp(com1, "getdate\0")){
    getDate();
  } else if(!strcmp(com1, "setdate\0")){
    setDate();
  } else {
    serial_println("Invalid command. Use 'help' to get a complete list." );
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
