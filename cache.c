#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#define MAX_CACHE_SIZE 256
#define MAX_BLOCK_SIZE 256

// **Note** this is a preprocessor macro. This is not the same as a function.
// Powers of 2 have exactly one 1 and the rest 0's, and 0 isn't a power of 2.
#define is_power_of_2(val) (val && !(val & (val - 1)))


/*
 * Accesses 1 word of memory.
 * addr is a 16-bit LC2K word address.
 * write_flag is 0 for reads and 1 for writes.
 * write_data is a word, and is only valid if write_flag is 1.
 * If write flag is 1, mem_access does: state.mem[addr] = write_data.
 * The return of mem_access is state.mem[addr].
 */
extern int mem_access(int addr, int write_flag, int write_data);

/*
 * Returns the number of times mem_access has been called.
 */
extern int get_num_mem_accesses(void);

//Use this when calling printAction. Do not modify the enumerated type below.
enum actionType
{
    cacheToProcessor,
    processorToCache,
    memoryToCache,
    cacheToMemory,
    cacheToNowhere
};

/* You may add or remove variables from these structs */
typedef struct blockStruct
{
    int data[MAX_BLOCK_SIZE];
    int dirty;
    int lruLabel;
    int tag;
    int valid;
} blockStruct;

typedef struct cacheStruct
{
    blockStruct blocks[MAX_CACHE_SIZE];
    int blockSize;
    int numSets;
    int blocksPerSet;
    int numHits;
    int numMisses;
    int numWritebacks;
    int numWordsAccessed;
    int currentDirtyBlocks;
    // add any variables for end-of-run stats
} cacheStruct;

/* Global Cache variable */
cacheStruct cache;
char memAccess[65536] = {0};

void printAction(int, int, enum actionType);
void printCache(void);

/*
 * Set up the cache with given command line parameters. This is
 * called once in main(). You must implement this function.
 */
void cache_init(int blockSize, int numSets, int blocksPerSet)
{
    if (blockSize <= 0 || numSets <= 0 || blocksPerSet <= 0) {
        printf("error: input parameters must be positive numbers\n");
        exit(1);
    }
    if (blocksPerSet * numSets > MAX_CACHE_SIZE) {
        printf("error: cache must be no larger than %d blocks\n", MAX_CACHE_SIZE);
        exit(1);
    }
    if (blockSize > MAX_BLOCK_SIZE) {
        printf("error: blocks must be no larger than %d words\n", MAX_BLOCK_SIZE);
        exit(1);
    }
    if (!is_power_of_2(blockSize)) {
        printf("warning: blockSize %d is not a power of 2\n", blockSize);
    }
    if (!is_power_of_2(numSets)) {
        printf("warning: numSets %d is not a power of 2\n", numSets);
    }
    printf("Simulating a cache with %d total lines; each line has %d words\n",
        numSets * blocksPerSet, blockSize);
    printf("Each set in the cache contains %d lines; there are %d sets\n",
        blocksPerSet, numSets);

    /********************* Initialize Cache *********************/
    cache.blockSize = blockSize;
    cache.numSets = numSets;
    cache.blocksPerSet = blocksPerSet;
    cache.numHits = cache.numMisses = cache.currentDirtyBlocks = cache.numWritebacks = 0;
    return;
}

/*
 * Access the cache. This is the main part of the project,
 * and should call printAction as is appropriate.
 * It should only call mem_access when absolutely necessary.
 * addr is a 16-bit LC2K word address.
 * write_flag is 0 for reads (fetch/lw) and 1 for writes (sw).
 * write_data is a word, and is only valid if write_flag is 1.
 * The return of mem_access is undefined if write_flag is 1.
 * Thus the return of cache_access is undefined if write_flag is 1.
 */
