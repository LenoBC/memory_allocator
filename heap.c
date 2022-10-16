#include <stdint.h>
#include <string.h>
#include "heap.h"
#include "custom_unistd.h"

#define size_ch 32
#define PAGE 4096
#define FENCE 16

int align_size(int size){
    return (size + PAGE -1) & ~(PAGE -1);
}

int heap_setup(void)
{
    memory_manager.memory_start = custom_sbrk(PAGE);
    if(memory_manager.memory_start == (void *)-1) return -1;
    memory_manager.first_memory_chunk = NULL;
    memory_manager.memory_size = PAGE;
    return 0;
}

void heap_clean(void)
{
    heap_validate();
    size_t heap_size = 0;
    void *ptr_sbrk = custom_sbrk(0);
    heap_size = (uint8_t*)ptr_sbrk - (uint8_t*)memory_manager.memory_start;
    custom_sbrk(-heap_size);
    memset(&memory_manager,0,sizeof(struct memory_manager_t));
}

void* heap_malloc(size_t size)
{
    if(size <= 0 ) return NULL;

    if (size + size_ch + 2*FENCE > memory_manager.memory_size) {
        size_t siz_need = size + size_ch + 2*FENCE - memory_manager.memory_size;
        siz_need = align_size(siz_need);

        void *ptr = custom_sbrk(siz_need);
        if(ptr == (void*)-1)
            return NULL;
        memory_manager.memory_size += siz_need;
    }

    struct memory_chunk_t *first_block = memory_manager.first_memory_chunk;

    if(first_block == NULL){
        if(size + size_ch + 2*FENCE <= memory_manager.memory_size)
        {
            first_block = (struct memory_chunk_t *) memory_manager.memory_start;
            memset((uint8_t*)memory_manager.memory_start + size_ch,'#', FENCE);
            memset((uint8_t*)memory_manager.memory_start + size_ch + FENCE + size,'#', FENCE);
            first_block->size = size;
            first_block->free = 0;
            first_block->next = NULL;
            first_block->prev = NULL;
            first_block->sum_control = 0;

            first_block->sum_control = fun_sum_control(first_block);
            memory_manager.memory_size += -size - size_ch - 2*FENCE;
            memory_manager.first_memory_chunk = first_block;
        }
    }
    else if(size + size_ch + 2*FENCE <= memory_manager.memory_size)
    {
        uint8_t *mem = (uint8_t*)memory_manager.memory_start;
        size_t offset = 0;
        struct memory_chunk_t* free_block = (struct memory_chunk_t*) memory_manager.memory_start;
        while(free_block->free == 0 || free_block->free == 1){
            if (free_block->free == 1 && free_block->size >= size +2*FENCE) break;
            if(free_block->free == 0) {
                offset += 2 * FENCE;
                offset += unused_size((uint8_t*)free_block + size_ch +FENCE);
            }
            offset += free_block->size + size_ch;
            if(free_block->next)
                free_block = free_block->next;
            else break;
        }

        void *brk_0 = custom_sbrk(0);
        if((uint8_t*)mem + offset + size_ch + 2*FENCE + size >= (uint8_t*)brk_0)
            return NULL;

        if (free_block->free == 0){
            //struct memory_chunk_t* next_block = (struct memory_chunk_t*) ((uint8_t*)mem + offset); // tu zle offset
            struct memory_chunk_t* next_block = (struct memory_chunk_t*) ((uint8_t*)free_block + size_ch + 2*FENCE + free_block->size);
            brk_0 = custom_sbrk(0);
            if(((uint8_t*)next_block + size_ch + 2*FENCE + size) >= (uint8_t*)brk_0)
                return NULL;

            memset((uint8_t*)next_block + size_ch,'#', FENCE);
            memset((uint8_t*)next_block + size_ch + size + FENCE,'#', FENCE);
            next_block->size = size;
            next_block->free = 0;
            next_block->next = NULL;
            next_block->prev = free_block;
            free_block->next = next_block;

            next_block->sum_control = 0;
            next_block->sum_control = fun_sum_control(next_block);
            free_block->sum_control = 0;
            free_block->sum_control = fun_sum_control(free_block);

            memory_manager.memory_size += -size - size_ch - 2*FENCE;

            typ_pointer = get_pointer_type((uint8_t*)next_block + size_ch + FENCE);
            if(typ_pointer == pointer_valid)
                return (uint8_t*)next_block + size_ch + FENCE;

           return NULL;
        }

        if(free_block->free == 1){
            memset((uint8_t*)free_block + size_ch,'#',FENCE);
            memset((uint8_t*)free_block + size_ch + size + FENCE,'#',FENCE);

            free_block->size = size;
            free_block->free = 0;
            free_block->sum_control = 0;
            free_block->sum_control = fun_sum_control(free_block);

            memory_manager.memory_size += -size - 2*FENCE;
            typ_pointer = get_pointer_type((uint8_t*)free_block + size_ch + FENCE);
            if(typ_pointer == pointer_valid)
                return (uint8_t*)free_block + size_ch + FENCE;

            return NULL;
        }
    }
    typ_pointer = get_pointer_type((uint8_t*)first_block + size_ch + FENCE);
    if(typ_pointer == pointer_valid)
        return (uint8_t*)first_block + size_ch + FENCE;

    return NULL;
}

