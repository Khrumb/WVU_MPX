/**
* @file console.h
* This is the command handler
*/

#ifndef _COMMANDHANDLER_H
#define _COMMANDHANDLER_H

extern int shutdown;

typedef struct command_history{
  struct entry* head;
  struct entry* tail;
  int length;
} command_history;


typedef struct entry{
  char command_buffer[30];
  char argument_buffer[50];
  int argument_length;

  struct entry* next;
  struct entry* prev;
} enrty;

void setupCommHand();

/**
 * function name: version
 * Description: displays the current version of the project
 * Parameters: none
 * Returns: a string of the latest version to the user
*/
void version();

/**
 * function name: turnOff
 * Description: prompts the user to shutdown the machine
 * Parameters: accepts a char(y or n) from the user
 * Returns: nothing, shuts down the machine
*/
void turnOff();

/**
 * function name: help
 * Description: displays the command list for the user
 * Parameters: none
 * Returns: a help menu to the user
*/
void help();

/**
 * function name: itoa
 * Description: converts an integer to a string
 * Parameters: takes in an integer value and a base decimal value(ie 8 for octal)
 * Returns: a string number
*/
char* itoa(int val, int base);

/**
 * function name: BCDtoDec
 * Description: converts BCD char to a decimal char value
 * Parameters: takes in a BCD char value
 * Returns: a decimal char value
*/
char BCDtoDec(char bcd);

/**
 * function name: DectoBCD
 * Description: converts decimal char to a BCD char value
 * Parameters: takes in a decimal char value
 * Returns: a BCD char value
*/
char DectoBCD(int dec);

/**
 * function name: formatNum
 * Description: adds a zero to any single digit values
 * Parameters: takes in a number character
 * Returns: a formatted number character
*/
char *formatNum(char *num);

/**
 * function name: asciiToDec
 * Description: converts ASCII characters to number values
 * Parameters: a number character
 * Returns: an integer
*/
int asciiToDec(char num);

/**
 * function name: getTime
 * Description: reads the internal clock and outputs the clock values
 * Parameters: none
 * Returns: a string clock value to the user
*/
void getTime();

/**
 * function name: parseTime
 * Description: parses the user input and sets the internal clock if valid input is given
 * Parameters: character buffer from the user
 * Returns: a boolean integer whether the clock was set or not
*/
int parseTime(char buffer[]);

/**
 * function name: setTime
 * Description: accepts input from the user to be used to change the internal clock
 * Parameters: none
 * Returns: nothing, changes the internal clock values
*/
void setTime(char** arguments);

/**
 * function name: getDate
 * Description: reads the internal date and outputs the date values
 * Parameters: none
 * Returns: a string date value to the user
*/
void getDate();

/**
 * function name: parseDate
 * Description: parses the user input and sets the internal date if valid input is given
 * Parameters: character buffer from the user
 * Returns: a boolean integer whether the date was set or not
*/
int parseDate(char buffer[]);

/**
 * function name: setDate
 * Description: accepts input from the user to be used to change the internal date
 * Parameters: none
 * Returns: nothing, changes the date clock values
*/
void setDate(char** arguments);

/**
 * function name: createPCB
 * Description: calls setupPCB() and inserts it into the appropriate queue
 * Parameters: process name, class, and priority
 * Returns: error if the command entered is invalid
*/
void createPCB(char **arguments);

/**
 * function name: deletePCB
 * Description: deletes pcb and frees all associated memory
 * Parameter: process name
 * Returns: error if pcb name doesn’t exist
*/
void deletePCB(char **arguments);

/**
 * function name: blockPCB
 * Description: puts PCB in a blocked state and re-inserts it to the appropriate queue
 * Parameter: process name
 * Returns: error if PCB is not found
*/
void blockPCB(char **arguments);

/**
 * function name: unblockPCB
 * Description: puts PCB in a unblocked state and re-inserts it to the appropriate queue
 * Parameter: process name
 * Returns: error if PCB is not found
*/
void unblockPCB(char **arguments);

/**
 * function name: suspendPCB
 * Description: suspends PCB and re-inserts it to the appropriate queue
 * Parameter: process name
 * Returns: error if PCB is not found
*/
void suspendPCB(char **arguments);

/**
 * function name: resumePCB
 * Description: changed the PCB to a non-suspended state and re-inserts it to the appropriate queue
 * Parameter: process name
 * Returns: error if PCB is not found
*/
void resumePCB(char **arguments);

/**
 * function name: setPriorityPCB
 * Description: changes PCB's priority and re-inserts it in the appropriate queue and position
 * Parameter: process name, new priority
 * Returns: error if PCB is not found or priority is outside 0-9
*/
void setPriority(char **arguments);

/**
 * function name: showPCB
 * Description: displaies a PCB's name, state, class, status, and priority
 * Parameter: process name
 * Returns: error if PCB is not found
*/
void showPCB(char **arguments);

/**
 * function name: showReady
 * Description: displays every PCB in the ready queue's name, state, class, status, and priority
 * Parameter: none
 * Returns: none
*/
void showReady();

/**
 * function name: showBlocked
 * Description: displays every PCB in the blocked queue's name, state, class, status, and priority
 * Parameter: none
 * Returns: none
*/
void showBlocked();

/**
 * function name: showAll
 * Description: displays every PCB in the ready and blocked queues' name, state, class, status, and priority
 * Parameter: none
 * Returns: none
*/
void showAll();

/**
 * function name: yield
 * Description: yields CPU time to other processes
 * Parameter: none
 * Returns: none
*/
void yield();

/**
 * function name: loadr3
 * Description: loads all R3 processes into the suspended ready queue
 * Parameter: none
 * Returns: pointer to a PCB
*/
struct pcb* loadr3();

/**
 * function name: parseCommand
 * Description: compares user input to the valid list of commands
 * Parameters: a character pointer that points to the user input
 * Returns: nothing, calls commands based on user input
*/
void parseCommand(char* command, char** arguments);

/**
 * function name: init_command_history
 * Description: creates memory for the command history and initializes the command history
 * Parameter: none
 * Returns: none
*/
void init_command_history();

/**
 * function name: add_history_entry
 * Description: adds a new entry to the command history
 * Parameter: pointer to a new entry to the command history
 * Returns: none
*/
void add_history_entry(struct entry* new_ent);

/**
 * function name: clearLine
 * Description: creates a new line
 * Parameter: none
 * Returns: none
*/
void clearLine();

/**
 * function name: resetBuffers
 * Description: resets the command and argument buffers to admit another command
 * Parameter: none
 * Returns: none
*/
void resetBuffers();

/**
 * function name: populateBuffers
 * Description: populates the command and argument buffers to include the next command
 * Parameter: the current command entry
 * Returns: none
*/
void populateBuffers(struct entry* current_entry);

/**
 * function name: commandHandler
 * Description: accepts input from the user and parses it to be used to handle commands
 * Parameters: none
 * Returns: nothing
*/
void commandHandler();

#endif
