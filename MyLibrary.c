// Hakan Duran 150200091

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <bits/mman-linux.h>
#include <sys/wait.h>
#include <string.h>

#include "main.h"

void* mem_init = NULL; // The start address of memory
static int count = 0; // The count for InitMyMalloc

int InitMyMalloc (int HeapSize){

    if (count != 0){
        return -1;
    }
    count++;

    if (HeapSize <= 0){
        return -1;
    }

    // mmap() returns a pointer to a chunk of free space.
    // MAP_SHARED used for processes to share the space
    mem_init = mmap(NULL, HeapSize, PROT_READ|PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (mem_init == MAP_FAILED) {
        perror("mmap");
        return -1;
    }

    // The first sizeof(void*) is reversed for pointer of head
    void* head = mem_init + sizeof(void*);

    // Each header consists of
    // Size (int)
    // Next (void*)
    // Prev (void*)

    int size = HeapSize - ( sizeof(int) + sizeof(void*) + sizeof(void*)) - sizeof(void*);
    void* next = NULL;
    void* prev = NULL;

    // Let's load our first header into memory
    memcpy(head, &size, sizeof(int));
    memcpy(head + sizeof(int), &next, sizeof(void*));
    memcpy(head + sizeof(int) + sizeof(void*), &prev, sizeof(void*));

    // The first reversed area is for free list's head
    memcpy(mem_init, &head, sizeof(void*));

    return 0;
}

void *MyMalloc (int size, int strategy){

    // selected is pointer which will represent allocated space
    void* selected = NULL;

    // void* iter = head;
    void* head;
    void* iter;
    memcpy(&iter, mem_init, sizeof(void*));
    memcpy(&head, mem_init, sizeof(void*));
    

    if (strategy == 0){ // Best-fit
        
        int bestsize;
        int itersize;
        memcpy(&bestsize, iter, sizeof(int));
        memcpy(&itersize, iter, sizeof(int));

        while (iter){
            memcpy(&itersize, iter, sizeof(int));

            if (itersize >= size && itersize <= bestsize){
                selected = iter;
                bestsize = itersize;
            }

            memcpy(&iter, iter + sizeof(int), sizeof(void*));

        }

    }

    else if (strategy == 1){ // Worst-fit
        
        int bestsize;
        int itersize;
        memcpy(&bestsize, iter, sizeof(int));
        memcpy(&itersize, iter, sizeof(int));

        while (iter){
            memcpy(&itersize, iter, sizeof(int));

            if (itersize >= size && itersize >= bestsize){
                selected = iter;
                bestsize = itersize;
            }
            
            memcpy(&iter, iter + sizeof(int), sizeof(void*));

        }
        
    }

    else if (strategy == 2){ // First-fit
        
        int bestsize;
        int itersize;
        memcpy(&bestsize, iter, sizeof(int));
        memcpy(&itersize, iter, sizeof(int));

        while (iter){

            memcpy(&itersize, iter, sizeof(int));

            if (itersize >= size){
                selected = iter;
                bestsize = itersize;
                break;
            }
            
            memcpy(&iter, iter + sizeof(int), sizeof(void*));

        }
        
    }

    else if (strategy == 3){ // Next-fit
        static int i = 0;
        
        int bestsize;
        int itersize;
        memcpy(&bestsize, iter, sizeof(int));
        memcpy(&itersize, iter, sizeof(int));

        for(int j=0;j<i;j++){
            
            memcpy(&iter, iter + sizeof(int), sizeof(void*));

        }

        while (iter){
            i++;
            memcpy(&itersize, iter, sizeof(int));

            if (itersize >= size){
                selected = iter;
                bestsize = itersize;
                break;
            }
            
            memcpy(&iter, iter + sizeof(int), sizeof(void*));

            
        }

        if (!iter){
            i = 0;
        }

    }

    else {
        printf("Strategy type: %d, FAILED\n", strategy);
        exit(EXIT_FAILURE); 
    }

    /* The are which we temporarily store selected's values */
    int t_selectedsize;
    memcpy(&t_selectedsize, selected, sizeof(int));
    if (t_selectedsize < size || selected == NULL){
        printf("Strategy type: %d, FAILED: Not enough space in memory\n\n", strategy);
        return NULL;
    }

    void* t_selectednext;
    memcpy(&t_selectednext, selected + sizeof(int), sizeof(void*));

    void* t_selectedprev;
    memcpy(&t_selectedprev, selected + sizeof(int) + sizeof(void*), sizeof(void*));

    /* The are for splitting is happening here */
    // selected : Allocated area
    // newptr : New free area

    void* nullptr = NULL;
    memcpy(selected, &size, sizeof(int));
    memcpy(selected + sizeof(int), &nullptr, sizeof(void*));
    memcpy(selected + sizeof(int) + sizeof(void*), &nullptr, sizeof(void*));

    void* newptr = selected + size + sizeof(int) + sizeof(void*) + sizeof(void*);

    int newsize = t_selectedsize - size - (int) sizeof(int) - (int) sizeof(void*) - (int) sizeof(void*);
    memcpy(newptr, &newsize, sizeof(int));
    memcpy(newptr + sizeof(int), &t_selectednext, sizeof(void*));
    memcpy(newptr + sizeof(int) + sizeof(void*), &t_selectedprev, sizeof(void*));

    // By using select pointer's prev pointer, we reach previous header and then change it's next value. 
    if (t_selectedprev){
        memcpy(t_selectedprev + sizeof(int), &newptr, sizeof(void*));
    }

    // In a case where head's header changes its place
    if (head == selected){
        memcpy(mem_init, &newptr ,sizeof(void*));
    }

    printf("Strategy type: %d, SUCCESS, %p\n\n", strategy, selected);

    return selected;
}

int MyFree (void *ptr){

    if (ptr == NULL){ // It returns 0 if ptr is NULL
        return 0;
    }

    // The freed space is added in free-list and became head.
    // oldhead = head;
    // ptr->next = head;
    // ptr->prev = NULL;
    // head = ptr;
    // oldhead->prev = head;

    void* nullptr = NULL;
    void* oldhead;
    memcpy(&oldhead, mem_init, sizeof(void*));

    memcpy(ptr + sizeof(int), mem_init, sizeof(void*));
    memcpy(ptr + sizeof(int) + sizeof(void*), &nullptr, sizeof(void*));

    memcpy(mem_init, &ptr, sizeof(void*));
    memcpy(oldhead + sizeof(int) + sizeof(void*), mem_init, sizeof(void*));


    void* head;
    memcpy(&head, mem_init, sizeof(void*));
    

    /* COALESCION */

    void* iter = NULL;
    void* old_iter = head;

    int headsize;
    memcpy(&headsize, head, sizeof(int));

    void* headnext;
    memcpy(&headnext, head + sizeof(int) ,sizeof(void*));
    if(headnext){
        iter = headnext;
    }

    // start_ptr and end_ptr are head's start and end points.
    // Since the new freed space is now head, it is enough to check
    // head's previous and next areas.

    void* start_ptr = head;
    void* end_ptr = head + headsize + sizeof(int) + 2*sizeof(void*);

    while(iter){
        int itersize;
        memcpy(&itersize, iter , sizeof(int));

        // i_start_ptr and i_end_ptr are areas where iter's start and end points.
        void* i_start_ptr = iter;
        void* i_end_ptr = iter + itersize + sizeof(int) + 2*sizeof(void*);

        if (i_start_ptr == end_ptr){

            // If iter's start is same with head's end:

            memcpy(old_iter + sizeof(int), iter + sizeof(int) , sizeof(void*));
            int sum = headsize + itersize + sizeof(int) + 2*sizeof(void*);
            memcpy(head , &sum, sizeof(int));
        }

        if (i_end_ptr == start_ptr){

            // If iter's end is same with head's start:

            memcpy(old_iter + sizeof(int), iter + sizeof(int) , sizeof(void*));
            memcpy(&iter, &head, sizeof(void*));
            int sum = headsize + itersize + sizeof(int) + 2*sizeof(void*);
            memcpy(head , &sum, sizeof(int));
        }

        old_iter = iter;
        memcpy(&iter, iter + sizeof(int), sizeof(void*));

    }

    // while(iter){
    //     void* i_start_ptr = iter->ptr;
    //     void* i_end_ptr = iter->ptr + iter->size + sizeof(struct header);

    //     if (i_start_ptr == end_ptr){
    //         old_iter->next = iter->next;
    //         head->size += sizeof(struct header) + iter->size; 
    //     }

    //     if (i_end_ptr == start_ptr){

    //         old_iter->next = iter->next;
    //         head->ptr = iter->ptr;
    //         head->size += sizeof(struct header) + iter->size;

    //     }

    //     old_iter = iter;
    //     iter = iter->next;
    // }

}

void DumpFreeList(){

    printf("Addr\tSize\tStatus\n");
    printf("0\t8\tReserved for head\n");

    void* iter;
    void* head;
    memcpy(&iter, mem_init, sizeof(void*));
    memcpy(&head, mem_init, sizeof(void*));
    // void* iter = head;

    int headsize;
    memcpy(&headsize, head, sizeof(int));
    headsize += sizeof(int) + sizeof(void*) + sizeof(void*);

    if (head-mem_init){
        printf("8\t%ld\tFull\n", head - mem_init - sizeof(void*));
        printf("%ld\t%d\tEmpty\n", head - mem_init, headsize);
    }

    else{
        printf("8\t%d\tEmpty\n", headsize);
    }

    void* iternext = NULL;
    void* old_iter = NULL;
    void* ptr_full = NULL;

    memcpy(&iternext, iter + sizeof(int) ,sizeof(void*));

    while(iternext){
        old_iter = iter;
        memcpy(&iter, iter + sizeof(int) ,sizeof(void*));
        if (!iter){
            break;
        }

        int old_iter_size;
        memcpy(&old_iter_size, old_iter, sizeof(int));

        ptr_full = old_iter + old_iter_size + sizeof(int) + 2* sizeof(void*);

        int iter_size;
        memcpy(&iter_size, iter, sizeof(int));

        printf("%ld\t%ld\tFull\n", ptr_full - mem_init, iter - ptr_full);
        printf("%ld\t%ld\tEmpty\n", iter - mem_init, iter_size + sizeof(int) + 2*sizeof(void*));
    }

    printf("\n");
}