void* heap_calloc(size_t number, size_t size)
{
    if(number <= 0 || size <= 0) return NULL;
    size_t size_b = number * size;

    if (size_b + size_ch + 2*FENCE > memory_manager.memory_size) {
        int siz_need = size_b + size_ch + 2*FENCE - memory_manager.memory_size;
        siz_need = align_size(siz_need);

        void *ptr = custom_sbrk(siz_need);
        if(ptr == (void*)-1)
            return NULL;
        memory_manager.memory_size += siz_need;
    }

    struct memory_chunk_t *first_block = memory_manager.first_memory_chunk;

    if(first_block == NULL){
        if(size_b + size_ch + 2*FENCE <= memory_manager.memory_size)
        {
            first_block = (struct memory_chunk_t *) memory_manager.memory_start;
            memset((uint8_t*)memory_manager.memory_start + size_ch,'#', FENCE);
            memset((uint8_t*)memory_manager.memory_start + size_ch + FENCE + size_b,'#', FENCE);
            first_block->size = size_b;
            first_block->free = 0;
            first_block->next = NULL;
            first_block->prev = NULL;
            first_block->sum_control = 0;

            first_block->sum_control = fun_sum_control(first_block);

            memset((uint8_t*)first_block + size_ch + FENCE,0, size_b);
            memory_manager.memory_size += -size_b - size_ch - 2*FENCE;

            memory_manager.first_memory_chunk = first_block;
        }
    }
    else if(size_b + size_ch + 2*FENCE <= memory_manager.memory_size)
    {
        uint8_t *mem = memory_manager.memory_start;
        int offset = 0;
        struct memory_chunk_t* free_block = (struct memory_chunk_t*) memory_manager.memory_start;
        while(free_block->free == 0 || free_block->free == 1){
            if (free_block->free == 1 && free_block->size >= size_b +2*FENCE) break;
            if(free_block->free == 0) {
                offset += 2 * FENCE;
                offset += unused_size((uint8_t*)free_block + size_ch +FENCE);
            }
            offset += free_block->size + size_ch;
            if(free_block->next)
                free_block = free_block->next;
            else break;
        }

        void *brk_0 = custom_sbrk(0);
        if((uint8_t*)mem + offset + size_ch + 2*FENCE + size_b >= (uint8_t*)brk_0)
            return NULL;

        if (free_block->free == 0){
            //struct memory_chunk_t* next_block = (struct memory_chunk_t*) ((uint8_t*)mem + offset);
            struct memory_chunk_t* next_block = (struct memory_chunk_t*) ((uint8_t*)free_block + size_ch + 2*FENCE + free_block->size);
            brk_0 = custom_sbrk(0);
            if(((uint8_t*)next_block + size_ch + 2*FENCE + size_b) >= (uint8_t*)brk_0)
                return NULL;

            memset((uint8_t*)next_block + size_ch,'#', FENCE);
            memset((uint8_t*)next_block + size_ch + size_b + FENCE,'#', FENCE);
            next_block->size = size_b;
            next_block->free = 0;
            next_block->next = NULL;
            next_block->prev = free_block;
            free_block->next = next_block;

            next_block->sum_control = 0;
            next_block->sum_control = fun_sum_control(next_block);
            free_block->sum_control = 0;
            free_block->sum_control = fun_sum_control(free_block);

            memset((uint8_t*)next_block + size_ch + FENCE,0, size_b);

            memory_manager.memory_size += -size_b - size_ch - 2*FENCE;

            typ_pointer = get_pointer_type((uint8_t*)next_block + size_ch + FENCE);
            if(typ_pointer == pointer_valid)
                return (uint8_t*)next_block + size_ch + FENCE;
            return NULL;
        }

        if(free_block->free == 1){
            memset((uint8_t*)free_block + size_ch,'#',FENCE);
            memset((uint8_t*)free_block + size_ch + size_b + FENCE,'#',FENCE);

            free_block->size = size_b;
            free_block->free = 0;
            free_block->sum_control = 0;

            free_block->sum_control = fun_sum_control(free_block);

            memset((uint8_t*)free_block + size_ch + FENCE,0, size_b);

            memory_manager.memory_size += -size_b - 2*FENCE;

            typ_pointer = get_pointer_type((uint8_t*)free_block + size_ch + FENCE);
            if(typ_pointer == pointer_valid)
                return (uint8_t*)free_block + size_ch + FENCE;
            return NULL;
        }
    }
    typ_pointer = get_pointer_type((uint8_t*)first_block + size_ch + FENCE);
    if(typ_pointer == pointer_valid)
        return (uint8_t*)first_block + size_ch + FENCE;
    return NULL;
}

void* heap_realloc(void* memblock, size_t size)
{
    if(memory_manager.memory_start == NULL) return NULL;
    if(size == 0){
        heap_free(memblock);
        return NULL;
    }

    typ_pointer = get_pointer_type(memblock);
    if(typ_pointer == pointer_valid || typ_pointer == pointer_null)
    {
        if (memblock == NULL) {
            void *m = heap_malloc(size);
            return (uint8_t *) m;
        }
        struct memory_chunk_t *block_realloc = (struct memory_chunk_t *) ((uint8_t *)memblock - FENCE - size_ch);
        if (size == block_realloc->size) return (uint8_t *) memblock;
        if (size < block_realloc->size) {
            memory_manager.memory_size += block_realloc->size - size;

            memset((uint8_t*)block_realloc + size_ch + FENCE + block_realloc->size,'r', FENCE);
            block_realloc->size = size;
            memset((uint8_t*)block_realloc + size_ch + FENCE + size,'#', FENCE);
            block_realloc->sum_control = 0;
            block_realloc->sum_control = fun_sum_control(block_realloc);
            typ_pointer = get_pointer_type(memblock);
            if(typ_pointer == pointer_valid)
                return (uint8_t*)memblock;
        }
        size_t size_2 = 0;
        if (size > block_realloc->size)
        {
            int unused_1 = unused_size(memblock);
            unsigned int to_sbrk = 0;
            void *p_brk = custom_sbrk(0);
            uint8_t *p_block_r = (uint8_t*)block_realloc;

            if(block_realloc->next == NULL && (uint8_t*)p_block_r + size_ch + block_realloc->size + 2*FENCE < (uint8_t*)p_brk) {
                while ((uint8_t *) p_block_r + size_ch + block_realloc->size + 2 * FENCE != (uint8_t *) p_brk) {
                    to_sbrk++;
                    p_block_r++;
                }
            }

             if((block_realloc->next) && (block_realloc->next->free == 1) && (block_realloc->size + block_realloc->next->size + unused_1 + size_ch) >= size) { // AaaaaB.... po AaaaaaaaB
                memory_manager.memory_size += size_ch;
                size_2 = size - block_realloc->size;
                memset((uint8_t*)block_realloc + size_ch + FENCE + block_realloc->size,'r', FENCE);
                block_realloc->size = size;
                block_realloc->next->next->prev = block_realloc;
                block_realloc->next = block_realloc->next->next;

                memset((uint8_t*)block_realloc + size_ch + FENCE + block_realloc->size,'#', FENCE);
                block_realloc->sum_control = 0;
                block_realloc->sum_control = fun_sum_control(block_realloc);

                block_realloc->next->sum_control = 0;
                block_realloc->next->sum_control = fun_sum_control(block_realloc->next);
                memory_manager.memory_size += -size_2;

                typ_pointer = get_pointer_type((uint8_t*)block_realloc + size_ch + FENCE);
                if(typ_pointer == pointer_valid)
                    return (uint8_t*)block_realloc + size_ch + FENCE;
            }

            if(block_realloc->next == NULL && block_realloc->size + to_sbrk >= size){
                void *brk_0 = custom_sbrk(0);
                if((uint8_t*)block_realloc + size_ch + 2*FENCE + size >= (uint8_t*)brk_0)
                    return NULL;
                memset((uint8_t*)block_realloc + size_ch + FENCE + block_realloc->size,'r', FENCE);
                size_2 = size - block_realloc->size;
                block_realloc->size = size;
                memset((uint8_t*)block_realloc + size_ch + size + FENCE,'#',FENCE);

                block_realloc->sum_control = 0;
                block_realloc->sum_control = fun_sum_control(block_realloc);
                memory_manager.memory_size += -size_2;

                typ_pointer = get_pointer_type(memblock);
                if(typ_pointer == pointer_valid)
                    return (uint8_t*)memblock;
            }

            if(block_realloc->next == NULL && block_realloc->size + to_sbrk < size){
                size_2 = size - block_realloc->size;
                size_t siz_need = size - block_realloc->size - to_sbrk;
                siz_need = align_size(siz_need);
                void *ptr = custom_sbrk(siz_need);
                if (ptr == (void *) -1)
                    return NULL;
                memory_manager.memory_size += siz_need;
                void *brk_0 = custom_sbrk(0);
                if((uint8_t*)block_realloc + size_ch + 2*FENCE + size >= (uint8_t*)brk_0)
                    return NULL;

                memset((uint8_t*)block_realloc + size_ch + FENCE + block_realloc->size,'r', FENCE);
                block_realloc->size = size;

                memset((uint8_t*)block_realloc + size_ch + size + FENCE,'#',FENCE);

                block_realloc->sum_control = 0;
                block_realloc->sum_control = fun_sum_control(block_realloc);
                memory_manager.memory_size += -size_2;

                typ_pointer = get_pointer_type(memblock);
                if(typ_pointer == pointer_valid)
                    return (uint8_t*)memblock;
            }

            if(block_realloc->next) {
                struct memory_chunk_t *free_block = (struct memory_chunk_t *) ((uint8_t *)memory_manager.memory_start);
                while (free_block) {
                    if ((free_block->free == 1) && (free_block->size >= size + 2*FENCE)) {
                        size_2 = free_block->size;
                        free_block->size = size;
                        free_block->free = 0;

                        free_block->sum_control = 0;
                        free_block->sum_control = fun_sum_control(free_block);

                        memcpy((uint8_t*)free_block + size_ch, (uint8_t*)block_realloc + size_ch,block_realloc->size + FENCE);
                        memset((uint8_t*)free_block + size_ch + size + FENCE, '#', FENCE);
                        heap_free((uint8_t*)block_realloc + size_ch + FENCE);

                        memory_manager.memory_size += -size -2*FENCE;
                        typ_pointer = get_pointer_type((uint8_t *) free_block + size_ch + FENCE);
                        if (typ_pointer == pointer_valid)
                            return (uint8_t *) free_block + size_ch + FENCE;
                    }
                    free_block = free_block->next;
                }

                size_t siz_need = size + 2*FENCE + size_ch;
                siz_need = align_size(siz_need);
                void *ptr = custom_sbrk(siz_need);
                if (ptr == (void *) -1)
                    return NULL;
                memory_manager.memory_size += siz_need;
                struct memory_chunk_t *wsk = block_realloc;

                while (wsk && wsk->next) {
                    wsk = wsk->next;
                }

                int unused_wsk = unused_size((uint8_t*)wsk+size_ch+FENCE);
                void *brk_0 = custom_sbrk(0);
                if((uint8_t*)wsk + 2*size_ch + 4*FENCE + wsk->size + unused_wsk + size >= (uint8_t*)brk_0)
                    return NULL;

                struct memory_chunk_t *new_block_r = (struct memory_chunk_t *) ((uint8_t *) wsk +
                                                                                size_ch +
                                                                                wsk->size + 2*FENCE + unused_wsk);
                memcpy((uint8_t *) new_block_r + size_ch,
                       (uint8_t *) block_realloc + size_ch, FENCE + block_realloc->size);
                new_block_r->size = size;
                new_block_r->free = 0;
                new_block_r->next = NULL;
                new_block_r->prev = wsk;
                wsk->next = new_block_r;

                new_block_r->sum_control = 0;
                new_block_r->sum_control = fun_sum_control(new_block_r);
                wsk->sum_control = 0;
                wsk->sum_control = fun_sum_control(wsk);
                memset((uint8_t *) new_block_r + size_ch + new_block_r->size + FENCE, '#', FENCE);

                heap_free((uint8_t*)block_realloc + size_ch + FENCE);

                memory_manager.memory_size += -size - size_ch - 2*FENCE;
                typ_pointer = get_pointer_type((uint8_t *) new_block_r + size_ch + FENCE);
                if (typ_pointer == pointer_valid)
                    return (uint8_t *) new_block_r + size_ch + FENCE;
            }
        }
    }
    return NULL;
}

