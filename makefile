default:
	gcc main.c heap.c -std=c11 -O0 -o heap

clean:
	rm heap
