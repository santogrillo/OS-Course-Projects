#include <stdio.h>  // needed for size_t
#include <unistd.h> // needed for sbrk
#include <assert.h> // needed for asserts
#include "dmm.h"

/* You can improve the below metadata structure using the concepts from Bryant
 * and OHallaron book (chapter 9).
 */



/*
Block Structure:
|-----------------------------------------------|
| metadata_t {									|
|       size allocated							|
|       bool free								|
|       next* / prev*  only used when free }	|
|-----------------------------------------------|
|												|
|												|
~				data (size)						~
|												|
|												|
|-----------------------------------------------|
|  metadata_b {									|
|		size, bool free}						|
|-----------------------------------------------|

Assumptions I've been making so far:
Pointers to next/previous only used if the block is free, and in the free list.
freelist will always point to the head of the free block list.
free block list will be sorted in ascending order based on size. (need to implement some sort of simple sort/insert method)
Coalesce each block when freed, will be able to to pointer arithmetic to check the header/footer of the next/previous chunks respectively

*/


typedef struct metadata {
  /* size_t is the return type of the sizeof operator. Since the size of an
   * object depends on the architecture and its implementation, size_t is used
   * to represent the maximum size of any object in the particular
   * implementation. size contains the size of the data object or the number of
   * free bytes
   */
  size_t size;
  bool free;
  struct metadata* next;
  struct metadata* prev;
} metadata_t;

typedef struct footer {
	size_t size;
	bool free;
} metadata_b;

/* freelist maintains all the blocks which are not in use; freelist is kept
 * sorted to improve coalescing efficiency 
 */

static metadata_t* freelist = NULL;

/* Header and footer pointers to contain the entire data structure */
static metadata_t* head = NULL;
static metadata_t* tail = NULL;

void* find_footer(metadata_t* chunk);
void* split(metadata_t* free_chunk, size_t numbytes);
bool coalesceforward(metadata_t* chunk);
bool coalesceback(metadata_t* chunk, bool remove);
void insertFreeChunk(metadata_t* chunk);
void removeFreeChunk(metadata_t* chunk);
void* nextPhysChunk(metadata_t* chunk);
bool ptrIsValid(void* ptr);


void* dmalloc(size_t numbytes) {
  /* initialize through sbrk call first time */
  if(freelist == NULL) { 			
    if(!dmalloc_init())
      return NULL;
  }

  assert(numbytes > 0);
  numbytes = ALIGN(numbytes);
  /* your code here */

  //return (void*)freelist;
  /* assuming that freelist is sorted in increasing order, 
    find the smallest available block of memory
	and split to accomodate */
  metadata_t* free = freelist; //set pointer to beginning of free chunk list
  while (free != tail) // iterate over free chunks until large enough chunk is found
  {
	  if (numbytes <= free->size)	// if the chunk is large enough to accomodate requested space,
	  {								// split and return the new chunk
		  return split(free, numbytes);
	  }
	  free = free->next;			//chunk wasn't large enough, advance to the next largest one
  }

  // didn't find a large enough chunk, return failure :(
  return NULL;
}


/* splits a large enough chunk from the given free chunk to accomodate numbytes space
   returns a void* pointer to the newly created chunk
   */
void* split(metadata_t* free_chunk, size_t numbytes)
{
	// check that free_chunk is valid
	if (free_chunk != NULL)
	{
		//align the requested byte size
		numbytes = ALIGN(numbytes);

		// housekeeping to maintain free chunk list
		// remove the free chunk from the free list, if there is extra space it will be re-added later

		removeFreeChunk(free_chunk);

		free_chunk->prev = NULL;
		free_chunk->next = NULL;

		//set header free flag to false
		free_chunk->free = false;

		//check if free chunk is perfectly sized, if so no need to split
		if (free_chunk->size == numbytes)
		{
			//dont need to split, update footer and return
			metadata_b* foot = (metadata_b*)find_footer(free_chunk);
			if(foot !=NULL) foot->free = false;
			//@TODO CHECK MY POINTER MATH NOT SURE THIS GIVES THE RIGHT THING
			printf("Returned: %p\n", free_chunk);
			return (void*)free_chunk + METADATA_T_ALIGNED;
		}

		// if the free chunk is larger than the requested chunk, split off only what is needed
		// | Header | <-------------------free space-------------------> | Footer|           current
		// | Header | <--allocated space--> | Footer | Header | <-free-> | Footer|			 new

		//@TODO CHECK MY POINTER MATH PLEASE
		metadata_t* newchunk = ((void*)free_chunk) + METADATA_T_ALIGNED + numbytes + ALIGN(sizeof(metadata_b));

		//set up new free chunk
		newchunk->free = true;
		newchunk->size = free_chunk->size - METADATA_T_ALIGNED - ALIGN(sizeof(metadata_b)) - numbytes;
		metadata_b* foot = (metadata_b*)find_footer(newchunk);

		if (foot != NULL)
		{
			foot->size = free_chunk->size - METADATA_T_ALIGNED - ALIGN(sizeof(metadata_b)) - numbytes;
			
			foot->free = true;
		}
		
		//free_chunk now holds the allocated space
		free_chunk->size = numbytes;
		foot = (metadata_b*)find_footer(free_chunk);
		if (foot != NULL)
		{
			foot->free = false;
			foot->size = numbytes;
		}

		

		//sort new free chunk into the list
		insertFreeChunk(newchunk);
		//@TODO CHECK MY POINTER MATH NOT SURE THIS GIVES THE RIGHT THING
		printf("Returned: %p\n", free_chunk);
		return ((void*)free_chunk) + METADATA_T_ALIGNED;
	}
	printf("returned null\n");
	return NULL;
}

