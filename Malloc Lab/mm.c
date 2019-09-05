/* Danning Yu
   CS 33 Discussion 1A
   Method: explicit free list
   Malloc will search through list of free nodes and find the best fit, splitting a block if necessary
   Free will free the block and then coalesce it with any neighboring blocks
   Realloc will either split block if reallocating for a smaller size, or malloc a new block if new size > old size.
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>
#include "mm.h"
#include "memlib.h"

 /*********************************************************
  * NOTE TO STUDENTS: Before you do anything else, please
  * provide your information in the following struct.
  ********************************************************/
team_t team = {
	/* Your UID */
	"",
	/* Your full name */
	"Danning Yu",
	/* Your email address */
	"",
	/* Leave blank */
	"",
	/* Leave blank */
	""
};

//macros
#define WSIZE 4       // Word and header/footer size (bytes)
#define DSIZE 8       // Double word size (bytes)

#define HEADER(bp) ((size_t*)((bp)-WSIZE)) //bp is pointer to payload
#define FOOTER(bp) ( (size_t*)( (bp)+PAYLOADSIZE(bp)) ) //bp is pointer to payload

//to get and manipulate sizes and if a block is allocated
#define PAYLOADSIZE(bp) (*(size_t*)((bp) - WSIZE) & ~0x7) //bp is pointer to payload, uses head to get size
#define PAYLOADSIZEHF(ptr) (*(size_t*)(ptr) & ~0x7) //get size from header or footer
#define ROUNDUPTOEIGHT(value) (((value) + 7) & (~0x7)) //round up to highest 8
#define ISALLOCATED(bp) (*HEADER(bp) & 0x7) //if allocated or not

//to get and set various locations in memory
#define NEXTPTR(bp) (*(void **)(bp)) //next value in free list if node is free
#define PREVPTR(bp) (*(void **)((bp) + WSIZE)) //previous value in free list if node is free
#define NEXTHEADER(bp) ((size_t*)((bp) + PAYLOADSIZE(bp) + WSIZE)) //header of next contiguous block
#define PREVFOOTER(bp) ((size_t*)((bp) - WSIZE - WSIZE)) //footer of previous contiguous block
#define PAYLOADFROMHEADER(headPtr) ((char*)(headPtr) + WSIZE) //given head, compute start of payload
#define PAYLOADFROMFOOTER(footPtr) ((char*)(footPtr) - PAYLOADSIZEHF(footPtr)) //given foot, compute start of payload
#define SETPTRVAL(ptr, val) (*(ptr) = (val)) //set value pointed to by ptr to val

// Global variables
static char *heapStart = 0;  // Pointer to first block
static char *freeList = 0; //pointer to first free block
static int freeListSize = 0; //# of blocks in free list

// Function prototypes for internal helper routines
static void place(void *bp, size_t asize, int inFreeList);
static void *coalesce(void *bp);
static void printBlock(void *bp);
static void removeBlock(void *bp);
static void insertBlock(void *bp);
static void *extend_heap(size_t size);
int mm_check(void);
int checkBlock(void *bp);

int mm_init(void) {	//initialize the heap with prologue and epilogue blocks
	if ((heapStart = mem_sbrk(4 * WSIZE)) == (void *)-1) //if failure occurs
		return -1;
	SETPTRVAL(heapStart, 0);                          // Alignment padding
	SETPTRVAL(heapStart + (1 * WSIZE), (WSIZE | 1)); //Prologue header
	SETPTRVAL(heapStart + (2 * WSIZE), (WSIZE | 1)); //Prologue footer
	SETPTRVAL(heapStart + (3 * WSIZE), 1);     // Epilogue header
	heapStart += (2 * WSIZE); //for the heap checker function
	freeList = NULL;
	freeListSize = 0;
	return 0;
}

