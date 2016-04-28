#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "f12.h"
//\x1b[0m == reset
//\x1b[33m == red
//\x1b[32m == green
//\x1b[31m == yellow

int fatArray[3072];
int ex = 0;

char command[100];
char curdir[100];
int cdi = 0;

char fname[9];
char suffix[4];

struct directory* cd;

int main(int argc, char* argv[]){

  if(argc > 3 || argc == 1) {
    printf("Useage: ./f12 [disk image filename] [OPTIONAL: directory]\n" );
    return 0;
 }
 disk_image = fopen(argv[1], "rb+");
 if(disk_image == NULL){
   printf("Invalid [disk image filename.]\n" );
   return 0;
 }
 loadBootSector();
 loadRoot();
 if(argc == 2){
   printf("\nFile Name: \x1b[32m %s \x1b[0m\n", argv[1]);
   printf("----------------------------------------\n");
   while(ex == 0){
     printf("%s> ", curdir);
     scanf("%s", command);
     parseCommand();
   }
 } else if(argc == 3){
      //parsing folder location
     char* pos = strchr(argv[2], '/');
     char* pp = argv[2];
     while (pos != NULL) {
       pp = pos+1;
       *pos = '\0';
       toUpperCase(argv[2]);
       changeDirectory(argv[2]);
       pos = strchr(argv[2], '/');
     }
     //parsing file name and suffix for easy comparing.
     pos = strchr(pp, '.');
     if(parseFileName(pp, fname) == 0){
       if(parseSuffix(pos+1, suffix) == 0){
         commandLineType();
       } else {
         printf("\x1b[31mERROR\x1b[0m: Invalid Suffix.\n");
       }
     } else {
       printf("\x1b[31mERROR\x1b[0m: Invalid File Name.\n");
     }
 }


}

void parseCommand(){
  int flush = 0;
  if(strcmp(command, "showBoot") == 0 || strcmp(command, "showboot") == 0){
    printBootSector();
  } else
  if(strcmp(command, "showFat")  == 0|| strcmp(command, "showfat") == 0){
    int arg;
    scanf("%d", &arg);
    flush = 1;
    printFAT(arg);
  } else
  if(strcmp(command, "showroot")  == 0|| strcmp(command, "showRoot") == 0){
    printRootDirectory();
  } else
  if(strcmp(command, "ls")  == 0){
    if(getchar() == 0xa){
      printCurrentDirectory(0);
    } else {
      scanf("%s", command);
      flush = 1;
      if (command[0] == '*') {
        if(parseSuffix(command+2, suffix) == 0){
          printCurrentDirectory(1);
        } else {
          printf("\x1b[31mERROR\x1b[0m: Invalid Suffix.\n");
        }
      } else if (strchr(command, '.') != NULL) {
        char* pos = strchr(command, '.');
        if(parseFileName(command, fname) == 0){
          if(parseSuffix(pos+1, suffix) == 0){
            printCurrentDirectory(2);
          } else {
            printf("\x1b[31mERROR\x1b[0m: Invalid Suffix.\n");
          }
        } else {
          printf("\x1b[31mERROR\x1b[0m: Invalid File Name.\n");
        }
      } else if(command[0] == '-' && command[1] == 'a'){
        printCurrentDirectory(-1);
      } else {
        printf("\x1b[31mERROR\x1b[0m: Invalid argument.\n");
      }
    }
  } else
  if(strcmp(command, "cd")  == 0){
    scanf("%s\0", command);
    flush = 1;
    toUpperCase(command);
    changeDirectory(command);
  } else
  if(strcmp(command, "rename")  == 0){
    char rname[9];
    scanf("%s\0", command);
    flush = 1;
    if(strlen(command) < 8){
      parseFileName(command, fname);
      scanf("%s\0", command);
      if(strlen(command) < 8){
        parseFileName(command, rname);
        renameItem(rname);
      } else {
        printf("\x1b[31mERROR\x1b[0m: %s is too long.\n", command);
      }
    } else {
      printf("\x1b[31mERROR\x1b[0m: %s is too long.\n", command);
    }
  } else
  if(strcmp(command, "type")  == 0){
    scanf("%s", command);
    flush = 1;
    if (strchr(command, '.') != NULL) {
      char* pos = strchr(command, '.');
      if(parseFileName(command, fname) == 0){
        if(parseSuffix(pos+1, suffix) == 0){
          if(strcmp(suffix, "BAT") == 0|| strcmp(suffix, "TXT") == 0 || strcmp(suffix, "C") == 0){
            type();
          } else {
            printf("\x1b[31mERROR\x1b[0m: Invalid File type. [.BAT, .TXT, or .C]\n");
          }
        } else {
          printf("\x1b[31mERROR\x1b[0m: Invalid Suffix.\n");
        }
      } else {
        printf("\x1b[31mERROR\x1b[0m: Invalid File Name.\n");
      }
    } else {
      printf("\x1b[31mERROR\x1b[0m: Invalid File Name.\n");
    }

  } else
  if(strcmp(command, "addFile")  == 0 || strcmp(command, "addfile")  == 0){
    scanf("%s\0", command);
    flush = 1;
    addFile(command);
  } else
  if(strcmp(command, "move")  == 0){
    char dest_buffer[50];
    scanf("%s\0", command);
    scanf("%s\0", dest_buffer);
    char* pos = strchr(command, '/');
    char* pp = command;
    int up = 0;
    while (pos != NULL) {
      *pos = '\0';
      toUpperCase(pp);
      changeDirectory(pp);
      pp = pos+1;
      pos = strchr(pp, '/');
      up++;
    }
    pos = strchr(pp, '.');
    if(parseFileName(pp, fname) == 0){
      if(pos != NULL && parseSuffix(pos+1, suffix) == 0){
        struct directory *dir = malloc(sizeof(struct directory));
        findDirectory(dir, fname, suffix);
        if(dir->flag != 4){
          move(dest_buffer, dir);
        }
      }
    }
    while(up-- != 0){
      changeDirectory("..");
    }
    flush = 1;
  } else
  if(strcmp(command, "help")  == 0){
    help();
  } else
  if(strcmp(command, "exit") == 0 || strcmp(command, "Exit") == 0){
    ex = 1;
  }
  else {
    flush = 1;
    printf("Invalid command, use 'Help' for more information.\n");
  }
  //flushes io stream. Just in case someone derps and puts too many spaces
  if(flush == 1){
    int ch;
    while((ch = getchar()) != EOF && ch != '\n');
    flush = 0;
  }
  clearBuffers();
}

