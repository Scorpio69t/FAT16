#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdint.h>
#include <inttypes.h>


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


int main(){
    return taskThree(1);
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

int outputBootSector() {
    BootSector taskTwoBootSector;
    int file = open("fat16.img", O_RDONLY);
    int toRead = read(file, &taskTwoBootSector, sizeof(taskTwoBootSector));
    printf("Bytes per Sector: %d\n",taskTwoBootSector.BPB_BytsPerSec);
    printf("Sectors per Cluster: %d\n",taskTwoBootSector.BPB_SecPerClus);
    printf("Reserved Sector Count: %d\n",taskTwoBootSector.BPB_RsvdSecCnt);
    printf("Number of copies of FAT: %d\n",taskTwoBootSector.BPB_NumFATs);
    printf("FAT12/FAT16: size of root DIR: %d\n",taskTwoBootSector.BPB_RootEntCnt);
    printf("Sectors, may be 0, see below: %d\n",taskTwoBootSector.BPB_TotSec16);
    printf("Sectors in FAT (FAT12 or FAT16): %d\n",taskTwoBootSector.BPB_FATSz16);
    printf("Sectors if BPB_TotSec16 == 0: %d\n",taskTwoBootSector.BPB_TotSec32);
    printf("Non zero terminated string: %d\n",taskTwoBootSector.BS_VolLab);
    close(file);
    return 0;
}

int taskThree(int startingClusterNumber){
    //Initialise the structures
    BootSector boot;
    Directory dir;
    //Get the size of a cluster
    int clusterSize = boot.BPB_BytsPerSec*boot.BPB_SecPerClus;
    //Get the number of sectors
    int numSec = boot.BPB_FATSz16;
    //Get the start location of the fat
    int startLocation = boot.BPB_RsvdSecCnt*boot.BPB_BytsPerSec;
    //Open the FAT file
    int file = open("fat16.img", O_RDONLY);
    //Seek to the start of the file
    lseek(file,startLocation,SEEK_SET);
    //Load the fat into memory
    uint16_t* fat = malloc(numSec*boot.BPB_BytsPerSec);
    //Now read the fat
    int readFat = read(file,fat,numSec*boot.BPB_BytsPerSec);
    printf("Start Location: %d\n", startLocation);
    for(int i = 0; i<10;i++){
        printf("%d\n", fat[i]);
    }
    close(file);
    return 0;
}