int cache_access(int addr, int write_flag, int write_data)
{
    int currentBlockOffset = addr % cache.blockSize;
    int currentIndex = (addr / cache.blockSize) % cache.numSets;
    int currentTag = (addr / cache.blockSize) / cache.numSets;
    int start = currentIndex * cache.blocksPerSet;
    int end = start + cache.blocksPerSet;
    int i,j;
    int hitIndex = -1;

    for(i = start; i < end; i++) //look for hit
    {
        if(cache.blocks[i].valid && cache.blocks[i].tag == currentTag)
        {
            hitIndex = i;
            break;
        }
    }

    if(hitIndex != -1) //found hit
    {
        cache.numHits++;
        //update lru for a hit
        int oldLRULabel = cache.blocks[hitIndex].lruLabel;
        for(i = start; i < end; i++)
        {
            if(cache.blocks[i].lruLabel < oldLRULabel)
            {
                cache.blocks[i].lruLabel++;
            }
        }
        cache.blocks[hitIndex].lruLabel = 0;
        if(!write_flag) //cache-to-processor
        {
            printAction(addr,1,cacheToProcessor);
            return cache.blocks[hitIndex].data[currentBlockOffset];
        }
        else //processor-to-cache
        {
            cache.blocks[hitIndex].data[currentBlockOffset] = write_data;
            if(!cache.blocks[hitIndex].dirty)
            {
                cache.currentDirtyBlocks++; //only increment if the current address wasnt already dirty
            }
            cache.blocks[hitIndex].dirty = 1;
            printAction(addr,1,processorToCache);
            return 0;
        }
    }
    //if we make it here, it was a cache miss
    cache.numMisses++;
    int freeIndex = -1;
    //find free block inside current set
    for(i = start; i < end; i++)
    {
        if(!cache.blocks[i].valid)
        {
            freeIndex = i;
            break;
        }
    }
    int replacementIndex;
    if(freeIndex != -1)
    {
        replacementIndex = freeIndex;
    }
    else
    {
        //replace based on LRU
        int maxLRU = -1;
        for(i = start; i < end; i++)
        {
            if(cache.blocks[i].lruLabel > maxLRU)
            {
                maxLRU = cache.blocks[i].lruLabel;
                replacementIndex = i;
            }
        }
        if(cache.blocks[replacementIndex].dirty) //writeback if dirty
        {
            int blockBaseAddress = ((cache.blocks[replacementIndex].tag * cache.numSets) + currentIndex) * cache.blockSize;
            for(j = 0; j < cache.blockSize; j++)
            {
                mem_access(blockBaseAddress + j,1,cache.blocks[replacementIndex].data[j]);
                cache.numWordsAccessed++;
                if (!memAccess[blockBaseAddress + j])
                {
                    memAccess[blockBaseAddress + j] = 1;
                    cache.numWordsAccessed++;
                }
            }
            cache.numWritebacks++;
            cache.currentDirtyBlocks--;
            printAction(blockBaseAddress,cache.blockSize,cacheToMemory);
        }
        else //clean eviction
        {
            int blockBaseAddress = ((cache.blocks[replacementIndex].tag * cache.numSets) + currentIndex) * cache.blockSize;
            printAction(blockBaseAddress,cache.blockSize,cacheToNowhere);
        }
    }
    //load new block from memory
    int blockBaseAddress = addr - (addr % cache.blockSize);
    for(i = 0; i < cache.blockSize; i++)
    {
        cache.blocks[replacementIndex].data[i] = mem_access(blockBaseAddress + i,0,0);
        cache.numWordsAccessed++;
        if (!memAccess[blockBaseAddress + i])
            {
                memAccess[blockBaseAddress + i] = 1;
            }
    }
    cache.blocks[replacementIndex].dirty = 0;
    cache.blocks[replacementIndex].tag = currentTag;
    cache.blocks[replacementIndex].valid = 1;
    //update lru for all values in block
    for(i = start; i < end; i++)
    {
        if(cache.blocks[i].valid)
        {
            cache.blocks[i].lruLabel++;
        }
    }
    cache.blocks[replacementIndex].lruLabel = 0;
    if(addr == 0)
    {
        printAction(addr,cache.blockSize,memoryToCache);
    }
    else
    {
        printAction(blockBaseAddress,cache.blockSize,memoryToCache);
    }
    if(write_flag)
    {
        //processor-to-cache
        cache.blocks[replacementIndex].data[currentBlockOffset] = write_data;
        cache.blocks[replacementIndex].dirty = 1;
        cache.currentDirtyBlocks++;
        printAction(addr,1,processorToCache);
        return 0;
    }
    else
    {
        //cache-to-processor
        printAction(addr,1,cacheToProcessor);
        return cache.blocks[replacementIndex].data[currentBlockOffset];
    }
    return 0;
}