//returns a pointer to the footer of the block, located right after the data section
//@TODO CHECK MY POINTER MATH NOT SURE THIS IS RIGHT
void* find_footer(metadata_t* chunk)
{
	if (chunk != NULL) { return ((void*)chunk) + METADATA_T_ALIGNED + chunk->size; }

	return NULL;
}


void dfree(void* ptr) {
	if (ptrIsValid(ptr - METADATA_T_ALIGNED) == true)
	{
		metadata_t* chunk = (metadata_t*)(ptr - METADATA_T_ALIGNED);
		metadata_b* foot = (metadata_b*)find_footer(chunk);
		chunk->free = true;
		foot->free = true;

		bool forward = coalesceforward(chunk);
		bool back = coalesceback(chunk, forward);
		if (forward == false && back == false)
		{
			insertFreeChunk(chunk);
		}
	}
}

bool coalesceforward(metadata_t* chunk)
{
	if (chunk != NULL) //check to make sure the passed chunk is valid
	{
		metadata_t* nextChunk = (metadata_t*)nextPhysChunk(chunk);	//get pointer to the next chunk
		if(nextChunk != tail)
		{
			if (nextChunk->free == true)
			{
				/*
					current:| Header | <---free---> | Footer | Header | <--free--> | Footer |
					new:	| Header | <-------------------free------------------> | Footer |
				*/
				chunk->size = chunk->size + nextChunk->size + METADATA_T_ALIGNED + ALIGN(sizeof(metadata_b));
				metadata_b* foot = (metadata_b*)find_footer(chunk);		//resets the foot to the new end
				foot->free = true;
				foot->size = chunk->size;					//footer size updated

				//remove next chunk from free list and re-sort in the new one
				removeFreeChunk(nextChunk);

				//insert newly coalesced chunk into the list
				insertFreeChunk(chunk);
				return true;
			}
		}
	}
	return false;
}

bool coalesceback(metadata_t* chunk, bool remove)
{
	if (chunk != NULL) //check to make sure the passed chunk is valid
	{
		if (chunk != head) //ensure that there is a previous block
		{
			metadata_b* foot = ((void*)chunk) - ALIGN(sizeof(metadata_b));
			
			if (foot->free == true)
			{
				metadata_t* prevChunk = ((void*)foot) - foot->size - METADATA_T_ALIGNED;	//get pointer to the previous chunk
				/*
					| prevChunk | <-----free space----> |prev foot| chunk | <---free space---> | chunk foot |
					| prevChunk | <------------------------free space------------------------> |  new foot  |
				*/
				prevChunk->size = chunk->size + prevChunk->size + METADATA_T_ALIGNED + ALIGN(sizeof(metadata_b));
				foot = (metadata_b*)find_footer(prevChunk);		//resets the foot to the new end
				foot->free = true;
				foot->size = prevChunk->size;

				//remove next chunk from free list and re-sort in the new one
				if (remove == true) { removeFreeChunk(chunk); }
				removeFreeChunk(prevChunk);
				

				//insert newly coalesced chunk into the list
				insertFreeChunk(prevChunk);
				return true;
			}
		}
	}
	return false;
}

void removeFreeChunk(metadata_t* chunk)
{
	if (chunk != NULL && chunk->free == true)
	{
		//remove next chunk from free list and re-sort in the new one
		chunk->prev->next = chunk->next;
		chunk->next->prev = chunk->prev;
		if (freelist == chunk)
		{
			freelist = chunk->next;
			freelist->prev = head;
			head->next = freelist;
		}
	}
}

