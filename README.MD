HEAP:

	Doubly linked list which nodes and data are placed right into memory right after each other

	Start of heap segment
	|
	V
	|Node||...data...||Node||.data.||Node||.........data.........||Node||.data.|-> More RAM
	 |  ^              ^  ^          ^  ^                          ^  |
	 V  |______________|  |__________|  |__________________________|  V
	NULL                                                             NULL

	Each node has size and flag, if the node is used
	User data is right after the node, meaning that the node is a header to the user allocated space
	
HEAP DATA SEGMENT:

	Is managed by functions brk() and sbrk()
	See more here: https://linux.die.net/man/2/sbrk

	Could be made more portable by using one malloc() and realloc()

ALLOCATION:

	Try to find free and big enough node to reuse our space
	If allocating space is small enough, split the free and big enough node
	Otherwise increase heap data segment and add a new node

DEALLOCATION:

	Set the node free flag and merge free nodes
