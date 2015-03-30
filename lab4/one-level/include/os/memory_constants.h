#ifndef	_memory_constants_h_
#define	_memory_constants_h_

//------------------------------------------------
// #define's that you are given:
//------------------------------------------------

// We can read this address in I/O space to figure out how much memory
// is available on the system.
#define	DLX_MEMSIZE_ADDRESS	0xffff0000

// Return values for success and failure of functions
#define MEM_SUCCESS 1
#define MEM_FAIL -1

//--------------------------------------------------------
// Put your constant definitions related to memory here.
// Be sure to prepend any constant names with "MEM_" so 
// that the grader knows they are defined in this file.
//--------------------------------------------------------

#define MEM_L1FIELD_FIRST_BITNUM    0xC
#define MAX_VIRTUAL_ADDRESS         0xFFFFF
#define MEM_MAX_SIZE                0x1FFFFF
#define MEM_PTE_READONLY            0x4
#define MEM_PTE_DIRTY               0x2
#define MEM_PTE_VALID               0x1


#define MEM_PAGESIZE                (0x1 << MEM_L1FIELD_FIRST_BITNUM)
#define MEM_ADDRESS_OFFSET_MASK     (MEM_PAGESIZE - 0x1)
#define MEM_PAGE_TABLE_SIZE         (MAX_VIRTUAL_ADDRESS + 1) >> MEM_L1FIELD_FIRST_BITNUM
#endif	// _memory_constants_h_
