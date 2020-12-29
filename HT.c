#include "HT.h"
#include "BF.h"


int hashingS(int bucket_size, char * ckey)
{
	int i=0;
	int position=0;

	while(ckey && ckey[i] != '\0')
	{
            position=(position+ckey[i]) % bucket_size; 
            ++i;
	}
	return position;
}

//Robert Jenkins' 32 bit integer hash function
//https://gist.github.com/badboy/6267743#robert-jenkins-32-bit-integer-hash-function
unsigned int hashingI(int bucket_size, unsigned int a)
{
   a = (a+0x7ed55d16) + (a<<12);
   a = (a^0xc761c23c) ^ (a>>19);
   a = (a+0x165667b1) + (a<<5);
   a = (a+0xd3a2646c) ^ (a<<9);
   a = (a+0xfd7046c5) + (a<<3);
   a = (a^0xb55a4f09) ^ (a>>16);
   a = a % bucket_size;
   return a;
}

/* 
    Dimiourgei ena neo arxeio katakermatismou me onoma filename. Sto 1o block apothikevontai oi plirofories "HTinfo"
    oi opoies voithoun metepita stin ilopoihsh ton ipoloipon sinartisewn. Oi plirofories perigrafontai analitika sto arxeio
    HT.h.
*/
int HT_CreateIndex(char *fileName, char attrType, char* attrName, int attrLength, int buckets)
{
    if(attrType != 'c' && attrType != 'i')
    {
        BF_PrintError("HT_CreateIndex_Error: attrType not valid.");
        return -1;
    }
    if(buckets <= 0 || attrLength < 2)
    {
        BF_PrintError("HT_CreateIndex_Error: buckets or attrLength not valid.");
        return -1;
    }

    if (BF_CreateFile(fileName)<0)
    {
        BF_PrintError("HT_CreateIndex_Error");
        return -1;
    }

    int fileDescriptor = BF_OpenFile(fileName); //fd is an ID for each file. 
    if(fileDescriptor<0)
    {
        BF_PrintError("HT_CreateIndex_Error");
        return -1;
    }

    if(BF_AllocateBlock(fileDescriptor)<0)
    {
        BF_PrintError("HT_CreateIndex_Error");
        BF_CloseFile(fileDescriptor);
        return -1;
    }
    
    char * block;
    int FirstBlock = BF_GetBlockCounter(fileDescriptor) - 1;
    if (BF_ReadBlock(fileDescriptor, FirstBlock, (void**)&block)<0)
    {
        BF_PrintError("HT_CreateIndex_Error");
        BF_CloseFile(fileDescriptor);
        return -1;
    }
    // Arxikopoiw tis aparaitites plirofories gia to arxeio katakermatismou.
    HT_info myHTinfo;
    myHTinfo.attrLength = strlen(attrName)+1;
    myHTinfo.attrName = malloc( sizeof(char) * (strlen(attrName)+1));
    strcpy(myHTinfo.attrName, attrName);
    myHTinfo.numBuckets = buckets;
    myHTinfo.attrType = attrType;
    myHTinfo.fileDesc = fileDescriptor;
        
    myHTinfo.hashtable = malloc( buckets*sizeof(int ));
    
    for (int i = 0; i < buckets; i++)
    {
        myHTinfo.hashtable[i] = -1;
    }

    // antigrafw tin pliroforia stin mnimi
    block[0] = HT_TYPE; //0 for HashTable
    memcpy(block+1, &myHTinfo, sizeof(myHTinfo));
    strcpy(block+1+sizeof(myHTinfo), myHTinfo.attrName);
    memcpy(block+1+sizeof(myHTinfo)+strlen(attrName)+1, myHTinfo.hashtable, buckets*sizeof(int));

    // grafw tis plirofories ston disko.
    if(BF_WriteBlock(fileDescriptor, FirstBlock)<0)
    {
        BF_PrintError("HT_CreateIndex_Error");
        BF_CloseFile(fileDescriptor);
        return -1;
    }

    BF_CloseFile(fileDescriptor);
    free (myHTinfo.attrName);
    free (myHTinfo.hashtable);
    return 0;
}