void help(){
  printf("-----------------\n");
  printf("| \x1b[32mCommand List:\x1b[0m |\n");
  printf("-----------------\n");

  printf("showBoot - displays the disk-image's Boot Sector.\n");
  printf("showRoot - displays the disk-image's Root directory.\n");
  printf("showFat  - displays the first 25 entries a selected FAT table.\n");
  printf("cd       - change the current directory.\n");
  printf("ls       - displays the contents of the current directory.\n");
  printf("move     - moves file into different directory.\n");
  printf("rename   - renames and file in the current directory.\n");
  printf("addFile  - adds a file to the disk image.\n");
  printf("type     - displays, at most the first 2000 characters of a .TXT, .BAT, or .C file.\n");
  printf("help     - displays a list of commands and their uses.\n");
  printf("exit     - exits the program.\n");
}

int parseFileName(char* from, char* to){
  int i = 0;
  memset(to, 0x00, 8);
  for (i = 0; i < 8 && from[i] != '.'; i++) {
    if(isspace(from[i]) == 0){
      if(from[i] >= 97){
        to[i] = from[i]-32;
      } else {
        to[i] = from[i];
      }
    } else {
      return -1;
    }
  }
  to[i+1] = '\0';
  return 0;
}

int parseSuffix(char* from, char* to){
  int i = 0;
  for (i = 0; i < 3; i++) {
    if(from[i] != '.'){
      if(from[i] >= 97){
        to[i] = from[i]-32;
      } else {
        to[i] = from[i];
      }
    } else {
      return -1;
    }
  }
  to[i+1] = '\0';
  return 0;
}

void clearBuffers(){
  int i = 0;
  for(i = 0; i < 9; i++){
    fname[i] = '\0';
  }
  for(i = 0; i < 4; i++){
    suffix[i] = '\0';
  }
  for(i = 0; i < 100; i++){
    command[i] = '\0';
  }
}

void toUpperCase(char* str){
  while(*str != '\0'){
    if(*str >= 97){
      *str = *str-32;
    }
    str++;
  }
}

