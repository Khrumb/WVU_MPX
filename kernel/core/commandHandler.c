#include <stdint.h>
#include <string.h>
#include <system.h>

#include <core/commandHandler.h>
#include <core/io.h>
#include <core/serial.h>
#include <core/interrupts.h>

int shutdown = 0;

/**
 * function name: version
 * Description: displays the current version of the project
 * Parameters: none
 * Returns: a string of the latest version to the user
*/
void version(){
  serial_println("Version: MODULE_R2");
}

/**
 * function name: turnOff
 * Description: prompts the user to shutdown the machine
 * Parameters: accepts a char(y or n) from the user
 * Returns: nothing, shuts down the machine
*/
void turnOff(){
  serial_println("Are you sure you want to shutdown (y/n):");
  serial_print("> ");
  char c[2];
  c[0] = 0;
  c[1] = '\0';
  while(shutdown==0){
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

/**
 * function name: help
 * Description: displays the command list for the user
 * Parameters: none
 * Returns: a help menu to the user
*/
void help(){
  serial_println("R1 Command List:");
  serial_println("getdate  - displays the current system date.");
  serial_println("gettime  - sets current system date (hh:mm:ss).");
  serial_println("help     - displays a list of commands and their uses.");
  serial_println("setdate  - sets current system date (dd/mm/yyyy).");
  serial_println("settime  - displays the current system time.");
  serial_println("shutdown - shuts down the OS.");
  serial_println("version  - displays version information.");
  serial_println("");
  serial_println("R2 Permanent Command List:");
  serial_println("suspend     - places a PCB in the suspended state");
  serial_println("resume      - places a PCB in the not suspended state");
  serial_println("setpriority - sets a PCB's priority and reinserts it into the correct queue");
  serial_println("showPCB     - displays the information for a PCB");
  serial_println("showall     - displays the information for all the PCBs");
  serial_println("showready   - displays the information for all PCBs in the ready queue");
  serial_println("showblocked - displays the information for all PCBs in the blocked queue");
  serial_println("");
  serial_println("R2 Temporary Command List:");
  serial_println("createPCB - creates a new PCB");
  serial_println("deletePCB - deletes a PCB and removes it from memory");
  serial_println("block     - places a PCB in the blocked state");
  serial_println("unblock   - places a PCB in the unblocked state");
}

/**
 * function name: itoa
 * Description: converts an integer to a string
 * Parameters: takes in an integer value and a base decimal value(ie 8 for octal)
 * Returns: a string number
*/
char* itoa(int val, int base){
  static char buf[32] = {0};
	int i = 30;
	for(; val && i ; --i, val /= base)
		buf[i] = "0123456789abcdef"[val % base];
	return &buf[i+1];
}

/**
 * function name: BCDtoDec
 * Description: converts BCD char to a decimal char value
 * Parameters: takes in a BCD char value
 * Returns: a decimal char value
*/
char BCDtoDec(char bcd){
  return (((bcd & 0xF0) >> 4) * 10) + (bcd & 0x0F);
}

/**
 * function name: DectoBCD
 * Description: converts decimal char to a BCD char value
 * Parameters: takes in a decimal char value
 * Returns: a BCD char value
*/
char DectoBCD(int dec){
  return ((dec / 10) << 4) + (dec % 10);
}

/**
 * function name: formatNum
 * Description: adds a zero to any single digit values
 * Parameters: takes in a number character
 * Returns: a formatted number character
*/
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

/**
 * function name: asciiToDec
 * Description: converts ASCII characters to number values
 * Parameters: a number character
 * Returns: an integer
*/
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

/**
 * function name: getTime
 * Description: reads the internal clock and outputs the clock values
 * Parameters: none
 * Returns: a string clock value to the user
*/
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

  char *n, *num;
  n = itoa(hour, 10);
  num = formatNum(n);
  serial_print(num);
  serial_print(":");
  n = itoa(min, 10);
  num = formatNum(n);
  serial_print(num);
  serial_print(":");
  n = itoa(sec, 10);
  num = formatNum(n);
  serial_print(num);
  serial_println("");
}

/**
 * function name: parseTime
 * Description: parses the user input and sets the internal clock if valid input is given
 * Parameters: character buffer from the user
 * Returns: a boolean integer whether the clock was set or not
*/
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
  if(m<0 || m>59)
    return 1;
  char min = DectoBCD(m);
  tens = asciiToDec(buffer[6]);
  ones = asciiToDec(buffer[7]);
  int s = tens * 10 + ones;
  if(s<0 || s>59)
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

/**
 * function name: setTime
 * Description: accepts input from the user to be used to change the internal clock
 * Parameters: none
 * Returns: nothing, changes the internal clock values
*/
void setTime(){
  serial_println("Enter the time in the format 00:00:00 or e to exit");
  char buffer[400];
  char c[2];
  int index = 0;
  int input = 1;
  c[0] = 0;
  c[1] = '\0';
  serial_print("> ");
  while(input==1){
   if (inb(COM1+5)&1){
     c[0] = inb(COM1);
     if(c[0]==13){
         serial_println("");
	 if(parseTime(buffer)==0){
	   input=0;
	 }
         else if(!strcmp(buffer, "e\0")){
	   serial_println("Exiting...");
	   input=0;
         }
	 else{
	   serial_println("Error invalid time. Please enter a valid time in the format 00:00:00 or e to exit");
	   serial_print("> ");
	 }
         do{
           buffer[index] = '\0';
         }while(index-- > 0);
         index = 0;
     }
     else if(c[0]==127){
         if(index != 0){
           buffer[index] = '\0';
           serial_print("\033[D ");
           serial_print("\033[D");
           index--;
         }
     }
     else if(c[0]==27){
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
	      break;
         }
     }
     else{
	buffer[index++] = c[0];
        serial_print(c);
     }
   }
  }
}

