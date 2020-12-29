#include "HP.h"

/* 
    Dimiourgei ena neo arxeio sorou me onoma filename. Sto 1o block apothikevontai oi plirofories "HPinfo"
    oi opoies voithoun metepita stin ilopoihsh ton ipoloipon sinartisewn. Oi plirofories perigrafontai analitika sto arxeio
    HP.h.
*/
int HP_CreateIndex(char * fileName, char attrType, char* attrName, int attrLength, int buckets)
{
    
    if(attrType != 'c' && attrType != 'i')
    {
        BF_PrintError("HP_CreateIndex_Error: attrType not valid.");
        return -1;
    }
    if(attrLength < 2)
    {
        BF_PrintError("HP_CreateIndex_Error: buckets or attrLength not valid.");
        return -1;
    }

    if (BF_CreateFile(fileName)<0)
    {
        BF_PrintError("HP_CreateIndex_Error");
        return -1;
    }

    int fileDescriptor = BF_OpenFile(fileName); //fd is an ID for each file. 
    if(fileDescriptor<0)
    {
        BF_PrintError("HP_CreateIndex_Error");
        return -1;
    }

    if(BF_AllocateBlock(fileDescriptor)<0)
    {
        BF_PrintError("HP_CreateIndex_Error");
        BF_CloseFile(fileDescriptor);
        return -1;
    }
    
    void * block;
    int FirstBlock = BF_GetBlockCounter(fileDescriptor) - 1;
    if (BF_ReadBlock(fileDescriptor, FirstBlock, &block)<0)
    {
        BF_PrintError("HP_CreateIndex_Error");
        BF_CloseFile(fileDescriptor);
        return -1;
    }
    
    // Dimiourgw tin pliroforia gia ta arxeia block pou molis anoiksa me skopo na tin apothikefsw meta sto 1o block. 
    // To 1o block periexei mono tis aparetites plirofories.  
    HP_info myHPinfo;
    myHPinfo.attrLength = strlen(attrName)+1;
    myHPinfo.attrName = malloc( strlen(attrName)+1);
    strcpy(myHPinfo.attrName, attrName);
    myHPinfo.attrType = attrType;
    myHPinfo.fileDesc = fileDescriptor;
    myHPinfo.firstBlockIndex = -1;    
    
    ((char *)block)[0] = HP_TYPE; //1 for Heap
    memcpy(block+1, &myHPinfo, sizeof(HP_info));
    strcpy(block+1+sizeof(HP_info), myHPinfo.attrName);
   

    if(BF_WriteBlock(fileDescriptor, FirstBlock)<0)
    {
        BF_PrintError("HP_CreateIndex_Error");
        BF_CloseFile(fileDescriptor);
        return -1;
    }

    BF_CloseFile(fileDescriptor);
    free( myHPinfo.attrName );
    return 0;
}