void loadBootSector(){
  fseek(disk_image, SEEK_SET, 0);
  fread(sector_buffer, (size_t) 1, (size_t) 512, disk_image);
  bps = (sector_buffer[12] << 8 | sector_buffer[11]); //bytes per sector
  spc =  (0 << 8 | sector_buffer[13]); //Sectors Per Cluster
  resv_Sec = (sector_buffer[15] << 8 | sector_buffer[14]); //Number of Reserved Sectors
  fat_Copy = (0 << 8 | sector_buffer[16]); //Number of FAT Copies
  root_Dirs = (sector_buffer[18] << 8 | sector_buffer[17]); //Max Number of Root Directory Entries
  total_Sec = (sector_buffer[20] << 8 | sector_buffer[19]); //Total number of Sectors in the File System
  spf = (sector_buffer[23] << 8 | sector_buffer[22]); //Number of Sector per FAT
  spt = (sector_buffer[25] << 8 | sector_buffer[24]); //Sectors per Track
  heads = (sector_buffer[27] << 8 | sector_buffer[26]);//Number of Heads
  total_Sec_F32 = (sector_buffer[35] << 24 |  sector_buffer[34] << 16 | sector_buffer[33] << 8 | sector_buffer[32]); //Total Sector Count for FAT12
  boot_sig = sector_buffer[38]; //Boot Signature
  volume_id = ((sector_buffer[42] << 8  | sector_buffer[41]) << 16  | (sector_buffer[40] << 8 | sector_buffer[39])); //Volume ID
  volume_label = &sector_buffer[43]; //Volume Label
  fst = &sector_buffer[54]; //FileSystemType
}

void loadRoot(){
  int seek = (19*bps*spc);
  fseek(disk_image, seek, SEEK_SET);
  cd = malloc(sizeof(struct directory));
  getDirectory(cd);
  int i;
  for(i = 0; i < 9; i++){
    if(!isspace(cd->name[i])){
      curdir[i] = cd->name[i];
    } else {
      curdir[i] = '\0';
      break;
    }
  }
  cdi = i-1;
  cd->seek = seek;//easy way to find shit again with a small memory footprint.
}

void printBootSector(){
  printf("---------------\n");
  printf("| \x1b[31mBoot Sector\x1b[0m |\n");
  printf("---------------\n");
  printf("Bytes Per Sector: %d\n", bps);
  printf("Sectors Per Cluster: %d\n", spc);
  printf("Number of Reserved Sectors: %d\n", resv_Sec);
  printf("Number of FAT Copies: %d\n", fat_Copy);
  printf("Max Number of Root Directory Entries: %u\n", root_Dirs) ;
  printf("Total number of Sectors in the File System: %d\n", total_Sec);
  printf("Number of Sector per FAT: %d\n", spf);
  printf("Sectors per Track: %d\n", spt);
  printf("Number of Heads: %d\n", heads);
  printf("Total Sector Count for FAT32: %d\n", total_Sec_F32);
  printf("Boot Signature: 0x%02x\n", boot_sig);
  printf("Volume ID: %u\n", volume_id);
  printf("Volume Label: %.11s\n", volume_label);
  printf("FileSystemType: %.8s\n\n", fst);
}

void loadFATTable(){
  int i = 0;
  for ( i = 0; i < 3072; i++) {
    fatArray[i] = getFATEntry(i);
  }
}

unsigned int getFATEntry(int entry_num){
  int seek = 512;
  unsigned int entry;
  unsigned char buffer[2];
  seek += (3*entry_num)/2;

  fseek(disk_image, seek, SEEK_SET);
  fread(&buffer, (size_t) 1, (size_t) 2, disk_image);
  if(entry_num%2 == 0){
    entry = ((buffer[1] & 0x0F) << 8 | buffer[0]);
  } else {
    entry = (buffer[1] << 4 | (buffer[0] & 0xF0) >> 4);
  }
  return entry;
}

void setFATEntry(int entry_num, int write){
  int seek = 512;
  unsigned int entry;
  unsigned char buffer[2];
  seek += (3*entry_num)/2;

  fseek(disk_image, seek, SEEK_SET);
  fread(&buffer, (size_t) 1, (size_t) 2, disk_image);
  fseek(disk_image, seek, SEEK_SET);
  if(entry_num%2 == 0){
    buffer[0] = write & 0x00FF;
    buffer[1] = (write >> 8) | (buffer[1] & 0xF0);
  } else {
    buffer[0] = ((write & 0x000F) << 4) | (buffer[0] & 0x0F);
    buffer[1] = (write & 0x0FF0) >> 4;
  }
  fwrite(buffer, sizeof(char), 2, disk_image);
}