void insertFreeChunk(metadata_t* chunk)
{
	//insert newly coalesced chunk into the list
	if (freelist != tail)
	{
		metadata_t* free = freelist; //set pointer to beginning of free chunk list
		while(free != tail) // iterate over free chunks until large enough chunk is found
		{
			//if the new chunk is smaller than the next in the list, insert it
			if (chunk->size <= free->size)
			{
				chunk->next = free;
				chunk->prev = free->prev;
				if (free == freelist) { freelist = chunk; }		//if inserted at the beginning, redirect free pointer to the new chunk
				if (free->prev != NULL) { free->prev->next = chunk; }
				free->prev = chunk;
				return;
			}
			free = free->next;
		} 
		// loop completed without finding a larger free chunk, add to end of list
		//free-> points to tail
		free = free->prev; //move free back 1 link
		chunk->prev = free;
		chunk->next = tail;
		free->next = chunk;
		tail->prev = chunk;
		return;
	}
	//freelist is empty, add the newly coalesced block
	freelist = chunk;
	freelist->next = tail;
	freelist->prev = head;
	tail->prev = freelist;
	head->next = freelist;

}


bool dmalloc_init() {

  /* Two choices: 
   * 1. Append prologue and epilogue blocks to the start and the
   * end of the freelist 
   *
   * 2. Initialize freelist pointers to NULL
   *
   * Note: We provide the code for 2. Using 1 will help you to tackle the 
   * corner cases succinctly.
   */

  size_t max_bytes = ALIGN(MAX_HEAP_SIZE);
  /* returns heap_region, which is initialized to freelist */
  freelist = (metadata_t*) sbrk(max_bytes); 
  /* Q: Why casting is used? i.e., why (void*)-1? */
  /* A: (void*)-1 is the result of a sbrk call that failed */
  if (freelist == (void *)-1)
    return false;  
  head = freelist;
  head->size = 0;
  freelist = (metadata_t*)nextPhysChunk(head);

  /* subtract the size of size_t to account for footer size */
  //freelist->size = max_bytes - METADATA_T_ALIGNED - ALIGN(sizeof(metadata_b));
  freelist->size = max_bytes - METADATA_T_ALIGNED * 3;  // | <-----------------max_bytes---------------> |
														// | Head | freelist | <--freelist size--> | tail|
  /* initialize the value of freelist to free */
  freelist->free = true;
  tail = (metadata_t*)nextPhysChunk(freelist);
  tail->free = false;
  tail->size = 0;
  tail->next = NULL;
  tail->prev = freelist;

  head->free = false;
  head->prev = NULL;
  head->next = freelist;


  freelist->next = tail;
  freelist->prev = head;

  /*
  printf("Initial setup:\n");
  print_physlist();
  print_freelist2();
  printf("\n");

  //debug prints
  printf("max_bytes size: %zd \n", max_bytes);
  printf("metadata_t size: %zd \n", METADATA_T_ALIGNED);
  printf("metadata_b size: %zd \n", ALIGN(sizeof(metadata_b)));
  printf("freelist initial size: %zd \n", freelist->size);
*/
  return true;
}

void* nextPhysChunk(metadata_t* chunk)
{
	return (void*)chunk + METADATA_T_ALIGNED + chunk->size + ALIGN(sizeof(metadata_b));
}

/* for debugging; can be turned off through -NDEBUG flag*/
void print_freelist() {
  metadata_t *freelist_head = freelist;
  while(freelist_head != NULL) {
    DEBUG("\tFreelist Size:%zd, Head:%p, Prev:%p, Next:%p\t",
	  freelist_head->size,
	  freelist_head,
	  freelist_head->prev,
	  freelist_head->next);
    freelist_head = freelist_head->next;
  }
  DEBUG("\n");
}

void print_freelist2()
{
	metadata_t *freelist_head = freelist;
	while (freelist_head != NULL) {
		printf("\tFreelist Size:%zd, Head:%p, Prev:%p, Next:%p\t",
			freelist_head->size,
			freelist_head,
			freelist_head->prev,
			freelist_head->next);
		printf("\n");
		freelist_head = freelist_head->next;
	}
}

void print_physlist()
{
	metadata_t* list_head = head;
	while (list_head != NULL) {
		printf("\tActual List: Size:%zd, Head:%p, Free?:%d\t",
			list_head->size,
			list_head,
			list_head->free);
		printf("\n");
		if (list_head != tail) { list_head = nextPhysChunk(list_head); }
		else break;
	}
}

bool ptrIsValid(void* ptr)
{
	metadata_t* list_head = nextPhysChunk(head);
	while (list_head != NULL && list_head != tail) {
		if (ptr == (void*)list_head) { return true; }
		if (list_head != tail) { list_head = nextPhysChunk(list_head); }
		else break;
	}
	return false;
}