void heap_free(void* memblock)
{
    typ_pointer = get_pointer_type(memblock);
    if(typ_pointer != pointer_valid)
        return;

    int free_size = 0, size_true = 0, unused = 0;
    struct memory_chunk_t *block = (struct memory_chunk_t*) ((uint8_t*)memblock - size_ch - FENCE);
    if(block->free == 1) return;

    memset((uint8_t*)block + size_ch,'?', FENCE);
    memset((uint8_t*)block + size_ch + FENCE + block->size,'?', FENCE);

    block->free = 1;
    free_size = block->size + 2*FENCE;

    if(block->next) {
        uint8_t *wsk = (uint8_t*)block + size_ch + 2*FENCE + block->size;
        while(wsk < (uint8_t*)block->next){
            unused++;
            wsk++;
        }
        block->size += unused;
        free_size += unused;
    }
    size_true = free_size;

    if((block->prev != NULL && block->next != NULL) && (block->prev->free == 1 && block->next->free == 1)){
        free_size += 2*size_ch;
        size_true += size_ch + block->prev->size;
        size_true += size_ch + block->next->size;
        if (block->next->next == NULL && block->prev->prev != NULL) // ostatnie bloki Ffff...'Bbbb'...
        {
            free_size += size_ch;
            block->prev->prev->next = NULL;
            block->prev->prev->sum_control = 0;
            block->prev->prev->sum_control = fun_sum_control(block->prev->prev);
            memory_manager.memory_size += free_size;
            return;
        }
        block->prev->size = size_true;
        block->prev->next = block->next->next;
        if(block->next->next != NULL) {
            block->next->next->prev = block->prev;
            block->next->next->sum_control = 0;
            block->next->next->sum_control = fun_sum_control(block->next->next);
        }
        block->prev->sum_control = 0;
        block->prev->sum_control = fun_sum_control(block->prev);
    }
    else if(block->prev != NULL && block->prev->free == 1){    //Fff...'Ccc'
        free_size += size_ch;
        size_true += size_ch + block->prev->size;
        if(block->next == NULL && block->prev->prev != NULL) //Fff...'Ccc'...
        {
            free_size += size_ch;
            block->prev->prev->next = NULL;
            block->prev->prev->sum_control = 0;
            block->prev->prev->sum_control = fun_sum_control(block->prev->prev);
            memory_manager.memory_size += free_size;
            return;
        }
        block->prev->size = size_true;
        block->prev->next = block->next;
        if(block->next != NULL) {
            block->next->prev = block->prev;
            block->next->sum_control = 0;
            block->next->sum_control = fun_sum_control(block->next);
        }
        block->prev->sum_control = 0;
        block->prev->sum_control = fun_sum_control(block->prev);
    }
    else if(block->next != NULL && block->next->free == 1){ //FffBbb'Ccc'D...
        free_size += size_ch;
        size_true += size_ch + block->next->size;
        if(block->next->next == NULL && block->prev != NULL)  //FffBbb'Ccc'...
        {
            free_size += size_ch;
            block->prev->next = NULL;
            block->prev->sum_control = 0;
            block->prev->sum_control = fun_sum_control(block->prev);
            memory_manager.memory_size += free_size;
            return;
        }
        block->size = size_true;
        if(block->next->next != NULL) {
            block->next->next->prev = block;
            block->next->next->sum_control = 0;
            block->next->next->sum_control = fun_sum_control(block->next->next);
        }
        block->next = block->next->next;
    }
    if(block->next == NULL && block->prev != NULL && block->prev->free == 0) //FffBbb'Ccc'...
    {
        free_size += size_ch;
        block->prev->next = NULL;
        block->prev->sum_control = 0;
        block->prev->sum_control = fun_sum_control(block->prev);
    }
    block->size = size_true;
    int fre = 0, n_block = 0;
    struct memory_chunk_t *ptr = memory_manager.first_memory_chunk;
    while (ptr){
        if(ptr->free == 1)
            fre++;
        n_block++;
        ptr = ptr->next;
    }
    if(fre == n_block) {
        free_size += size_ch;
        memory_manager.first_memory_chunk = NULL;
    }
    block->sum_control = 0;
    block->sum_control = fun_sum_control(block);

    memory_manager.memory_size += free_size - unused;
}