void printFAT(int fat_num){
  if(fat_num == 1 || fat_num == 2){
    int num_entries = bps * spf *2 /3;
    printf("----------------------------\n");
    printf("| \x1b[32mFile Allocation Table %d\x1b[0m  |\n", fat_num);
    printf("| \x1b[32m%04d Entries\x1b[0m             |\n", num_entries);
    printf("----------------------------\n");
    int i = 0;
    unsigned int entry;
    do{
      entry = getFATEntry(i);
      printf(" Entry %03d: %#x\n", i, entry);
      i++;
    } while (i <= 25);
    printf("\n");
  } else {
    printf("Invalid FAT number. Valid range is [1 - 2].\n");
  }
}

void getDirectory(struct directory *dir){
    unsigned char buffer[32];
    fread(&buffer, (size_t) 1, (size_t) 32, disk_image);
    if(buffer[0] == 0x00){
      dir->flag = 2;
    } else if(buffer[0] == 0xE5){
      dir->flag = 1;
    } else if(buffer[0] == 0x2E){
      dir->flag = 3;
    } else {
      int i = 0;
      for(i = 0; i < 8; i++){
        if(i < 3){
          dir->extension[i] = buffer[8+i];
        }
        if(!isspace(buffer[i])){
          dir->name[i] = buffer[i];
        } else {
          dir->name[i] = '\0';
        }
      }
      dir->name[8] = '\0';
      dir->extension[3] = '\0';
      dir->attr = buffer[11];
      dir->c_time = buffer[15] << 8 | buffer[14];
      dir->c_date = buffer[17] << 8 | buffer[16];
      dir->lad = buffer[19] << 8 | buffer[18];
      dir->lwt = buffer[23] << 8 | buffer[22];
      dir->lwd = buffer[25] << 8 | buffer[24];
      dir->flc = buffer[27] << 8 | buffer[26];
      dir->size = buffer[31] << 24 | buffer[30] << 16 | buffer[29] << 8 | buffer[28];
      dir->flag = 0;
      dir->prev = NULL;
    }
}

void findDirectory(struct directory *dir, char* file_name, char* file_extension){
  int i = 0;
  fseek(disk_image, cd->seek, SEEK_SET);
  for (i = 0; i < 224; i++) {
    getDirectory(dir);
    if(strcmp(file_name, dir->name) == 0){
      if(strcmp(file_extension, dir->extension) == 0 || strlen(file_extension) == 0){
        dir->seek = ftell(disk_image)-32;
        break;
      }
    }
  }
  if(i == 224){
    dir->flag = 4;
    printf("\x1b[31mERROR\x1b[0m: Cannot find, '%s'.\n", file_name);
  }
}

void printDirectory(struct directory *dir){
  if(dir->flag == 0){
    printf("--------------------\n");
    printf("| \x1b[32mDirectory Entry\x1b[0m  |\n");
    printf("--------------------\n");
    printf("Name: %s\n", dir->name);
    printf("Extension: %s\n", dir->extension);
    printf("Attributes:" "\x1b[0m");
    if(dir->attr & 0x01){
      printf(" Read-only");
    }
    if(dir->attr & 0x02){
      printf(" Hidden");
    }
    if(dir->attr & 0x04){
      printf(" System");
    }
    if(dir->attr & 0x08){
      printf(" Volume-Label");
    }
    if(dir->attr & 0x10){
      printf(" Subdirectory");
    }
    if(dir->attr & 0x20){
      printf(" Archive");
    }
    if(dir->attr & 0x40){
      printf(" Device");
    }
    //time: %d:%02d:%02d\n", (dir->c_time & 0xF800) >> 11, (dir->c_time & 0x07E0) >> 5,  (dir->c_time & 0x001F)*2
    //date %02d/%02d/%04d\n", (dir->c_date & 0x01E0) >> 5, (dir->c_date & 0x001F),  1980+((dir->c_date & 0xFE00) >> 9)
    printf("\nCreation Time: %d:%02d:%02d\n", (dir->c_time & 0xF800) >> 11, (dir->c_time & 0x07E0) >> 5,  (dir->c_time & 0x001F)*2);
    printf("Creation Date: %02d/%02d/%04d\n", (dir->c_date & 0x01E0) >> 5, (dir->c_date & 0x001F),  1980+((dir->c_date & 0xFE00) >> 9));
    printf("Last Write Time: %d:%02d:%02d\n", (dir->lwt & 0xF800) >> 11, (dir->lwt & 0x07E0) >> 5,  (dir->lwt & 0x001F)*2);
    printf("Last Write Date: %02d/%02d/%04d\n", (dir->lwd & 0x01E0) >> 5, (dir->lwd & 0x001F),  1980+((dir->lwd & 0xFE00) >> 9));
    printf("Last Access Date: %02d/%02d/%04d\n", (dir->lad & 0x01E0) >> 5, (dir->lad & 0x001F),  1980+((dir->lad & 0xFE00) >> 9));
    printf("First Logical Cluster: %d\n", dir->flc);
    printf("Size: %d Bytes\n\n", dir->size);
  } else {
    switch (dir->flag) {
      case 1:
        //printf("Directory Entry %02d: DELETED\n\n", dir_num);
        break;
      case 2:
        //printf("Empty\n\n");
        break;
    }
  }
}

