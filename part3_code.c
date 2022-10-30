#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
char *trace_file_name;

struct L1Cache
{
    unsigned valid_field[1024];
    unsigned dirty_field[1024];
    uint64_t tag_field[1024];
    char data_field[1024][64];
    int hits;
    int misses;
};
struct L2Cache
{
    unsigned valid_field[16384];
    unsigned dirty_field[16384];
    uint64_t tag_field[16384];
    char data_field[16384][64];
    int hits;
    int misses;
};

uint64_t convert_address(char memory_addr[])
{
    int k = 0;
    uint64_t binary = 0;
    
    while (memory_addr[k] != '\n')
    {
        if (memory_addr[k] <= '9' && memory_addr[k] >= '0')
        {
            binary = (binary * 16) + (memory_addr[k] - '0');
        }
        else
        {
            if (memory_addr[k] == 'a' || memory_addr[k] == 'A')
            {
                binary = (binary * 16) + 10;
            }
            if (memory_addr[k] == 'b' || memory_addr[k] == 'B')
            {
                binary = (binary * 16) + 11;
            }
            if (memory_addr[k] == 'c' || memory_addr[k] == 'C')
            {
                binary = (binary * 16) + 12;
            }
            if (memory_addr[k] == 'd' || memory_addr[k] == 'D')
            {
                binary = (binary * 16) + 13;
            }
            if (memory_addr[k] == 'e' || memory_addr[k] == 'E')
            {
                binary = (binary * 16) + 14;
            }
            if (memory_addr[k] == 'f' || memory_addr[k] == 'F')
            {
                binary = (binary * 16) + 15;
            }
        }
        k++;
    }

#ifdef DBG
    printf("%s converted to %llu\n", memory_addr, binary);
#endif
    return binary;
}

int is_Data_Cache_L1(uint64_t address, struct L1Cache *l1, int nway)
{
    uint64_t block_addr = address >> (unsigned)log2(64);
    uint64_t tag = block_addr >> (unsigned)log2(512);
    int set_Number = block_addr % 512;
    int start_Index = ((int)set_Number) * nway;
    int loop_Index = start_Index;
    int nway_Temp = nway;
    while (nway_Temp > 0)
    {
        if (l1->valid_field[loop_Index] && l1->tag_field[loop_Index] == tag)
        {
            return 1;
        }
        loop_Index += 1;
        nway_Temp--;
    }
    return 0;
}
int is_Data_Cache_L2(uint64_t address, struct L2Cache *l2, int nway)
{
    uint64_t block_addr = address >> (unsigned)log2(64);
    uint64_t tag = block_addr >> (unsigned)log2(2048);
    int set_Number = block_addr % 2048;
    int start_Index = ((int)set_Number) * nway;
    int loop_Index = start_Index;
    int nway_Temp = nway;
    while (nway_Temp > 0)
    {
        if (l2->valid_field[loop_Index] && l2->tag_field[loop_Index] == tag)
        {
            return 1;
        }
        loop_Index += 1;
        nway_Temp--;
    }
    return 0;
}
void is_Data_in_L1_Cache(uint64_t address, struct L1Cache *l1, int nway)
{
    uint64_t block_addr = address >> (unsigned)log2(64);
    uint64_t tag = block_addr >> (unsigned)log2(512);
    int set_Number = block_addr % 512;
    int start_Index = ((int)set_Number) * nway;
    int loop_Index = start_Index;
    int nway_Temp = nway;
    int is_Empty_Space = 0;
    int end_Index = start_Index + nway - 1;
    while (nway_Temp > 0)
    {
        if (l1->valid_field[loop_Index] == 0)
        {
            is_Empty_Space = 1;
        }
        loop_Index++;
        nway_Temp--;
    }
    if (is_Empty_Space > 0)
    {
        nway_Temp = nway;
        loop_Index = start_Index;
        while (nway_Temp > 0)
        {
            if (l1->valid_field[loop_Index] == 0)
            {
                l1->valid_field[loop_Index] = 1;
                l1->tag_field[loop_Index] = tag;
                break;
            }

            loop_Index += 1;
            nway_Temp--;
        }
    }
    else
    {
        int rand_Index = (rand() % (end_Index - start_Index + 1)) + start_Index;
        //   printf("Picking a rand variable %d",rand_Index);
        l1->valid_field[rand_Index] = 1;
        l1->tag_field[rand_Index] = tag;
    }
}
void insertDataInL2Cache(uint64_t address, int nway, struct L2Cache *l2)
{
    uint64_t block_addr = address >> (unsigned)log2(64);
    uint64_t tag = block_addr >> (unsigned)log2(2048);
    int set_Number = block_addr % 2048;
    int start_Index = ((int)set_Number) * nway;
    int loop_Index = start_Index;
    int nway_Temp = nway;
    int end_Index = start_Index + nway - 1;
    int is_Empty_Space = 0;
    while (nway_Temp > 0)
    {
        if (l2->valid_field[loop_Index] == 0)
        {
            is_Empty_Space = 1;
        }
        loop_Index++;
        nway_Temp--;
    }
    if (is_Empty_Space > 0)
    {
        nway_Temp = nway;
        loop_Index = start_Index;
        while (nway_Temp > 0)
        {
            if (l2->valid_field[loop_Index] == 0)
            {
                l2->valid_field[loop_Index] = 1;
                l2->tag_field[loop_Index] = tag;
                break;
            }
            loop_Index += 1;
            nway_Temp--;
        }
    }
    else
    {
        int rand_Index = (rand() % (end_Index - start_Index + 1)) + start_Index;
        //   printf("Picking a random variable %d",rand_Index);
        l2->valid_field[rand_Index] = 1;
        l2->tag_field[rand_Index] = tag;
    }
}