int fun_sum_control(const struct memory_chunk_t* block)
{
    int s = 0;
    uint8_t *p = (uint8_t*) block;
    for (int i = 0; i < (int)size_ch; i++) {
        s+= *p++;
    }
    return s;
}

size_t heap_get_largest_used_block_size(void)
{
    if(memory_manager.memory_start == NULL || memory_manager.first_memory_chunk == NULL) return 0;
    int val = heap_validate();
    if(val == 1 || val == 2 || val == 3) return 0;
    size_t size = 0;
    struct memory_chunk_t *the_largest_block = memory_manager.first_memory_chunk;

    while (the_largest_block && the_largest_block->free == 1) {
        the_largest_block = the_largest_block->next;
    }
    if(the_largest_block) {
        size = the_largest_block->size;
        while (the_largest_block) {
            if (the_largest_block->next)
                if (the_largest_block->next->free == 0 && the_largest_block->next->size > size)
                    size = the_largest_block->next->size;

            the_largest_block = the_largest_block->next;
        }
    }
    return size;
}

enum pointer_type_t get_pointer_type(const void* const pointer)
{
    if(pointer == NULL) return pointer_null;
    int value = heap_validate();
    if(value == 1 || value == 2 || value == 3)  return pointer_heap_corrupted;
    if(memory_manager.first_memory_chunk == NULL) return pointer_unallocated;;

    uint8_t *point = (uint8_t*)pointer;
    int dist = 0;
    struct memory_chunk_t *block = memory_manager.first_memory_chunk;

    while (block)
    {
        dist = point - (uint8_t*)block;

        int start = 0;
        if (dist >= start && dist < (int)size_ch)
            return pointer_control_block;

        start = size_ch;
        if (dist >= start && dist < start + FENCE && *(char*)pointer == '#')
            return pointer_inside_fences;

        start += FENCE;
        if((dist == start) && (block->free == 0))
            return pointer_valid;
        else if (dist > start && (dist < start + (int)block->size) && (block->free == 0))
            return pointer_inside_data_block;

        start += block->size;
        if (dist >= start && (dist < start + FENCE ) && *(char*)pointer == '#')
            return pointer_inside_fences;

        block = block->next;
    }
    return pointer_unallocated;
}

int heap_validate(void)
{
    if (memory_manager.memory_start == NULL) return 2;
    if(memory_manager.first_memory_chunk == NULL) return 0;

    struct memory_chunk_t *heap = memory_manager.first_memory_chunk;
    int l = 0;

    while(heap) {
        struct memory_chunk_t temp;
        memcpy(&temp,heap, size_ch);
        temp = *heap;

        int sum_b = 0;
        temp.sum_control = 0;
        uint8_t *b = (uint8_t*)&temp;
        for (int i = 0; i < (int)size_ch; i++) {
            sum_b += *b;
            b++;
        }
        if(heap->sum_control != sum_b)
            return 3;

        if(heap->free == 1){
            heap = heap->next;
            continue;
        }
        char *wsk = (char *)heap;
        wsk = wsk + size_ch;
        while (*wsk == '#' && l < FENCE) {
            l++;
            wsk++;
        }
        if (l != FENCE)
            return 1;

        while (*(wsk + heap->size) == '#' && l < 2*FENCE) {
            l++;
            wsk++;
        }
        if (l != 2*FENCE)
            return 1;

        heap = heap->next;
        l = 0;
    }
    return 0;
}

int unused_size(void* memblock)
{
    struct memory_chunk_t *block = (struct memory_chunk_t*) ((uint8_t*)memblock - size_ch - FENCE);
    int unused = 0;
    if(block->next) {
        uint8_t *wsk = (uint8_t*)block + size_ch + 2*FENCE + block->size;
        while(wsk < (uint8_t*)block->next){
            unused++;
            wsk++;
        }
    }
    return unused;
}