//MALLOC, FREE, REALLOC, AND MM_CHECK
void *mm_malloc(size_t size) {
	size_t asize = size;      //adjusted block size
	if (mem_heapsize() == 0) {
		mm_init();
	}
	if (size == 0) {
		return NULL;
	}
	asize = ROUNDUPTOEIGHT(asize); //round to nearest multiple of 8
	if (asize <= 2 * WSIZE) { //8+8 for the two ptrs
		asize = 2 * WSIZE; //asize must be at least 16 (for 2 pts)
	}

	//best fit search algorithm
	char* temp = 0;
	char* bestPtr = 0;
	int counter = 0;
	size_t bestSize = 0;
	int initial = 0;
	for (temp = freeList; temp != NULL && counter <=1000; temp = NEXTPTR(temp)) {
		if (initial && PAYLOADSIZE(temp) >=asize && PAYLOADSIZE(temp) <= bestSize) {
			bestPtr = temp;
			bestSize = PAYLOADSIZE(temp);
		}
		else if (PAYLOADSIZE(temp) >= asize) { //compare sizes...
			initial = 1;
			bestPtr = temp;
			bestSize = PAYLOADSIZE(temp);
		}
		counter++;
	}
	temp = bestPtr;

	//CASE 1: FIRST FIND MEMORY AMONG THE FREE LIST	
	if (temp != NULL && counter <= 1000) {
		place(temp, asize, 1); //place assumes sufficient space has been found for asize+16	
		return temp;
	}

	//CASE 2: NO SUITABLE FREE BLOCK FOUND, SO ALLOCATE NEW MEMORY
	char* bp = extend_heap(asize + 2 * WSIZE); //+16 for footer and epilogue	
	if ((long)bp == -1 || bp == NULL) {
		return NULL;
	}
	place(bp, asize, 0); //initialize the various elements - that's all place does
	return bp;
}

void mm_free(void *bp) {
	if (bp == 0) { return; }
	SETPTRVAL(HEADER(bp), PAYLOADSIZE(bp)); //clear out header and footer
	SETPTRVAL(FOOTER(bp), PAYLOADSIZE(bp));
	insertBlock(bp); //insert curr node into free list
	bp = coalesce(bp); //then coalesce it
}

void *mm_realloc(void *ptr, size_t size) {
	if (size == 0) {
		mm_free(ptr);
		return 0;
	}

	//if ptr == null, then treat as if it's malloc(size)
	if (ptr == NULL) {
		return mm_malloc(size);
	}

	size_t oldBlockSize = PAYLOADSIZE(ptr);
	size_t newBlockSize = ROUNDUPTOEIGHT(size); //round to nearest multiple of 8
	if (newBlockSize <= 2 * WSIZE) { //8+8 for the two ptrs
		newBlockSize = 2 * WSIZE; //asize must be at least 16 (for 2 pts)
	}

	if (oldBlockSize == newBlockSize) {
		//case 1: same size
		return ptr; //no changes needed
	}
	else if (newBlockSize < oldBlockSize) {
		//case 2: smaller, so split the block
		if (oldBlockSize - newBlockSize <= 32) { //need space for overhead
			return ptr;
		}
		SETPTRVAL(HEADER(ptr), (newBlockSize | 1));
		SETPTRVAL(FOOTER(ptr), (newBlockSize | 1));
		char* blockToFree = (char*)(ptr)+newBlockSize + 2 * WSIZE;
		SETPTRVAL(HEADER(blockToFree), (oldBlockSize - newBlockSize - 2 * WSIZE));
		SETPTRVAL(FOOTER(blockToFree), (oldBlockSize - newBlockSize - 2 * WSIZE));
		mm_free(blockToFree);
		return ptr;
	}

	//case 3: newsize is begger than old size, so just make a new block	
	char* newBlock = mm_malloc(size);
	if (newBlock == NULL) {
		return 0;
	}
	if (size < oldBlockSize) {
		oldBlockSize = size; //if only part of the data needs to be copied
	}
	memcpy(newBlock, ptr, oldBlockSize);
	mm_free(ptr); //first free to that coalescing might occur
	return newBlock;
}

int mm_check(void) { 
	//heap checker that loops through all blocks and checks alignment, header/footer, and prints info about the block
	char* tempPtr = heapStart;
	char* heapEnd = mem_heap_hi();
	int numOfFreeBlocks = 0;
	//loop through all blocks and check them
	int blocksOkay = 1;
	for (tempPtr = heapStart; tempPtr != heapEnd; tempPtr = tempPtr + PAYLOADSIZE(tempPtr)) {
		if (!checkBlock(tempPtr)) {
			blocksOkay = 0;
		}
		if (!ISALLOCATED(tempPtr)) {
			numOfFreeBlocks++;
		}
	}
	return blocksOkay && (numOfFreeBlocks == freeListSize); //check if freeListSize is correct
}

