/*
 * Brody Williams
 * Cache Simulator
 * CS 3375 - Fall 2017 - Texas Tech University
 * CacheSimulator.c - Program file
 * Some sections and/or pieces of code reused from Dr. Chen's cachesim program
 */

 /* Included Files */
 #include <stdio.h>
 #include <stdint.h>
 #include <string.h>
 #include <stdlib.h>
 #include <time.h>
 #include <math.h>
 #include "CacheSimulator.h"

 /* Main Function. Takes command line arguments, creates cache, and passes execution to appropriate *
  * algorithm. If n-way cache, prompts user for number of ways and replacement algorithm.           */
 int main(int argc, char* argv[])
 {
   /* Incorrect number of command line arguments */
   if (argc != 3)
   {
     printf("\nUsage: %s <associativity> <trace file name>\n", argv[0]);
     printf("Associativity = 'direct' || 'n-way' || 'fully_associative'\n\n");
     return 1;
   }

   char* trace_file_name = argv[2];
   char* cache_type = argv[1];

   /* Command line argument = direct */
   if((strncmp(cache_type, "direct", 6)==0)||(strncmp(cache_type, "DIRECT", 6)==0))
   {
     Cache current_cache = {.cache_type = "Direct-Mapped Cache", .ways = 1, .sets = NUM_BLOCKS, .replacement_algorithm = "N/A"};
     run_dmc_simulation(&current_cache, trace_file_name);
   }
   /* Command line argument = fully_associative */
   else if((strncmp(cache_type, "fully_associative", 17)==0)||(strncmp(cache_type, "FULLY_ASSOCIATIVE", 17)==0))
   {
     Cache current_cache = {.cache_type = "Fully-Associative Cache", .ways = NUM_BLOCKS, .sets = 1, .replacement_algorithm = "NRU"};
     run_fac_simulation(&current_cache, trace_file_name);
   }
   /* Command line argument = n-way */
   else if(strncmp(cache_type, "n-way", 5)==0)
   {
     char replacement_algorithm[4];
     unsigned ways;

     printf("\nHow many ways per set? : ");
     scanf("%d", &ways);
     printf("Replacement algorithm? (NRU, PLRU, RR) : ");
     scanf("%s", replacement_algorithm);

     char cache_string[27];
     sprintf(cache_string, "%d-Way Associative Cache", ways);

     /* replacement_algorithm = nru */
     if((strncmp(replacement_algorithm, "NRU", 4)==0)||(strncmp(replacement_algorithm, "nru", 4)==0))
     {
       Cache current_cache = {.ways = ways, .sets = NUM_BLOCKS/ways, .replacement_algorithm = "NRU"};
       strncpy(current_cache.cache_type, cache_string, 27);
       run_nru_simulation(&current_cache, trace_file_name);
     }
      /* replacement_algorithm = plru */
     else if((strncmp(replacement_algorithm, "PLRU", 4)==0)||(strncmp(replacement_algorithm, "plru", 4)==0))
     {
       Cache current_cache = {.ways = ways, .sets = NUM_BLOCKS/ways, .replacement_algorithm = "PLRU"};
       strncpy(current_cache.cache_type, cache_string, 27);
       run_plru_simulation(&current_cache, trace_file_name);
     }
      /* replacement_algorithm = rr */
     else if((strncmp(replacement_algorithm, "RR", 4)==0)||(strncmp(replacement_algorithm, "rr", 4)==0))
     {
       Cache current_cache = {.ways = ways, .sets = NUM_BLOCKS/ways, .replacement_algorithm = "RR"};
       strncpy(current_cache.cache_type, cache_string, 27);
       run_rr_simulation(&current_cache, trace_file_name);
     }
     else
     {
       printf("\nInvalid replacement algorithm!\n\n");
       return 1;
     }
   }
   else
   {
     printf("\nInvalid associativity\n\n");
     return 1;
   }
   return 0;
 }

 /* Dr. Chen's function to convert a hex address to the appropriate value */
 uint64_t convert_address(char memory_addr[])
 {
     uint64_t binary = 0;
     int i = 0;

     while (memory_addr[i] != '\n') {
         if (memory_addr[i] <= '9' && memory_addr[i] >= '0') {
             binary = (binary*16) + (memory_addr[i] - '0');
         } else {
             if(memory_addr[i] == 'a' || memory_addr[i] == 'A') {
                 binary = (binary*16) + 10;
             }
             if(memory_addr[i] == 'b' || memory_addr[i] == 'B') {
                 binary = (binary*16) + 11;
             }
             if(memory_addr[i] == 'c' || memory_addr[i] == 'C') {
                 binary = (binary*16) + 12;
             }
             if(memory_addr[i] == 'd' || memory_addr[i] == 'D') {
                 binary = (binary*16) + 13;
             }
             if(memory_addr[i] == 'e' || memory_addr[i] == 'E') {
                 binary = (binary*16) + 14;
             }
             if(memory_addr[i] == 'f' || memory_addr[i] == 'F') {
                 binary = (binary*16) + 15;
             }
         }
         i++;
     }

     #ifdef DEBUG
        printf("%s converted to %llu\n", memory_addr, binary);
     #endif
     return binary;
 }

 /* Function to execute simulation of a direct mapped cache. DMC simulation *
  * could also be accomplished using nru algorithm with sets = NUM_BLOCKS   *
  * and ways = 1. I checked results this way but thought a separate         *
  * function might improve efficiency. Not sure if it actually does.        */
 void run_dmc_simulation(Cache* cache, char* trace_file_name)
 {
   // Cache details when debugging
   #ifdef DEBUG
       printf("\nBLOCK SIZE = %d Bytes\n", BLOCK_SIZE);
       printf("CACHE SIZE = %d Bytes\n", CACHE_SIZE);
       printf("NUMBER OF BLOCKS = %d\n", NUM_BLOCKS);
       printf("%d-WAY\n", cache->ways);
       printf("NUMBER OF SETS = %d\n", cache->sets);
       printf("\n");
   #endif

   /* Since a direct mapped cache only has 1 way/set 1-d arrays suffice for tag and valid fields. */
   uint64_t tag_field[NUM_BLOCKS];
   unsigned valid_field[NUM_BLOCKS];
   char mem_request[20];
   uint64_t address;

   /* Initialization of arrays*/
   for(int i=0; i<NUM_BLOCKS; i++)
   {
     valid_field[i] = 0;
     tag_field[i] = 0;
   }

   /* Open trace file in read mode*/
   FILE *fp;
   fp = fopen(trace_file_name, "r");

   /* Process each address */
   while(fgets(mem_request, 20, fp)!= NULL)
   {
       address = convert_address(mem_request); // get value of hex address
       uint64_t block_addr = address >> (unsigned)log2(BLOCK_SIZE); // get black address
       uint64_t index = block_addr % NUM_BLOCKS; // index = block address % number of sets(NUM_BLOCKS for dmc)
       uint64_t tag = block_addr >> (unsigned)log2(NUM_BLOCKS); //tag = block address shifted right by n bits where 2^n is number of sets. (NUM_BLOCKS for dmc)

       #ifdef DEBUG
          printf("Memory address: %llu, Block address: %llu, Index: %llu, Tag: %llu ", address, block_addr, index, tag);
       #endif

       /* Check for hit. Valid field occupied and index matches. */
       if((valid_field[index]) && (tag_field[index] == tag))
       {
         cache->hits++;
         #ifdef DEBUG
            printf("Hit!\n");
         #endif
       }
       /* If not a hit replace only possibility in dmc */
       else
       {
         cache->misses++;
         valid_field[index] = 1;
         tag_field[index] = tag;
         #ifdef DEBUG
            printf("Miss!\n");
         #endif
       }
   }
   /* Calculate and set hit and miss rates */
   cache->hit_rate = (((double)(cache->hits))/((double)(cache->hits+cache->misses)));
   cache->miss_rate = (((double)(cache->misses))/((double)(cache->hits+cache->misses)));

   /* Call function to print results to screen*/
   print_results(cache, trace_file_name);

   /*Call function to write results to file*/
   write_to_file(cache, trace_file_name);

   /* Close trace file */
   fclose(fp);
 }

 /* Function to execute simulation of a fully associative cache. FAC simulation *
  * could also be accomplished using nru algorithm with sets = 1 and            *
  * ways = NUM_BLOCKS. I checked results this way but thought a separate        *
  * function might improve efficiency. Not sure if it actually does.            */
 void run_fac_simulation(Cache* cache, char* trace_file_name)
 {
   // Cache details when debugging
   #ifdef DEBUG
       printf("\nBLOCK SIZE = %d Bytes\n", BLOCK_SIZE);
       printf("CACHE SIZE = %d Bytes\n", CACHE_SIZE);
       printf("NUMBER OF BLOCKS = %d\n", NUM_BLOCKS);
       printf("%d-WAY\n", cache->ways);
       printf("NUMBER OF SETS = %d\n", cache->sets);
       printf("\n");
   #endif

   /* As with dmc only 1d arrays are necessary, but a block can now go in any slot. NRU field is also needed*/
   uint64_t tag_field[NUM_BLOCKS];
   unsigned valid_field[NUM_BLOCKS];
   unsigned nru_reference[NUM_BLOCKS];
   char mem_request[20];
   uint64_t address;

   /* Initialization of arrays*/
   for(int i=0; i<cache->ways; i++)
   {
     nru_reference[i] = 0;
     valid_field[i] = 0;
     tag_field[i] = 0;
   }

   /* Open trace file in read mode */
   FILE *fp;
   fp = fopen(trace_file_name, "r");

   /* Process each address */
   while(fgets(mem_request, 20, fp)!= NULL)
   {
       address = convert_address(mem_request); // convert hex address
       uint64_t block_addr = address >> (unsigned)log2(BLOCK_SIZE); // get block address
       /* No index necessary for FAC */
       /* index = block addr for FAC */

       #ifdef DEBUG
          printf("Memory address: %llu, Sets: %u, Ways: %u, Block address: %llu", address, cache->sets, cache->ways, block_addr);
       #endif

       /* Flag is set to 1 when address is processed */
       int processed_flag = 0;

       /* Check for hit */
       for (int i=0; i<NUM_BLOCKS; i++)
       {
         /* If cache space is occupied and tag matches then Hit*/
         if((valid_field[i])&&(tag_field[i]==block_addr))
         {
           cache->hits++;
           nru_reference[i] = 1; // set nru reference field to true
           processed_flag = 1;
           #ifdef DEBUG
                   printf("Hit!\n");
           #endif
           break;
         }
       }
       /* If not a hit then process miss*/
       if(processed_flag==0)
       {
         /* First look for an unused way */
         for(int i=0; i<NUM_BLOCKS; i++)
         {
           /* If an unused way exists place block*/
           if(valid_field[i]==0)
           {
             cache->misses++;
             valid_field[i] = 1;
             tag_field[i] = block_addr;
             nru_reference[i] = 1;
             processed_flag = 1;
             #ifdef DEBUG
                     printf("Miss!\n");
             #endif
             break;
           }
         }
         /* if no unused ways, replace first way where nru = 0 */
         if(processed_flag==0)
         {
           for(int i=0; i<NUM_BLOCKS; i++)
           {
             if(nru_reference[i]==0)
             {
               tag_field[i] = block_addr;
               nru_reference[i] = 1;
               cache->misses++;
               processed_flag = 1;
               #ifdef DEBUG
                       printf("Miss!\n");
               #endif
               break;
             }
           }
         }
         /* Finally if nru = 1 for every way, replace the first entry and reset all the nru bits*/
         if(processed_flag==0)
         {
           tag_field[0] = block_addr;
           cache->misses++;
           #ifdef DEBUG
                   printf("Miss!\n");
           #endif
           /* Reset all other nru bits */
           for(int i=1; i<cache->ways; i++)
           {
             nru_reference[i] = 0;
           }
         }
       }
   }

   /* Calculate and set hit and miss rates */
   cache->hit_rate = (((double)(cache->hits))/((double)(cache->hits+cache->misses)));
   cache->miss_rate = (((double)(cache->misses))/((double)(cache->hits+cache->misses)));

   /* Call function to print results to screen*/
   print_results(cache, trace_file_name);

   /*Call function to write results to file*/
   write_to_file(cache, trace_file_name);

   /* Close trace file */
   fclose(fp);
 }

 /* Function to execute simulation of an n-way associative cache using nru replacement algorithm */
 void run_nru_simulation(Cache* cache, char* trace_file_name)
 {
   // Cache details when debugging
   #ifdef DEBUG
       printf("\nBLOCK SIZE = %d Bytes\n", BLOCK_SIZE);
       printf("CACHE SIZE = %d Bytes\n", CACHE_SIZE);
       printf("NUMBER OF BLOCKS = %d\n", NUM_BLOCKS);
       printf("%d-WAY\n", cache->ways);
       printf("NUMBER OF SETS = %d\n", cache->sets);
       printf("\n");
   #endif

   /* 2-d arrays are necessary for n-way caches since a block can only be placed in a way *
    * for the appropriate set and neither NUM_BLOCKS or sets must be == 1. Has NRU bit.   */
   uint64_t tag_field[cache->sets][cache->ways];
   unsigned valid_field[cache->sets][cache->ways];
   unsigned nru_reference[cache->sets][cache->ways];
   char mem_request[20];
   uint64_t address;

   /* Initialization of arrays*/
   for(int i=0; i<cache->sets; i++)
   {
     for(int j=0; j<cache->ways; j++)
     {
       nru_reference[i][j] = 0;
       valid_field[i][j] = 0;
       tag_field[i][j] = 0;
     }
   }

   /* Open trace file in read mode */
   FILE *fp;
   fp = fopen(trace_file_name, "r");

   /* Process each address */
   while(fgets(mem_request, 20, fp)!= NULL)
   {
       address = convert_address(mem_request); // get value of hex address
       uint64_t block_addr = address >> (unsigned)log2(BLOCK_SIZE); // get block address
       uint64_t index = block_addr % cache->sets; // index = block address % number of sets
       uint64_t tag = block_addr >> (unsigned)log2(cache->sets); //tag = block address shifted right by n bits where 2^n is number of sets.

       #ifdef DEBUG
          printf("Memory address: %llu, Sets: %u, Ways: %u, Block address: %llu, Index: %llu, Tag: %llu ", address, cache->sets, cache->ways, block_addr, index, tag);
       #endif

       /* Flag is set to 1 when address is processed */
       int processed_flag = 0;

       /* Check for hit in appropriate set */
       for (int i=0; i<cache->ways; i++)
       {
         /* If cache space is occupied and tag matches then Hit*/
         if((valid_field[index][i])&&(tag_field[index][i]==tag))
         {
           cache->hits++;
           nru_reference[index][i] = 1;
           processed_flag = 1;
           #ifdef DEBUG
                   printf("Hit!\n");
           #endif
           break;
         }
       }
       /* If not a hit then process miss*/
       if(processed_flag==0)
       {
         /* First look for an unused way in the proper set */
         for(int i=0; i<cache->ways; i++)
         {
           if(valid_field[index][i]==0)
           {
             cache->misses++;
             valid_field[index][i] = 1;
             tag_field[index][i] = tag;
             nru_reference[index][i] = 1;
             processed_flag = 1;
             #ifdef DEBUG
                     printf("Miss!\n");
             #endif
             break;
           }
         }
         /* if no unused way, replace first way in appropriate set where nru = 0 */
         if(processed_flag==0)
         {
           for(int i=0; i<cache->ways; i++)
           {
             if(nru_reference[index][i]==0)
             {
               tag_field[index][i] = tag;
               nru_reference[index][i] = 1;
               cache->misses++;
               processed_flag = 1;
               #ifdef DEBUG
                       printf("Miss!\n");
               #endif
               break;
             }
           }
         }
         /* Finally if nru = 1 for each way in the set, replace the first entry in the set and reset all the the nru bits in that set*/
         if(processed_flag==0)
         {
           tag_field[index][0] = tag;
           cache->misses++;
           #ifdef DEBUG
                   printf("Miss!\n");
           #endif
           for(int i=1; i<cache->ways; i++)
           {
             nru_reference[index][i] = 0;
           }
         }
       }
   }

   /* Calculate set hit and miss rates */
   cache->hit_rate = (((double)(cache->hits))/((double)(cache->hits+cache->misses)));
   cache->miss_rate = (((double)(cache->misses))/((double)(cache->hits+cache->misses)));

   /* Call function to print results to screen */
   print_results(cache, trace_file_name);

   /* Call function to write results to file */
   write_to_file(cache, trace_file_name);

   /* Close trace file */
   fclose(fp);
 }

 /* Function to execute simulation of an n-way associative cache using plru replacement algorithm */
 void run_plru_simulation(Cache* cache, char* trace_file_name)
 {
   // Cache details when debugging
   #ifdef DEBUG
       printf("\nBLOCK SIZE = %d Bytes\n", BLOCK_SIZE);
       printf("CACHE SIZE = %d Bytes\n", CACHE_SIZE);
       printf("NUMBER OF BLOCKS = %d\n", NUM_BLOCKS);
       printf("%d-WAY\n", cache->ways);
       printf("NUMBER OF SETS = %d\n", cache->sets);
       printf("\n");
   #endif

   /* As n-way nru algorithm need 2d arrays to store field data*/
   uint64_t tag_field[cache->sets][cache->ways];
   unsigned valid_field[cache->sets][cache->ways];
   tree_node* trees[cache->sets]; //Array of pointers to bst trees for each set
   char mem_request[20];
   uint64_t address;

   /* Initialization of arrays*/
   for(int i=0; i<cache->sets; i++)
   {
     for(int j=0; j<cache->ways; j++)
     {
       valid_field[i][j] = 0;
       tag_field[i][j] = 0;
     }
   }

   /* Calls function to create BST trees for each set in the cache *
    * using n-1 nodes where n is the number of ways. Sets pointer  *
    * at appropriate index of tree array                           */
   for(int i=0; i<cache->sets; i++)
   {
     trees[i] = create_tree(cache->ways-1);
   }

   /* Open trace file in read mode */
   FILE *fp;
   fp = fopen(trace_file_name, "r");

   /* Process each address */
   while(fgets(mem_request, 20, fp)!= NULL)
   {
       address = convert_address(mem_request); // get adress from hex value
       uint64_t block_addr = address >> (unsigned)log2(BLOCK_SIZE); // get block address
       uint64_t index = block_addr % cache->sets; // index = block address % number of sets
       uint64_t tag = block_addr >> (unsigned)log2(cache->sets); //tag = block address shifted right by n bits where 2^n is number of sets.

       #ifdef DEBUG
          printf("Memory address: %llu, Sets: %u, Ways: %u, Block address: %llu, Index: %llu, Tag: %llu ", address, cache->sets, cache->ways, block_addr, index, tag);
       #endif

       /* Flag is set to 1 when address is processed */
       int processed_flag = 0;

       /* Check for hit in appropriate set */
       for (int way=0; way<(cache->ways); way++)
       {
         /* If cache space is occupied and tag matches then Hit*/
         if((valid_field[index][way])&&(tag_field[index][way]==tag))
         {
           cache->hits++;
           hit_update_bst(trees[index], cache->ways, way); //update appopriate BST for hit
           processed_flag = 1;
           #ifdef DEBUG
                   printf("Hit!\n");
           #endif
           break;
         }
       }
       /* If not a hit then process miss*/
       if(processed_flag==0)
       {
         /* First look for an unused way in the proper set */
         for(int i=0; i<cache->ways; i++)
         {
           if(valid_field[index][i]==0)
           {
             cache->misses++;
             valid_field[index][i] = 1;
             tag_field[index][i] = tag;
             processed_flag = 1;
             #ifdef DEBUG
                     printf("Miss!\n");
             #endif
             break;
           }
         }
         /* If all ways are occupied then traverse bst and replace indicated way*/
         if(processed_flag==0)
         {
           cache->misses++;
           unsigned replacement = find_plru_way(trees[index], cache->ways); // get PLRU way from bst
           tag_field[index][replacement] = tag;
           #ifdef DEBUG
                   printf("Miss!\n");
           #endif
         }
       }
   }

   /* Calculate and set hit and miss rates */
   cache->hit_rate = (((double)(cache->hits))/((double)(cache->hits+cache->misses)));
   cache->miss_rate = (((double)(cache->misses))/((double)(cache->hits+cache->misses)));

   /* Call function to print results to screen*/
   print_results(cache, trace_file_name);

   /*Call function to write results to file*/
   write_to_file(cache, trace_file_name);

   /* Close trace file */
   fclose(fp);
 }

