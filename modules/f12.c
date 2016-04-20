#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "f12.h"
//colors \x1b[XXm \x1b[0m

int fatArray[3072];
int ex = 0;

char command[100];
char dir[100];

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
 printf("\nFile Name: \x1b[32m %s \x1b[0m\n", argv[1]);
 printf("----------------------------------------\n");
 loadBootSector();
 loadRoot();
 while(ex == 0){
   printf("%s> ", dir);
   scanf("%s", command);
   parseCommand();
 }

}

void parseCommand(){
  if(strcmp(command, "showBoot") == 0 || strcmp(command, "showboot") == 0){
    printBootSector();
  } else if(strcmp(command, "showFat")  == 0|| strcmp(command, "showfat") == 0){
    int arg;
    scanf("%d", &arg);
    printFAT(arg);
  } else if(strcmp(command, "showroot")  == 0|| strcmp(command, "showRoot") == 0){
    printRootDirectory();
  } else if(strcmp(command, "ls")  == 0){
    printCurrentDirectory();
  } else if(strcmp(command, "exit") == 0 || strcmp(command, "Exit") == 0){
    ex = 1;
  } else {
    printf("Invalid command, use 'Help' for more information.\n");
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
    if(cd->name[i] != ' '){
      dir[i] = cd->name[i];
    } else {
      dir[i] = '\0';
    }
  }
  cd->flag = 9;//saying its root
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
  int seek = 0;
  unsigned int entry;
  unsigned char buffer[2];

  if(entry_num%2 == 0){
    seek = 1+(3*entry_num)/2;
  } else {
    seek = (3*entry_num)/2;
  }
  fseek(disk_image, seek, SEEK_SET);
  fread(&buffer, (size_t) 1, (size_t) 2, disk_image);
  if(entry_num%2 == 0){
    entry = ((buffer[1] & 0x0F) << 8 | buffer[0]);
  } else {
    entry = ( buffer[1] << 4 | (buffer[0] & 0xF0));
  }
  return entry;
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
      printf(" Entry %03d: 0x%03x\n", i, entry);
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
    if(buffer[0] != 0x00 && buffer[0] != 0xE5){
      int i = 0;
      for(i = 0; i < 8; i++){
        if(i < 3){
          dir->extension[i] = buffer[8+i];
        }
        dir->name[i] = buffer[i];
      }
      dir->name[8] = '\0';
      dir->extension[3] = '\0';
      dir->attr = buffer[11];
      dir->c_time = buffer[14] << 8 | buffer[13];
      dir->c_date = buffer[16] << 8 | buffer[15];
      dir->lad = buffer[18] << 8 | buffer[17];
      dir->lwt = buffer[22] << 8 | buffer[21];
      dir->lwd = buffer[24] << 8 | buffer[23];
      dir->flc = buffer[26] << 8 | buffer[25];
      dir->size = buffer[31] << 24 | buffer[30] << 16 | buffer[29] << 8 | buffer[28];
      dir->flag = 0;
      dir->prev = NULL;
    } else {
      if(buffer[0] == 0x00){
        dir->flag = 2;
      } else if(buffer[0] == 0xE5){
        dir->flag = 1;
      }
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
    printf("\nCreation Time: %u\n", dir->c_time );
    printf("Creation Date: %u\n", dir->c_date);
    printf("Last Access Date: %u\n", dir->lad);
    printf("Last Write Time: %u\n", dir->lwt);
    printf("Last Write Date: %u\n", dir->lwd);
    printf("First Logical Cluster: %u\n", dir->flc);
    printf("Size: %u Bytes\n\n", dir->size);
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
    printDirectory(dir);
  }
}

void printCurrentDirectory(){

}
