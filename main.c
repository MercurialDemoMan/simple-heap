#include "heap.h"

#include <stdio.h>
#include <string.h>

int main(int argc, char* argv[])
{
	(void)argc;
	(void)argv;

	heap_init();

	void* c = heap_alloc(333);
	void* a = heap_alloc(5);
	void* b = heap_alloc(1233);

	heap_print();
	printf("----\n");

	a = heap_realloc(a, 3333);

	heap_print();
	printf("----\n");

	heap_free(a);
	heap_free(b);

	heap_print();
	printf("----\n");

	heap_reset();
	return 0;
}