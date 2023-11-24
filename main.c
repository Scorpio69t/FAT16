#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>


typedef struct __attribute__((__packed__)) {
    uint8_t BS_jmpBoot[ 3 ]; // x86 jump instr. to boot code
    uint8_t BS_OEMName[ 8 ]; // What created the filesystem
    uint16_t BPB_BytsPerSec; // Bytes per Sector
    uint8_t BPB_SecPerClus; // Sectors per Cluster
    uint16_t BPB_RsvdSecCnt; // Reserved Sector Count
    uint8_t BPB_NumFATs; // Number of copies of FAT
    uint16_t BPB_RootEntCnt; // FAT12/FAT16: size of root DIR
    uint16_t BPB_TotSec16; // Sectors, may be 0, see below
    uint8_t BPB_Media; // Media type, e.g. fixed
    uint16_t BPB_FATSz16; // Sectors in FAT (FAT12 or FAT16)
    uint16_t BPB_SecPerTrk; // Sectors per Track
    uint16_t BPB_NumHeads; // Number of heads in disk
    uint32_t BPB_HiddSec; // Hidden Sector count
    uint32_t BPB_TotSec32; // Sectors if BPB_TotSec16 == 0
    uint8_t BS_DrvNum; // 0 = floppy, 0x80 = hard disk
    uint8_t BS_Reserved1; //
    uint8_t BS_BootSig; // Should = 0x29
    uint32_t BS_VolID; // 'Unique' ID for volume
    uint8_t BS_VolLab[ 11 ]; // Non zero terminated string
    uint8_t BS_FilSysType[ 8 ]; // e.g. 'FAT16 ' (Not 0 term.)
} BootSector;

typedef struct __attribute__((__packed__)) {
    uint8_t DIR_Name[ 11 ]; // Non zero terminated string
    uint8_t DIR_Attr; // File attributes
    uint8_t DIR_NTRes; // Used by Windows NT, ignore
    uint8_t DIR_CrtTimeTenth; // Tenths of sec. 0...199
    uint16_t DIR_CrtTime; // Creation Time in 2s intervals
    uint16_t DIR_CrtDate; // Date file created
    uint16_t DIR_LstAccDate; // Date of last read or write
    uint16_t DIR_FstClusHI; // Top 16 bits file's 1st cluster combine the upper and lower then read the cluster in 
    uint16_t DIR_WrtTime; // Time of last write
    uint16_t DIR_WrtDate; // Date of last write
    uint16_t DIR_FstClusLO; // Lower 16 bits file's 1st cluster
    uint32_t DIR_FileSize; // File size in bytes 
    //use this size combined with the bytes per cluster and sectors per cluster, once ive read in the first cluster 
} Directory;
BootSector boot;
Directory dir;
int file;

int main(){
    //TASK TWO
    //OPEN the FAT file
    file = open("fat16.img", O_RDONLY);
    readBoot();
    readRootDirectory();
    //getClusters(5);
    close(file);
    return 0;
}
void readBoot(){
    int toRead = read(file, &boot, sizeof(boot));
    printf("Bytes per Sector: %d\n",boot.BPB_BytsPerSec);
    printf("Sectors per Cluster: %d\n",boot.BPB_SecPerClus);
    printf("Reserved Sector Count: %d\n",boot.BPB_RsvdSecCnt);
    printf("Number of copies of FAT: %d\n",boot.BPB_NumFATs);
    printf("FAT12/FAT16: size of root DIR: %d\n",boot.BPB_RootEntCnt);
    printf("Sectors, may be 0, see below: %d\n",boot.BPB_TotSec16);
    printf("Sectors in FAT (FAT12 or FAT16): %d\n",boot.BPB_FATSz16);
    printf("Sectors if BPB_TotSec16 == 0: %d\n",boot.BPB_TotSec32);
    printf("Non zero terminated string: %d\n",boot.BS_VolLab);
}

void printDate(uint16_t date){
    int year = ((date >> 9) & 0x7f)+1980;
    int month = (date >> 5) & 0x0F;
    int day = date & 0x1F;
    printf("%04d-%02d-%02d  ",year,month,day);
}
void printTime(uint16_t time){
    int seconds = (time >> 11)&0x1f;
    int minutes = (time >> 5)&0x3f;
    int hours = (time & 0x1f)*2;
     printf("%02d-%02d-%02d  ",seconds,minutes,hours);
}
void getFileAttributes(uint8_t attribute){
    if(attribute & 5){
        printf("A");
    }
    if(attribute >> 4 == 1){
        printf("D");
    }
    if(attribute >> 3 == 1){
        printf("V");
    }
    if(attribute >> 2){};
}
void getClusters(int startingCluster){
    //TASK 3
    //Get the start location of the FAT
    off_t FATstartLocation = boot.BPB_RsvdSecCnt*boot.BPB_BytsPerSec;
    //Get the size of the FAT
    int fatSize = boot.BPB_FATSz16*boot.BPB_BytsPerSec;
    //Seek to the start of the file
    lseek(file,FATstartLocation,SEEK_SET);
    //Allocate space in memory for the fat
    uint16_t* fat = malloc(fatSize);
    //READ the FAT
    read(file,fat,fatSize);
    //Checks to see if the starting cluster
    if(fat[startingCluster] != 0){
        //Creates an infinite loop to get to the end of the cluster
        while(fat[startingCluster] < 0xfff8){
            //Prints out the cluster location
            printf("%d ",fat[startingCluster]);
            //Chains to the next cluster
            startingCluster = fat[startingCluster];
        }
        //Outputs the final cluster
        printf("%d END\n", fat[startingCluster]);
    }
}

void readRootDirectory(){
    off_t ROOTstartLocation = (boot.BPB_RsvdSecCnt+boot.BPB_NumFATs*boot.BPB_FATSz16)*boot.BPB_BytsPerSec;
    ssize_t rootSize = sizeof(dir)*boot.BPB_RootEntCnt;
    lseek(file,ROOTstartLocation,SEEK_SET);
    Directory* rootDirEx = malloc(rootSize);
    read(file,rootDirEx,rootSize);
    printf("Starting Cluster  Last Modified         File Attributes  File Length  File Name\n");
    for(int i = 0; i< boot.BPB_RootEntCnt;i++){
        Directory currentDir = rootDirEx[i];
        if(currentDir.DIR_Attr != 0x0){
            printf("%-18u",((uint16_t)(currentDir.DIR_FstClusHI << 16)| currentDir.DIR_FstClusLO));
            printDate(currentDir.DIR_CrtDate);
            printTime(currentDir.DIR_CrtTime);
            printATTR(currentDir.DIR_Attr);
            printf("           ");
            printf("%-13u",currentDir.DIR_FileSize);
            printf("%-15s\n",currentDir.DIR_Name);
            
        }
    }
}

void printATTR(uint8_t attr) {
    char letters[] = "ADVSHR";
    for (int i = 0; i < 6; i++) {
        if (attr & (1 << (5 - i))) {
            printf("%c", letters[i]);
        } else {
            printf("-");
        }
    }
}

int readSectionOfTextFile() {
    int file = open("test.txt", O_RDONLY);
    int length = lseek(file, 0, SEEK_END);
    lseek(file,0,SEEK_SET);
    char *contents = malloc(length * sizeof(char));
    int status = read(file, contents, length);
    printf("%s",contents);
    close(file);
    return 0;
}

