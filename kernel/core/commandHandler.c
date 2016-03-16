#include <stdint.h>
#include <string.h>
#include <system.h>

#include <core/PCB.h>
#include <core/commandHandler.h>
#include <core/sys_call.h>
#include <core/io.h>
#include <core/serial.h>
#include <core/interrupts.h>

#include "modules/mpx_supt.h"
#include "modules/load3r.c"

int shutdown = 0;
struct command_history* history;

//innit buffers
char command_buffer[30];
char argument_buffer[50];
char* buffer = command_buffer;

//init index pointers
int command_index = 0;
int argument_index = 0;
int* index = &command_index;

//init arguments parser
char * arguments[10];
int numberOfArguments = 0;


/**
 * function name: version
 * Description: displays the current version of the project
 * Parameters: none
 * Returns: a string of the latest version to the user
*/
void version(){
  serial_println("Version: MODULE_R3/4");
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
  serial_println("gettime  - sets current system date.");
  serial_println("help     - displays a list of commands and their uses.");
  serial_println("setdate  - sets current system date. Syntax: setdate [dd/mm/yy]");
  serial_println("settime  - displays the current system time.  Syntax: settime [hh:mm:ss]");
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
  serial_println("createPCB - creates a new PCB Syntax: createPCB [name class priority(0-9)]");
  serial_println("deletePCB - deletes a PCB and removes it from memory");
  serial_println("block     - places a PCB in the blocked state");
  serial_println("unblock   - places a PCB in the unblocked state");
  serial_println("");
  serial_println("R3 Temporary Command List:");
  serial_println("yield - yields CPU time to other processes");
  serial_println("loadr3 - loads all R3 processes into memory in a suspended ready state");
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
void setTime(char ** argument){
  int length = strlen(argument[0]);
  if(length == 8){
    int i;
    for(i = 0; i <= 8; i++){
      switch (i) {
        case 0:
        case 1:
        case 3:
        case 4:
        case 6:
        case 7:
          if(asciiToDec(argument[0][i]) == -1){
            serial_println("Error: Invalid time. Use 'help' for syntax.");

            i = 9;
          }
          break;
        case 2:
        case 5:
          if(argument[0][i] != ':'){
            serial_println("Error: Invalid time. Use 'help' for syntax.");
            i = 9;
          }
          break;
        case 8:
          if(argument[0][i] == '\0'){
            parseTime(argument[0]);
            serial_println("Time Set.");
          }
          break;
        default:
          serial_println("Error: Argument too long.");
      }
    }
  } else {
    serial_println("Error: Argument too long. Use 'help' for syntax.");
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
void setDate(char ** argument){
  int length = strlen(argument[0]);
  if(length == 8){
    int i;
    for(i = 0; i <= 8; i++){
      switch (i) {
        case 0:
        case 1:
        case 3:
        case 4:
        case 6:
        case 7:
          if(asciiToDec(argument[0][i]) == -1){
            serial_println("Error: Invalid date. Use 'help' for syntax.");
            i = 9;
          }
          break;
        case 2:
        case 5:
          if(argument[0][i] != '/'){
            serial_println("Error: Invalid date. Use 'help' for syntax.");
            i = 9;
          }
          break;
        case 8:
          if(argument[0][i] == '\0'){
            parseDate(argument[0]);
            serial_println("Date Set.");
          }
          break;
        default:
          serial_println("Error: Argument too long.");
      }
    }
  } else {
    serial_println("Error: Argument too long. Use 'help' for syntax.");
  }
}

/**
 * function name: createPCB
 * Description: calls setupPCB() and inserts it into the appropriate queue
 * Parameters: process name, class, and priority
 * Returns: error if the command entered is invalid
*/
void createPCB(char **arguments){
  int class = asciiToDec(arguments[1][0]);
  int priority = asciiToDec(arguments[2][0]);

  if(class != -1 && class  <= 1 && arguments[1][1] == '\0'){
    if(priority != -1 && arguments[2][1] == '\0'){
      struct pcb* newPCB = SetupPCB(arguments[0], class, priority);
      if(newPCB == NULL){
        serial_println("ERROR: PROCESS NAME TAKEN.");
      } else {
        InsertPCB(newPCB);
        serial_println("PCB created.");
      }
    } else {
      serial_println("ERROR: invalid priority. [LEVELS: 0 - 9]");
    }
  } else {
    serial_println("ERROR: invalid class. [0:SYSTEM | 1:APPLICATION]");
  }
}

/**
 * function name: deletePCB
 * Description: deletes pcb and frees all associated memory
 * Parameter: process name
 * Returns: error if pcb name doesn’t exist
*/
void deletePCB(char **arguments){
  struct pcb* remove_pcb = FindPCB(arguments[0]);
  if(remove_pcb != NULL){
    int error = RemovePCB(remove_pcb);
    if(error == 0){
      serial_println("PCB deleted.");
    } else {
      serial_println("ERROR: could not remove PCB.");
    }
  } else {
    serial_println("ERROR: No PCB with that name found.");
  }
}

/**
 * function name: blockPCB
 * Description: puts PCB in a blocked state and re-inserts it to the appropriate queue
 * Parameter: process name
 * Returns: error if PCB is not found
*/
void blockPCB(char **arguments){
  struct pcb* blocked_pcb = FindPCB(arguments[0]);
  if(blocked_pcb != NULL){
    if(blocked_pcb->running_state != 2){
      RemovePCB(blocked_pcb);
      blocked_pcb->running_state = BLOCKED;
      InsertPCB(blocked_pcb);
      serial_println("PCB blocked and moved to the blocked queue.");
    } else{
      serial_println("PCB is already blocked");
    }
  } else {
    serial_println("ERROR: No PCB with that name found.");
  }
}

/**
 * function name: unblockPCB
 * Description: puts PCB in a unblocked state and re-inserts it to the appropriate queue
 * Parameter: process name
 * Returns: error if PCB is not found
*/
void unblockPCB(char **arguments){
  struct pcb* unblocked_pcb = FindPCB(arguments[0]);
  if(unblocked_pcb != NULL){
    if(unblocked_pcb->running_state != 0){
      RemovePCB(unblocked_pcb);
      unblocked_pcb->running_state = READY;
      InsertPCB(unblocked_pcb);
      serial_println("PCB unblocked and returned to the ready queue.");
    } else{
      serial_println("PCB is already unblocked");
    }
  } else {
    serial_println("ERROR: No PCB with that name found.");
  }
}

/**
 * function name: suspendPCB
 * Description: suspends PCB and re-inserts it to the appropriate queue
 * Parameter: process name
 * Returns: error if PCB is not found
*/
void suspendPCB(char **arguments){
  struct pcb* suspended_pcb = FindPCB(arguments[0]);
  if(suspended_pcb != NULL){
    suspended_pcb->suspended_state = SUSPENDED;
    serial_println("PCB suspended.");
  } else {
    serial_println("ERROR: No PCB with that name found.");
  }
}

/**
 * function name: resumePCB
 * Description: changed the PCB to a non-suspended state and re-inserts it to the appropriate queue
 * Parameter: process name
 * Returns: error if PCB is not found
*/
void resumePCB(char **arguments){
  struct pcb* resumed_pcb = FindPCB(arguments[0]);
  if(resumed_pcb != NULL){
    resumed_pcb->suspended_state = NOT_SUSPENDED;
    serial_println("PCB resumed.");
  } else {
    serial_println("ERROR: No PCB with that name found.");
  }
}

/**
 * function name: setPriorityPCB
 * Description: changes PCB's priority and re-inserts it in the appropriate queue and position
 * Parameter: process name, new priority
 * Returns: error if PCB is not found or priority is outside 0-9
*/
void setPriority(char **arguments){
  struct pcb* current_pcb = FindPCB(arguments[0]);
  int priority = asciiToDec(arguments[1][0]);
  if(current_pcb != NULL){
    if(current_pcb->running_state != 2 && priority <= 9 && arguments[1][1] == '\0'){
      RemovePCB(current_pcb);
      current_pcb->priority = priority;
      InsertPCB(current_pcb);
      serial_println("PCB priority changed.");
    } else if(priority <= 9 && arguments[1][1] == '\0') {
        current_pcb->priority = priority;
        serial_println("PCB priority changed but PCB is still blocked");
    } else {
        serial_println("ERROR: Invalid Priority found.");
    }
  } else {
    serial_println("ERROR: No PCB with that name found.");
  }
}

/**
 * function name: printPCB
 * Description: prints out information about a given PCB
 * Parameters: a valid PCB
 * Returns: none
*/
void printPCB(struct pcb* current_pcb){
  char *info, *num;
  info = current_pcb->name;
  serial_print("Name: ");
  serial_println(info);
  serial_print("Class: ");
  if(current_pcb->class == 0)
    serial_println("SYSTEM");
  else if(current_pcb->class == 1)
    serial_println("APPLICATION");
  serial_print("State: ");
  if(current_pcb->running_state == 0)
    serial_println("READY");
  else if(current_pcb->running_state == 1)
    serial_println("RUNNING");
  else if(current_pcb->running_state == 2)
    serial_println("BLOCKED");
  serial_print("Suspended Status: ");
  if(current_pcb->suspended_state == 0)
    serial_println("SUSPENDED");
  else if(current_pcb->suspended_state == 1)
    serial_println("NOT_SUSPENDED");
  info = itoa(current_pcb->priority, 10);
  num = formatNum(info);
  serial_print("Priority: ");
  serial_println(num);
  serial_println("");
}

/**
 * function name: showPCB
 * Description: displaies a PCB's name, state, class, status, and priority
 * Parameter: process name
 * Returns: error if PCB is not found
*/
void showPCB(char **arguments){
  char *name = arguments[0];
  if(FindPCB(name) != NULL){
    struct pcb* current_pcb = FindPCB(name);
    printPCB(current_pcb);
  }
  else
    serial_println("PCB not found");
}

/**
 * function name: showReady
 * Description: displays every PCB in the ready queue's name, state, class, status, and priority
 * Parameter: none
 * Returns: none
*/
void showReady(){
  struct pcb* current_pcb;
  if(ready->head != NULL  && ready->count > 0){
    serial_println("		Ready Queue:");
    current_pcb = ready->head;
    int i;
    for(i=0; i<ready->count; i++){
      printPCB(current_pcb);
      current_pcb = current_pcb->next;
    }
  }
  else
    serial_println("No PCBs in the ready queue");
}

/**
 * function name: showBlocked
 * Description: displays every PCB in the blocked queue's name, state, class, status, and priority
 * Parameter: none
 * Returns: none
*/
void showBlocked(){
  struct pcb* current_pcb;
  if(blocked->head != NULL && blocked->count > 0){
    serial_println("		Blocked Queue:");
    current_pcb = blocked->head;
    int i;
    for(i=0; i<blocked->count; i++){
      printPCB(current_pcb);
      current_pcb = current_pcb->next;
    }
  }
  else
    serial_println("No PCBs in the blocked queue");
}

/**
 * function name: showAll
 * Description: displays every PCB in the ready and blocked queues' name, state, class, status, and priority
 * Parameter: none
 * Returns: none
*/
void showAll(){
  showReady();
  showBlocked();
}

/**
 * function name: yield
 * Description: yields CPU time to other processes
 * Parameter: none
 * Returns: none
*/
void yield(){
  asm volatile("int $60");
}

/**
 * function name: loadr3
 * Description: loads all R3 processes into the suspended ready queue
 * Parameter: none
 * Returns: pointer to a PCB
*/
struct pcb* loadr3(){
  char* name = "ok";
  pcb* new_pcb = SetupPCB(name, 1, 1);
  context* cp = (context*)(new_pcb->stack_top);
  memset(cp, 0, sizeof(context));
  cp->fs = 0x10;
  cp->gs = 0x10;
  cp->ds = 0x10;
  cp->es = 0x10;
  cp->cs = 0x8;
  cp->ebp = (u32int)(new_pcb->stack_bottom);
  cp->esp = (u32int)(new_pcb->stack_top);
  cp->eip = (u32int)(proc1); //will be replaced by func name ie) proc1
  cp->eflags = 0x202;
  InsertPCB(new_pcb);
  name = "ok1";
  new_pcb = SetupPCB(name, 1, 1);
  cp = (context*)(new_pcb->stack_top);
  memset(cp, 0, sizeof(context));
  cp->fs = 0x10;
  cp->gs = 0x10;
  cp->ds = 0x10;
  cp->es = 0x10;
  cp->cs = 0x8;
  cp->ebp = (u32int)(new_pcb->stack_bottom);
  cp->esp = (u32int)(new_pcb->stack_top);
  cp->eip = (u32int)(proc2); //will be replaced by func name ie) proc1
  cp->eflags = 0x202;
  InsertPCB(new_pcb);
  name = "ok2";
  new_pcb = SetupPCB(name, 1, 1);
  cp = (context*)(new_pcb->stack_top);
  memset(cp, 0, sizeof(context));
  cp->fs = 0x10;
  cp->gs = 0x10;
  cp->ds = 0x10;
  cp->es = 0x10;
  cp->cs = 0x8;
  cp->ebp = (u32int)(new_pcb->stack_bottom);
  cp->esp = (u32int)(new_pcb->stack_top);
  cp->eip = (u32int)(proc3); //will be replaced by func name ie) proc1
  cp->eflags = 0x202;
  InsertPCB(new_pcb);
  name = "ok3";
  new_pcb = SetupPCB(name, 1, 1);
  cp = (context*)(new_pcb->stack_top);
  memset(cp, 0, sizeof(context));
  cp->fs = 0x10;
  cp->gs = 0x10;
  cp->ds = 0x10;
  cp->es = 0x10;
  cp->cs = 0x8;
  cp->ebp = (u32int)(new_pcb->stack_bottom);
  cp->esp = (u32int)(new_pcb->stack_top);
  cp->eip = (u32int)(proc4); //will be replaced by func name ie) proc1
  cp->eflags = 0x202;
  InsertPCB(new_pcb);
  name = "ok4";
  new_pcb = SetupPCB(name, 1, 1);
  cp = (context*)(new_pcb->stack_top);
  memset(cp, 0, sizeof(context));
  cp->fs = 0x10;
  cp->gs = 0x10;
  cp->ds = 0x10;
  cp->es = 0x10;
  cp->cs = 0x8;
  cp->ebp = (u32int)(new_pcb->stack_bottom);
  cp->esp = (u32int)(new_pcb->stack_top);
  cp->eip = (u32int)(proc5); //will be replaced by func name ie) proc1
  cp->eflags = 0x202;
  return new_pcb;
}

/**
 * function name: parseCommand
 * Description: compares user input to the valid list of commands
 * Parameters: a character pointer that points to the user input
 * Returns: nothing, calls commands based on user input
*/
void parseCommand(char* command, char** arguments){
    if(!strcmp(command, "shutdown\0")){
      turnOff();
    } else if(!strcmp(command, "version\0")){
      version();
    } else if(!strcmp(command, "help\0")){
      help();
    } else if(!strcmp(command, "gettime\0") || !strcmp(command, "getTime\0")){
      getTime();
    } else if(!strcmp(command, "getdate\0") || !strcmp(command, "getDate\0")){
      getDate();
    } else if(!strcmp(command, "showall\0") || !strcmp(command, "showAll\0")){
      showAll();
    } else if(!strcmp(command, "showready\0") || !strcmp(command, "showReady\0")){
      showReady();
    } else if(!strcmp(command, "showblocked\0") || !strcmp(command, "showBlocked\0")){
      showBlocked();
    } else if(!strcmp(command, "yield\0")){
      yield();
    } else if(!strcmp(command, "loadr3\0")){
      InsertPCB(loadr3());
    } else
    /*
    PLEASE NOTE ARGUMENTS IS AN ARRAY OF CHARACTER ARRAYS, (ARRAY OF POINTERS)
    */
    if(!strcmp(command, "settime\0") || !strcmp(command, "setTime\0")){
      setTime(arguments);
    } else if(!strcmp(command, "setdate\0") || !strcmp(command, "setDate\0")){
      setDate(arguments);
    } else if(!strcmp(command, "createPCB\0")){
      createPCB(arguments);
    } else if(!strcmp(command, "deletePCB\0")){
      deletePCB(arguments);
    } else if(!strcmp(command, "showPCB\0")){
      showPCB(arguments);
    } else if(!strcmp(command, "setpriority\0") || !strcmp(command, "setPriority\0")){
      setPriority(arguments);
    } else if(!strcmp(command, "block\0")){
      blockPCB(arguments);
    } else if(!strcmp(command, "unblock\0")){
      unblockPCB(arguments);
    } else if(!strcmp(command, "suspend\0")){
      suspendPCB(arguments);
    } else if(!strcmp(command, "resume\0")){
      resumePCB(arguments);
    } else {
      serial_println("Invalid command. Use 'help' to get a complete list." );
    }

}

void init_command_history() {
  history =  (struct command_history*) sys_alloc_mem((size_t) sizeof(struct command_history));
  history->length = 0;
  history->head = NULL;
  history->tail = NULL;
}

void add_history_entry(struct entry* new_ent){
  if(history->tail == NULL){
    history->head = new_ent;
    history->tail = new_ent;
    history->length += 1;
    new_ent->next = NULL;
    new_ent->prev = NULL;
  } else {

    history->tail->next = new_ent;
    new_ent->prev = history->tail;
    new_ent->next = NULL;
    history->tail = new_ent;
    history->length += 1;
  }
}

void clearLine(){
  int i = 0;
  while(i++ <= 30){
    serial_print("\033[C");
  }
  i = 0;
  while(i++ <= 50){
    serial_print("\033[D \033[D");
  }
  serial_print("> ");
}

void resetBuffers(){
  command_index = 29;
  do{
    command_buffer[command_index] = '\0';
  }while(command_index-- != -1);
  argument_index = 49;
  do{
    argument_buffer[argument_index] = '\0';
  }while(argument_index-- != -1);
  numberOfArguments++;
  do{
    arguments[numberOfArguments] = NULL;
  }while(numberOfArguments-- >= 0);
  argument_index = 0;
  command_index = 0;
  numberOfArguments = 0;

  buffer = command_buffer;
  index = &command_index;
}

void populateBuffers(struct entry* current_entry){
  int i = 0;
  while(current_entry->command_buffer[i]!= '\0'){
    command_buffer[i] = current_entry->command_buffer[i];
    i++;
  }
  //command_buffer[i] = '\0';
  command_index = i;
  i = 0;
  while(current_entry->argument_buffer[i] != '\0' ){
    argument_buffer[i] = current_entry->argument_buffer[i];
    i++;
  }
  if(i > 0){
    buffer = argument_buffer;
    index = &argument_index;
  } else {
    buffer = command_buffer;
    index = &command_index;
  }
  //argument_buffer[i] = '\0';
  argument_index = i;
}
/**
 * function name: commandHandler
 * Description: accepts input from the user and parses it to be used to handle commands
 * Parameters: none
 * Returns: nothing
*/
void commandHandler(){
  //init history structure
  init_command_history();
  struct entry* current_entry = NULL;
  current_entry = NULL;
  resetBuffers();

  char* character = "~";

  serial_println("-----------------------------------------");
  serial_println("Welcome to the latest GovEmps OS version!");
  serial_println("Please enter a valid command to begin or 'help' to see the list of valid commands");
  serial_print("> ");
  init_queues();
  while(shutdown != 1){
   if (inb(COM1+5)&1){
     *character = inb(COM1);
     if(isspace(character)!=1){
       switch(*character){
         case 27:
             *character = inb(COM1);
             *character = inb(COM1);
             switch(*character){
                case 'A':
                    if(current_entry == NULL){
                      if(history->tail !=NULL){
                        current_entry = history->tail;
                      }
                    } else {
                      if(current_entry->prev != NULL){
                        current_entry = current_entry->prev;
                      }
                    }
                    if(current_entry!= NULL){
                      resetBuffers();
                      clearLine();
                      serial_print(current_entry->command_buffer);
                      serial_print(" ");
                      serial_print(current_entry->argument_buffer);
                      populateBuffers(current_entry);

                    }
  	            break;
               case 'B':
                  resetBuffers();
                  clearLine();
                  if(current_entry!= NULL && current_entry->next != NULL){
                    current_entry = current_entry->next;
                    serial_print(current_entry->command_buffer);
                    serial_print(" ");
                    serial_print(current_entry->argument_buffer);
                    populateBuffers(current_entry);
                  } else {
                    current_entry = NULL;
                  }
   	            break;
               case 'C':
                 if(buffer[*index] != '\0'){
                   serial_print("\033[C");
                   (*index)++;
                 }
                 break;
               case 'D':
                 if(*index != 0){
                   serial_print("\033[D");
                   (*index)--;
                 } else {
                   if(index!=&command_index){
                     index = &command_index;
                     serial_print("\033[D");
                   }
                 }
  	              break;
               }
           break;
        case 127:
          if(*index != 0){
            serial_print("\b \b");
            (*index)--;
            buffer[*index] = ' ';

          } else {
            if(index != &command_index){
              index = &command_index;
              buffer = command_buffer;
            }
            buffer[*index] = ' ';
          }
           break;
         default:
           buffer[*index] = *character;
           (*index)++;
           serial_print(character);
         }
     } else {
       if(*character == 13){
         serial_println("");

          //adding command to history.
         struct entry* new_entry =  (struct entry*) sys_alloc_mem((size_t) sizeof(struct entry));
         new_entry->argument_length = argument_index;
         int i = 0;
         do{
           new_entry->command_buffer[command_index] = command_buffer[command_index];
         }while(command_index-- >= 0);
         do{
           new_entry->argument_buffer[i] = argument_buffer[i];
            i++;
         }while(argument_buffer[i] != '\0');
         if(strlen(new_entry->command_buffer) != 0 ){
           add_history_entry(new_entry);
         }
         i = 0;

         //parsing arguments
         int argnum = 1;
         arguments[0] = argument_buffer;
         do{
          if(argument_buffer[i] == 32){
            argument_buffer[i] = '\0';
            arguments[argnum] = argument_buffer+i+1;
            argnum++;
          }
          i++;
         }while(argument_buffer[i] != '\0');
         arguments[argnum] = NULL;
         parseCommand(command_buffer, arguments);
         current_entry = NULL;
         resetBuffers();
         serial_print("> ");
       } else {
         if(buffer != argument_buffer){
           buffer[(*index)++] = '\0';
           buffer = argument_buffer;
           index = &argument_index;
           serial_print(" ");
         } else {
           buffer[*index] = *character;
           (*index)++;
           serial_print(character);
         }
       }
      }
     }
   }
 }
