/** @file alloc.c */
#include <stdio.h>
#include <unistd.h>
#include <string.h>

typedef struct s_block *t_block;
struct s_block {
    size_t size;
    t_block prev;
    t_block next;
    char data[1];
};

#define BLOCK_SIZE 24
#define PAGE_SIZE 65536

void* first_block = NULL;
void* low_ptr = NULL;  // is the begin of heap
void* max_ptr = NULL;  // is the heap top location
void* now_ptr = NULL;  // which next is the start of new alloc

size_t virt_addr(void* ptr) {
    return (size_t)(ptr - low_ptr);
}

t_block get_block(void *p) {
    char *tmp;
    tmp = p;
    return (p = tmp -= BLOCK_SIZE);
}

void* more_bytes(size_t s) {
    if (now_ptr == NULL) {
        now_ptr = sbrk(0);
        max_ptr = now_ptr;
        low_ptr = now_ptr;
    }
    if (now_ptr + s > max_ptr) {
        size_t delta = s - (max_ptr - now_ptr);
        delta = (((delta&(PAGE_SIZE-1))?1:0) + (delta / PAGE_SIZE)) * PAGE_SIZE;
        sbrk(delta);
        max_ptr += delta;
    } else {
    }
    now_ptr += s;
    return now_ptr - s + 1;
}

void delete_block(t_block b) {
    if (b == first_block) {
        first_block = b->next;
    }
    if (b->prev) {
        b->prev->next = b->next;
    }
    if (b->next) {
        b->next->prev = b->prev;
    }
    b->prev = NULL;
    b->next = NULL;
}

void insert_block(t_block b) {
    if (first_block == NULL) {
        b->prev = NULL;
        b->next = NULL;
        first_block = b;
    } else {
        t_block ptr = first_block, last = first_block;
        while (ptr && ptr < b) {
            last = ptr;
            ptr = ptr->next;
        }
        if (ptr == first_block) {
            b->next = ptr;
            ptr->prev = b;
            b->prev = NULL;
            first_block = b;
        } else {
            b->prev = last;
            b->next = ptr;
            if (ptr) {
                ptr->prev = b;
            }
            last->next = b;
        }
    }
}
t_block merge(t_block b) {
    while (b && b->next && (void*)(b->next) == (void*)b+BLOCK_SIZE+b->size) {
        b->size += BLOCK_SIZE + b->next->size;
        b->next = b->next->next;
        if (b->next) {
            b->next->prev = b;
        }
    }
    while (b && b->prev && (void*)(b->prev)+b->prev->size+BLOCK_SIZE == (void*)b) {
        b->prev->size += BLOCK_SIZE + b->size;
        b->prev->next = b->next;
        if (b->next) {
            b->next->prev = b->prev;
        }
        b = b->prev;
    }
    return b;
}

t_block extend_heap(size_t s) {
    t_block b = more_bytes(BLOCK_SIZE + s);
    b->size = s;
    b->next = NULL;
    b->prev = NULL;
    return b;
}

void split_block(t_block b, size_t s) {
    t_block _new;
    _new = (t_block)(b->data + s);
    _new->size = b->size - s - BLOCK_SIZE;
    b->size = s;
    delete_block(b);
    insert_block(_new);
}

size_t align8(size_t s) {
    if ((s & 0x7) == 0) {
        return s;
    }
    return ((s >> 3) + 1) << 3;
}

t_block find_block(size_t size) {
    if (!first_block) {
        return NULL;
    }
    t_block b = first_block;
    while (b && b->size < size + BLOCK_SIZE) {
        b = b->next;
    }
    return b;
}

void copy_block(t_block src, t_block dst) {
    size_t *sdata, *ddata;
    size_t i;
    sdata = (size_t*)src->data;
    ddata = (size_t*)dst->data;
    memcpy(ddata, sdata, src->size);
}

/**
 * Allocate space for array in memory
 *
 * Allocates a block of memory for an array of num elements, each of them size
 * bytes long, and initializes all its bits to zero. The effective result is
 * the allocation of an zero-initialized memory block of (num * size) bytes.
 *
 * @param num
 *    Number of elements to be allocated.
 * @param size
 *    Size of elements.
 *
 * @return
 *    A pointer to the memory block allocated by the function.
 *
 *    The type of this pointer is always void*, which can be cast to the
 *    desired type of data pointer in order to be dereferenceable.
 *
 *    If the function failed to allocate the requested block of memory, a
 *    NULL pointer is returned.
 *
 * @see http://www.cplusplus.com/reference/clibrary/cstdlib/calloc/
 */
