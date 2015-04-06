#ifndef	_memory_h_
#define	_memory_h_

// Put all your #define's in memory_constants.h
#include "memory_constants.h"

extern int lastosaddress; // Defined in an assembly file
extern int pagemap[512];
extern int heap[128];

//--------------------------------------------------------
// Existing function prototypes:
//--------------------------------------------------------

int MemoryGetSize();
void MemoryModuleInit();
uint32 MemoryTranslateUserToSystem (PCB *pcb, uint32 addr);
int MemoryMoveBetweenSpaces (PCB *pcb, unsigned char *system, unsigned char *user, int n, int dir);
int MemoryCopySystemToUser (PCB *pcb, unsigned char *from, unsigned char *to, int n);
int MemoryCopyUserToSystem (PCB *pcb, unsigned char *from, unsigned char *to, int n);
int MemoryPageFaultHandler(PCB *pcb);

//---------------------------------------------------------
// Put your function prototypes here
//---------------------------------------------------------
// All function prototypes including the malloc and mfree functions go here
int MemoryAllocPage(void);
uint32 MemorySetupPte(uint32 page);
void MemoryFreePage(uint32 page);
void PrintPagemap();

int malloc(PCB *pcb, int memsize );
int mfree(PCB *pcb, void *ptr);

inline int pow(int base, int exp){
    if(exp == 0){
        return 1;
    }else if (exp % 2) {
        return base * pow(base, exp -1);
    }else{
        int temp = pow(base, exp /2 );
        return temp * temp;
    }

}


#endif	// _memory_h_