void printRootDirectory(){
  int i = 0;
  int seek = (19*bps*spc);
  fseek(disk_image, seek, SEEK_SET);
  struct directory *dir = malloc(sizeof(struct directory));
  for (i = 0; i < 224; i++) {
    getDirectory(dir);
    if(dir->flag != 2){
      if(!(dir->attr & 0x02)){
        printDirectory(dir);
      }
    } else {
      break;
    }
  }
}

void printCurrentDirectory(int filter){
  int i = 0;
  printf("\x1b[32mFLC\tSize(B)\tName\x1b[0m\n");
  fseek(disk_image, cd->seek, SEEK_SET);
  struct directory *dir = malloc(sizeof(struct directory));
  for (i = 0; i < 224; i++) {
    getDirectory(dir);
    if(dir->flag != 2){ //last entry?
      printFiltered(dir, filter);
    } else {
      break;
    }
  }
  printf(" Subdirectory = \x1b[33m█\x1b[0m\n");
}

void printFiltered(struct directory *dir, int flag){
  int print = 0;
  if(dir->flag != 3 && !(dir->attr & 0x08)){ //always filter labled VOLUME
    if(!(dir->attr & 0x02)){ //no hidden-files
      if (flag == 1 && !strcmp(dir->extension, suffix)) { //matching suffix
        print = 1;
      }
      if (flag == 2 && !strcmp(dir->name, fname) && !strcmp(dir->extension, suffix)) { //matching suffix
        print = 1;
      }
      if (flag == 0) { // normal list
        print = 1;
      }
    }
    if (flag == -1 ) { // -a showing hidden
      print = 1;
    }
  }
  if(print == 1){
    if(isspace(dir->extension[0])){
      printf("%d\t\t\x1b[33m%s\x1b[0m\n", dir->flc, dir->name);
    } else {
      printf("%d\t%dKB\t%s.%s\n", dir->flc, dir->size/1000, dir->name, dir->extension);
    }
  }
}

void changeDirectory(char* subdir){
  int i = 0;
  if(!strcmp(subdir, "..")){
    cd = cd->prev;
    for (i = 0; i < 9; i++) {
      if(curdir[cdi-i] = '/'){
        curdir[cdi-i-1] = '\0';
        cdi = cdi-i;
        break;
      }
    }
  } else {
    fseek(disk_image, cd->seek, SEEK_SET);
    struct directory *dir = malloc(sizeof(struct directory));
    for (i = 0; i < 224; i++) {
      getDirectory(dir);
      if(!strcmp(subdir, dir->name) && dir->attr & 0x10){
        dir->prev = cd;
        cd = dir;
        curdir[cdi-1]='/';
        cd->seek = (cd->flc+31)*(bps*spc);
        int ii = 0;
        for(ii = 0; ii < 9; ii++){
          if(cd->name[ii] != ' '){
            curdir[ii+cdi] = cd->name[ii];
          } else {
            curdir[ii+cdi] = '\0';
            break;
          }
        }
        cdi = ii-1;
        break;
      }
    }
    if(i == 224){
      printf("\x1b[31mERROR\x1b[0m: Unable to find directory '%s'.\n", subdir);
    }
  }
}

