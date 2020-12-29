#ifndef COMMON_
#define COMMON_

#include "BF.h"


typedef struct Record
{
    int id;
    char name[15];
    char surname[25];
    char address[50];
}Record;

typedef struct BucketInfo
{
    unsigned int bitmap; 
    int nextBlockIndex;
    int numRecords; //plithos eggrafwn
}BucketInfo;

#define HT_TYPE 0
#define HP_TYPE 1
#define MAX_RECORDS ((BLOCK_SIZE-sizeof(BucketInfo))/sizeof(Record))

#endif