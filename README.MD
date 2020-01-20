HEAP:

	Doubly linked list which nodes and data are placed into memory right after each other

	Start of heap segment
	|
	V
	|Node||...data...||Node||.data.||Node||.........data.........||Node||.data.|-> More RAM
	 |  ^              ^  ^          ^  ^                          ^  |
	 V  |______________|  |__________|  |__________________________|  V
	NULL                                                             NULL

	Each node has size and usage flag
	User data is right after the node, meaning that the node is kind of a header to the user allocated space
	
	Note: This has potential of user overwriting nodes which can lead to corrupted heap
	
HEAP DATA SEGMENT:

	Is managed by functions brk() and sbrk() which can change the end of the heap data segment
	See more here: https://linux.die.net/man/2/sbrk

	Note: Could be made more portable by using malloc() and realloc()

ALLOCATION:

	Try to find free and big enough node to reuse our already allocated space
	If requested allocating space is small enough, split the free and big enough node into 2 nodes
	Otherwise increase heap data segment and add a new node

	Reusing space:

		Requested space: |Node||...data...|
		Heap Before: |Node||...free...||Node||.data.|
		Heap After:  |Node||...data...||Node||.data.|

	Splitting node:

		Requested space: |Node||...data...|
		Heap Before: |Node||...........free...........||Node||.data.|
		Heap After:  |Node||...data...||Node|..free...||Node||.data.|

	Creating new node:

		Requested space: |Node||...data...|
		Heap Before: |Node||..data..||Node||.free.|
		Heap After:  |Node||..data..||Node||.free.||Node||...data...|

DEALLOCATION:

	Set the node free flag and merge free nodes

	Merging free nodes:
		                                    free this
		                                        |
		                                        V
		Heap Before: |Node||..free..||Node||....data....||Node||..free..||Node||.........data.........|
		Heap After:  |Node||....................free....................||Node||.........data.........|
