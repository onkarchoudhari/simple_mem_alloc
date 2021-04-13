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
        struct header_t *next;      /* make it a linked list */
    } s;
    ALIGN stub;
};

typedef union header header_t;

header_t *head, *tail;

/* 
* To prevent two or more threads from concurrently accessing memory, we put a basic locking mechanism in place.
* A global lock -- before every action on memory you have to acquire the lock, and once you are done you have to release the lock.
*/ 
pthread_mutex_t global_malloc_lock;


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
        curr = curr->next;
    }

    return NULL;
}



  





















