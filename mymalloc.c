#include <stdio.h>
#include <stdlib.h>
#include "mymalloc.h"

static const size_t metadataSize = sizeof(short) + 1;
static char myBlock[4096];

char* nextMeta(char* start, size_t size){
    //start points to the metadata of the last block
    if (start + metadataSize + size >= myBlock + 4096){ 
        return NULL;
    }
    return start + metadataSize + size;
}

void insertMetadata(char* start, size_t size, char use){
    short* blockSize = (short*)start;
    *blockSize = size;
    char* inUse = (char*)(blockSize + 1);
    *inUse = use;
}

void* insertData(char* start, size_t size){
    short* blockSize = (short*)start;
    //inserting metadata for leftover unused space after allocating enough for the insert block
    if (*blockSize != size){
        insertMetadata(start + metadataSize + size, *blockSize - metadataSize - size, 0);
    }
    insertMetadata(start, size, 1);
    return start + metadataSize;
}

void * mymalloc(size_t size, char* file, int line){
    static int init = 0;
    if (!init){
        //initialize metadata when empty
        insertMetadata(myBlock, 4096-metadataSize, 0);
        init = 1;
    }
    char* ptr = myBlock;
    while (ptr != NULL){
        short* blockSize = (short*)ptr;
        char* inUse = (char*)(blockSize + 1);
        //find the first block with enough available space
        if (*blockSize >= size && *inUse == 0){
            //if the remaining space in the original blockSize does not have enough space for metadataSize and unused space,
            //allocate the entire blockSize for the data to be inserted
            if (*blockSize - size - metadataSize <= 0){
                return insertData(ptr, *blockSize);
            }
            return insertData(ptr, size);
        }
        ptr = nextMeta(ptr, *blockSize);
    }
    //could not find a block with enough space
    printf("Out of space on line %d in file %s", line, file);
    return NULL;
}

void mergeLeft(char* prev, char* cur){
    short* prevSize = (short*)prev;
    short* curSize = (short*)cur;
    *prevSize += *curSize + metadataSize;
}

void mergeFragments(char* prev, char* cur, char* next){
    char prevInUse = (prev != NULL)? *(prev + sizeof(short)): 1;
    char nextInUse = (next != NULL)? *(next + sizeof(short)): 1;
    //check if both left and right blocks are free
    if (!prevInUse && !nextInUse){
        mergeLeft(prev, cur);
        mergeLeft(prev, next);
    }
    //check if left block is free
    else if (!prevInUse){
        mergeLeft(prev, cur);
    }
    //check if right block is free
    else if (!nextInUse){
        mergeLeft(cur, next);
    }
}

void myfree(void* ptr, char* file, int line){
    char* traversePtr = myBlock;
    char* prev = NULL;
    while (traversePtr != NULL){
        char* inUse = traversePtr + 1;
        short blockSize = *((short*)traversePtr);
        if (inUse + 1 == ptr && *inUse != 0){
            *inUse = 0;
            mergeFragments(prev, ptr, nextMeta(traversePtr, blockSize));
            return;
        }
        //freeing unused memory
        else if(*inUse == 0){
            break;
            //error
        }
        else{
            prev = traversePtr;
            traversePtr = nextMeta(traversePtr, blockSize);
        }
    }
    //did not find pointer in memory
    printf("Free error on line %d in file %s", line, file);
}
