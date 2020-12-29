#ifndef HT_H_
#define HT_H_

#include "common.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "BF.h"


typedef struct HT_struct
{
    int fileDesc; /* αναγνωριστικός αριθμός ανοίγματος αρχείου από το επίπεδο block */ 
    char attrType; /* ο τύπος του πεδίου που είναι κλειδί για το συγκεκριμένο αρχείο, 'c' ή'i' */ 
    char * attrName; /* το όνομα του πεδίου που είναι κλειδί για το συγκεκριμένο αρχείο */ 
    int attrLength; /* το μέγεθος του πεδίου που είναι κλειδί για το συγκεκριμένο αρχείο */
    long int numBuckets; /* το πλήθος των “κάδων” του αρχείου κατακερματισμού */
    int * hashtable;
}HT_info;



int HT_CreateIndex(char *fileName, char attrType, char* attrName, int attrLength, int buckets);
HT_info* HT_OpenIndex( char *fileName);
int HT_CloseIndex( HT_info* header_info);
int HT_InsertEntry(HT_info header_info, Record record);
int HT_GetAllEntries(HT_info header_info, void * value);
int HT_DeleteEntry(HT_info header_info, void * value);
int HashStatistics(char * filename);
#endif