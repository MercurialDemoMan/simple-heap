#include "heap.h"

#include <stdio.h>
#include <string.h>

int main(int argc, char* argv[])
{
	(void)argc;
	(void)argv;

	heap_init();

	void* a = heap_alloc(333);

	heap_print();
	printf("----\n");

	heap_free(a);

	a = heap_alloc(1);

	heap_print();
	printf("----\n");

	heap_reset();
	return 0;
}