/*
    H HT_OpenIndex anoigei to arxeio katakermatismou filename kai "pairnei" apo mesa tis plirofories gia to arxeio. 
    Tis apothikevei se mia prosorini domh.
*/
HT_info * HT_OpenIndex(char * fileName)
{
    int fileDescriptor = BF_OpenFile(fileName); //fd is an ID for each file. 
    if(fileDescriptor<0)
    {
        BF_PrintError("HT_OpenIndex_Error");
        return NULL;
    }
    
    char * block;
    int FirstBlock = 0;
    if (BF_ReadBlock(fileDescriptor, FirstBlock, (void**)&block)<0)
    {
        BF_PrintError("HT_OpenIndex_Error");
        BF_CloseFile(fileDescriptor);
        return NULL;
    }
    if(block[0]!=HT_TYPE)
    {
        BF_PrintError("HT_OpenIndex_Error");
        BF_CloseFile(fileDescriptor);
        return NULL;
    }

    HT_info * myHTinfo = malloc(sizeof( HT_info));
    memcpy(myHTinfo, block+1, sizeof(HT_info));
    myHTinfo->attrName = malloc(sizeof( char)*myHTinfo->attrLength);
    strcpy(myHTinfo->attrName, block+1+sizeof(HT_info));
    myHTinfo->hashtable = malloc( myHTinfo->numBuckets *sizeof(int ));
    memcpy(myHTinfo->hashtable, block+1+sizeof(HT_info) + myHTinfo->attrLength, myHTinfo->numBuckets*sizeof(int));
    myHTinfo->fileDesc = fileDescriptor;

    return myHTinfo;
}

/*
    Diavazei to 1o block kai vlepei an egine kapoia allagh se auto. Kanei overwrite kai kleinei to arxeio.
*/
int HT_CloseIndex(HT_info * header_info)
{
    char * block;
    int FirstBlock = 0;
    if (BF_ReadBlock(header_info->fileDesc, FirstBlock, (void**)&block)<0)
    {
        BF_PrintError("HT_CloseIndex_Error");
        BF_CloseFile(header_info->fileDesc);
        return -1;
    }

    //memcpy(block+1, header_info, sizeof(HT_info));
    //strcpy(block+1+sizeof(HT_info), header_info->attrName);
    memcpy(block+1+sizeof(HT_info)+header_info->attrLength, header_info->hashtable, header_info->numBuckets*sizeof(int));

    if(BF_WriteBlock(header_info->fileDesc, FirstBlock)<0)
    {
        BF_PrintError("HT_CloseIndex_Error");
        BF_CloseFile(header_info->fileDesc);
        return -1;
    }

    BF_CloseFile(header_info->fileDesc);
    free( header_info->attrName );
    free( header_info->hashtable);
    return 0;
}

