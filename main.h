//Hakan Duran 150200091

// Size of header is 20 byte, int+void*+void*
// In my memory, the first 8 byte is reversed for head.

#ifndef HEADERFILE_H
#define HEADERFILE_H

int InitMyMalloc (int HeapSize);
void *MyMalloc (int size, int strategy);
int MyFree (void *ptr);
void DumpFreeList();

#endif