//HELPER FUNCTIONS
static void *extend_heap(size_t size) { //size is how much to extend by and not payload size
	char *bp;
	if ((long)(bp = mem_sbrk(size)) == -1)
		return NULL;
	SETPTRVAL(HEADER(bp), size - 2 * WSIZE);
	SETPTRVAL((char*)(bp)+size - 2 * WSIZE, size - 2 * WSIZE);
	SETPTRVAL(bp + size - WSIZE, 1);
	return bp;
}

static void *coalesce(void *bp) {
	size_t* prevFooter = PREVFOOTER(bp); //footer of prev = bp - 8 (header) - 8 (footer of prev)
	size_t* nextHeader = NEXTHEADER(bp); //header of next = bp + payload + 8 (footer)
	size_t prevAlloc = (*prevFooter) & 0x7; //if allocated or not
	size_t nextAlloc = (*nextHeader) & 0x7;
	size_t payloadSize = PAYLOADSIZE(bp);
	
	if (freeListSize == 1 || (prevAlloc && nextAlloc)) {
		//case 0: top and bottom allocated, no need to do anything
		return bp;
	}
	else if (!prevAlloc && !nextAlloc && PAYLOADSIZEHF(prevFooter) != 0 && PAYLOADSIZEHF(nextHeader) != 0) {
		//case 1: both before and after free
		payloadSize = payloadSize + 4*WSIZE + PAYLOADSIZEHF(prevFooter) + PAYLOADSIZEHF(nextHeader); //twice as many pointers, so +32
		removeBlock(bp);
		removeBlock(PAYLOADFROMHEADER(nextHeader));
		size_t* bpHeader = (size_t*)((char*)PREVFOOTER(bp) - PAYLOADSIZEHF(prevFooter) - WSIZE);
		SETPTRVAL(bpHeader, payloadSize);
		size_t* bpFooter = (size_t*)((char*)bpHeader + payloadSize + WSIZE);
		SETPTRVAL(bpFooter, payloadSize);
		bp = (char*)(bpHeader)+WSIZE; //fix bp
	}
	else if (!nextAlloc && prevAlloc && PAYLOADSIZEHF(nextHeader) != 0) {
		//case 2: prev allocated, but next free
		payloadSize = payloadSize + 2*WSIZE + PAYLOADSIZEHF(nextHeader); //combine sizes together, plus 8+8 for two ptrs eliminated
		removeBlock(PAYLOADFROMHEADER(nextHeader));
		SETPTRVAL(HEADER(bp), payloadSize);
		SETPTRVAL(FOOTER(bp), payloadSize); //must keep in this order, header and then footer
		//  printBlock(bp);
	}
	else if (!prevAlloc && PAYLOADSIZEHF(prevFooter) != 0) {
		//case 3: prev free, next allocated
		payloadSize = payloadSize + PAYLOADSIZEHF(prevFooter) + 2*WSIZE; //calculate new size
		removeBlock(bp); //remove current block
		size_t* bpHeader = (size_t*)((char*)PREVFOOTER(bp) - PAYLOADSIZEHF(prevFooter) - WSIZE);
		SETPTRVAL(bpHeader, payloadSize);
		size_t* bpFooter = (size_t*)((char*)bpHeader + payloadSize + WSIZE);
		SETPTRVAL(bpFooter, payloadSize);
	}
	return bp;
}