int main(int argc, char *argv[])
{
    FILE *fp;
    trace_file_name = argv[2];
    char mem_request[20];
    uint64_t address;
    struct L1Cache l1;
    struct L2Cache l2;
    int numOfBlocksin_l1 = 1024;
    int numOfBlocksin_l2 = 16384;
    int l1_nway = 2;
    int l2_nway = 8;
    int numOfSets_l1 = 512;
    int numOfSets_l2 = 2048;
    for (int i = 0; i < numOfBlocksin_l1; i++)
    {
        l1.valid_field[i] = 0;
        l1.dirty_field[i] = 0;
        l1.tag_field[i] = 0;
    }
    for (int i = 0; i < numOfBlocksin_l2; i++)
    {
        l2.valid_field[i] = 0;
        l2.dirty_field[i] = 0;
        l2.tag_field[i] = 0;
    }

    l1.hits = 0;
    l1.misses = 0;
    l2.hits = 0;
    l2.misses = 0;
    fp = fopen(trace_file_name, "r");
    if (strncmp(argv[1], "direct", 6) == 0)
    {
        while (fgets(mem_request, 20, fp) != NULL)
        {
            address = convert_address(mem_request);
            int dataInL1 = is_Data_Cache_L1(address, l1_nway, &l1);
            if (dataInL1 == 1)
            {
                l1.hits++;
                l2.hits++;
            }
            else
            {
                l1.misses++;
                int dataInL2 = is_Data_Cache_L2(address, l2_nway, &l2);
                if (dataInL2)
                {
                    l2.hits += 1;
                }
                else
                {
                    l2.misses++;
                    insertDataInL2Cache(address, l2_nway, &l2);
                }
                is_Data_in_L1_Cache(address, l1_nway, &l1);
            }
        }
        printf("\n=======================================\n");
        printf("Cache type:l1\n");
        printf("=========================================\n");
        printf("Cache Hits:%d\n", l1.hits);
        printf("Cache Misses:%d\n", l1.misses);
        printf("Cache Hit Rate :%0.9f%%\n", ((float)l1.hits / (float)(l1.hits + l1.misses)) * 100);
        printf("Cache Miss Rate :%0.9f%%\n", ((float)l1.misses / (float)(l1.hits + l1.misses)) * 100);
        printf("\n");
        printf("\n=======================================\n");
        printf("Cache type:l2\n");
        printf("=========================================\n");
        printf("Cache Hits:%d\n", l2.hits);
        printf("Cache Misses:%d\n", l2.misses);
        printf("Cache Hit Rate :%0.9f%%\n", ((float)l2.hits / (float)(l2.hits + l2.misses)) * 100);
        printf("Cache Miss Rate :%0.9f%%\n", ((float)l2.misses / (float)(l2.hits + l2.misses)) * 100);
        printf("\n");
        printf("\n");
    }
    fclose(fp);
    return 0;
}
