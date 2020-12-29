#ifndef HP_H_
#define HP_H_
#include "common.h"
#include "BF.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>




typedef struct HP_info
{
    int fileDesc; /* αναγνωριστικός αριθμός ανοίγματος αρχείου από το επίπεδο block */ 
    char attrType; /* ο τύπος του πεδίου που είναι κλειδί για το συγκεκριμένο αρχείο, 'c' ή'i' */ 
    char *attrName; /* το όνομα του πεδίου που είναι κλειδί για το συγκεκριμένο αρχείο */ 
    int attrLength; /* το μέγεθος του πεδίου που είναι κλειδί για το συγκεκριμένο αρχείο */
    int firstBlockIndex;
}HP_info;


int HP_CreateIndex(char * fileName, char attrType, char* attrName, int attrLength, int buckets);
HP_info* HP_OpenFile(char * fileName);
int HP_CloseFile(HP_info * header_info);
int HP_InsertEntry(HP_info header_info, Record record);
int HP_GetAllEntries(HP_info header_info, void * value);
int HP_DeleteEntry(HP_info header_info, void * value);


#endif
