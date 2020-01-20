#include "heap.h"

#include <stdio.h>
#include <string.h>

int main(int argc, char* argv[])
{
	(void)argc;
	(void)argv;

	heap_init();

	void* a = heap_alloc(333);
	void* b = heap_alloc(1);

	heap_print();
	printf("----\n");

	heap_free(b);

	heap_print();
	printf("----\n");

	void* c = heap_alloc(555);

	heap_print();
	printf("----\n");

	heap_reset();
	return 0;
}