/* Function to execute simulation of an n-way associative cache using random replacement algorithm */
void run_rr_simulation(Cache* cache, char* trace_file_name)
{
  // Cache details when debugging
   #ifdef DEBUG
       printf("\nBLOCK SIZE = %d Bytes\n", BLOCK_SIZE);
       printf("CACHE SIZE = %d Bytes\n", CACHE_SIZE);
       printf("NUMBER OF BLOCKS = %d\n", NUM_BLOCKS);
       printf("%d-WAY\n", cache->ways);
       printf("NUMBER OF SETS = %d\n", cache->sets);
       printf("\n");
   #endif

   /* 2d arrays for tag and valid field. No nru field needed.*/
   uint64_t tag_field[cache->sets][cache->ways];
   unsigned valid_field[cache->sets][cache->ways];
   char mem_request[20];
   uint64_t address;
   srand(time(NULL)); //seed for use in random replacement

   /* Initialization of arrays*/
   for(int i=0; i<cache->sets; i++)
   {
     for(int j=0; j<cache->ways; j++)
     {
       valid_field[i][j] = 0;
       tag_field[i][j] = 0;
     }
   }

   /* Open trace file in read mode */
   FILE *fp;
   fp = fopen(trace_file_name, "r");

   /* Process each address */
   while(fgets(mem_request, 20, fp)!= NULL)
   {
      address = convert_address(mem_request); // get value of hex address
      uint64_t block_addr = address >> (unsigned)log2(BLOCK_SIZE); // get block address
      uint64_t index = block_addr % cache->sets; // index = block address % number of sets
      uint64_t tag = block_addr >> (unsigned)log2(cache->sets); //tag = block address shifted right by n bits where 2^n is number of sets.

       #ifdef DEBUG
          printf("Memory address: %llu, Sets: %u, Ways: %u, Block address: %llu, Index: %llu, Tag: %llu ", address, cache->sets, cache->ways, block_addr, index, tag);
       #endif

       int processed_flag = 0;

       /* Check for hit in appropriate set */
       for (int i=0; i<cache->ways; i++)
       {
         /* If cache space is occupied and tag matches then Hit*/
         if((valid_field[index][i])&&(tag_field[index][i]==tag))
         {
           cache->hits++;
           processed_flag = 1;
           #ifdef DEBUG
                   printf("Hit!\n");
           #endif
           break;
         }
       }
       /* If not a hit then process miss*/
       if(processed_flag==0)
       {
         /* First look for an unused block in the proper set */
         for(int i=0; i<cache->ways; i++)
         {
           if(valid_field[index][i]==0)
           {
             cache->misses++;
             valid_field[index][i] = 1;
             tag_field[index][i] = tag;
             processed_flag = 1;
             #ifdef DEBUG
                     printf("Miss!\n");
             #endif
             break;
           }
         }
         /* if no unused blocks, replace random block in correct set */
         if(processed_flag==0)
         {
             cache->misses++;
             unsigned random_index = rand() % cache->ways; //random number mod number of way gives index in range
             tag_field[index][random_index] = tag;
             #ifdef DEBUG
                     printf(", In Set %llu replaced way = %u! ", index, random_index);
                     printf("Miss!\n");
             #endif
         }
       }
     }

     /* Calculate set hit and miss rates */
     cache->hit_rate = (((double)(cache->hits))/((double)(cache->hits+cache->misses)));
     cache->miss_rate = (((double)(cache->misses))/((double)(cache->hits+cache->misses)));

     /* Call function to print results to screen */
     print_results(cache, trace_file_name);

     /* Call function to write results to file */
     write_to_file(cache, trace_file_name);

     /* Close trace file */
     fclose(fp);
  }

