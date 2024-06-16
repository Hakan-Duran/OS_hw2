// Hakan Duran 150200091

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <bits/mman-linux.h>
#include <sys/wait.h>
#include <string.h>

#include "main.h"

extern void* mem_init; // The start address of memory

int main(){

    printf("How much memory you want?: ");

    int memsize;
    scanf("%d", &memsize);

    int add = memsize % getpagesize();
    if (add){
        memsize = memsize + getpagesize() - add ;
    }

    InitMyMalloc(memsize);



    // void* ptr = MyMalloc(100, 2);
    // void* ptr2 = MyMalloc(250, 2);
    // void* ptr3 = MyMalloc(100, 2);
    // void* ptr4 = MyMalloc(200, 2);
    // void* ptr5 = MyMalloc(100, 2);
    // void* ptr6 = MyMalloc(50, 2);
    // void* ptr7 = MyMalloc(100, 2);
    // MyFree(ptr6);
    // MyFree(ptr4);
    // MyFree(ptr2);
    // DumpFreeList();
    // void* ptr8 = MyMalloc(120, 2);
    // DumpFreeList();
    // void* ptr9 = MyMalloc(20, 2);
    // DumpFreeList();



    // PROCESS 1

    int pid = fork();

    if (pid < 0) {
        // Error occurred
        fprintf(stderr, "Fork failed\n");
        return 1;
    } else if (pid == 0) {
        // Child process

        printf("\nChild PID: %d\n", getpid());

        int strategy;
        printf("Please enter the strategy type (0,1,2,3): ");
        scanf("%d", &strategy);
        printf("\n");

        void* ptr = MyMalloc(100, strategy);

        DumpFreeList();

        return 0;
    } else {
        // Parent process
        wait(NULL);
    }

    // PROCESS 2

    pid = fork();

    if (pid < 0) {
        // Error occurred
        fprintf(stderr, "Fork failed\n");
        return 1;
    } else if (pid == 0) {
        // Child process

        printf("\nChild PID: %d\n", getpid());

        int strategy;
        printf("Please enter the strategy type (0,1,2,3): ");
        scanf("%d", &strategy);
        printf("\n");

        void* ptr = MyMalloc(200, strategy);

        DumpFreeList();

        return 0;
    } else {
        // Parent process
        wait(NULL);
    }

    // PROCESS 3

    pid = fork();

    if (pid < 0) {
        // Error occurred
        fprintf(stderr, "Fork failed\n");
        return 1;
    } else if (pid == 0) {
        // Child process

        printf("\nChild PID: %d\n", getpid());

        int strategy;
        printf("Please enter the strategy type (0,1,2,3): ");
        scanf("%d", &strategy);
        printf("\n");

        void* ptr = MyMalloc(300, strategy);

        DumpFreeList();

        return 0;
    } else {
        // Parent process
        wait(NULL);
    }

    // PROCESS 4

    pid = fork();

    if (pid < 0) {
        // Error occurred
        fprintf(stderr, "Fork failed\n");
        return 1;
    } else if (pid == 0) {
        // Child process

        printf("\nChild PID: %d\n", getpid());

        int strategy;
        printf("Please enter the strategy type (0,1,2,3): ");
        scanf("%d", &strategy);
        printf("\n");

        void* ptr = MyMalloc(400, strategy);

        DumpFreeList();

        return 0;
    } else {
        // Parent process
        wait(NULL);
    }

    // Free all the memory
    munmap(mem_init, memsize);

    return 0;
}