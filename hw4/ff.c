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

        if (head == NULL) {
            head = mmap(0, 20000, PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE, -1, 0);
            head->size = 20000 - 32;
            head->free = 1;
            head->prev = NULL;
            head->next = NULL;
        }

        // FF
        block *cur = head;

        while (cur != NULL) {
            if (cur->free && cur->size > size) {
                // new block with head and free data
                block *new_free = cur + 1 + size/32;
                new_free->size = cur->size - size - 32;
                new_free->free = 1;
                new_free->prev = cur;
                new_free->next = cur->next;

                if (cur->next != NULL)
                    cur->next->prev = new_free;

                cur->next = new_free;
                cur->free = 0;
                cur->size = size;

                return cur + 1;
            }
            else if (cur->free && cur->size == size) {
                cur->free = 0;
                return cur + 1;
            }
            cur = cur->next;
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