void* heap_malloc_aligned(size_t count)
{
    size_t size = count;
    if(size <= 0 ) return NULL;
    size_t size_free_L = 0,size_free_R = 0,size_L = 0,free_R = 0;

    struct memory_chunk_t *block = memory_manager.first_memory_chunk;
    uint8_t *ptr = NULL;
    short flag = 0;
    int ile = 0;
    while (block) {
        ile = 0;
        flag = 0;
        size_L = 0;
        free_R = 0;
        if (block->free == 1) {
            if (size + 2*FENCE < block->size)
            {
                ptr = (uint8_t *)block + size_ch + FENCE;
                while (ptr < ((uint8_t*)block + size_ch + block->size - FENCE)) {
                    if (((intptr_t)ptr & (intptr_t)(PAGE - 1)) == 0) {
                        flag = 1;
                        break;
                    }
                    ptr++;
                    ile++;
                }
                if(flag == 1){
                    if((struct memory_chunk_t*)((uint8_t*)ptr - size_ch - FENCE) == block->next){
                        flag = 0;
                        block = block->next;
                        continue;
                    }
                    size_free_L = (uint8_t *)ptr - (uint8_t *)block;
                    size_free_R = block->size - size_free_L;

                    size_L = size_free_L - size_ch - FENCE;
                    if (size_L > 0 && size_L < size_ch){
                        block = block->next;
                        flag = 0;
                        continue;
                    }
                    else if (size_L > size_ch)
                        size_L = size_L - size_ch;

                    free_R = 0;
                    if (block->next == NULL)
                        free_R = 0;
                    else if (block->size >= (size_free_L + size + FENCE + size_ch))
                        free_R = block->size - (size_free_L + size + FENCE + size_ch);
                    else {
                        block = block->next;
                        flag = 0;
                        continue;
                    }
                    if (flag == 1)
                        break;
                }
            }
        }
        block = block->next;
    }

    if (memory_manager.first_memory_chunk == NULL || flag == 0)
    {
        size_t siz_need = size + size_ch + 2*FENCE;
        siz_need = align_size(siz_need);

        void *ptr_sbrk = custom_sbrk(siz_need);
        if(ptr_sbrk == (void*)-1)
            return NULL;
        memory_manager.memory_size += siz_need;
    }

    struct memory_chunk_t *first_block = memory_manager.first_memory_chunk;

    if(first_block == NULL){
        first_block = (struct memory_chunk_t*)memory_manager.memory_start;
        first_block->free = 1;
        first_block->size = PAGE - 2 * size_ch - FENCE;
        first_block->prev = NULL;
        first_block->next = NULL;
        first_block->sum_control = 0;
        first_block->sum_control = fun_sum_control(first_block);

        memory_manager.memory_size -= size_ch;
        memory_manager.first_memory_chunk = first_block;
    }

    if(flag == 1){
        struct memory_chunk_t block_ok;
        memcpy(&block_ok,block,size_ch);

        struct memory_chunk_t *block_aligned = (struct memory_chunk_t*)((uint8_t*)ptr - size_ch - FENCE);

        if(size_free_L == size_ch + FENCE && size_free_R == size + FENCE){
            block->size = size;
            block->free = 0;
            memset((uint8_t*)block+ size_ch,'#', FENCE);
            memset((uint8_t*)block + size_ch + size + FENCE,'#', FENCE);

            block->sum_control = 0;
            block->sum_control = fun_sum_control(block);

            memory_manager.memory_size += -size - 2*FENCE;
            typ_pointer = get_pointer_type((uint8_t*)block + size_ch + FENCE);
            if(typ_pointer == pointer_valid)
                return (uint8_t*)block + size_ch + FENCE;

            return NULL;
        }
        else {
            block_aligned->size = size;
            block_aligned->free = 0;
            memset((uint8_t*)block_aligned+ size_ch,'#', FENCE);
            memset((uint8_t*)block_aligned + size_ch + size + FENCE,'#', FENCE);

            if (size_L > 0) {
                if(block->prev && block->prev->free == 1){
                    block->prev->size += size_L + size_ch;
                    block_aligned->next = block->next;
                    block->next->prev = block_aligned;

                    block->prev->next = block_aligned;
                    block_aligned->prev = block->prev;

                    block->prev->sum_control = 0;
                    block->prev->sum_control = fun_sum_control(block->prev);
                    block_aligned->next->sum_control = 0;
                    block_aligned->next->sum_control = fun_sum_control(block_aligned->next);

                    memory_manager.memory_size += size_L + size_ch;
                }
                else {
                    block->size = size_L;
                    block->free = 1;
                    block_aligned->next = block->next;
                    block->next->prev = block_aligned;

                    block_aligned->prev = block;
                    block->next = block_aligned;

                    block->sum_control = 0;
                    block->sum_control = fun_sum_control(block);
                    block_aligned->next->sum_control = 0;
                    block_aligned->next->sum_control = fun_sum_control(block_aligned->next);
                }
            }

            if (free_R > 0 && block_ok.next && block_ok.next->free == 0) {

                struct memory_chunk_t *block_free_R = (struct memory_chunk_t *) ((uint8_t *) ptr + size + FENCE);
                block_free_R->size = free_R;
                block_free_R->free = 1;

                if (size_free_L == size_ch + FENCE) {
                    block_free_R->next = block->next;
                    block->next->prev = block_free_R;
                    block_free_R->prev = block_aligned;
                    block_aligned->next = block_free_R;

                    block_free_R->sum_control = 0;
                    block_free_R->sum_control = fun_sum_control(block_free_R);
                    block_free_R->next->sum_control = 0;
                    block_free_R->next->sum_control = fun_sum_control(block_free_R->next);
                }
                if (size_L > 0) {
                    block_free_R->next = block_aligned->next;
                    block_aligned->next->prev = block_free_R;
                    block_free_R->prev = block_aligned;
                    block_aligned->next = block_free_R;

                    block_free_R->sum_control = 0;
                    block_free_R->sum_control = fun_sum_control(block_free_R);
                    block_free_R->next->sum_control = 0;
                    block_free_R->next->sum_control = fun_sum_control(block_free_R->next);
                }
                memory_manager.memory_size += -size_ch;
            }
            block_aligned->sum_control = 0;
            block_aligned->sum_control = fun_sum_control(block_aligned);

            if(block_aligned != block)
                memory_manager.memory_size += -size_ch;
            memory_manager.memory_size += - size - 2*FENCE;

            typ_pointer = get_pointer_type((uint8_t*)block_aligned + size_ch + FENCE);
            if(typ_pointer == pointer_valid)
                return (uint8_t*)block_aligned + size_ch + FENCE;
            return NULL;
        }
    }
    else{
        struct memory_chunk_t *the_last_block = memory_manager.first_memory_chunk;
        while (the_last_block->next) {
            the_last_block = the_last_block->next;
        }
        size_t offset = (uint8_t *)the_last_block - (uint8_t *)memory_manager.memory_start;
        offset = offset + the_last_block->size + size_ch;
        if (the_last_block->free == 0)
            offset = offset + 2*FENCE;

        if ((align_size(offset) - offset < size_ch + FENCE))
        {
            void* ptr_sbrk = custom_sbrk(PAGE);
            if(ptr_sbrk == (void*)-1)
                return NULL;
            memory_manager.memory_size += PAGE;
            offset = offset + PAGE;
        }

        offset = align_size(offset);
        offset += -size_ch - FENCE;

        void* ptr_sbrk_last = custom_sbrk(0);
        if((uint8_t*)memory_manager.memory_start + offset + size_ch + 2*FENCE + size >= (uint8_t*)ptr_sbrk_last)
            return NULL;

        struct memory_chunk_t* next_block = (struct memory_chunk_t*) ((uint8_t*)memory_manager.memory_start + offset);
        memset((uint8_t*)next_block + size_ch,'#', FENCE);
        memset((uint8_t*)next_block + size_ch + size + FENCE,'#', FENCE);
        next_block->size = size;
        next_block->free = 0;
        next_block->next = NULL;
        next_block->prev = the_last_block;
        the_last_block->next = next_block;

        next_block->sum_control = 0;
        next_block->sum_control = fun_sum_control(next_block);
        the_last_block->sum_control = 0;
        the_last_block->sum_control = fun_sum_control(the_last_block);

        memory_manager.memory_size += -size - size_ch - 2*FENCE;

        typ_pointer = get_pointer_type((uint8_t*)next_block + size_ch + FENCE);
        if(typ_pointer == pointer_valid)
            return (uint8_t*)next_block + size_ch + FENCE;
        return NULL;
    }
}

