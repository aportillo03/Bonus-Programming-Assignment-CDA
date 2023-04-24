#include <stdio.h>
#include <stdlib.h>

#define NUM_BLOCKS 32
#define BLOCK_SIZE 32
#define INT_MAX 2147483647

#define LRU_POLICY 0
#define RANDOM_POLICY 1

typedef struct {
    unsigned int tag;
    int valid;
    int counter;
} CacheBlock;

void init_cache(CacheBlock cache[], int num_blocks, int set_associativity);
void read_traces(const char* filename, CacheBlock cache[], int num_blocks, int set_associativity, int replacement_policy);
int access_cache(CacheBlock cache[], int num_blocks, unsigned int tag, int set_associativity, int replacement_policy);

int main() {
    
    // Define cache types
    CacheBlock direct_mapped[NUM_BLOCKS];
    CacheBlock two_way[NUM_BLOCKS / 2][2];
    CacheBlock four_way[NUM_BLOCKS / 4][4];
    CacheBlock* fully_associative = malloc(NUM_BLOCKS * sizeof(CacheBlock));

    // Initialize caches
    init_cache(direct_mapped, NUM_BLOCKS, 1);
    init_cache((CacheBlock*)two_way, NUM_BLOCKS / 2, 2);
    init_cache((CacheBlock*)four_way, NUM_BLOCKS / 4, 4);
    init_cache(fully_associative, NUM_BLOCKS, NUM_BLOCKS);
    
    // Read traces and access caches
    printf("Direct Mapped Cache:\n");
    read_traces("traces.txt", direct_mapped, NUM_BLOCKS, 1, LRU_POLICY);
//
    printf("\n2-Way Set Associative Cache (LRU):\n");
    read_traces("traces.txt", (CacheBlock*)two_way, NUM_BLOCKS / 2, 2, LRU_POLICY);
    printf("\n2-Way Set Associative Cache (Random):\n");
    read_traces("traces.txt", (CacheBlock*)two_way, NUM_BLOCKS / 2, 2, RANDOM_POLICY);
//
    printf("\n4-Way Set Associative Cache (LRU):\n");
    read_traces("traces.txt", (CacheBlock*)four_way, NUM_BLOCKS / 4, 4, LRU_POLICY);
    printf("\n4-Way Set Associative Cache (Random):\n");
    read_traces("traces.txt", (CacheBlock*)four_way, NUM_BLOCKS / 4, 4, RANDOM_POLICY);
//
    printf("\nFully Associative Cache (LRU):\n");
    read_traces("traces.txt", fully_associative, NUM_BLOCKS, NUM_BLOCKS, LRU_POLICY);
//
    printf("\nFully Associative Cache (Random):\n");
    read_traces("traces.txt", fully_associative, NUM_BLOCKS, NUM_BLOCKS, RANDOM_POLICY);
    
    free(fully_associative);

    return 0;
}

void init_cache(CacheBlock cache[], int num_blocks, int set_associativity) {
    int total_blocks = num_blocks * set_associativity;
    for (int i = 0; i < total_blocks; i++) {
        //printf("%d, %d", i, total_blocks);
        cache[i].tag = 0;
        cache[i].valid = 0;
        cache[i].counter = 0;
    }
}

void read_traces(const char* filename, CacheBlock cache[], int num_blocks, int set_associativity, int replacement_policy) {
    //printf("here");
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        printf("Error: Unable to open input file: %s\n", filename);
        return;
    }

    unsigned int address;
    int hits = 0;
    int total_accesses = 0;
    
    while (fscanf(file, "%x", &address) != EOF) {
        total_accesses++;
        unsigned int tag = address / BLOCK_SIZE;
        int hit = access_cache(cache, num_blocks, tag, set_associativity, replacement_policy);
        hits += hit;
    }

    fclose(file);
    //printf("here");
    double hit_rate = (double)hits / total_accesses * 100;
    printf("Hits: %d\nTotal Accesses: %d\nHit Rate: %.2f%%\n", hits, total_accesses, hit_rate);
}

int access_cache(CacheBlock cache[], int num_blocks, unsigned int tag, int set_associativity, int replacement_policy) {
    int index;
    int hit = 0;
    int empty_slot = -1;
    int lru_index = 0;
    int min_counter = INT_MAX;
    int random_index = rand() % set_associativity;

    for (int i = 0; i < set_associativity; i++) {
        index = tag % num_blocks * set_associativity + i;
        CacheBlock *block = &cache[index];

        if (block->valid && block->tag == tag) {
            hit = 1;
            block->counter++;
            break;
        }

        if (!block->valid && empty_slot == -1) {
            empty_slot = i;
        }

        if (block->counter < min_counter) {
            min_counter = block->counter;
            lru_index = i;
        }
    }

    if (!hit && empty_slot != -1) {
        CacheBlock *block = &cache[tag % num_blocks * set_associativity + empty_slot];
        block->valid = 1;
        block->tag = tag;
        block->counter = 1;
    } else if (!hit) {
        if (replacement_policy == LRU_POLICY) {
            CacheBlock *block = &cache[tag % num_blocks * set_associativity + lru_index];
            block->tag = tag;
            block->counter = 1;
        } else {
            CacheBlock *block = &cache[tag % num_blocks * set_associativity + random_index];
            block->tag = tag;
            block->counter = 1;
        }
    }

    return hit;
}

