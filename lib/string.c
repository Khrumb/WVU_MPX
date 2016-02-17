#include <system.h>
#include <string.h>

/**
 * function name: strlen
 * Description: finds the length of a string
 * Parameters: takes in a char
 * Returns: string length
*/
int strlen(const char *s)
{
	int length = 0;
	while(*(s+length) != '\0'){length++;}
  return length; // return length of string
}

/**
  * function name: strcpy
  * Description: copies a string to create a new string which is a copy of the orginal
  * Parameters: takes in a char pointer s1 as source and constant char pointer as a dest
  * Returns: a pointer to the destination string
*/
char* strcpy(char *s1, const char *s2){
	int i;
	for(i=0; s2[i] != '\0'; ++i)
	{
  		s1[i]=s2[i];
	}
	s1[i] = '\0';
return s1;
}

/*!
  function name: atoi
  Description: converts a const char pointer into an int
  Parameters: takes in a constant char
  Returns: an intiger representation of original char
*/

int atoi(const char *s)
{
  	int myRes = 0;
		int i;
	for(i = 0; s[i] != '\0'; i++)
	{
		myRes = myRes*10 + s[i] - '0';
	}
	return myRes; // return integer
}
/*!
  function name: strcmp
  Description: compares 2 strings
  Parameters: takes in 2 constant chars
  Returns: diffrence in strings
*/
int strcmp(const char *s1, const char *s2)
{

while (*s1 == *s2)
{
	if(*s1 == '\0')
	break;

	s1++;
	s2++;
}
 if(*s1 == '\0' && *s2 == '\0')
	return 0;
 else
	return ( *(unsigned char *)s1 - *(unsigned char *)s2 );

  // Remarks:
  // 1) If we made it to the end of both strings (i. e. our pointer points to a
  //    '\0' character), the function will return 0
  // 2) If we didn't make it to the end of both strings, the function will
  //    return the difference of the characters at the first index of
  //    indifference.

}




/* And finally....
   For the brave ones! (Note: you'll need to add a prototype to string.h)
   sprintf must work properly for the following types to receive extra credit:
     1) characters
     2) strings
     3) signed integers
     4) hexadecimal numbers may be useful
     ...
     \infty) Or feel free to completely implement sprintf
             (Read the man Page: $ man sprintf)
   int sprintf(char *str, const char *format, ...);
*/


/* ---------------------------------------
    Functions below this point are given.
    No need to tamper with these!
   --------------------------------------- */

/*
  Procedure..: strcat
  Description..: Concatenate the contents of one string onto another.
  Params..: s1-destination, s2-source
*/
char* strcat(char *s1, const char *s2)
{
  char *rc = s1;
  if (*s1) while(*++s1);
  while( (*s1++ = *s2++) );
  return rc;
}

/*
  Procedure..: isspace
  Description..: Determine if a character is whitespace.
  Params..: c-character to check
*/
int isspace(const char *c)
{
  if (*c == ' '  ||
      *c == '\n' ||
      *c == '\r' ||
      *c == '\f' ||
      *c == '\t' ||
      *c == '\v'){
    return 1;
  }
  return 0;
}

/*
  Procedure..: memset
  Description..: Set a region of memory.
  Params..: s-destination, c-byte to write, n-count
*/
void* memset(void *s, int c, size_t n)
{
  unsigned char *p = (unsigned char *) s;
  while(n--){
    *p++ = (unsigned char) c;
  }
  return s;
}

/*
  Procedure..: strtok
  Description..: Split string into tokens
  Params..: s1-string, s2-delimiter
*/
char* strtok(char *s1, const char *s2)
{
  static char *tok_tmp = NULL;
  const char *p = s2;

  //new string
  if (s1!=NULL){
    tok_tmp = s1;
  }
  //old string cont'd
  else {
    if (tok_tmp==NULL){
      return NULL;
    }
    s1 = tok_tmp;
  }

  //skip leading s2 characters
  while ( *p && *s1 ){
    if (*s1==*p){
      ++s1;
      p = s2;
      continue;
    }
    ++p;
  }

  //no more to parse
  if (!*s1){
    return (tok_tmp = NULL);
  }

  //skip non-s2 characters
  tok_tmp = s1;
  while (*tok_tmp){
    p = s2;
    while (*p){
      if (*tok_tmp==*p++){
	*tok_tmp++ = '\0';
	return s1;
      }
    }
    ++tok_tmp;
  }

  //end of string
  tok_tmp = NULL;
  return s1;
}
