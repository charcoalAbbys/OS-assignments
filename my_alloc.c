#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>


typedef struct s_block
{
    struct s_block *prev;
    struct s_block *next;
    int data_size;
} t_block;

typedef struct s_heap
{
    t_block *firstBlock;
    int block_count;
} t_heap;

t_heap *a;

int my_init()
{
    a = (t_heap *)mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON, -1, 0);
    if (a == NULL)
    {
        perror("mmap error. System did not allocate enough memory \n");
        return 1;
    }
    t_block *firstBlock = (t_block *)((void *)a + sizeof(t_heap)); // doubt
    firstBlock->next = NULL;
    firstBlock->prev = NULL;
    firstBlock->data_size = 4096 - (sizeof(t_heap)) - (sizeof(t_block));
    a->firstBlock = firstBlock;
    a->block_count = 0;
    return 0;
}

void *my_alloc(int count)
{
    if(count%8 != 0 || count <= 0){
        return NULL;
    }
    t_block *tra;
    tra = a->firstBlock;
    while (tra != NULL)
    {
        if (tra->data_size >= count)
        {
            break;
        }
        else
        {
            tra = tra->next;
        }
    }
    // No free block available with the given requirement
    if (tra == NULL)
    {
        return NULL;
    }

    if (tra->data_size > sizeof(t_block) + count)
    {
        // Spliting the free block 
        t_block *new_block = (t_block *)((void *)tra + (sizeof(t_block) + count));
        new_block->prev = tra->prev;
        new_block->next = tra->next;
        new_block->data_size = tra->data_size - count -sizeof(t_block);
        if(tra->prev != NULL){
            tra->prev->next = new_block;
        }
        else{
            new_block -> prev = NULL;
            a ->firstBlock = new_block;
        }
        if(tra->next != NULL){
            tra->next->prev = new_block;
        }
        
        tra -> next = NULL;
        tra -> data_size = count;
        tra -> prev = NULL;
    }
    else
    {
        // allocating the entire block for this malloc
        if(tra->next == NULL && tra->prev == NULL){
            a -> firstBlock = NULL;
        }
        else if (tra->next == NULL){
            tra->prev->next = NULL;
        }
        else if (tra->prev == NULL){
            a->firstBlock = tra->next;
            tra-> next -> prev = NULL;
        }
        else{
            tra ->prev->next = tra -> next;
            tra-> next->prev = tra -> prev;
        }
        tra -> next = NULL;
        tra -> prev = NULL;
    }

    a->block_count += 1;        // Maintaining the number of blocks allocated
    
    return (void *)tra + sizeof(t_block);
}