/* This function prints results of a cache simulation to the screen */
void print_results(Cache* cache, char* trace_file_name)
{
  printf("\n========================================\n");
  printf("Cache type:    %s\n", cache->cache_type);
  printf("Replacement:   %s\n", cache->replacement_algorithm);
  printf("Test File:     %s\n", trace_file_name);
  printf("========================================\n");
  printf("Cache Size:    %d\n", CACHE_SIZE);
  printf("Block Size:    %d\n", BLOCK_SIZE);
  printf("Total Blocks:  %d\n", NUM_BLOCKS);
  printf("Sets:          %d\n", cache->sets);
  printf("Ways/Set:      %d\n", cache->ways);
  printf("========================================\n");
  printf("Cache Hits:    %d\n", cache->hits);
  printf("Cache Misses:  %d\n", cache->misses);
  printf("Hit Rate:      %.5f\n", cache->hit_rate);
  printf("Miss Rate:     %.5f\n", cache->miss_rate);
  printf("========================================\n");
}

/* This function writes the results of a cache simulation to file specified in CacheSimulator.h */
void write_to_file(Cache* cache, char* trace_file_name)
{
  FILE *fp;
  fp = fopen(OUTPUT_FILE, "a"); // open in appeand mode

  fprintf(fp,"\n========================================\n");
  fprintf(fp, "Cache type:    %s\n", cache->cache_type);
  fprintf(fp, "Replacement:   %s\n", cache->replacement_algorithm);
  fprintf(fp, "Test File:     %s\n", trace_file_name);
  fprintf(fp, "========================================\n");
  fprintf(fp, "Cache Size:    %d\n", CACHE_SIZE);
  fprintf(fp, "Block Size:    %d\n", BLOCK_SIZE);
  fprintf(fp, "Total Blocks:  %d\n", NUM_BLOCKS);
  fprintf(fp, "Sets:          %d\n", cache->sets);
  fprintf(fp, "Ways/Set:      %d\n", cache->ways);
  fprintf(fp, "========================================\n");
  fprintf(fp, "Cache Hits:    %d\n", cache->hits);
  fprintf(fp, "Cache Misses:  %d\n", cache->misses);
  fprintf(fp, "Hit Rate:      %.5f\n", cache->hit_rate);
  fprintf(fp, "Miss Rate:     %.5f\n", cache->miss_rate);
  fprintf(fp, "========================================\n");

  fclose(fp);
}

