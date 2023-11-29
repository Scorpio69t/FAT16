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

typedef struct __attribute__((__packed__)){
    off_t startingCluster;
    uint8_t attributes;
}File;

typedef struct __attribute__((__packed__)){
    char* dirEntry;
}ShortDirEntry;

typedef struct __attribute__((__packed__)){

}Volume;

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

} Directory;

typedef struct __attribute__((__packed__)){
    uint8_t LDIR_Ord; // Order/ position in sequence/ set
    uint8_t LDIR_Name1[ 10 ]; // First 5 UNICODE characters
    uint8_t LDIR_Attr; // = ATTR_LONG_NAME (xx001111)
    uint8_t LDIR_Type; // Should = 0
    uint8_t LDIR_Chksum; // Checksum of short name
    uint8_t LDIR_Name2[ 12 ]; // Middle 6 UNICODE characters
    uint16_t LDIR_FstClusLO; // MUST be zero
    uint8_t LDIR_Name3[ 4 ]; // Last 2 UNICODE characters
}LongDirectory;

BootSector boot;
Directory dir;
LongDirectory ldir;
Volume vol;
int file;

int main(){
    file = open("fat16.img", O_RDONLY);
    //openFile("SCC.211");
    readBoot();
    readRootDirectory();
    //scanFile(2457,0);
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

void openFile(char* shortFile){
    read(file, &boot, sizeof(boot));
    File toReturn;
    off_t ROOTstartLocation = (boot.BPB_RsvdSecCnt+boot.BPB_NumFATs*boot.BPB_FATSz16)*boot.BPB_BytsPerSec;
    ssize_t rootSize = sizeof(dir)*boot.BPB_RootEntCnt;
    lseek(file,ROOTstartLocation,SEEK_SET);
    Directory* rootDirEx = malloc(rootSize);
    read(file,rootDirEx,rootSize);
    for(int i = 0; i< boot.BPB_RootEntCnt;i++){
        Directory currentDir = rootDirEx[i];
        if(currentDir.DIR_Attr != 0x0){
            printf("%-15s , %s\n",currentDir.DIR_Name, shortFile);
            char* dirName = malloc(sizeof(char)*12);
            strncpy(dirName, currentDir.DIR_Name,11);
            dirName[11] = '\\0';
            if(strcmp(dirName,shortFile)==0){
                printf("The directory name matches the short file name!");
            } 
        }
    }
    //return toReturn;
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

int getClusters(int startingCluster){
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
        //Checks to see if it is the end of the cluster
        if(fat[startingCluster] < 0xfff8){
            //Returns the next cluster location
            return fat[startingCluster];
        }else{
            return 0xfff8;
        }
    }
}

char getByte(off_t currentCluster, off_t offset) {
    // Get the start location of the data section
    off_t startData = (boot.BPB_RsvdSecCnt + boot.BPB_NumFATs * boot.BPB_FATSz16) * boot.BPB_BytsPerSec + sizeof(dir) * boot.BPB_RootEntCnt;
    // Calculate the byte offset of the requested byte
    off_t byteOffset = startData + (currentCluster - 2) * boot.BPB_SecPerClus * boot.BPB_BytsPerSec + offset;
    // Seek to the byte offset
    lseek(file, byteOffset, SEEK_SET);
    // Read the byte
    char value;
    ssize_t valueRead = read(file, &value, 1);
    return value;
}

void scanFile(int startingCluster, off_t offset) {
    while (startingCluster <= 0xfff8) {
        //printf("%d ", startingCluster);
        char value = getByte(startingCluster, offset);
        printf("%c", value);
        offset++;
        if (offset >= boot.BPB_SecPerClus * boot.BPB_BytsPerSec) {
            startingCluster = getClusters(startingCluster);
            offset = 0;
        }
    }
}


void readRootDirectory(){
    off_t ROOTstartLocation = (boot.BPB_RsvdSecCnt+boot.BPB_NumFATs*boot.BPB_FATSz16)*boot.BPB_BytsPerSec;
    ssize_t rootSize = sizeof(dir)*boot.BPB_RootEntCnt;
    lseek(file,ROOTstartLocation,SEEK_SET);
    
    Directory* rootDirEx = malloc(rootSize);

    read(file,rootDirEx,rootSize);
    int count = 0;
    printf("Starting Cluster  Last Modified         File Attributes  File Length  File Name\n");
    for(int i = 0; i< boot.BPB_RootEntCnt;i++){
        Directory currentDir = rootDirEx[i];
        if(currentDir.DIR_Attr != 0x0){
            printf("%-18u",(currentDir.DIR_FstClusHI << 16| currentDir.DIR_FstClusLO));
            printDate(currentDir.DIR_CrtDate);
            printTime(currentDir.DIR_CrtTime);
            printATTR(currentDir.DIR_Attr);
            printf("           ");
            printf("%-13u",currentDir.DIR_FileSize);
            if(currentDir.DIR_Attr != 0x0f){
                printf("%-15s\n",currentDir.DIR_Name); 
            }else{
                lseek(file,ROOTstartLocation+(i*sizeof(LongDirectory)),SEEK_SET);
                LongDirectory* rootLDirEx = malloc(sizeof(LongDirectory));
                read(file,rootLDirEx,sizeof(LongDirectory));
                for(int j = 0; j<10;j=j+2){
                    printf("%c",(char)rootLDirEx[0].LDIR_Name1[j]);
                }
                for(int j = 0; j<12;j=j+2){
                    printf("%c",(char)rootLDirEx[0].LDIR_Name2[j]);
                }
                for(int j = 0; j<4;j=j+2){
                    printf("%c",(char)rootLDirEx[0].LDIR_Name3[j]);
                }
                printf("\n");
            }
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
