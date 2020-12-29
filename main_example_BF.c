#include <stdio.h>
#include <stdlib.h>


#include "BF.h"
#include "HT.h"
#include "HP.h"
#include "common.h"


#define indexname  "test_hash_index.txt"
#define hpindexname  "test_heap_index.txt"


int main(int argc, char** argv)
{
    FILE *f = fopen("records15K.txt", "r");
        
    char line[100];
    char *tok;

    Record record;    
    BF_Init();

    char answer; 
    printf("Do you want to execute for Hash? [y]es/[n]o: ");
    scanf("%c", &answer);

    if(answer=='y')
    {

        HT_info *ht_info;


        HT_CreateIndex(indexname, 'c', "name", 5, 50);
        /*
        work for each
        HT_CreateIndex(indexname, 'c', "surname", 5, 35);
        HT_CreateIndex(indexname, 'c', "address", 5, 35);
        HT_CreateIndex(indexname, 'i', "id", 5, 35);
        */ 
        ht_info = HT_OpenIndex(indexname);
        
        int i=0;
        while (!feof(f) && i<5000) {
            i++;
            fgets(line, 100, f);
            
            
            
            tok = strtok( line, "{},\"" );
            
            record.id = atoi(tok);
            strcpy(record.name, strtok( NULL, "{},\"" ));
            strcpy(record.surname, strtok( NULL, "{},\"" ));
            strcpy(record.address, strtok( NULL, "{},\"" ));
            
            //printf("%d %s %s %s\n", record.id, record.name, record.surname, record.address);
            
            HT_InsertEntry(*ht_info, record);
        }
        
        HT_GetAllEntries(*ht_info, ((void *)"name_501") );

        HT_DeleteEntry(*ht_info, ((void *)"name_501") );
        
        HT_GetAllEntries(*ht_info, ((void *)"name_501") );
        
        
        HT_CloseIndex(ht_info);
        BF_PrintError("Errors");
        
        fclose(f);

        HashStatistics(indexname);
        free(ht_info);
        return 0;
    }
    else if(answer == 'n')
    {
        puts("Executing for Heap but there is nothing to see...Hope there is no seg fault..wish me luck.");

        HP_info *hp_info;


        HP_CreateIndex(hpindexname, 'c', "name", 5, 35);
        
        hp_info = HP_OpenFile(hpindexname);
        
        int i=0;
        while (!feof(f) && i<5000) {
            i++;
            fgets(line, 100, f);
            
            
            
            tok = strtok( line, "{},\"" );
            
            record.id = atoi(tok);
            strcpy(record.name, strtok( NULL, "{},\"" ));
            strcpy(record.surname, strtok( NULL, "{},\"" ));
            strcpy(record.address, strtok( NULL, "{},\"" ));
            
            //printf("%d %s %s %s\n", record.id, record.name, record.surname, record.address);
            
            HP_InsertEntry(*hp_info, record);
        }
        
        HP_GetAllEntries(*hp_info, ((void *)"name_501") );

        HP_DeleteEntry(*hp_info, ((void *)"name_501") );
        
        HP_GetAllEntries(*hp_info, ((void *)"name_501") );
        
        
        HP_CloseFile(hp_info);
        BF_PrintError("Errors");
        
        fclose(f);
        return 0;
    }
    else
    {
        perror("400-Bad request");
        return 0; 
    }
    
}