/* This function is used when performing a plru cache simulation. It generates *
 * a binary tree with n nodes where n = number_nodes. The nodes are inserted   *
 * one at a time in such a way that the posiion attriubute corresponds to a    *
 * level order traversal. A pointer to the tree is returned                    */
tree_node* create_tree(unsigned number_nodes)
{
  /* Create root node with position = 0*/
  tree_node *tree_root = create_tree_node(0,0);

  #ifdef BSTDEBUG
    printf("\nInserting node %d as root node.\n", tree_root->position);
  #endif

  /* Loop for i = 1 to i<number of nodes creating and inserting nodes at position = i */
  for(unsigned i = 1; i<number_nodes; i++)
  {
    queue *current_queue = create_queue(); /* create new queue structure to traverse to first unused place in tree*/
    enqueue(current_queue, tree_root); /* Enqueue root node*/

    tree_node *temp = create_tree_node(i, 0); /* New node to be placed in tree*/
    tree_node *cursor; /* cursor to traverse tree in level order*/

    /* continue traversing until the end of tree*/
    while(current_queue->front!=NULL)
    {
      /* pop from queue*/
      cursor = dequeue(current_queue);
      /* if at the end of tree and left child == null insert node*/
      if(cursor->left_child==NULL)
      {
        cursor->left_child = temp;
        #ifdef BSTDEBUG
          printf("\nInserting node %d left of node %d.\n", temp->position, cursor->position);
        #endif
        break;
      }
      /* otherwise enqueue left child */
      else
      {
        enqueue(current_queue, cursor->left_child);
      }
      /* if at the end of tree, left child != null, right child ==null, insert node*/
      if(cursor->right_child==NULL)
      {
        #ifdef BSTDEBUG
          printf("\nInserting node %d right of node %d.\n", temp->position, cursor->position);
        #endif
        cursor->right_child = temp;
        break;
      }
      /* otherwise enqueue right child */
      else
      {
        enqueue(current_queue, cursor->right_child);
      }
    }
  }
  /* return pointer to root with nodes inserted */
  return tree_root;
}

