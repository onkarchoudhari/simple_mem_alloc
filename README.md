# Simple Memory Allocator
This memory allocator is not too optimized, but it works


freeing a block of memory does not necessarily mean we release memory back to OS. 
It just means that we keep the block marked as free. This block marked as free may be reused on a later malloc() call. 
Since memory not at the end of the heap can’t be released, this is the only way ahead for us.

The header is internally managed, and is kept completely hidden from the calling program.


So to keep track of the memory allocated by our malloc, we will put them in a linked list.