/*
    Anoigei to arxeio sorou filename kai "pairnei" apo mesa tis plirofories gia to arxeio. 
    Tis apothikevei se mia prosorini domh.
*/
HP_info* HP_OpenFile(char * fileName)
{
    int fileDescriptor = BF_OpenFile(fileName); //fd is an ID for each file. 
    if(fileDescriptor<0)
    {
        BF_PrintError("HP_OpenIndex_Error");
        return NULL;
    }
    
    void * block;
    int FirstBlock = 0;
    if (BF_ReadBlock(fileDescriptor, FirstBlock, (void**)&block)<0)
    {
        BF_PrintError("HP_OpenIndex_Error");
        BF_CloseFile(fileDescriptor);
        return NULL;
    }
    if(((char *)block)[0]!=HP_TYPE)
    {
        BF_PrintError("HP_OpenIndex_Error");
        BF_CloseFile(fileDescriptor);
        return NULL;
    }

    HP_info * myHPinfo = malloc( sizeof(HP_info) );
    
    memcpy(myHPinfo, block+1, sizeof(HP_info));
    
    myHPinfo->attrName = malloc( myHPinfo->attrLength);
    strcpy(myHPinfo->attrName, block+1+sizeof(HP_info));
    myHPinfo->fileDesc = fileDescriptor;

    return myHPinfo;
}
/*
    Diavazei to 1o block kai vlepei an egine kapoia allagh se auto. Kanei overwrite kai kleinei to arxeio.
*/
int HP_CloseFile(HP_info * header_info)
{
    char * block;
    int FirstBlock = 0;
    if (BF_ReadBlock(header_info->fileDesc, FirstBlock, (void**)&block)<0)
    {
        BF_PrintError("HP_CloseIndex_Error");
        BF_CloseFile(header_info->fileDesc);
        return -1;
    }

    memcpy(block+1, header_info, sizeof(HP_info));
//    memcpy(block+1+sizeof(HP_info), header_info->attrName, strlen(header_info->attrName)+1);


    if(BF_WriteBlock(header_info->fileDesc, FirstBlock)<0)
    {
        BF_PrintError("HP_CloseIndex_Error");
        BF_CloseFile(header_info->fileDesc);
        return -1;
    }

    BF_CloseFile(header_info->fileDesc);
    free( header_info->attrName );
    free( header_info );
    return 0;
}

