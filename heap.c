#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>

#include "heap.h"

/* ------------------------------ */
/* MACROS */
/* ------------------------------ */

/**
 * \brief check if we are running on x86_64
 */
#ifndef __x86_64__
#error "heap is expected to run in 64-bit mode on x86_64"
#endif

/**
 * \brief node signature used for validating node
 */
#define HNODE_SIGN 0x55DE55AD55BE55EF
/**
 * \brief define cache line size for x86_64
 */
#define CACHE_LINE_SIZE 64
/**
 * \brief align data
 */
#define ALIGN(x, a) (((x) + (a) - 1) & ~((a) - 1))

/* ------------------------------ */
/* STRUCTURES */
/* ------------------------------ */

/**
 * \brief heap node structure
 */
typedef struct __hnode_t
{
	u64 sign;
	u64 used;
	u64 size;
	struct __hnode_t* prev;
	struct __hnode_t* next;
	u64 padding[3]; /* x86_64 line cache size is 64 bytes */
} hnode_t;

/**
 * \brief global structure for managing heap
 */
static struct
{
	u64      start;
	u64      init;
	hnode_t* first;
} __heap;

/* ------------------------------ */
/* PRIVATE FUNCTIONS */
/* ------------------------------ */

/**
 * \brief get current end of the data segment
 */
static u64 __dataseg_curr()
{
	return (u64)sbrk(0);
}
/**
 * \brief increase data segment end
 */
static u64 __dataseg_add(u64 size)
{
	return (u64)sbrk(size);
}
/**
 * \brief decrease data segment end
 */
static u64 __dataseg_sub(u64 size)
{
	//check heap
	if(__heap.init == false)
	{
		printf("__dataseg_sub() error: heap not initialized\n"); return (u64)-1;
	}

	//calculate new data segment end
	u64 dataseg_end = __dataseg_curr() - size;

	//check for underflow
	if(dataseg_end <= __heap.start)
	{
		printf("__dataseg_sub() error: heap underflow\n"); return (u64)-1;
	}

	//change data segment end address
	if((u64)brk((void*)dataseg_end) == (u64)-1)
	{
		printf("__dataseg_sub() error: brk failed\n"); return (u64)-1;
	}

	return dataseg_end;
}

/* ------------------------------ */
/* PUBLIC FUNCTIONS */
/* ------------------------------ */

/**
 * \brief place heap at the beggining of the data segment
 */
void  heap_init()
{
	if(__heap.init == true)
	{
		printf("heap_init() error: cannot double initialize heap\n"); return;
	}
	__heap.start = __dataseg_curr();
	__heap.first = NULL;
	__heap.init  = true;
}
/**
 * allocate new space in heap
 */
void* heap_alloc(u64 size)
{
	//check heap
	if(__heap.init == false)
	{
		printf("heap_alloc() error: heap not initialized\n"); return NULL;
	}

	//helper variables
	hnode_t* node;

	//align size to x86_64 cache size
	size = ALIGN(size, CACHE_LINE_SIZE);

	//heap is empty
	if(__heap.first == NULL)
	{
		//resize data segment
		if(__dataseg_add(size + sizeof(hnode_t)) == (u64)-1)
		{
			printf("heap_alloc() error: sbrk failed\n"); return NULL;
		}

		//write node to the heap start
		node        = (hnode_t*)__heap.start;
		node->sign  = HNODE_SIGN;
		node->used  = true;
		node->size  = size;
		node->prev  = NULL;
		node->next  = NULL;

		//modify heap
		__heap.first = node;

		//return address after the node
		return (void*)((u64)node + sizeof(hnode_t));
	}

	//find free node
	node = __heap.first;
	while(true)
	{
		//if the node is not used and it's size is big enough
		//occupy this node
		if(node->used == false && (size <= node->size))
		{
			//calculate leftover size
			u64 leftover_size = node->size - size;

			//if there is enough space, split the node into 2 nodes
			if(leftover_size >= (sizeof(hnode_t) * 4))
			{
				//calculate new node address
				hnode_t* new = (hnode_t*)((u64)node + sizeof(hnode_t) + size);

				//modify the new node
				new->size       = leftover_size - sizeof(hnode_t);
				node->sign      = HNODE_SIGN;
				new->used       = false;
				new->next       = node->next;
				if(new->next != NULL)
				{
					new->next->prev = new;
				}
				new->prev       = node;
				node->next      = new;
			}
			//if there is not enough space for additional node
			//just allocate more but aligned space
			else
			{
				size += leftover_size;
			}

			//update the node
			node->size = size;
			node->used = true;

			//return node data segment
			return (void*)((u64)node + sizeof(hnode_t));
		}

		//there are no free or big enough nodes
		if(node->next == NULL)
		{
			break;
		}
		//move onto the next node
		else
		{
			node = node->next;
		}
	}

	//we don't have free or big enough node
	//create new node at the end of the heap

	//resize data segment
	if(__dataseg_add(size + sizeof(hnode_t)) == (u64)-1)
	{
		printf("heap_alloc() error: sbrk failed\n"); return NULL;
	}

	//create new node
	hnode_t* new = (hnode_t*)((u64)node + sizeof(hnode_t) + node->size);
	new->sign = HNODE_SIGN;
	new->used = true;
	new->size = size;
	new->prev = node;
	new->next = NULL;
	node->next = new;

	//return node data segment
	return (void*)((u64)new + sizeof(hnode_t));
}
/**
 * allocate new space in heap and zero out the data
 */