/* Creates queue data strucure used in node insertion process */
queue* create_queue()
{
  queue* temp = (queue*)malloc(sizeof(queue));
  temp->front = NULL;
  temp->rear = NULL;
  return temp;
}

/* Enqueue a provided tree_node into the specified queue */
void enqueue(queue* queue, tree_node* node)
{
  // if queue is empty
  if(queue->rear==NULL)
  {
    queue->rear = node;
    queue->front = node;
  }
  else
  {
    queue->rear->next = node;
    queue->rear = node;
  }
}

/* Dequeue the first tree_node in the specified queue and return a pointer to it */
tree_node* dequeue(queue* queue)
{
  //if queue is empty
  if(queue->front == NULL)
  {
    return NULL;
  }
  else
  {
    //pop and set front to new front
    tree_node *temp = queue->front;
    queue->front = queue->front->next;
    if(queue->front == NULL)
    {
      queue->rear = NULL;
    }
    return temp;
  }
}

/* Function to create a bst node used in the plru calculation */
tree_node* create_tree_node(unsigned position, unsigned flag)
{
  tree_node *temp = malloc(sizeof(tree_node));
  temp->position = position;
  temp->flag = flag;
  temp->left_child = NULL;
  temp->right_child = NULL;
  return temp;
}

/* This function is called when a hit occurs during the simulation of a plru  *
 * n-way associative cache. It updates the flags along the path to the        *
 * specified way to direct a plru search away from that way. The flags are    *
 * are set accoring to the bitwise complement of the desired way at each level*/