/* 
    Prospathi na eisagei to record entos tou arxeiou.
    Elegxei an exei ksanaginei kapoia eggrafh
    (I) An den exei ksanaginei fimiourgei ena neo block kai apothikevei tin pliroforia tou neou block (Bucket info). Eisagei epita to record stin diathesimi thesh. 
    Allh periptwsh: An exei ginei alli eisagwgh nwritera kai iparxei thesi paei stin amesws epomenh diathesimi. 
    An to block exei max records kanei neo kai epanalamvanei oti sto (I).
*/
int HP_InsertEntry(HP_info header_info, Record record)
{   
    char isChar;
    isChar = (header_info.attrType =='c') ? 1 : 0;
    
    header_info.firstBlockIndex = BF_GetBlockCounter(header_info.fileDesc) - 1 == 0 ? -1 : 1;
    
    if(header_info.firstBlockIndex == -1)
    {
        if(BF_AllocateBlock(header_info.fileDesc) < 0)
        {
            BF_PrintError("HP_InsertEntry_Error: Allocate error");
            return -1;
        }
        int new_block = BF_GetBlockCounter(header_info.fileDesc) - 1;
        header_info.firstBlockIndex = new_block;
        
        BucketInfo bi;
        bi.bitmap = 0;
        bi.nextBlockIndex = -1;
        bi.numRecords = 0;

        char * block;
        if (BF_ReadBlock(header_info.fileDesc, header_info.firstBlockIndex, (void**)&block)<0)
        {
            BF_PrintError("HP_InsertEntry_Error");
            BF_CloseFile(header_info.fileDesc);
            return -1;
        }

        memcpy(block, &bi, sizeof(BucketInfo));
        if(BF_WriteBlock(header_info.fileDesc, new_block)<0)
        {
            BF_PrintError("HP_InsertEntry_Error");
            BF_CloseFile(header_info.fileDesc);
            return -1;
        }
        
    }
    int tmp_block_index = header_info.firstBlockIndex;
    while (1)
    {
        char * block;
        if (BF_ReadBlock(header_info.fileDesc, tmp_block_index, (void**)&block)<0)
        {
            BF_PrintError("HP_InsertEntry_Error");
            BF_CloseFile(header_info.fileDesc);
            return -1;
        }
        BucketInfo bi;
        memcpy(&bi, block, sizeof(BucketInfo));
        
        if(bi.nextBlockIndex > 0 && bi.numRecords == MAX_RECORDS)
        {
            tmp_block_index = bi.nextBlockIndex;
            continue;
        }
        else
        {
            
            if (bi.numRecords == MAX_RECORDS)
            {
                if(BF_AllocateBlock(header_info.fileDesc) < 0)
                {
                    BF_PrintError("HP_InsertEntry_Error: Allocate error");
                    return -1;
                }
                int new_block = BF_GetBlockCounter(header_info.fileDesc) - 1;
                bi.nextBlockIndex = new_block;
                
                memcpy(block, &bi, sizeof(BucketInfo));
                if(BF_WriteBlock(header_info.fileDesc, tmp_block_index)<0)
                {
                    BF_PrintError("HP_InsertEntry_Error");
                    BF_CloseFile(header_info.fileDesc);
                    return -1;
                }

                bi.bitmap = 0;
                bi.nextBlockIndex = -1;
                bi.numRecords = 0;

                if (BF_ReadBlock(header_info.fileDesc, new_block, (void**)&block)<0)
                {
                    BF_PrintError("HP_InsertEntry_Error");
                    BF_CloseFile(header_info.fileDesc);
                    return -1;
                }

                memcpy(block, &bi, sizeof(BucketInfo));
                if(BF_WriteBlock(header_info.fileDesc, new_block)<0)
                {
                    BF_PrintError("HP_InsertEntry_Error");
                    BF_CloseFile(header_info.fileDesc);
                    return -1;
                }
                tmp_block_index = new_block;
            }
            int availableIndex = 0; 
            
            while((bi.bitmap >> availableIndex) & 1)
            {
                availableIndex++;
            }
            memcpy(block+sizeof(BucketInfo)+availableIndex*sizeof(Record), &record, sizeof(Record));
            bi.numRecords++;
            bi.bitmap = bi.bitmap | (1 << availableIndex);

            memcpy(block, &bi, sizeof(BucketInfo));
            
            if(BF_WriteBlock(header_info.fileDesc, tmp_block_index)<0)
            {
                BF_PrintError("HP_InsertEntry_Error");
                BF_CloseFile(header_info.fileDesc);
                return -1;
            }
//            printf("%d %s %s %s\n", record.id, record.name, record.surname, record.address);
            break;
        }   
    }
}
/*
    Gia tin timi value koitame ola ta records tou kathe bucket tou. 
    An to paidio kleidi einai idio me to value tipwnoume to stixio. 
    Borei parapanw apo ena record na tipwthei.
*/
int HP_GetAllEntries(HP_info header_info, void * value)
{
    char isChar; 
    isChar = (header_info.attrType =='c') ? 1 : 0;
    
    
    header_info.firstBlockIndex = BF_GetBlockCounter(header_info.fileDesc) - 1 == 0 ? -1 : 1;
    
    int tmp_block_index = header_info.firstBlockIndex;
    int block_count = 0 ; 
    while (tmp_block_index != -1)
    {
        char * block;
        if (BF_ReadBlock(header_info.fileDesc, tmp_block_index, (void**)&block)<0)
        {
            BF_PrintError("HP_GetAll_Error");
            BF_CloseFile(header_info.fileDesc);
            return -1;
        }
        BucketInfo bi;
        memcpy(&bi, block, sizeof(BucketInfo));

        int availableIndex = 0;
        Record record_found;

        while(bi.bitmap != 0)
        {
            if((bi.bitmap) & 1 )
            {
                memcpy(&record_found, block+sizeof(BucketInfo)+availableIndex*sizeof(Record), sizeof(Record));
                if(!isChar && record_found.id == (*(int*)value))
                {
                    printf("ID: %d\n", record_found.id);
                }
                else if(isChar && strcmp(record_found.name, ((char*)value))==0 && strcmp((header_info.attrName) , "name" )==0 )
                {
                    printf("ID: %d\n", record_found.id);
                }
                else if(isChar && strcmp(record_found.surname, ((char*)value))==0 && strcmp((header_info.attrName) , "surname" )==0 )
                {
                    printf("ID: %d\n", record_found.id);
                }
                else if(isChar && strcmp(record_found.address, ((char*)value))==0 && strcmp((header_info.attrName) , "address" )==0 )
                {
                    printf("ID: %d\n", record_found.id);
                }
            }
            availableIndex++;
            bi.bitmap = bi.bitmap >> 1; 
        }
        block_count++;


        tmp_block_index = bi.nextBlockIndex; 
    }
    return block_count;
}

