/**
* @file console.h
* This is the command handler
*/

#ifndef _COMMANDHANDLER_H
#define _COMMANDHANDLER_H

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
void setTime( char* arguments);

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
void setDate(char* arguments);

/**
 * function name: parseCommand
 * Description: compares user input to the valid list of commands
 * Parameters: a character pointer that points to the user input
 * Returns: nothing, calls commands based on user input
*/
void parseCommand(char* command, char* arguments);

/**
 * function name: commandHandler
 * Description: accepts input from the user and parses it to be used to handle commands
 * Parameters: none
 * Returns: nothing
*/
void commandHandler();

#endif