void renameItem(char * newName){
  int i = 0;
  fseek(disk_image, cd->seek, SEEK_SET);
  struct directory *dir = malloc(sizeof(struct directory));
  for (i = 0; i < 224; i++) {
    getDirectory(dir);
    if(!strcmp(fname, dir->name)){
      break;
    }
  }
  fseek(disk_image, (cd->seek+(i*32)), SEEK_SET);
  fwrite(newName, sizeof(char), 8, disk_image);
}

void type(){
  int i = 0;
  fseek(disk_image, cd->seek, SEEK_SET);
  struct directory *dir = malloc(sizeof(struct directory));
  for (i = 0; i < 224; i++) {
    getDirectory(dir);
    if(strcmp(fname, dir->name) == 0){
      if(strcmp(suffix, dir->extension) == 0){
        char* buffer[512];
        printf("\nTyping: \x1b[32m%s.%s\x1b[0m\n",fname,suffix);
        printf("------------------------------------------------\n");
        int current_cluster = dir->flc;
        int seek = (current_cluster+31)*(bps*spc);
        while(current_cluster != 0xFFF && getchar() != 'q'){
          fseek(disk_image, seek, SEEK_SET);
          fread(&buffer, 1, 512, disk_image);
          printf("%s\n", buffer);
          printf("\nPress \x1b[32mAny Key\x1b[0m followed by \x1b[32mEnter\x1b[0m to continue... '\x1b[31mQ\x1b[0m' to quit.\n");
          current_cluster = getFATEntry(current_cluster);
          seek = (current_cluster+31)*(bps*spc);
        }
        break;
      }
    }
  }
  if(i == 224){
    printf("\x1b[31mERROR\x1b[0m: Unable to find file '%s.%s'.\n", fname, suffix);
  }
}

void commandLineType(){
  int i = 0;
  fseek(disk_image, cd->seek, SEEK_SET);
  struct directory *dir = malloc(sizeof(struct directory));
  for (i = 0; i < 224; i++) {
    getDirectory(dir);
    if(strcmp(fname, dir->name) == 0){
      if(strcmp(suffix, dir->extension) == 0){
        char* buffer[512];
        int current_cluster = dir->flc;
        int seek = (current_cluster+31)*(bps*spc);
        while(current_cluster != 0xFFF){
          fseek(disk_image, seek, SEEK_SET);
          fread(&buffer, 1, 512, disk_image);
          printf("%s\n", buffer);
          current_cluster = getFATEntry(current_cluster);
          seek = (current_cluster+31)*(bps*spc);
        }
        break;
      }
    }
  }
  if(i == 224){
    printf("\x1b[31mERROR\x1b[0m: Unable to find file '%s.%s'.\n", fname, suffix);
  }
}