void* heap_calloc(u64 size)
{
	//check heap
	if(__heap.init == false)
	{
		printf("heap_calloc() error: heap not initialized\n"); return NULL;
	}

	//allocate space
	void* space = heap_alloc(size);
	if(space == NULL) { return NULL; }
	//set it to zero
	memset(space, 0, size);
	return space;
}
/**
 * reallocate space in heap
 */
void* heap_realloc(void* data, u64 size)
{
	//check heap
	if(__heap.init == false)
	{
		printf("heap_realloc() error: heap not initialized\n"); return NULL;
	}

	//get the node address
	hnode_t* node = (hnode_t*)((u64)data - sizeof(hnode_t));

	//check for node signature
	if(node->sign != HNODE_SIGN)
	{
		printf("heap_realloc() error: corrupted heap"); return NULL;
	}

	//align size to x86_64 cache size
	size = ALIGN(size, CACHE_LINE_SIZE);

	//if it already has the desired size -> return
	if(node->size == size) 
	{ 
		return (void*)((u64)node + sizeof(hnode_t)); 
	}
	//if we are shrinking the node
	//just change the node size and 
	//if there is enough space, create new node
	else if(node->size > size)
	{
		u64 leftover_size = node->size - size;

		//if there is enough space, split the node into 2 nodes
		if(leftover_size >= (sizeof(hnode_t) * 4))
		{
			//calculate new node address
			hnode_t* new = (hnode_t*)((u64)node + sizeof(hnode_t) + size);

			//modify the new node
			new->size       = leftover_size - sizeof(hnode_t);
			new->used       = false;
			new->next       = node->next;
			new->next->prev = new;
			new->prev       = node;
			node->next      = new;
		}
		//if there is not enough space for additional node
		//just allocate more but aligned space
		else
		{
			size += leftover_size;
		}

		//update the node
		node->size = size;

		//return node data segment
		return (void*)((u64)node + sizeof(hnode_t));
	}
	//if we are enbiggening the node
	//we are fucked
	else/* if(node->size < size)*/
	{
		//if it's the last node in heap
		//increase the data segment
		if(node->next == NULL)
		{
			u64 leftover_size = size - node->size;
			if(__dataseg_add(leftover_size) == (u64)-1)
			{
				printf("heap_realloc() error: sbrk failed\n"); return NULL;
			}

			node->size = size;

			return (void*)((u64)node + sizeof(hnode_t));
		}
		else
		{
			//allocate new space
			void* new_data = heap_alloc(size);
			if(new_data == NULL) { return NULL; }

			//copy old data into the new buffer
			memcpy(new_data, (void*)((u64)node + sizeof(hnode_t)), node->size);

			//deallocate old space
			heap_free((void*)((u64)node + sizeof(hnode_t)));

			return new_data;
		}
	}
}
/**
 * \brief remove space in heap
 */
void  heap_free(void* data)
{
	//check heap
	if(__heap.init == false)
	{
		printf("heap_free() error: heap not initialized\n"); return;
	}

	//get the node address
	hnode_t* node = (hnode_t*)((u64)data - sizeof(hnode_t));

	//check for node signature
	if(node->sign != HNODE_SIGN)
	{
		printf("heap_free() error: corrupted heap\n"); return;
	}

	//check for node usage
	if(node->used == false)
	{
		printf("heap_free() error: trying to free already freed node\n"); return;
	}

	//node is no longer in use
	node->used = false;

	//iterate over the whole heap
	//and concatenate free nodes that are immediately one after another
	node = __heap.first;
	while(node != NULL)
	{
		//if node not used
		if(node->used == false)
		{
			//and previous node is also not used
			if(node->prev != NULL && node->prev->used == false)
			{
				//resize the previous node and delete
				//the current one
				node->prev->size += node->size + sizeof(hnode_t);
				node->prev->next = node->next;
				if(node->next != NULL)
				{
					node->next->prev = node->prev;
				}
			}
		}
		//move onto the next node
		node = node->next;
	}
}
/**
 * \brief print heap state
 */
void  heap_print()
{
	//check heap
	if(__heap.init == false)
	{
		printf("heap_print() error: heap not initialized\n"); return;
	}

	hnode_t* node = __heap.first;

	while(node != NULL)
	{
		printf("node: [used: %llu] [size: %llu]\n", node->used, node->size);
		node = node->next;
	}
}
/**
 * \brief reset heap
 */
void heap_reset()
{
	if((u64)brk((void*)__heap.start) != (u64)-1)
	{
		printf("heap_reset() error: brk failed\n"); return;
	}
	__heap.first = NULL;
	__heap.init  = false;
	__heap.start = 0;
}