/*
    PANIKOS!! 
    Vimatika i eksigisi tis sinartisis. 
    1. Dinoume stin katalili katakermatismou to value me vasi to opoio theloume na kanoume eisagegh (c or i && if(c) name or surname or address)
    2.A. Arxiki periptosh na min exei ginei alli eisagwgh (no collision). Se auth thn periptwsh dimiourgoume ena neo block mesa sto opoio tha apo thikeusoume thn eggrafh
        i. Prin kanoume tin eisagwgh tis neas eggrafhs apothikevooume stin prwth thesh tou neou block thn pliroforia tou Block (emeis to leme kai bucket). Oi plirofories einai sto arxeio common.h
        ii. Istera apothikevetai to neo record stin 1h diathesimi thesi. 
    2.B. Alli periptwsh einai na exei ginei idi eisagwgh (collision). Ekei ta vimata einai ws eksis. 
        i. Diasxizei ta blocks mexri na vrei tin prwti diathesimi thesi. Molis tin vrei apothikevetai ekei.
*/
int HT_InsertEntry(HT_info header_info, Record record)
{   
    char isChar;
    int index;
    isChar = (header_info.attrType =='c') ? 1 : 0;
    
    if(isChar)//hash for char.
    {
        if(strcmp(header_info.attrName, "name")==0)
        {
            index = hashingS(header_info.numBuckets, record.name);
        }
        else if(strcmp(header_info.attrName, "surname")==0)
        {
            index = hashingS(header_info.numBuckets, record.surname);
        }
        else
        {
            index = hashingS(header_info.numBuckets, record.address);
        }   
    }
    else//hash for int 
    {
        index = hashingI(header_info.numBuckets, record.id);
    }
    // no collision.
    if(header_info.hashtable[index] == -1)
    {
        if(BF_AllocateBlock(header_info.fileDesc) < 0)
        {
            BF_PrintError("HT_InsertEntry_Error: Allocate error");
            return -1;
        }
        int new_block = BF_GetBlockCounter(header_info.fileDesc) - 1;
        header_info.hashtable[index] = new_block;
        
        BucketInfo bi;
        bi.bitmap = 0;
        bi.nextBlockIndex = -1;
        bi.numRecords = 0;

        char * block;
        if (BF_ReadBlock(header_info.fileDesc, header_info.hashtable[index], (void**)&block)<0)
        {
            BF_PrintError("HT_InsertEntry_Error");
            BF_CloseFile(header_info.fileDesc);
            return -1;
        }

        memcpy(block, &bi, sizeof(BucketInfo));
        if(BF_WriteBlock(header_info.fileDesc, new_block)<0)
        {
            BF_PrintError("HT_InsertEntry_Error");
            BF_CloseFile(header_info.fileDesc);
            return -1;
        }
    }
    int tmp_block_index = header_info.hashtable[index];
    while (1)
    {
        // collision
        char * block;
        if (BF_ReadBlock(header_info.fileDesc, tmp_block_index, (void**)&block)<0)
        {
            BF_PrintError("HT_InsertEntry_Error");
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
                    BF_PrintError("HT_InsertEntry_Error: Allocate error");
                    return -1;
                }
                int new_block = BF_GetBlockCounter(header_info.fileDesc) - 1;
                bi.nextBlockIndex = new_block;
                
                memcpy(block, &bi, sizeof(BucketInfo));
                if(BF_WriteBlock(header_info.fileDesc, tmp_block_index)<0)
                {
                    BF_PrintError("HT_InsertEntry_Error");
                    BF_CloseFile(header_info.fileDesc);
                    return -1;
                }

                bi.bitmap = 0;
                bi.nextBlockIndex = -1;
                bi.numRecords = 0;

                if (BF_ReadBlock(header_info.fileDesc, new_block, (void**)&block)<0)
                {
                    BF_PrintError("HT_InsertEntry_Error");
                    BF_CloseFile(header_info.fileDesc);
                    return -1;
                }

                memcpy(block, &bi, sizeof(BucketInfo));
                if(BF_WriteBlock(header_info.fileDesc, new_block)<0)
                {
                    BF_PrintError("HT_InsertEntry_Error");
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
                BF_PrintError("HT_InsertEntry_Error");
                BF_CloseFile(header_info.fileDesc);
                return -1;
            } 
            
            break;
        }   
    }
}


