#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include "mymalloc.h"

double calcMean(long runtime[]) {
	double sum = 0;
	for (int i = 0; i < 50; i++){
		sum += runtime[i];
    }
    return sum / 50;
}

int main(int argc, char* argv[]) {
	struct timeval start, end;
    const size_t metadataSize = sizeof(short) + 1;
	long runtime[5][50];
    for(int j = 0; j < 50; j++) {
    
        //A Workload
        gettimeofday(&start,NULL);
        for(int i = 0; i < 120; i++) {
            char* c = malloc(sizeof(char));
            free(c);
        }
        gettimeofday(&end,NULL);
        runtime[0][j] = end.tv_usec - start.tv_usec;
        
        //B Workload
        gettimeofday(&start,NULL);
        char* arr[120];
        for(int i = 0; i < 120; i++) {
            arr[i] = malloc(sizeof(char));
        }
        for(int i = 0; i < 120; i++) {
            free(arr[i]);
        }
        gettimeofday(&end,NULL);
        runtime[1][j] = end.tv_usec - start.tv_usec;

        //C Workload
        gettimeofday(&start,NULL);
        int numMalloc = 0;
        int numFree = 0;
        char* randArr[120];
        int curSize = 0;
        while (numMalloc < 120 || numFree < 120)
        {
            if (numMalloc == 120){
                free(randArr[--curSize]);
                ++numFree;
                continue;
            }
            int choice = rand() % 2;
            if (choice == 0 && numMalloc < 120){
                randArr[curSize++] = malloc(sizeof(char));
                numMalloc++;
            }
            else if(choice == 1 && numFree < 120 && curSize >= 1) {
                free(randArr[--curSize]);
                ++numFree;
            }
        }
        gettimeofday(&end,NULL);
        runtime[2][j] = end.tv_usec - start.tv_usec;

        //D Workload
        gettimeofday(&start,NULL);
        char* d = malloc(4096 - metadataSize - 1);
        //D error: Saturation of dynamic memory
        char* e = malloc(1);
        //A error: Free()ing addresses that are not pointers
        free((int*)20);
        //B error: Free()ing pointers that were not allocated by malloc()
        free(e);
        //C error: Redundant free()ing of the same pointer
        free(d);
        free(d);
        gettimeofday(&end,NULL);
        runtime[3][j] = end.tv_usec - start.tv_usec;

        //E Workload
        gettimeofday(&start,NULL);
        char* f = malloc(200);
        char* g = malloc(200);
        char* h = malloc(200);
        char* i = malloc(200);

        free(g);
        free(h); // left merge
        printf("%u\n", *((short*)(f+200))); //should print 400+ metadataSize

        //test insert with newly freed space
        char* k = malloc(300 + metadataSize);
        printf("%u\n", *((short*)(f+200))); //should print 300+ metadataSize

        free(k); // right merge 
        printf("%u\n", *((short*)(f+200))); //should print 400 + metadataSize

        free(i); // left and right merge
        printf("%u\n", *((short*)(f+200))); // should print 4096-2*metadataSize-200
        free (f);
        gettimeofday(&end,NULL);
        runtime[4][j] = end.tv_usec - start.tv_usec;
    }

	for (int i = 0; i < 5; i++) {
        printf("Avg runtime of task %d: %lf microsecs\n", i, calcMean(runtime[i]));
    }
}