/**
 * function name: getDate
 * Description: reads the internal date and outputs the date values
 * Parameters: none
 * Returns: a string date value to the user
*/
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

  char *n, *num;
  n = itoa(month, 10);
  num = formatNum(n);
  serial_print(num);
  serial_print("/");
  n = itoa(day, 10);
  num = formatNum(n);
  serial_print(num);
  serial_print("/");
  n = itoa(year, 10);
  num = formatNum(n);
  serial_print(num);
  serial_println("");
}

/**
 * function name: parseDate
 * Description: parses the user input and sets the internal date if valid input is given
 * Parameters: character buffer from the user
 * Returns: a boolean integer whether the date was set or not
*/
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

/**
 * function name: setDate
 * Description: accepts input from the user to be used to change the internal date
 * Parameters: none
 * Returns: nothing, changes the date clock values
*/
void setDate(){
  serial_println("Enter the date in the format 01/01/01 or e to exit");
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
         else if(!strcmp(buffer, "e\0")){
	   serial_println("Exiting...");
	   input=0;
         }
	 else{
	   serial_println("Error invalid date. Please enter a valid date in the format 00/00/00 or e to exit");
	   serial_print("> ");
	 }
         do{
           buffer[index] = '\0';
         }while(index-- > 0);
         index = 0;
     }
     else if(c[0]==127){
         if(index != 0){
           buffer[index] = '\0';
           serial_print("\033[D ");
           serial_print("\033[D");
           index--;
         }
     }
     else if(c[0]==27){
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
	      break;
         }
     }
     else{
	buffer[index++] = c[0];
        serial_print(c);
     }
   }
  }
}

/**
 * function name: parseCommand
 * Description: compares user input to the valid list of commands
 * Parameters: a character pointer that points to the user input
 * Returns: nothing, calls commands based on user input
*/
void parseCommand(char* command){
  if(!strcmp(command, "shutdown\0")){
    turnOff();
  } else if(!strcmp(command, "version\0")){
    version();
  } else if(!strcmp(command, "help\0")){
    help();
  } else if(!strcmp(command, "gettime\0") || !strcmp(command, "getTime\0")){
    getTime();
  } else if(!strcmp(command, "settime\0") || !strcmp(command, "setTime\0")){
    setTime();
  } else if(!strcmp(command, "getdate\0") || !strcmp(command, "getDate\0")){
    getDate();
  } else if(!strcmp(command, "setdate\0") || !strcmp(command, "setDate\0")){
    setDate();
  } else if(!strcmp(command, "suspend\0")){
    help();
  } else if(!strcmp(command, "resume\0")){
    help();
  } else if(!strcmp(command, "setpriority\0") || !strcmp(command, "setPriority\0")){
    help();
  } else if(!strcmp(command, "showPCB\0")){
    help();
  } else if(!strcmp(command, "showall\0") || !strcmp(command, "showAll\0")){
    help();
  } else if(!strcmp(command, "showready\0") || !strcmp(command, "showReady\0")){
    help();
  } else if(!strcmp(command, "showblocked\0") || !strcmp(command, "showBlocked\0")){
    help();
  } else if(!strcmp(command, "createPCB\0")){
    help();
  } else if(!strcmp(command, "deletePCB\0")){
    help();
  } else if(!strcmp(command, "block\0")){
    help();
  } else if(!strcmp(command, "unblock\0")){
    help();
  } else {
    serial_println("Invalid command. Use 'help' to get a complete list." );
  }
}

/**
 * function name: commandHandler
 * Description: accepts input from the user and parses it to be used to handle commands
 * Parameters: none
 * Returns: nothing
*/
void commandHandler(){
  char buffer[400];
  char c[2];
  int index = 0;
  c[0] = 0;
  c[1] = '\0';
  serial_println("-----------------------------------------");
  serial_println("Welcome to the latest GovEmps OS version!");
  serial_println("Please enter a valid command to begin or 'help' to see the list of valid commands");
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
	       break;
             }
         break;
       default:
         buffer[index++] = c[0];
         serial_print(c);
       }
     }
   }
}