/*
    Pername thn timi value apo tin katalili sinartisi katakermatismou.
    Gia ton index pou prokiptei koitame ola ta records tou kathe bucket tou. 
    An to paidio kleidi einai idio me to value tipwnoume to stixio. 
    Borei parapanw apo ena record na tipwthei.
*/
int HT_GetAllEntries(HT_info header_info, void * value)
{
    char isChar; 
    int index;
    isChar = (header_info.attrType =='c') ? 1 : 0;
    
    if(isChar)  // Hash for char.
        index = hashingS(header_info.numBuckets, (char*)value);

    else    // Hash for int 
        index = hashingI(header_info.numBuckets, *((int*)value));

    if(header_info.hashtable[index] == -1)
        return 0;   // There were no blocks.

    
    int tmp_block_index = header_info.hashtable[index];
    int block_count = 0 ; 
    while (tmp_block_index != -1)
    {
        // collision
        char * block;
        if (BF_ReadBlock(header_info.fileDesc, tmp_block_index, (void**)&block)<0)
        {
            BF_PrintError("HT_InsertEntry_Error");
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
                else if(isChar && strcmp(record_found.name,(char*)value) == 0 && strcmp(header_info.attrName, "name")==0)
                {
                    printf("ID: %d\n", record_found.id);
                }
                else if(isChar && strcmp(record_found.name,(char*)value) == 0 && strcmp(header_info.attrName, "surname")==0)
                {
                    printf("ID: %d\n", record_found.id);
                }
                else if(isChar && strcmp(record_found.name,(char*)value) == 0 && strcmp(header_info.attrName, "address")==0)
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
    Pername thn timi value apo tin katalili sinartisi katakermatismou.
    Gia ton index pou prokiptei koitame ola ta records tou kathe bucket tou. 
    An to paidio kleidi einai idio me to value diagrafoume to stixio. 
    H diagrafh epitigxanetai allazontas to bitmap apo 1 se 0 gia to record kai mionontas ton sinoliko arithmo eggrafwn.
*/
int HT_DeleteEntry(HT_info header_info, void * value)
{
    char isChar; 
    int index;
    isChar = (header_info.attrType =='c') ? 1 : 0;
    
    if(isChar)  // Hash for char.
        index = hashingS(header_info.numBuckets, (char*)value);

    else    // Hash for int 
        index = hashingI(header_info.numBuckets, *((int*)value));

    if(header_info.hashtable[index] == -1)
        return 0;   // There were no blocks.

    
    int tmp_block_index = header_info.hashtable[index];
    int block_count = 0 ; 
    while (tmp_block_index != -1)
    {
        char * block;
        if (BF_ReadBlock(header_info.fileDesc, tmp_block_index, (void**)&block)<0)
        {
            BF_PrintError("HT_InsertEntry_Error");
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
                else if(isChar && strcmp(record_found.name,(char*)value) == 0 && strcmp(header_info.attrName, "name")==0)
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
                else if(isChar && strcmp(record_found.name,(char*)value) == 0 && strcmp(header_info.attrName, "surname")==0)
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
                else if(isChar && strcmp(record_found.name,(char*)value) == 0 && strcmp(header_info.attrName, "address")==0)
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


void HT_Printer(HT_info header_info)
{
    char isChar; 
    isChar = (header_info.attrType =='c') ? 1 : 0;    
    int one_more_counter = 0;
    int tmp_block_index;
    int block_count = 0;
    int records_count = 0;
    int cicle_block_counter = 0; 
    int temp_records = 0;
    long int mysize = header_info.numBuckets;
    int records_per_block[mysize]; 

        
    // Gia kathe kouva
    for (int i = 0; i < header_info.numBuckets; i++)
    {    
        tmp_block_index = header_info.hashtable[i];
        
        // Gia kathe block tou kouva
        while (tmp_block_index != -1)
        {
            char * block;
            if (BF_ReadBlock(header_info.fileDesc, tmp_block_index, (void**)&block)<0)
            {
                BF_PrintError("HT_InsertEntry_Error");
                BF_CloseFile(header_info.fileDesc);
                return;
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
                    // Gia kathe egkyri eggrafi
                    records_count++;  
                    temp_records++;               
                }
                availableIndex++;
                tmp_bit = tmp_bit >> 1;
            }
            block_count++;
            cicle_block_counter++;
            tmp_block_index = bi.nextBlockIndex; 
        }
    records_per_block[i] = temp_records;
    if(cicle_block_counter>=2)
    {
        printf("For bucket %d there are %d blocks.\n", i, cicle_block_counter);
        one_more_counter++;
    }
        
    temp_records = 0;
    cicle_block_counter = 0;
    }

    int max = records_per_block[0];
    int min = records_per_block[0];

    for (int i = 1; i < sizeof(records_per_block)/sizeof(records_per_block[0]); i++)
    {
        if(records_per_block[i]>max)
            max = records_per_block[i];

        if(records_per_block[i]<min);
            min = records_per_block[i];
    }
    puts("");
    puts("Printing Hash Statistics");
    puts("------------------------");

    printf("File has: \n%d blocks in total.\n%ld records per bucket in average. \n%d is the minimum num of records in a bucket and\n%d is the maximum num of records in a bucket.\n",block_count, records_count/header_info.numBuckets, min, max);
    printf("%ld blocks/bucket in average and\n", block_count/header_info.numBuckets);
    printf("%d overflown buckets as well.\n", one_more_counter);
}


int HashStatistics(char * filename)
{
    HT_info * myHtinfoTemp = HT_OpenIndex(filename);
    
    HT_Printer(*myHtinfoTemp);
   
    HT_CloseIndex(myHtinfoTemp);
}
