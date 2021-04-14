# Simple Memory Allocator

This is a simple memory allocator.
It implements <a href="http://man7.org/linux/man-pages/man3/free.3.html">malloc()</a>, <a href="http://man7.org/linux/man-pages/man3/free.3.html">calloc()</a>, <a href="http://man7.org/linux/man-pages/man3/free.3.html">realloc()</a> and <a href="http://man7.org/linux/man-pages/man3/free.3.html">free()</a>.
It is not too optimized. 
It works only for `ls` command, for any other command it fails (as of now). 

#### Article ####
Learned this allocator by refering to the article - 
http://arjunsreedharan.org/post/148675821737/write-a-simple-memory-allocator


##### Study Notes ##### 
1st. Freeing a block of memory does not necessarily mean we release memory back to OS.  It just means that we keep the block marked as free. This block marked as free may be reused on a later malloc() call. 

2nd. The header is internally managed, and is kept completely hidden from the calling program.

#### Compile and Run ####

```
gcc -o simple_mem_alloc.so -fPIC -shared simple_mem_alloc.c
```

The `-fPIC` and `-shared` options makes sure the compiled output has position-independent code and tells the linker to produce a shared object suitable for dynamic linking.

On Linux, if you set the enivornment variable `LD_PRELOAD` to the path of a shared object, that file will be loaded before any other library. We could use this trick to load our compiled library file first, so that the later commands run in the shell will use our malloc(), free(), calloc() and realloc().

```
export LD_PRELOAD=$PWD/memalloc.so
```

Now, if you run something like `ls`, it will use our memory allocator.
```
ls
README.md  simple_mem_alloc.c  simple_mem_alloc.so
```

Once you're done experimenting, you can do `unset LD_PRELOAD` to stop using our allocator.