void* heap_calloc_aligned(size_t number, size_t size_of)
{
    if(number <= 0 || size_of <= 0) return NULL;
    size_t size = number * size_of;
    size_t size_free_L = 0, size_free_R = 0, size_L = 0, free_R = 0;
    struct memory_chunk_t *block = memory_manager.first_memory_chunk;

    uint8_t *ptr = NULL;
    short flag = 0;
    int ile = 0;
    while (block) {
        ile = 0;
        flag = 0;
        size_L = 0;
        free_R = 0;
        if (block->free == 1) {
            if (size + 2*FENCE < block->size)
            {
                ptr = (uint8_t *)block + size_ch + FENCE;
                while (ptr < ((uint8_t*)block + size_ch + block->size - FENCE)) {
                    if (((intptr_t)ptr & (intptr_t)(PAGE - 1)) == 0) {
                        flag = 1;
                        break;
                    }
                    ptr++;
                    ile++;
                }
                if(flag == 1){
                    if((struct memory_chunk_t*)((uint8_t*)ptr - size_ch - FENCE) == block->next){
                        flag = 0;
                        block = block->next;
                        continue;
                    }
                    size_free_L = (uint8_t *)ptr - (uint8_t *)block;
                    size_free_R = block->size - size_free_L;

                    size_L = size_free_L - size_ch - FENCE;
                    if (size_L > 0 && size_L < size_ch){
                        block = block->next;
                        flag = 0;
                        continue;
                    }
                    else if (size_L > size_ch)
                        size_L = size_L - size_ch;

                    free_R = 0;
                    if (block->next == NULL)
                        free_R = 0;
                    else if (block->size >= (size_free_L + size + FENCE + size_ch))
                        free_R = block->size - (size_free_L + size + FENCE + size_ch);
                    else {
                        block = block->next;
                        flag = 0;
                        continue;
                    }
                    if (flag == 1)
                        break;
                }
            }
        }
        block = block->next;
    }

    if (memory_manager.first_memory_chunk == NULL || flag == 0)
    {
        size_t siz_need = size + size_ch + 2*FENCE;
        siz_need = align_size(siz_need);

        void *ptr_sbrk = custom_sbrk(siz_need);
        if(ptr_sbrk == (void*)-1)
            return NULL;
        memory_manager.memory_size += siz_need;
    }

    struct memory_chunk_t *first_block = memory_manager.first_memory_chunk;

    if(first_block == NULL){
        first_block = (struct memory_chunk_t*)memory_manager.memory_start;
        first_block->free = 1;
        first_block->size = PAGE - 2 * size_ch - FENCE;
        first_block->prev = NULL;
        first_block->next = NULL;
        first_block->sum_control = 0;
        first_block->sum_control = fun_sum_control(first_block);

        memory_manager.memory_size -= size_ch;
        memory_manager.first_memory_chunk = first_block;
    }

    if(flag == 1){
        struct memory_chunk_t block_ok;
        memcpy(&block_ok,block,size_ch);

        struct memory_chunk_t *block_aligned = (struct memory_chunk_t*)((uint8_t*)ptr - size_ch - FENCE);

        if(size_free_L == size_ch + FENCE && size_free_R == size + FENCE){
            block->size = size;
            block->free = 0;
            memset((uint8_t*)block + size_ch,'#', FENCE);
            memset((uint8_t*)block + size_ch + FENCE,0, size);
            memset((uint8_t*)block + size_ch + size + FENCE,'#', FENCE);

            block->sum_control = 0;
            block->sum_control = fun_sum_control(block);

            memory_manager.memory_size += -size - 2*FENCE;
            typ_pointer = get_pointer_type((uint8_t*)block + size_ch + FENCE);
            if(typ_pointer == pointer_valid)
                return (uint8_t*)block + size_ch + FENCE;
            return NULL;
        }
        else {
            block_aligned->size = size;
            block_aligned->free = 0;
            memset((uint8_t*)block_aligned+ size_ch,'#', FENCE);
            memset((uint8_t*)block_aligned + size_ch + FENCE,0, size);
            memset((uint8_t*)block_aligned + size_ch + size + FENCE,'#', FENCE);

            if (size_L > 0) {
                if(block->prev && block->prev->free == 1){
                    block->prev->size += size_L + size_ch;
                    block_aligned->next = block->next;
                    block->next->prev = block_aligned;

                    block->prev->next = block_aligned;
                    block_aligned->prev = block->prev;

                    block->prev->sum_control = 0;
                    block->prev->sum_control = fun_sum_control(block->prev);
                    block_aligned->next->sum_control = 0;
                    block_aligned->next->sum_control = fun_sum_control(block_aligned->next);

                    memory_manager.memory_size += size_L + size_ch;
                }
                else {
                    block->size = size_L;
                    block->free = 1;

                    block_aligned->next = block->next;
                    block->next->prev = block_aligned;

                    block_aligned->prev = block;
                    block->next = block_aligned;

                    block->sum_control = 0;
                    block->sum_control = fun_sum_control(block);
                    block_aligned->next->sum_control = 0;
                    block_aligned->next->sum_control = fun_sum_control(block_aligned->next);
                }
            }

            if (free_R > 0 && block_ok.next && block_ok.next->free == 0) {

                struct memory_chunk_t *block_free_R = (struct memory_chunk_t *) ((uint8_t *) ptr + size + FENCE);
                block_free_R->size = free_R;
                block_free_R->free = 1;

                if (size_free_L == size_ch + FENCE) {
                    block_free_R->next = block->next;
                    block->next->prev = block_free_R;
                    block_free_R->prev = block_aligned;
                    block_aligned->next = block_free_R;

                    block_free_R->sum_control = 0;
                    block_free_R->sum_control = fun_sum_control(block_free_R);
                    block_free_R->next->sum_control = 0;
                    block_free_R->next->sum_control = fun_sum_control(block_free_R->next);
                }
                if (size_L > 0) {
                    block_free_R->next = block_aligned->next;
                    block_aligned->next->prev = block_free_R;
                    block_free_R->prev = block_aligned;
                    block_aligned->next = block_free_R;

                    block_free_R->sum_control = 0;
                    block_free_R->sum_control = fun_sum_control(block_free_R);
                    block_free_R->next->sum_control = 0;
                    block_free_R->next->sum_control = fun_sum_control(block_free_R->next);
                }
                memory_manager.memory_size += -size_ch;
            }
            block_aligned->sum_control = 0;
            block_aligned->sum_control = fun_sum_control(block_aligned);

            if(block_aligned != block)
                memory_manager.memory_size += -size_ch;
            memory_manager.memory_size += - size - 2*FENCE;

            typ_pointer = get_pointer_type((uint8_t*)block_aligned + size_ch + FENCE);
            if(typ_pointer == pointer_valid)
                return (uint8_t*)block_aligned + size_ch + FENCE;

            return NULL;
        }
    }
    else{
        struct memory_chunk_t *the_last_block = memory_manager.first_memory_chunk;
        while (the_last_block->next) {
            the_last_block = the_last_block->next;
        }

        size_t offset = (uint8_t *)the_last_block - (uint8_t *)memory_manager.memory_start;
        offset = offset + the_last_block->size + size_ch;
        if (the_last_block->free == 0)
            offset = offset + 2*FENCE;

        if ((align_size(offset) - offset < size_ch + FENCE))
        {
            void* ptr_sbrk = custom_sbrk(PAGE);
            if(ptr_sbrk == (void*)-1)
                return NULL;
            memory_manager.memory_size += PAGE;
            offset = offset + PAGE;
        }

        offset = align_size(offset);
        offset += -size_ch - FENCE;

        void* ptr_sbrk_last = custom_sbrk(0);
        if((uint8_t*)memory_manager.memory_start + offset + size_ch + 2*FENCE + size >= (uint8_t*)ptr_sbrk_last)
            return NULL;

        struct memory_chunk_t* next_block = (struct memory_chunk_t*) ((uint8_t*)memory_manager.memory_start + offset);
        memset((uint8_t*)next_block + size_ch,'#', FENCE);
        memset((uint8_t*)next_block + size_ch + FENCE,0, size);
        memset((uint8_t*)next_block + size_ch + size + FENCE,'#', FENCE);
        next_block->size = size;
        next_block->free = 0;
        next_block->next = NULL;
        next_block->prev = the_last_block;
        the_last_block->next = next_block;

        next_block->sum_control = 0;
        next_block->sum_control = fun_sum_control(next_block);
        the_last_block->sum_control = 0;
        the_last_block->sum_control = fun_sum_control(the_last_block);

        memory_manager.memory_size += -size - size_ch - 2*FENCE;

        typ_pointer = get_pointer_type((uint8_t*)next_block + size_ch + FENCE);
        if(typ_pointer == pointer_valid)
            return (uint8_t*)next_block + size_ch + FENCE;
        return NULL;
    }
}