/*
 * print end of run statistics like in the spec. **This is not required**,
 * but is very helpful in debugging.
 * This should be called once a halt is reached.
 * DO NOT delete this function, or else it won't compile.
 * DO NOT print $$$ in this function
 */
void printStats(void)
{
    printf("$$$ Main memory words accessed: %d\n",cache.numWordsAccessed);
    printf("End of run statistics:\n");
    printf("hits %d, misses %d, writebacks %d\n",
    cache.numHits,cache.numMisses,cache.numWritebacks);
    printf("%d dirty cache blocks left\n",cache.currentDirtyBlocks);
    return;
}

/*
 * Log the specifics of each cache action.
 *
 *DO NOT modify the content below.
 * address is the starting word address of the range of data being transferred.
 * size is the size of the range of data being transferred.
 * type specifies the source and destination of the data being transferred.
 *  -    cacheToProcessor: reading data from the cache to the processor
 *  -    processorToCache: writing data from the processor to the cache
 *  -    memoryToCache: reading data from the memory to the cache
 *  -    cacheToMemory: evicting cache data and writing it to the memory
 *  -    cacheToNowhere: evicting cache data and throwing it away
 */
void printAction(int address, int size, enum actionType type)
{
    printf("$$$ transferring word [%d-%d] ", address, address + size - 1);

    if (type == cacheToProcessor) {
        printf("from the cache to the processor\n");
    }
    else if (type == processorToCache) {
        printf("from the processor to the cache\n");
    }
    else if (type == memoryToCache) {
        printf("from the memory to the cache\n");
    }
    else if (type == cacheToMemory) {
        printf("from the cache to the memory\n");
    }
    else if (type == cacheToNowhere) {
        printf("from the cache to nowhere\n");
    }
    else {
        printf("Error: unrecognized action\n");
        exit(1);
    }

}

/*
 * Prints the cache based on the configurations of the struct
 * This is for debugging only and is not graded, so you may
 * modify it, but that is not recommended.
 */
void printCache(void)
{
    int blockIdx;
    int decimalDigitsForWaysInSet = (cache.blocksPerSet == 1) ? 1 : (int)ceil(log10((double)cache.blocksPerSet));
    printf("\ncache:\n");
    for (int set = 0; set < cache.numSets; ++set) {
        printf("\tset %i:\n", set);
        for (int block = 0; block < cache.blocksPerSet; ++block) {
            blockIdx = set * cache.blocksPerSet + block;
            if(cache.blocks[set * cache.blocksPerSet + block].valid) {
                printf("\t\t[ %0*i ] : ( V:T | D:%c | LRU:%-*i | T:%i )\n\t\t%*s{",
                    decimalDigitsForWaysInSet, block,
                    (cache.blocks[blockIdx].dirty) ? 'T' : 'F',
                    decimalDigitsForWaysInSet, cache.blocks[blockIdx].lruLabel,
                    cache.blocks[blockIdx].tag,
                    7+decimalDigitsForWaysInSet, "");
                for (int index = 0; index < cache.blockSize; ++index) {
                    printf(" 0x%08X", cache.blocks[blockIdx].data[index]);
                }
                printf(" }\n");
            }
            else {
                printf("\t\t[ %0*i ] : ( V:F )\n\t\t%*s{     }\n", decimalDigitsForWaysInSet, block, 7+decimalDigitsForWaysInSet, "");
            }
        }
    }
    printf("end cache\n");
}