void my_free(void *ptr)
{
    // If ptr is NULL then we dont free anything.
    if(ptr == NULL){
        return;
    }

    a->block_count -= 1;

    t_block *temp = ptr - (sizeof(t_block));    // Location of the header of the used block to free.
    if(a->firstBlock == NULL){  // If the free list is empty
        temp->next = NULL;
        temp->prev = NULL;
        a -> firstBlock = temp;
        return;
    }

    // Finding the location of the current block in the free list by looping over the free list
    t_block *ptr1 = a->firstBlock;
    t_block *ptr2 = NULL;
    int found = 0;
    while(ptr1 != NULL){
        if(ptr1 > temp){
            found = 1;
            break;
        }
        ptr2 = ptr1;
        ptr1 = ptr1->next;
    }
    if(found == 1){
        if(ptr2 == NULL){
            // Case 1 : current block must be added at the very begining of the free list
            if((void *)temp + temp->data_size + (sizeof(t_block)) == (void *)ptr1){
                // Combine them
                if(ptr1->next == NULL){
                    temp->data_size += sizeof(t_block) + ptr1->data_size;
                    temp -> next = NULL;
                    temp -> prev = NULL;
                    a->firstBlock = temp;
                }
                else{
                    temp->data_size += sizeof(t_block) + ptr1->data_size;
                    temp -> next = ptr1 -> next;
                    ptr1 -> next -> prev = temp;
                    temp -> prev = NULL;
                    a->firstBlock = temp;   
                }
            }
            else{
                temp -> next = ptr1;
                ptr1 -> prev = temp;
                temp -> prev = NULL;
                a->firstBlock = temp;

            }
        }
        else{
            // Case 3 current block is added between two free blocks
            if(((void *)ptr2 + ptr2->data_size + (sizeof(t_block)) == (void *)temp) && ((void *)temp + temp->data_size + (sizeof(t_block)) == (void *)ptr1)){
                //combine from both next and prev
                ptr2 -> data_size += temp -> data_size + ptr1 -> data_size + 2*(sizeof(t_block));
                if(ptr1 -> next == NULL){
                    ptr2 -> next = NULL;
                }
                else{
                    ptr2 -> next = ptr1 -> next;
                    ptr1 -> next -> prev = ptr2;
                }
            }
            else if(((void *)temp + temp->data_size + (sizeof(t_block)) == (void *)ptr1)){
                // combine with next
                if(ptr1->next == NULL){
                    temp->data_size += sizeof(t_block) + ptr1->data_size;
                    temp -> next = NULL;
                    // temp -> prev = NULL;
                    temp -> prev = ptr2;
                    ptr2 -> next = temp;
                }
                else{
                    temp->data_size += sizeof(t_block) + ptr1->data_size;
                    temp -> next = ptr1 -> next;
                    ptr1 -> next -> prev = temp;
                    // temp -> prev = NULL;
                    temp -> prev = ptr2;
                    ptr2 -> next = temp;
                }
            }
            else if(((void *)ptr2 + ptr2->data_size + (sizeof(t_block)) == (void *)temp)){
                // combine with prev
                ptr2 -> data_size += temp -> data_size + sizeof(t_block);
            }
            else{
                temp->next = ptr1;
                temp->prev = ptr2;
                ptr2->next = temp;
                ptr1->prev = temp;
            }
        }
    }
    else{
        // Case 2 current block is being added at the end
        if((void *)ptr2 + ptr2->data_size + (sizeof(t_block)) == (void *)temp){
            //Combine
            ptr2 -> data_size += temp -> data_size + sizeof(t_block);
        }
        else{
            ptr2 -> next = temp;
            temp -> prev = ptr2;
            temp -> next = NULL;
        }
    }
    return;
}

void my_clean()
{
    // Deallocating memory using mummap
    munmap(a, 4096);
    return;
}

void my_heapinfo()
{
    int aa, b, c, d, e, f;
    aa = 4096 - sizeof(t_heap); // size(t_heap) is the size of the heap header
    d = a->block_count;         // i am maintaining this each time I allocate and free.
    c = 0;
    //calculate c, e and f here
    e = 4096;
    f = 0;
    t_block *tra = a->firstBlock;   //traversing through the free list to find the largest and the smallest free block.
    if(tra == NULL){
        e = 0;
        f = 0;
    }
    else{
        while (tra != NULL)
        {
            c += tra->data_size; 
            if (tra->data_size < e)
            {
                e = tra->data_size;
            }
            if (tra->data_size > f)
            {
                f = tra->data_size;
            }
            tra = tra->next;
        }
    }
    
    b = aa - c;    

    // Do not edit below output format
    printf("=== Heap Info ================\n");
    printf("Max Size: %d\n", aa);
    printf("Current Size: %d\n", b);
    printf("Free Memory: %d\n", c);
    printf("Blocks allocated: %d\n", d);
    printf("Smallest available chunk: %d\n", e);
    printf("Largest available chunk: %d\n", f);
    printf("==============================\n");
    // Do not edit above output format
    return;
}


// Fuction for printing the free list
void printList(){
    t_block *tra = a->firstBlock;
    printf("=== List Info ================\n");

    while(tra != NULL){
        printf("%d\t", tra->data_size);
        tra = tra->next;
    }
    printf("\n");
    printf("==============================\n");
    printf("\n");
}
