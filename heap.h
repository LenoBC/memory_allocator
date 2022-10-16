#if !defined(_HEAP_H_)
#define _HEAP_H_

struct memory_manager_t
{
    void *memory_start;
    size_t memory_size;
    struct memory_chunk_t *first_memory_chunk;

} __attribute__(( packed ));

struct memory_manager_t memory_manager;

struct memory_chunk_t
{
    struct memory_chunk_t* prev;
    struct memory_chunk_t* next;
    size_t size;
    int free;
    int sum_control;
} __attribute__(( packed ));

enum pointer_type_t
{
    pointer_null,
    pointer_heap_corrupted,
    pointer_control_block,
    pointer_inside_fences,
    pointer_inside_data_block,
    pointer_unallocated,
    pointer_valid
};

int heap_setup(void);
void heap_clean(void);
int fun_sum_control(const struct memory_chunk_t* block);
int unused_size(void* memblock);

void* heap_malloc(size_t size);
void* heap_calloc(size_t number, size_t size);
void* heap_realloc(void* memblock, size_t size);
void  heap_free(void* memblock);

size_t   heap_get_largest_used_block_size(void);
enum pointer_type_t get_pointer_type(const void* const pointer);
enum pointer_type_t typ_pointer;
int heap_validate(void);

void* heap_malloc_aligned(size_t count);
void* heap_calloc_aligned(size_t number, size_t size_of);
void* heap_realloc_aligned(void* memblock, size_t size);

#endif // _HEAP_H