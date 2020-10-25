#include <stdio.h>
#include <stdlib.h>
#include "mymalloc.h"

static const size_t metadataSize = sizeof(short) + 1;
static char myBlock[4096];
//Locates the next block's metadata
//It is assumed that start != NULL and points to the beginning of a block's meta data (guaranteed by mymalloc)
//If there is no next block, the function returns NULL
char* nextMeta(char* start, size_t size){
    //if start points to the metadata of the last block
    if (start + metadataSize + size >= myBlock + 4096){ 
        return NULL;
    }
    return start + metadataSize + size;
}
//Creates metadata for block
//It is assumed that start is a valid pointer to metadata inside myBlock (guaranteed by insertData) and use is either 0 or 1
void insertMetadata(char* start, size_t size, char use){
    short* blockSize = (short*)start;
    *blockSize = size;
    char* inUse = (char*)(blockSize + 1);
    *inUse = use;
}
//Creates a new block
//It is assumed start is a valid pointer to metadata inside myBlock (guaranteed by myMalloc) and size <= unused space in the current block (guaranteed by myMalloc)
void* insertData(char* start, size_t size){
    short* blockSize = (short*)start;
    //inserting metadata for leftover unused space after allocating enough for the insert block
    if (*blockSize != size){
        char* newMetadataStart = start + metadataSize + size;
        size_t newBlockSize = *blockSize - metadataSize - size;
        insertMetadata(newMetadataStart, newBlockSize, 0);
    }
    insertMetadata(start, size, 1);
    return start + metadataSize;
}
//Returns a contiguous block of memory of size (size)
//No assumptions about the parameters were made
//If there is no unused block with space >= size or if size == 0, an error is printed out to stdout
void * mymalloc(size_t size, char* file, int line){
    static int init = 0;
    if (!init){
        //initialize metadata when empty
        insertMetadata(myBlock, 4096-metadataSize, 0);
        init = 1;
    }
    //User requests block with size 0
    if (size == 0){
        printf("Mallocing size 0\n");
        return NULL;
    }
    char* ptr = myBlock;
    //traverse through metadata in myBlock
    while (ptr != NULL){
        short* blockSize = (short*)ptr;
        char* inUse = (char*)(blockSize + 1);
        //find the first block with enough available space
        if (*blockSize >= size && *inUse == 0){
            //if the remaining space in the original blockSize does not have enough space for metadataSize and unused space,
            //allocate the entire blockSize for the data to be inserted
            if (*blockSize - (long)size - (long)metadataSize <= 0){
                return insertData(ptr, *blockSize);
            }
            return insertData(ptr, size);
        }
        ptr = nextMeta(ptr, *blockSize);
    }
    //could not find a block with enough space
    printf("Out of space on line %d in file %s\n", line, file);
    return NULL;
}
//Merges an unused block with an unused block located directly on its left side
//It is assumed cur points to the beginning of a block's metadata(guaranteed by mergeFragments) and prev points to the beginning of the previous block's metadata(guaranteed by mergeFragments)
void mergeLeft(char* prev, char* cur){
    short* prevSize = (short*)prev;
    short* curSize = (short*)cur;
    *prevSize += *curSize + metadataSize;
}
//Merges the current block with blocks directly on its left and right if they are not being used
//It is assumed that cur points to the beginning of a block's metadata(guaranteed by myFree) and if prev or next are not NULL they point to the metadata of the previous and next block respectively
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
//Frees a block of data that was obtained from mymalloc
//No assumptions were made about the parameters
//If ptr does not point to the beginning of a block, an error is printed out to stdout
void myfree(void* ptr, char* file, int line){
    char* traversePtr = myBlock;
    char* prev = NULL;
    //traverse through metadata in myBlock
    while (traversePtr != NULL){
        char* inUse = traversePtr + sizeof(short);
        short blockSize = *((short*)traversePtr);
        if (inUse + 1 == ptr && *inUse != 0){
            *inUse = 0;
            mergeFragments(prev, traversePtr, nextMeta(traversePtr, blockSize));
            return;
        }
        //freeing unused memory
        else if(*inUse == 0 && inUse + 1 == ptr){
            break;
            //error
        }
        else{
            prev = traversePtr;
            traversePtr = nextMeta(traversePtr, blockSize);
        }
    }
    //did not find pointer in memory
    printf("Free error on line %d in file %s\n", line, file);
}