/*
    Gia tin timi value koitame ola ta records tou kathe bucket tou. 
    An to paidio kleidi einai idio me to value diagrafoume to stixio. 
    H diagrafh epitigxanetai allazontas to bitmap apo 1 se 0 gia to record kai mionontas ton sinoliko arithmo eggrafwn.
*/
int HP_DeleteEntry(HP_info header_info, void * value)
{
    char isChar; 
    int index;
    isChar = (header_info.attrType =='c') ? 1 : 0;
    
    header_info.firstBlockIndex = BF_GetBlockCounter(header_info.fileDesc) - 1 == 0 ? -1 : 1;
    int tmp_block_index = header_info.firstBlockIndex;
    int block_count = 0; 
    while (tmp_block_index != -1)
    {
        char * block;
        if (BF_ReadBlock(header_info.fileDesc, tmp_block_index, (void**)&block)<0)
        {
            BF_PrintError("HP_Delete_Error");
            BF_CloseFile(header_info.fileDesc);
            return -1;
        }
        BucketInfo bi;
        memcpy(&bi, block, sizeof(BucketInfo));

        int availableIndex = 0;
        Record record_found;
        unsigned int tmp_bit = bi.bitmap;
        while(tmp_bit != 0)
        {
            if((tmp_bit) & 1 )
            {
                memcpy(&record_found, block+sizeof(BucketInfo)+availableIndex*sizeof(Record), sizeof(Record));
                if(!isChar && record_found.id == (*(int*)value))
                {
                    bi.bitmap = bi.bitmap ^ (1 << availableIndex);
                    bi.numRecords--;
                    memcpy(block, &bi, sizeof(BucketInfo));
                    if(BF_WriteBlock(header_info.fileDesc, tmp_block_index)<0)
                    {
                        BF_PrintError("HT_Delete_Error");
                        BF_CloseFile(header_info.fileDesc);
                        return -1;
                    }
                    
                    return 0;
                }
                else if(isChar && strcmp(record_found.name, ((char*)value))==0 && strcmp((header_info.attrName) , "name" )==0 )
                {
                    bi.bitmap = bi.bitmap ^ (1 << availableIndex);
                    bi.numRecords--;
                    memcpy(block, &bi, sizeof(BucketInfo));
                    if(BF_WriteBlock(header_info.fileDesc, tmp_block_index)<0)
                    {
                        BF_PrintError("HT_Delete_Error");
                        BF_CloseFile(header_info.fileDesc);
                        return -1;
                    }
                    return 0;
                }
                else if(isChar && strcmp(record_found.surname, ((char*)value))==0 && strcmp((header_info.attrName) , "surname" )==0 )
                {
                    bi.bitmap = bi.bitmap ^ (1 << availableIndex);
                    bi.numRecords--;
                    memcpy(block, &bi, sizeof(BucketInfo));
                    if(BF_WriteBlock(header_info.fileDesc, tmp_block_index)<0)
                    {
                        BF_PrintError("HT_Delete_Error");
                        BF_CloseFile(header_info.fileDesc);
                        return -1;
                    }
                    return 0;
                }
                else if(isChar && strcmp(record_found.address, ((char*)value))==0 && strcmp((header_info.attrName) , "address" )==0 )
                {
                    bi.bitmap = bi.bitmap ^ (1 << availableIndex);
                    bi.numRecords--;
                    memcpy(block, &bi, sizeof(BucketInfo));
                    if(BF_WriteBlock(header_info.fileDesc, tmp_block_index)<0)
                    {
                        BF_PrintError("HT_Delete_Error");
                        BF_CloseFile(header_info.fileDesc);
                        return -1;
                    }
                    return 0;
                }

            }
            availableIndex++;
            tmp_bit = tmp_bit >> 1;
        }
        block_count++;


        tmp_block_index = bi.nextBlockIndex; 
    }
    return block_count;
}