void hit_update_bst(tree_node* root, unsigned total_ways, unsigned target_way)
{
  #ifdef BSTDEBUG
    printf("\nHit occured! Updating BST Tree.\n");
  #endif

  tree_node *cursor = root; // pointer to root of bst tree

  unsigned n = log2(total_ways); //number of bits needed = number of loop iterations

  unsigned m = n-1; //counter for number of bits to shift off

 /* loop for number of bits necessary, isolating the most significan bit first and then moving towards the least signicant bit */
  for(int i=0; i<n; i++)
  {
    unsigned flag_bit = (((target_way>>m)%2)==0) ? 1 : 0; //convoluted way to move across number's binary form and isolate one digit at a time, then store the complement
    cursor->flag = flag_bit; // Set flag to value

    /* Debugging statements */
    #ifdef BSTDEBUG
      printf("\nTarget Way: %u, Target Way >> m: %u, *%%2: %u, *~: %u, flag bit: %u\n", target_way, (target_way>>m), ((target_way>>m)%2), (~((target_way>>m)%2)), flag_bit);
      printf("\nSetting flags for hit at target_way %u.\n", target_way);
      printf("Setting flag at position %u to value %u.\n", cursor->position, cursor->flag);
    #endif

    // flag bit one indicates go right for plru way. so go left on path to current way
    if(flag_bit==1)
    {
      cursor = cursor->left_child; //move cursor to left child

      #ifdef BSTDEBUG
        printf("Moving cursor to left_child\n");
      #endif
    }
    // flag bit one indicates go left for plru way. so go right on path to current way
    else if(flag_bit==0)
    {
      cursor = cursor->right_child; // move cursor to right child

      #ifdef BSTDEBUG
        printf("Moving cursor to right_child\n");
      #endif
    }
    m--; //decrement m so move to next most signifcant bit on next iteration
  }
}