void addFile(char* file){
  FILE* f = fopen(file, "rb");
  if(f == NULL){
    printf("\x1b[31mERROR\x1b[0m: Cannot find '%s'.\n", file);
    return 0;
  }
  struct directory *dir = malloc(sizeof(struct directory));
  int i = 0;
  fseek(disk_image, (19*bps*spc), SEEK_SET);
  for (i = 0; i < 224; i++) {
    getDirectory(dir);
    if(dir->flag >= 1){
      dir->seek = (19*bps*spc)+(i*32);
      break;
    }
  }
  char* pos = strchr(command, '.');
  if(parseFileName(command, dir->name) == 0){
    if(parseSuffix(pos+1, dir->extension) == 0){
      struct directory *comp_dir = malloc(sizeof(struct directory));
      int i = 0;
      fseek(disk_image, (19*bps*spc), SEEK_SET);
      for (i = 0; i < 224 && comp_dir->flag != 2; i++) {
        getDirectory(comp_dir);
        if(strcmp(dir->name, comp_dir->name) == 0){
          if(strcmp(dir->extension, comp_dir->extension) == 0){
            printf("\x1b[31mERROR\x1b[0m: File Name is taken.\n");
            return 0;
          }
        }
      }
      char buffer[512];
      dir->attr = 0x20;
      time_t t = time(NULL);
      struct tm tm = *localtime(&t);
      dir->c_time = ((tm.tm_hour & 0x001F) << 11) | ((tm.tm_min & 0x003F) << 5) | ((tm.tm_sec/2) & 0x001F);
      dir->c_date = (((tm.tm_year-80) & 0x007F) << 9) | (((tm.tm_mon+1) & 0x0007) << 5) | (tm.tm_mday & 0x001F);
      dir->lad = dir->c_date;
      dir->lwt = dir->c_time;
      dir->lwd = dir->c_date;
      fseek(f, 0 ,SEEK_END);
      dir->size = ftell(f);
      fseek(f, 0,SEEK_SET);
      int cc = 2;
      while(getFATEntry(cc) != 0x00)
        {cc++;}
      dir->flc = cc;
      int lc = cc; //need for later
      int seek = (cc+31)*512;
      setFATEntry(cc, 0xFFF);
      fread(&buffer, (size_t) 1, (size_t) 512, f);
      fseek(disk_image, seek,SEEK_SET);
      fwrite(&buffer, (size_t) 1, (size_t) 512, disk_image);
      for(i = 0; i <=  (dir->size/512.0); i++){
        while(getFATEntry(cc) != 0x00)
          {cc++;}
        seek = (cc+31)*512;
        setFATEntry(lc, cc);
        lc = cc;
        fread(&buffer, (size_t) 1, (size_t) 512, f);
        fseek(disk_image, seek,SEEK_SET);
        fwrite(&buffer, (size_t) 1, (size_t) 512, disk_image);
      }
      setFATEntry(cc, 0xFFF);
      writeRootDir(dir);
    } else {
      printf("\x1b[31mERROR\x1b[0m: Invalid Suffix.\n");
    }
  } else {
    printf("\x1b[31mERROR\x1b[0m: Invalid File Name.\n");
  }
  printf("\x1b[32m%s\x1b[0m succuessfully added to Root.\n", command);

}

void writeRootDir(struct directory *dir){
  char buffer[32];
  memset(buffer, 0, sizeof(buffer));
  int i = 0;
  for (i = 0; i < 8; i++) {
    buffer[i]= dir->name[i];
    if(i < 3){
      buffer[i+8] = dir->extension[i];
    }
  }
  buffer[11] = 0x20;
  buffer[14] = dir->c_time & 0x00FF;
  buffer[15] = (dir->c_time & 0xFF00) >> 8;
  buffer[16] = dir->c_date & 0x00FF;
  buffer[17] = (dir->c_date & 0xFF00) >> 8;
  buffer[18] = dir->lad & 0x00FF;
  buffer[19] = (dir->lad & 0xFF00) >> 8;
  buffer[22] = dir->lwt & 0x00FF;
  buffer[23] = (dir->lwt & 0xFF00) >> 8;
  buffer[24] = dir->lwd & 0x00FF;
  buffer[25] = (dir->lwd & 0xFF00) >> 8;
  buffer[26] = (dir->flc & 0x00FF);
  buffer[27] = (dir->flc & 0xFF00)>>8;
  buffer[28] =  dir->size & 0x000000FF;
  buffer[29] = (dir->size & 0x0000FF00) >> 8;
  buffer[30] = (dir->size & 0x00FF0000) >> 16;
  buffer[31] = (dir->size & 0xFF000000) >> 24;

  fseek(disk_image, dir->seek, SEEK_SET);
  fwrite(buffer, sizeof(char), 32, disk_image);
}

void move(char* folder, struct directory *fileDir){
  //f12.c subdir
  int temp_seek = fileDir->seek;
  struct directory *foldir = malloc(sizeof(struct directory));
  fseek(disk_image, foldir->seek, SEEK_SET);
  toUpperCase(folder);
  findDirectory(foldir, folder, "");
  int seek = (foldir->flc+31)*(bps*spc);
  int i = 0;
  fseek(disk_image, seek, SEEK_SET);
  struct directory *dir = malloc(sizeof(struct directory));
  for (i = 0; i < 224; i++) {
    getDirectory(dir);
    seek += 32;
    if(dir->flag == 2){ //last entry?
      seek-=32;
      fileDir->seek = seek;
      writeRootDir(fileDir);
      fseek(disk_image, temp_seek, SEEK_SET);
      char* buffer[1];
      buffer[0] = 0xE5;
      fwrite(&buffer, sizeof(char), 1, disk_image);
      break;
    }
  }
}
