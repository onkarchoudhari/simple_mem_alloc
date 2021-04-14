#include <unistd.h>
#include <string.h>
#include <pthread.h>
/* Only for the debug printf */
#include <stdio.h>
void print_mem_list();

typedef char ALIGN[16];

/* 
* Header to every newly allocated memory block 
* `union` makes the header end up on a memory address aligned to 16 bytes 
* size of union = larger size of its members 
* Thus union guarantees that end of the header is memory alligned 
* The end of the header is where the actual memory block begins and therefore 
* the memory provided to the caller by the allocator will be aligned to 16 bytes 
*/
union header {
    struct {
        size_t size;
        unsigned is_free;
        union header *next;      /* make it a linked list */
    } s;
    ALIGN stub;
};

typedef union header header_t;

header_t *head = NULL, *tail = NULL;
 
/* To prevent two or more threads from concurrently accessing memory, we put a basic locking mechanism in place.
* A global lock -- before every action on memory you have to acquire the lock, and once you are done you have to release the lock.
*/ 
pthread_mutex_t global_malloc_lock;

/*
* Traverse the linked list and see if there already exist a block of memory that is marked as free and can accomodate the given size.
* First-fit approach taken in searching the linked list.
*/
header_t *get_free_block(size_t size)
{
    header_t *curr = head;
    while (curr) {
        if (curr->s.is_free && curr->s.size >= size)
            return curr;
        curr = curr->s.next;
    }

    return NULL;
}

void *malloc(size_t size)
{
    size_t total_size;
    void *block;
    header_t *header;
    
    /* return NULL if requested size is 0 */
    if (!size) 
        return NULL;

    pthread_mutex_lock(&global_malloc_lock);

    /* see if there exist free compatible block already */
    header = get_free_block(size);

    if (header) {
        header->s.is_free = 0;
        pthread_mutex_unlock(&global_malloc_lock);
        /* we hide the very existence of the header to an outside party. 
         * When we do (header + 1), it points to the byte right after the end of the header = 1st byte of actual mem block */
        return (void *)(header + 1);
    }

    /* if free block not found, we extend the heap by calling sbrk() 
     * requesting OS to increment the program break (brk) */
    total_size = sizeof(header_t) + size;
    block = sbrk(total_size);
    if (block == (void *)-1) {
        pthread_mutex_unlock(&global_malloc_lock);
        return NULL;
    }

    header = block;
    header->s.size = size;
    header->s.is_free = 0;
    header->s.next = NULL;
    if (!head)
        head = header;
    if (tail)
        tail->s.next = header;
    tail = header;
    pthread_mutex_unlock(&global_malloc_lock);
    /* returning first byte of actual mem block */
    return (void *)(header + 1);
}

/* free */
/* if the block-to-be-freed is at the end of the heap release it to the OS. 
 * Otherwise, mark it ‘free’, hoping to reuse it later. */
void free(void *block)
{
    header_t *header, *tmp;
    void *programbreak;

    if (!block)
        return;
    pthread_mutex_lock(&global_malloc_lock);
    header = (header_t *)block - 1;                         /* we cast 'block' to a header pointer type and move it behind by 1 unit */

    programbreak = sbrk(0);                                 /* gives current value of the program break */

    /* compute end of the current block and compared it with program break */
    /* we cast 'block' to char pointer and move it by block size (so it will move that many bytes) */
    if ((char *)block + header->s.size == programbreak) {   
        if (head == tail) {
            head = tail = NULL;
        } else {
            tmp = head;
            while (tmp) {
                if (tmp->s.next == tail) {
                    tmp->s.next = NULL;
                    tail = tmp;
                }
                tmp = tmp->s.next;
            }
        }

        sbrk(0 - sizeof(header_t) - header->s.size);
        pthread_mutex_unlock(&global_malloc_lock);
        return;
    }

    header->s.is_free = 1;
    pthread_mutex_unlock(&global_malloc_lock);
}

/* allocates memory for an array of num elements of nsize bytes each and returns a pointer to the allocated memory */
void *calloc(size_t num, size_t nsize) 
{
    size_t size;
    void *block;
    if (!num || !nsize) {
        return NULL;
    }

    size = num * nsize;
    /* check multiplication overflow */
    if (nsize != size / num) {
        return NULL;
    }

    block = malloc(size);
    
    if (!block) {
        return NULL;
    }

    memset(block, 0, size);                 /* Clears the allocated memory */
    return block;
}

/* changes the size of the given memory block to the size given */
void *realloc(void *block, size_t size) 
{
    header_t *header;
    void *ret;

    if (!block || !size) {
        return malloc(size);
    }

    header = (header_t *)block - 1;

    if (header->s.size >= size) {
        return block;
    }

    ret = malloc(size);
    if (ret) {
        memcpy(ret, block, header->s.size);     /* preserves the original data */
        free(block);
    }

    return ret;
}
     
/* A debug function to print the entire link list */
void print_mem_list()
{
	header_t *curr = head;
	printf("head = %p, tail = %p \n", (void*)head, (void*)tail);
	while(curr) {
		printf("addr = %p, size = %zu, is_free=%u, next=%p\n",
			(void*)curr, curr->s.size, curr->s.is_free, (void*)curr->s.next);
		curr = (header_t *)(curr->s.next);
	}
}