static void place(void *bp, size_t asize, int inFreeList) {
	//case 1: check first if allocating from another block and can split
	if (inFreeList && (PAYLOADSIZE(bp) - asize >= 4*WSIZE)) { //to split: min size left must be 32: header + ptr + ptr + footer
		size_t blockSize = PAYLOADSIZE(bp);
		removeBlock(bp); //cut out the entire block
		SETPTRVAL(HEADER(bp), (asize | 1));
		SETPTRVAL(FOOTER(bp), (asize | 1));
		bp = (char*)bp + asize + 2*WSIZE; //bp + size, +8 (footer) +8 (header for next block)
		SETPTRVAL(HEADER(bp), blockSize - asize - 2 * WSIZE);
		SETPTRVAL(FOOTER(bp), blockSize - asize - 2 * WSIZE);
		insertBlock(bp); //insert the new, smaller block
		return;
	}
	//case 2: allocating from another block but no split
	else if (inFreeList) {
		SETPTRVAL(HEADER(bp), (PAYLOADSIZE(bp) | 1));
		SETPTRVAL(FOOTER(bp), (PAYLOADSIZE(bp) | 1));
		removeBlock(bp); //need to remove node from freeList
		return;
	}
	else {
		//case 3: allocating new memory
		SETPTRVAL(HEADER(bp), (asize | 1));
		SETPTRVAL(FOOTER(bp), (asize | 1));
	}
}

static void insertBlock(void *bp) { //inserts block at start
	NEXTPTR(bp) = freeList; //bp (next) points to whatever freeList is pointing at
	PREVPTR(bp) = NULL; //bp's previous is null (first item in list): bp->prev = NULL
	if (freeListSize != 0) { //if the list isn't empty
		PREVPTR(freeList) = bp;
	}	
	freeList = bp; //sentinel points to bp: freelist = bp
	freeListSize++;
}

static void removeBlock(void *bp) {
	//this just redirects pointers as needed
	if (bp == NULL) {
		return;
	}
	if (freeListSize == 1) { //CASE 0: only one node
		freeList = NULL;
	}
	else if (NEXTPTR(bp) == NULL) {//CASE 3: end block (next = null)
		NEXTPTR(PREVPTR(bp)) = NULL;
	}
	else if (PREVPTR(bp) == NULL) {//CASE 1: first block (prev = null)	
		freeList = NEXTPTR(bp);
		PREVPTR(NEXTPTR(bp)) = NULL;
	}
	else {//CASE 2: middle block
		NEXTPTR(PREVPTR(bp)) = NEXTPTR(bp);
		PREVPTR(NEXTPTR(bp)) = PREVPTR(bp);
	}
	freeListSize--;
}

static void printBlock(void *bp) {	
	if (ISALLOCATED(bp) == 1) { //allocated case
		printf("\tprinting allocated node\n");
		printf("\tallocated node, starting at 0x%x ", (unsigned int)HEADER(bp)); //address of start of node
		printf("\theader raw: 0x%x ", *(size_t*)HEADER(bp)); //raw header, LSB should be 1
		printf("\tPayload starts at: 0x%x ", (unsigned int)bp); //start of payload
		printf("\tpayload size: %d ", PAYLOADSIZE(bp)); //prints size after rounding, for block size, add 16
		printf("\tblock size: %d", PAYLOADSIZE(bp) + 2*WSIZE);
		printf("\tfooter address: 0x%x", (unsigned int)FOOTER(bp));
		printf("\tfooter raw: 0x%x\n", *(size_t*)FOOTER(bp));
	}
	else { //free case
		printf("\tprinting free node\n");
		printf("\theader raw: 0x%x ", *(size_t*)HEADER(bp)); //raw header, LSB should be 0
		printf("\tPayload starts at: 0x%x ", (unsigned int)bp); //start of payload
		printf("\tpayload size: %d ", PAYLOADSIZE(bp)); //prints size after rounding, for block size, add 16		
		printf("\tblock size: %d ", PAYLOADSIZE(bp) + 2*WSIZE);
		printf("\tnext ptr: 0x%x ", *(size_t*)NEXTPTR(bp));
		printf("\tprev ptr: 0x%x", *(size_t*)PREVPTR(bp)); //prev should be a next previously printed
		printf("\tfooter address: 0x%x", (unsigned int)FOOTER(bp));
		printf("\tfooter raw: 0x%x\n", *(size_t*)FOOTER(bp));
	}
}

int checkBlock(void *bp){
	int aligned = !((size_t)bp & ~0x7); //check if multiple of eight
	int headerFooterSame = *(size_t*)HEADER(bp) == *(size_t*)FOOTER(bp);
	int minimumSize = (PAYLOADSIZE(bp) + 8) >= 24; //size should be at least 24 (header (4) + next (8) + prev (8) + 4 (footer))
	printBlock(bp); //print diagnostic information about the block
	return aligned && headerFooterSame && minimumSize;

}
