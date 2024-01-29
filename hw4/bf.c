#include <sys/mman.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

typedef struct block {
    size_t size;
    int free;
    struct block *prev;
    struct block *next;
}block;

block *head = NULL;

void *malloc(size_t size) {
    if (size) {
        size = (size + 31) / 32 * 32;

        // when head is NULL (first malloc), mmap to allocate 20000 bytes.
        if (head == NULL) {
            head = mmap(0, 20000, PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE, -1, 0);
            head->size = 20000 - 32;
            head->free = 1;
            head->prev = NULL;
            head->next = NULL;
        }

        // BF
        block *cur = head, *min_blk = NULL;
        size_t min_free = 20000;

        // find the min free block
        while (cur != NULL) {
            if (cur->free && cur->size >= size) {
                if (cur->size < min_free) {
                    min_free = cur->size;
                    min_blk = cur;
                }
            }
            cur = cur->next;
        }
        
        // no next block
        if (min_free == size) {
            min_blk->free = 0;
            return min_blk + 1;
        } else {
            block *new_free = min_blk + 1 + size/32;
            new_free->size = min_blk->size - size - 32;
            new_free->free = 1;
            new_free->prev = min_blk;
            new_free->next = min_blk->next;

            if (min_blk->next != NULL)
                min_blk->next->prev = new_free;

            min_blk->next = new_free;
            min_blk->free = 0;
            min_blk->size = size;

            return min_blk + 1;
        }
        return 0;
    } else {// end case

        block *cur = head;
        size_t max_size = 0;
        while(cur != NULL)
        {
            if(cur->size > max_size && cur->free)
                max_size = cur->size;
            cur = cur->next;
        }
        
        char str[100];
        memset(str, 0, 100);
        snprintf(str, 100, "Max Free Chunk Size = %ld\n", max_size);
        write(1, str, 100);

        munmap(head, 20000);
        return 0;
    }
}

void free(void *ptr){
    block *cur = ptr;
    cur -= 1;

    // merge left and current
    if (cur->prev && cur->prev->free) {
        cur->prev->size += cur->size + 32;
        if (cur->next)
            cur->next->prev = cur->prev;
        cur->prev->next = cur->next;
        cur = cur->prev; // move cur to left
    }

    // merge right and current
    if (cur->next && cur->next->free) {
        cur->size += cur->next->size + 32;
        if (cur->next->next)
            cur->next->next->prev = cur;
        cur->next = cur->next->next;
    }
 
    cur->free = 1;
    return;
}