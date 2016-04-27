#include <stdio.h>
#include <stdlib.h>
#include <string.h>

FILE *disk_image;
unsigned char sector_buffer[512];

unsigned int bps;
unsigned int spc;
unsigned int spf;
unsigned int spt;
unsigned int heads;
unsigned int total_Sec_F32;
unsigned int total_Sec;
unsigned int resv_Sec;
unsigned int fat_Copy;
unsigned int root_Dirs;
unsigned int boot_sig;
unsigned int volume_id;
char* volume_label;
char*  fst;

struct directory {
  char name[9];
  char extension[4];
  unsigned int attr;
  unsigned int c_time;
  unsigned int c_date;
  unsigned int lad;
  unsigned int lwt;
  unsigned int lwd;
  unsigned int flc;
  unsigned int size;
  unsigned int flag;

  int seek;
  struct directory * prev;
} ;

void parseCommand();

void help();

int parseFileName(char* from, char* to);

int parseSuffix(char* from, char* to);

void toUpperCase(char* str);

void clearBuffers();

void loadBootSector();

void loadRoot();

void printBootSector();

void loadFATTable();

unsigned int getFATEntry(int entry_num);

void printFAT(int fat_num);

void getDirectory(struct directory *dir);

void findDirectory(struct directory *dir, char* file_name, char* file_extension);

void printDirectory(struct directory *dir);

void printRootDirectory();

void printCurrentDirectory(int filter);

void printFiltered(struct directory *dir, int flags);

void changeDirectory(char* subdir);

void renameItem(char * newName);

void addFile(char* file);

void writeRootDir(struct directory *dir);

void move(char* folder, struct directory *fileDir);