void *calloc(size_t num, size_t size)
{
    /* Note: This function is complete. You do not need to modify it. */
    size_t *_new;
    size_t s8, i;
    _new = malloc(num * size);
    if (_new) {
        s8 = align8(num * size) << 3;
        memset(_new, 0, s8);
    }
    return _new;
}


/**
 * Allocate memory block
 *
 * Allocates a block of size bytes of memory, returning a pointer to the
 * beginning of the block.  The content of the newly allocated block of
 * memory is not initialized, remaining with indeterminate values.
 *
 * @param size
 *    Size of the memory block, in bytes.
 *
 * @return
 *    On success, a pointer to the memory block allocated by the function.
 *
 *    The type of this pointer is always void*, which can be cast to the
 *    desired type of data pointer in order to be dereferenceable.
 *
 *    If the function failed to allocate the requested block of memory,
 *    a null pointer is returned.
 *
 * @see http://www.cplusplus.com/reference/clibrary/cstdlib/malloc/
 */
void *malloc(size_t size)
{
    t_block b, last;
    size_t s;
    s = align8(size);
    b = find_block(s);
    if (b) {
        if (b->size - s >= BLOCK_SIZE + 8) {
            split_block(b, s);
        } else {
            delete_block(b);
        }
    } else {
        b = extend_heap(s);
    }
    return b->data;
}


/**
 * Deallocate space in memory
 *
 * A block of memory previously allocated using a call to malloc(),
 * calloc() or realloc() is deallocated, making it available again for
 * further allocations.
 *
 * Notice that this function leaves the value of ptr unchanged, hence
 * it still points to the same (now invalid) location, and not to the
 * null pointer.
 *
 * @param ptr
 *    Pointer to a memory block previously allocated with malloc(),
 *    calloc() or realloc() to be deallocated.  If a null pointer is
 *    passed as argument, no action occurs.
 */
void free(void *ptr)
{
    // "If a null pointer is passed as argument, no action occurs."
    t_block b;
    b = get_block(ptr);
    insert_block(b);
    b = merge(b);
}


/**
 * Reallocate memory block
 *
 * The size of the memory block pointed to by the ptr parameter is changed
 * to the size bytes, expanding or reducing the amount of memory available
 * in the block.
 *
 * The function may move the memory block to a new location, in which case
 * the new location is returned. The content of the memory block is preserved
 * up to the lesser of the new and old sizes, even if the block is moved. If
 * the new size is larger, the value of the newly allocated portion is
 * indeterminate.
 *
 * In case that ptr is NULL, the function behaves exactly as malloc, assigning
 * a new block of size bytes and returning a pointer to the beginning of it.
 *
 * In case that the size is 0, the memory previously allocated in ptr is
 * deallocated as if a call to free was made, and a NULL pointer is returned.
 *
 * @param ptr
 *    Pointer to a memory block previously allocated with malloc(), calloc()
 *    or realloc() to be reallocated.
 *
 *    If this is NULL, a new block is allocated and a pointer to it is
 *    returned by the function.
 *
 * @param size
 *    New size for the memory block, in bytes.
 *
 *    If it is 0 and ptr points to an existing block of memory, the memory
 *    block pointed by ptr is deallocated and a NULL pointer is returned.
 *
 * @return
 *    A pointer to the reallocated memory block, which may be either the
 *    same as the ptr argument or a new location.
 *
 *    The type of this pointer is void*, which can be cast to the desired
 *    type of data pointer in order to be dereferenceable.
 *
 *    If the function failed to allocate the requested block of memory,
 *    a NULL pointer is returned, and the memory block pointed to by
 *    argument ptr is left unchanged.
 *
 * @see http://www.cplusplus.com/reference/clibrary/cstdlib/realloc/
 */
void *realloc(void *ptr, size_t size)
{
    // "In case that ptr is NULL, the function behaves exactly as malloc()"
    size_t s;
    t_block b, _new;
    void *newp;
    if (!ptr) {
        return malloc(size);
    }
    s = align8(size);
    b = get_block(ptr);
    b = merge(b);
    if (b->size >= s) {
        if (b->size - s >= (BLOCK_SIZE + 8)) {
            split_block(b, s);
        }
    } else {
        newp = malloc(s);
        _new = get_block(newp);
        copy_block(b, _new);
        free(b->data);
        return newp;
    }
    return b->data;
}