void* heap_realloc_aligned(void* memblock, size_t size)
{
    if(memory_manager.memory_start == NULL) return NULL;
    if(size == 0){
        heap_free(memblock);
        return NULL;
    }
    uint8_t *ptr = NULL;

    size_t size_free_L = 0,size_free_R = 0, size_L = 0,free_R = 0;;
    int nextfree_R = 0;
    short flag = 0, flagfind = 0;

    typ_pointer = get_pointer_type(memblock);
    if(typ_pointer == pointer_valid || typ_pointer == pointer_null)
    {
        if (memblock == NULL) {
            void *m = heap_malloc_aligned(size);
            return (uint8_t *) m;
        }
        int unused_memb = unused_size(memblock);

        nextfree_R = 0;
        struct memory_chunk_t *myblock = (struct memory_chunk_t *) ((uint8_t *)memblock - FENCE - size_ch);
        if (myblock->next)
        {
            if (myblock->next->free == 1)
                nextfree_R = myblock->next->size + size_ch;
        }
        ptr = (uint8_t *)myblock + size_ch + FENCE; //xl +FENCE

        while (ptr < ((uint8_t*)myblock + size_ch + FENCE + myblock->size + FENCE + unused_memb + nextfree_R - FENCE)) {
            flag = 0;
            if (((intptr_t)ptr & (intptr_t)(PAGE - 1)) == 0) {
                flag = 1;

                size_free_L = (uint8_t *)ptr - (uint8_t *)myblock;
                size_free_R = myblock->size - size_free_L;

                size_L = size_free_L - size_ch - FENCE;
                if (size_L > 0 && size_L < size_ch){
                    flag = 0;
                    ptr += PAGE;
                    continue;
                }
                else if (size_L > size_ch)
                    size_L = size_L - size_ch;

                free_R = 0;
                if (myblock->next == NULL)
                    free_R = 0;
                else if (myblock->size + unused_memb + nextfree_R >= (size_free_L + size + FENCE + size_ch))// 2*FENCE
                    free_R = myblock->size + unused_memb + nextfree_R - (size_free_L + size + FENCE + size_ch);// +
                else {
                    flag = 0;
                    ptr += PAGE;
                    continue;
                }
                if (flag == 1) {
                    break;
                }
            }
            ptr++;
        }

        struct memory_chunk_t *block_sea = NULL;
        if (flag != 1) {
            flagfind = 0;
            block_sea = memory_manager.first_memory_chunk;

            while (block_sea) {
                flagfind = 0;
                size_L = 0;
                free_R = 0;

                if (block_sea->free == 1 && block_sea->size >= size){
                    ptr = (uint8_t *)block_sea + size_ch + FENCE;
                    nextfree_R = 0;

                    while (ptr < ((uint8_t*)block_sea + size_ch + block_sea->size - FENCE)) {
                        if (((intptr_t)ptr & (intptr_t)(PAGE - 1)) == 0) {
                            flagfind = 1;
                            break;
                        }
                        ptr++;
                    }
                    if(flagfind == 1){
                        if((struct memory_chunk_t*)((uint8_t*)ptr - size_ch - FENCE) == block_sea->next){
                            flagfind = 0;
                            block_sea = block_sea->next;
                            continue;
                        }
                        size_free_L = (uint8_t *)ptr - (uint8_t *)block_sea;
                        size_free_R = block_sea->size - size_free_L;

                        size_L = size_free_L - size_ch - FENCE;
                        if (size_L > 0 && size_L < size_ch){
                            block_sea = block_sea->next;
                            flagfind = 0;
                            continue;
                        }
                        else if (size_L > size_ch)
                            size_L = size_L - size_ch;

                        free_R = 0;
                        if (block_sea->next == NULL)
                            free_R = 0;
                        else if (block_sea->size >= (size_free_L + size + FENCE + size_ch))// +
                            free_R = block_sea->size - (size_free_L + size + FENCE + size_ch);// +
                        else {
                            block_sea = block_sea->next;
                            flagfind = 0;
                            continue;
                        }
                        if (flagfind == 1) {
                            break;
                        }

                    }
                }
                block_sea = block_sea->next;
            }
        }
        struct memory_chunk_t *block = NULL;

        if (flag == 1)
            block = myblock;
        else if (flagfind == 1)
            block = block_sea;
        else
            block = NULL;

        if (block != NULL) {
            struct memory_chunk_t block_ok;
            memcpy(&block_ok,block,size_ch);
            struct memory_chunk_t *block_aligned = (struct memory_chunk_t*)((uint8_t*)ptr - size_ch - FENCE);

            if(block->next == NULL){
                if(block->size < size){
                    size_t siz_need = size - block->size;
                    siz_need = align_size(siz_need);

                    void *ptr_sbrk = custom_sbrk(siz_need);
                    if(ptr_sbrk == (void*)-1)
                        return NULL;
                    memory_manager.memory_size += siz_need;
                }

                memset((uint8_t*)block + size_ch + FENCE + block->size,'a', FENCE);
                block->size = size;
                block->free = 0;
                memset((uint8_t*)block + size_ch + FENCE + size,'#', FENCE);

                block->sum_control = 0;
                block->sum_control = fun_sum_control(block);

                memory_manager.memory_size += -size - 2*FENCE;
                typ_pointer = get_pointer_type((uint8_t*)block + size_ch + FENCE);
                if(typ_pointer == pointer_valid)
                    return (uint8_t*)block + size_ch + FENCE;
                return NULL;
            }

            if(size_free_L == size_ch + FENCE && size_free_R == size + FENCE){
                memset((uint8_t*)block + size_ch + FENCE + block->size,'a', FENCE);
                block->size = size;
                block->free = 0;

                memset((uint8_t*)block + size_ch + FENCE + size,'#', FENCE);

                block->sum_control = 0;
                block->sum_control = fun_sum_control(block);

                memory_manager.memory_size += -size - 2*FENCE;
                typ_pointer = get_pointer_type((uint8_t*)block + size_ch + FENCE);
                if(typ_pointer == pointer_valid)
                    return (uint8_t*)block + size_ch + FENCE;

                return NULL;
            }
            else {
                if(size_free_L == size_ch + FENCE && myblock->size >= size){
                    memory_manager.memory_size += myblock->size - size;
                    memset((uint8_t*)block + size_ch + FENCE + block->size ,'a', FENCE);

                    block->size = size;
                    block->free = 0;
                    memset((uint8_t*)block + size_ch,'#', FENCE);
                    memset((uint8_t*)block + size_ch + FENCE + size ,'#', FENCE);
                    block->sum_control = 0;
                    block->sum_control = fun_sum_control(block);

                    typ_pointer = get_pointer_type((uint8_t*)block + size_ch + FENCE);
                    if(typ_pointer == pointer_valid)
                        return (uint8_t*)block + size_ch + FENCE;
                    return NULL;
                }

                block_aligned->size = size;
                block_aligned->free = 0;
                memset((uint8_t*)block_aligned + size_ch,'#', FENCE);
                if(block == myblock && myblock->free == 0)
                    memcpy((uint8_t*)block_aligned + size_ch + FENCE, (uint8_t*)myblock + size_ch + FENCE , block_ok.size);

                if (size_L > 0) {
                    if(block->prev && block->prev->free == 1){
                        block->prev->size += size_L + size_ch;
                        block_aligned->next = block->next;
                        block->next->prev = block_aligned;

                        block->prev->next = block_aligned;
                        block_aligned->prev = block->prev;

                        block->prev->sum_control = 0;
                        block->prev->sum_control = fun_sum_control(block->prev);
                        block_aligned->next->sum_control = 0;
                        block_aligned->next->sum_control = fun_sum_control(block_aligned->next);

                        memory_manager.memory_size += size_L + size_ch;

                    }
                    else {
                        block->size = size_L;
                        block->free = 1;
                        block_aligned->next = block->next;
                        block->next->prev = block_aligned;

                        block_aligned->prev = block;
                        block->next = block_aligned;

                        block->sum_control = 0;
                        block->sum_control = fun_sum_control(block);
                        block_aligned->next->sum_control = 0;
                        block_aligned->next->sum_control = fun_sum_control(block_aligned->next);
                    }

                }

                if (free_R > 0 && block_ok.next && block_ok.next->free == 0) {
                    struct memory_chunk_t *block_free_R = (struct memory_chunk_t *) ((uint8_t *) ptr + size +
                                                                                     FENCE);
                    block_free_R->size = free_R;
                    block_free_R->free = 1;

                    if (size_free_L == size_ch + FENCE) {
                        block_free_R->next = block->next;
                        block->next->prev = block_free_R;
                        block_free_R->prev = block_aligned;
                        block_aligned->next = block_free_R;

                        block_free_R->sum_control = 0;
                        block_free_R->sum_control = fun_sum_control(block_free_R);
                        block_free_R->next->sum_control = 0;
                        block_free_R->next->sum_control = fun_sum_control(block_free_R->next);
                    }
                    if (size_L > 0) {
                        block_free_R->next = block_aligned->next;
                        block_aligned->next->prev = block_free_R;
                        block_free_R->prev = block_aligned;
                        block_aligned->next = block_free_R;

                        block_free_R->sum_control = 0;
                        block_free_R->sum_control = fun_sum_control(block_free_R);
                        block_free_R->next->sum_control = 0;
                        block_free_R->next->sum_control = fun_sum_control(block_free_R->next);
                    }
                    memory_manager.memory_size += -size_ch;
                }
                else if(free_R > 0 && block_ok.next && block_ok.next->free == 1 && block_ok.next->next)
                {
                    block_aligned->next = block_ok.next->next;
                    block_ok.next->next->prev = block_aligned;

                    block_aligned->next->sum_control = 0;
                    block_aligned->next->sum_control = fun_sum_control(block_aligned->next);
                }
                memset((uint8_t*)block_aligned + size_ch + FENCE + size ,'#', FENCE);
                block_aligned->free = 0;
                block_aligned->sum_control = 0;
                block_aligned->sum_control = fun_sum_control(block_aligned);

                if(block_aligned != block)
                    memory_manager.memory_size += -size_ch;
                memory_manager.memory_size += - size - 2*FENCE;

                if (block != myblock)
                    heap_free((uint8_t*)myblock + size_ch + FENCE);

                typ_pointer = get_pointer_type((uint8_t*)block_aligned + size_ch + FENCE);
                if(typ_pointer == pointer_valid)
                    return (uint8_t*)block_aligned + size_ch + FENCE;
                return NULL;
            }
        }
        else{
            struct memory_chunk_t *the_last_block = memory_manager.first_memory_chunk;
            while (the_last_block->next) {
                the_last_block = the_last_block->next;
            }

            size_t offset = (uint8_t *)the_last_block - (uint8_t *)memory_manager.memory_start;
            offset = offset + the_last_block->size + size_ch;
            if (the_last_block->free == 0)
                offset = offset + 2*FENCE;

            if ((align_size(offset) - offset < size_ch + FENCE))
            {
                void* ptr_sbrk = custom_sbrk(PAGE);
                if(ptr_sbrk == (void*)-1)
                    return NULL;
                memory_manager.memory_size += PAGE;
                offset =  offset + PAGE;
            }
            if (flag == 0)
            {
                size_t siz_need = size + size_ch + 2*FENCE;
                siz_need = align_size(siz_need);

                void *ptr_sbrk = custom_sbrk(siz_need);
                if(ptr_sbrk == (void*)-1)
                    return NULL;
                memory_manager.memory_size += siz_need;
            }
            offset = align_size(offset);
            offset += -size_ch - FENCE;

            void* ptr_sbrk_last = custom_sbrk(0);
            if((uint8_t*)memory_manager.memory_start + offset + size_ch + 2*FENCE + size >= (uint8_t*)ptr_sbrk_last)
                return NULL;

            struct memory_chunk_t* next_block = (struct memory_chunk_t*) ((uint8_t*)memory_manager.memory_start + offset);
            memset((uint8_t*)next_block + size_ch,'#', FENCE);
            memcpy((uint8_t*)next_block + size_ch + FENCE, (uint8_t*)myblock + size_ch + FENCE, myblock->size);
            memset((uint8_t*)next_block + size_ch + size + FENCE,'#', FENCE);
            heap_free((uint8_t*)myblock + size_ch + FENCE);

            next_block->size = size;
            next_block->free = 0;
            next_block->next = NULL;
            next_block->prev = the_last_block;
            the_last_block->next = next_block;

            next_block->sum_control = 0;
            next_block->sum_control = fun_sum_control(next_block);
            the_last_block->sum_control = 0;
            the_last_block->sum_control = fun_sum_control(the_last_block);

            memory_manager.memory_size += -size - size_ch - 2*FENCE;

            typ_pointer = get_pointer_type((uint8_t*)next_block + size_ch + FENCE);
            if(typ_pointer == pointer_valid)
                return (uint8_t*)next_block + size_ch + FENCE;
            return NULL;
        }
    }
    return NULL;
}