/* This function is used during a plru n-way cache simulation to find which   *
 * way is targeted for replacement. This is done by reading the flag bit at   *
 * each level of the tree, calculating its contribution in binary form, and   *
 * summing these values. The resulting value is the value of the way least    *
 * recently used (according to the plru algorithm) and is returned.           */
unsigned find_plru_way(tree_node* root, unsigned total_ways)
{
  #ifdef BSTDEBUG
    printf("\nMiss Occured! Cache Full! Determining PLRU way option.\n");
  #endif

  tree_node *cursor = root; // Cursor for tree staring at root
  unsigned accumulator = 0; // Accumulated value to be returned
  unsigned two_power = (log2(total_ways)-1); //Power of two to muliply flag by is equal to log2(total_ways) - 1. Decreases by one at each level of the tree.

  //Continue while there is another node to read
  while(cursor != NULL)
  {
    unsigned flag_value = cursor->flag;
    accumulator = (accumulator + (flag_value*(1<<two_power))); // Accumulated value = value + (flag * (2^(two_power)))

    #ifdef BSTDEBUG
      printf("Flag value %u at position %u. Adding %u to value at depth %u.\n", flag_value, cursor->position, (flag_value*(1<<two_power)), two_power);
    #endif

    two_power--; //as depth increases two_power decreases

    /* if flag == 0 go to left child */
    if(flag_value==0)
    {
      cursor = cursor->left_child;

      #ifdef BSTDEBUG
        printf("Moving to left child.\n");
      #endif
    }
    /* if flag == 1 go to right child */
    else if(flag_value==1)
    {
      cursor = cursor->right_child;

      #ifdef BSTDEBUG
        printf("Moving to right child.\n");
      #endif
    }
  }

  #ifdef BSTDEBUG
    printf("Returning way value %u as replacment candidate.\n", accumulator);
  #endif
  //Value returned is equal to way that needs to be replaced
  return accumulator;
}
