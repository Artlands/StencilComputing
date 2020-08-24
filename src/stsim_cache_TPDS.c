/*
 * _FINITE_C_
 *
 * FUNCTION TO GENERATOR STENCIL GRID MEMORY TRACES
 * ONLY CONSIDER 3-D FINITE DIFFERENCE COMPUTATION
 *
 */

/* Included Files*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <inttypes.h>
#include <fcntl.h>
#include <getopt.h>
#include <math.h>
#include "pims.h"

// #define GENTRACE     // generate traces
// #define DEBUGPT      //debug page table
// #define DEBUGVP      //debug virtual page and offset
// #define DEBUGSET     //debug settings
// #define DEBUGCACHE   //debug cache

/* Global variables for page table*/
uint64_t oldestAge = 0;
uint64_t indexOfOldest = 0;
uint64_t nextEntryIndex = 0;

/* Global variables for cache*/
int *valid;
int **tag;
int **lru;
/* ---------------------------------------------- FUNCTION PROTOTYPES*/
extern int getshiftamount( uint32_t bsize, uint32_t *shiftamt );

extern int getpimsid (int *pimsid,
                      uint64_t addr,
                      uint32_t shiftamt,
                      uint32_t num_vaults);

extern void write_to_file(FILE* fp,
                          char* op,
                          int num_bytes,
                          uint64_t addr);

extern int geneRandom(int size);

extern void mapVirtualaddr(uint64_t virtual_addr,
                           uint64_t ste_entries,
                           uint64_t page_size,
                           pta_node *page_table,
                           uint64_t *physical_addr);

// Write stencil information
extern void write_sten_info(FILE* fp,
                            char* filename,
                            int dim,
                            int dim_x,
                            int dim_y,
                            int dim_z,
                            uint32_t cntr_size,
                            int sten_order);

// Write cache information
extern void write_cache_info( FILE* fp,
                              char* filename,
                              uint64_t ways,
                              uint64_t cache_size,
                              uint64_t block_size,
                              uint64_t total_HOST_REQ,
                              uint64_t total_HOST_RD,
                              uint64_t total_HOST_WR,
                              uint64_t total_HOST_LA,
                              uint64_t total_PIMS_RD,
                              uint64_t total_DRAM_RD,
                              uint64_t total_THROUGHPUT,
                              uint64_t hits,
                              uint64_t misses );

int host_lru_simulation( cache_node *cache, uint64_t address);

/* Main Function. Takes command line arguments, generates stencil grid addresses*/
int main(int argc, char* argv[])
{
  /* vars */
  int ret = 0;
  int i = 0;
  int j = 0;
  int k = 0;
  int r = 0;
  int n = 0;
  uint64_t idx = 0;

  /* STENCIL GRID FEATURES
   *
   * b[i] = a[i]
   * Jocobi iteration stencil, read grid <--> write grid
   */
  int dim = 3;
  int dim_x = 0;
  int dim_y = 0;
  int dim_z = 0;
  int tdim_x = 0;              // total x length
  int tdim_y = 0;              // total y length
  int tdim_z = 0;              // total z length
  uint32_t cntr_size = 0;      // container size
  uint64_t offset = 0x00ll;    // offset between base_a and base_b, base_b and base_c
  int stor_size = 8;           // storage size: double-8
  int sten_order = 2;          // stencil order,refers to the number of elements in each dimension, not counting the element at the intersection
  int sten_ptnum = 1;          // stencil computing elements
  int iteration = 1;           // iterations

  /* HMC */
  uint32_t capacity = 0;    // HMC capacity
  uint32_t bsize = 0;       // HMC blocksize, the less significant bsize bits are ignored
  uint32_t vaults = 0;      // number of vaults
  uint32_t shiftamt = 0;    // based on the blocksize, the consectutive addresses will be distribued across vaults

  /* PIMS */
  int flag = 0;                   // PIMS flag, without PIMS, flag = 0; with PIMS, flag = 1;

  /* MEMORY TRACE */
  uint64_t base_a = 0x00ll;
  uint64_t base_b = 0x00ll;
  FILE *tracefile = NULL;         // save host-memory trace
  FILE *tracelogfile = NULL;     // save trace log
  char filename[1024];
  char tracelog[1024];
  char ops[10];                  // operations

  uint64_t total_HOST_REQ = 0;  // host total requests
  uint64_t total_HOST_RD = 0;   // host memory read requests
  uint64_t total_HOST_WR = 0;   // host memory write requests
  uint64_t total_HOST_LA = 0;
  uint64_t total_PIMS_RD = 0;   // pims memory read requests
  uint64_t total_DRAM_RD = 0;   // pims in memory requests
  uint64_t total_THROUGHPUT = 0;// total throughput

  /* MALLOC MEMORY*/
  uint64_t *tmp_point;               // temporary pointer, for swap grids
  uint64_t *data_cntr_a;             // data container
  uint64_t *data_cntr_b;             // data container
  uint64_t *v_pointer;
  uint64_t *u_pointer;
  uint64_t virtual_addr_a = 0x00ll;  // temporary virtual addr
  uint64_t virtual_addr_b = 0x00ll;  // temporary virtual addr

  /* PAGE TABLE*/
  uint64_t mem_full = 0;               // Main memory size
  uint64_t mem_sten = 0;               // Memory for stencil
  uint64_t page_size = PAGESIZE;       // page size, 4k
  uint64_t all_entries = 0;            // number of all entries
  uint64_t ste_entries = 0;            // entries for stencil pagetable
  pta_node *page_table = NULL;         // page table
  int randframe;                       // generate random page frame

  /* CACHE */
  char cachelog[1024];                 // cache log file name
  FILE *cachelogfile = NULL;
  int cache_size;                      // specified by user
  int num_block;                       // number of blocks in cache

/*----------------------------------------------------------------------------*/

  while( (ret = getopt( argc, argv, "x:y:z:o:i:p:c:b:v:a:h")) != -1 )
  {
    switch( ret )
    {
      case 'x':
        dim_x = (uint32_t)(atoi(optarg));
        break;
      case 'y':
        dim_y = (uint32_t)(atoi(optarg));
        break;
      case 'z':
        dim_z = (uint32_t)(atoi(optarg));
        break;
      case 'o':
        sten_order = (int)(atoi(optarg));
        break;
      case 'i':
        iteration = (int)(atoi(optarg));
        break;
      case 'p':
        flag = (int)(atoi(optarg));
        break;
      case 'c':
        capacity = (uint32_t)(atoi(optarg));
        break;
      case 'b':
        bsize = (uint32_t)(atoi(optarg));
        break;
      case 'v':
        vaults = (uint32_t)(atoi(optarg));
        break;
      case 'a':
        cache_size = (int)(atoi(optarg));
        break;
      case 'h':
        printf("%s%s%s\n", "usage : ", argv[0], " -xyzoipcbvah");
        printf(" -x <stencil grid size on dim x>\n");
        printf(" -y <stencil grid size on dim y>\n");
        printf(" -z <stencil grid size on dim z>\n");
        printf(" -o <stencil order>\n");
        printf(" -i <stencil number of iterations>\n");
        printf(" -p <PIMS flag: 0(without PIMS), 1(with PIMS)>\n");
        printf(" -c <HMC capacity: 4, 8>");
        printf(" -b <HMC blocksize: 32, 64, 128, 256>");
        printf(" -v <HMC vaults: 16, 32>");
        printf(" -a <Cache size: 32, 64, 128, 256, 512, 1024 in KB>");
        printf(" h ...print help\n");
        return 0;
        break;
      default:
          printf("%s%s%s\n", "Unknown option: see ", argv[0], " -xyzoipcbvah" );
          return -1;
          break;
    }
  }

#ifdef DEBUGSET
    printf("Dimension X is %" PRId32"\n", dim_x);
    printf("Dimension Y is %" PRId32"\n", dim_y);
    printf("Dimension Z is %" PRId32"\n", dim_z);
    printf("HMC capacity is %d\n", capacity);
    printf("HMC blocksize is %" PRId32"\n", bsize);
#endif

  /* ---- Sanity Check ---- */
  if( dim_x < 0 || dim_y < 0 || dim_z < 0 ) {
    printf("ERROR: Grid size is invalid\n");
    return -1;
  }

  if ( sten_order < 2 || (sten_order % 2) != 0 ) {
    printf("ERROR: Stencil order is invalid\n");
    return -1;
  }

  if( iteration < 1 )
  {
    printf("ERROR: Stencil number of iterations is invalid\n");
    return -1;
  }

  if( flag != 0 ) {
    printf("ERROR: PIMS flag is invalid");
    return -1;
  }

  if ( capacity != 4 && capacity != 8 ) {
    printf("ERROR: Capacity is invalid\n");
    return -1;
  }

  if ( bsize != 32 && bsize != 64 && bsize != 128 && bsize != 256 ) {
    printf("ERROR: Block size is invalid\n");
    return -1;
  }

  if ( vaults != 16 && vaults != 32 ) {
    printf("ERROR: Number of Vaults is invalid\n");
    return -1;
  }

  sprintf(tracelog, "../traces/trace_ana_TPDS.log");
  tracelogfile = fopen(tracelog, "a");
  if( tracelogfile == NULL )
  {
    printf("ERROR: Cannot open trace log file\n");
    return -1;
  }

  sprintf(cachelog, "../traces/cache_ana_TPDS.log");
  cachelogfile = fopen(cachelog, "a");
  if( cachelogfile == NULL )
  {
    printf("ERROR: Cannot open cache log file\n");
    return -1;
  }
  /* ---- End Sanity Check ---- */

  /* Set throughput coefficient */

  /*Get total grid size*/
  tdim_x = dim_x + sten_order;
  tdim_y = dim_y + sten_order;
  tdim_z = dim_z + sten_order;

  /* Get Container Size */
  cntr_size = (uint32_t)((uint32_t)(tdim_x)
                       * (uint32_t)(tdim_y)
                       * (uint32_t)(tdim_z));

#ifdef DEBUGSET
        printf("%s%u\n", "Grid size (including ghost cells): ", cntr_size);
        printf("%s%ld\n", "One GB: ", (long)SLOTS_PER_GB);
        printf("%s%ld\n", "HMC capacity (in GB): ",
              (long)((long)capacity * (long)SLOTS_PER_GB));
        printf("%s%u\n", "Need memory: ",
              (uint32_t)(cntr_size * 2 * stor_size));
#endif

  /* Make sure we have enough HMC capacity */
  if( (long)((long)capacity * (long)SLOTS_PER_GB) <
      (long)(cntr_size * 2 * stor_size) )
  {
    printf("ERROR: NOT ENOUGH AVAILABLE PHYSICAL STORAGE\n");
    return -1;
  }

  /* Get the shift amount based upon the block size */
  if( getshiftamount( bsize, &shiftamt) != 0 ) {
    printf("ERROR: Failed to retrive shift amount\n");
    return -1;
  }
  /*----------------------------------------------------------------------------
   *
   * Initialize the pagetable
   *
   */
  printf("Initialize Page Table...\n");
  mem_full = (uint64_t)((uint64_t)capacity * (uint64_t)SLOTS_PER_GB);
  all_entries = (uint64_t)((uint64_t)mem_full / (uint64_t)page_size);

  // memory allocated for stencil kernel
  mem_sten = (uint64_t)((uint64_t)(cntr_size * 2) * (uint64_t)(stor_size));

  // stencil entries
  ste_entries = (uint64_t)(((uint64_t)mem_sten / (uint64_t)page_size) + 1);

  randframe = geneRandom((int)all_entries); // 2^21 -1

#ifdef DEBUGSET
  printf("%s%d\n",   "Capacity       : ", capacity);
  printf("%s%llu\n", "Stencil memory : ", mem_sten);
  printf("%s%llu\n", "All entries    : ", all_entries);
  printf("%s%llu\n", "Stencil entries: ", ste_entries);
#endif

  page_table = (pta_node *)malloc(ste_entries * sizeof(pta_node));
  if( page_table == NULL )
  {
    printf("Error: Out of memory\n");
    goto cleanup;
  }

  /* Generate random page frame to page table */
  for( i = 0; i < ste_entries ; i++ )
  {
#ifdef DEBUGSET
    printf("%s%d\n", "Random frame     : ", randframe);
#endif
    page_table[i].virtual_page = -1;
    page_table[i].page_frame = randframe;
    page_table[i].age = 0;
    randframe = geneRandom(-1);
  }
  printf("Done!\n");
  /*----------------------------------------------------------------------------
   *
   * Initialize the host cache
   *
   */
  printf("Initialize Host Cache...\n");
  num_block = (cache_size * 1024)/BLOCKSIZE;
  cache_node host_cache = {.cache_size = cache_size * 1024,
                           .block_size = BLOCKSIZE,
                           .ways = WAYS,
                           .sets = num_block/WAYS,
                           .hits = 0,
                           .misses = 0};
#ifdef DEBUGSET
  printf("# Blocks: %d\n", num_block);
#endif

  valid  = (int *) malloc( sizeof( int ) * host_cache.sets);
  tag = (int **) malloc( sizeof( int *) * host_cache.sets);
  lru = (int **) malloc( sizeof( int *) * host_cache.sets);

  if( valid == NULL || tag == NULL || lru == NULL )
  {
    printf("Error: Out of memory\n");
    goto cleanup;
  }

  for( i = 0; i < host_cache.sets; i++ )
  {
    tag[i] = (int *) malloc( sizeof( int ) * host_cache.ways);
    lru[i] = (int *) malloc( sizeof( int ) * host_cache.ways);
    if(tag[i] == NULL || lru[i] == NULL)
    {
      printf("Error: Out of memory\n");
      goto cleanup;
    }
  }

  printf("Initialize Cache...\n");
  for( i = 0; i < host_cache.sets; i++ )
  {
    for( j = 0; j < host_cache.ways; j++ )
    {
      tag[i][j] = -1;
      lru[i][j] = -1;
    }
    valid[i] = 0;
  }
  printf("Done!\n");

  /*----------------------------------------------------------------------------
   *
   * Caculate the base of each vector
   *
   */
  offset = (uint64_t)((uint64_t)stor_size * (uint64_t)(cntr_size + 1));

  /* Manually select the start address, it can be any arbitrary address */
  base_a = 0x0000000072CE2096;
  base_b = base_a + offset + 0x00000000000A03F0;

  printf("%s\n", "Allocating memory space... ");

  /* Allocate contiguous memory space for storing stencil grid addresses and coefficients */
  data_cntr_a = (uint64_t *) malloc( sizeof( uint64_t ) * cntr_size);
  data_cntr_b = (uint64_t *) malloc( sizeof( uint64_t ) * cntr_size);

  if( data_cntr_a == NULL || data_cntr_b == NULL )
  {
    printf("Error: Out of memory\n");
    goto cleanup;
  }

  printf("Done!\n");

  printf("%s\n", "Generating addresses... ");

  /* Generate memory addresses, store them in data container */
  for( i = 0; i < cntr_size; i++ )
  {
    virtual_addr_a = (uint64_t)( base_a + (uint64_t)(i * stor_size) );
    virtual_addr_b = (uint64_t)( base_b + (uint64_t)(i * stor_size) );
    mapVirtualaddr(virtual_addr_a,
                   ste_entries,
                   page_size,
                   page_table,
                   &data_cntr_a[i]);
    mapVirtualaddr(virtual_addr_b,
                   ste_entries,
                   page_size,
                   page_table,
                   &data_cntr_b[i]);
  }
  /* End Allocation */

  /*
   *
   * Order varies
   *
   */
  sten_ptnum = sten_order * 3 + 1;
  if( flag == 1 )
  {
    sprintf(filename, "../traces/PIMS-%dD-%dpoints-O%dX%dY%dZ%d",
            dim, sten_ptnum, sten_order, dim_x, dim_y, dim_z);
  }
  else
  {
    sprintf(filename, "../traces/HOST-%dD-%dpoints-O%dX%dY%dZ%d",
            dim, sten_ptnum, sten_order, dim_x, dim_y, dim_z);
  }
#ifdef GENTRACE
  tracefile = fopen(filename, "w");
  if( tracefile == NULL )
  {
    printf("ERROR: Cannot open trace file\n");
    goto cleanup;
  }
#endif
  // Write Stencil information
  write_sten_info(tracelogfile,
                  filename,
                  dim,
                  dim_x,
                  dim_y,
                  dim_z,
                  cntr_size,
                  sten_order);
  /*
   *  Cache simulation and generate memory trace files
   *
   */
  for( n = 0; n < iteration; n++ )
  {
   // Read grid a, wirte grid b----------------------------------------------
    for( k = sten_order/2; k < (dim_z+sten_order/2); k++)
    {
      for( j = sten_order/2; j < (dim_y+sten_order/2); j++)
      {
        v_pointer = &data_cntr_a[j*tdim_x + k * tdim_x * tdim_y];
        u_pointer = &data_cntr_b[j*tdim_x + k * tdim_x * tdim_y];
        for( i = sten_order/2; i < (dim_x+sten_order/2); i++ )
        {
          if( flag == 0 )
          {
            // Normal
            memset(ops, 0, sizeof(ops));
            sprintf(ops, "HOST_RD");

            // Read central point
            ret = host_lru_simulation(&host_cache, v_pointer[i]);
            if( ret == -1 )
            {
#ifdef GENTRACE
              write_to_file(tracefile, ops, stor_size, v_pointer[i]);
#endif
            }
            total_HOST_RD++;

            // Read order points
            for( r = 1; r <= (sten_order/2); r++ )
            {

              // dim_x
              ret = host_lru_simulation(&host_cache, v_pointer[i-r]);
              if( ret == -1 )
              {
#ifdef GENTRACE
                write_to_file(tracefile, ops, stor_size, v_pointer[i-r]);
#endif
              }
              total_HOST_RD++;

              ret = host_lru_simulation(&host_cache, v_pointer[i+r]);
              if( ret == -1 )
              {
#ifdef GENTRACE
                write_to_file(tracefile, ops, stor_size, v_pointer[i+r]);
#endif
              }
              total_HOST_RD++;
              // dim_y
              ret = host_lru_simulation(&host_cache, v_pointer[i - r * tdim_x]);
              if( ret == -1 )
              {
#ifdef GENTRACE
                write_to_file(tracefile, ops, stor_size, v_pointer[i - r * tdim_x]);
#endif
              }
              total_HOST_RD++;

              ret = host_lru_simulation(&host_cache, v_pointer[i + r * tdim_x]);
              if( ret == -1 )
              {
#ifdef GENTRACE
                write_to_file(tracefile, ops, stor_size, v_pointer[i + r * tdim_x]);
#endif
              }
              total_HOST_RD++;
              // dim_z
              ret = host_lru_simulation(&host_cache, v_pointer[i - r * tdim_x * tdim_y]);
              if( ret == -1 )
              {
#ifdef GENTRACE
                write_to_file(tracefile, ops, stor_size, v_pointer[i - r * tdim_x * tdim_y]);
#endif
              }
              total_HOST_RD++;

              ret = host_lru_simulation(&host_cache, v_pointer[i + r * tdim_x * tdim_y]);
              if( ret == -1 )
              {
#ifdef GENTRACE
                write_to_file(tracefile, ops, stor_size, v_pointer[i + r * tdim_x * tdim_y]);
#endif
              }
              total_HOST_RD++;
            }
            // Write result
            memset(ops, 0, sizeof(ops));
            sprintf(ops, "HOST_WR");

            // Add write cache  -TPDS2020-revise
            ret = host_lru_simulation(&host_cache, u_pointer[i]);
            if( ret == -1)
            {
#ifdef GENTRACE
              write_to_file(tracefile, ops, stor_size, u_pointer[i]);
#endif  
            }
            total_HOST_WR++;
          }  //Endif flag
          else{
             /* PIMS SIMULATION*/
          }
        }    //Endfor i
      }      //Endfor j
    }        //Endfor k
    /*switch container pointer*/
    tmp_point = data_cntr_a;
    data_cntr_a = data_cntr_b;
    data_cntr_b = tmp_point;
  }          // Endfor iteration

  printf("Done!\n");

  if( flag == 0 )
  {
    total_HOST_REQ = total_HOST_RD + total_HOST_WR;
    total_THROUGHPUT = (uint64_t)(host_cache.misses * (uint64_t)(64));
  }
  else
  {
    total_HOST_REQ = total_HOST_RD + total_HOST_WR + total_PIMS_RD;
    total_THROUGHPUT = (uint64_t)(total_HOST_RD * (uint64_t)(8))
                     + (uint64_t)(host_cache.misses * (uint64_t)(64));
  }
  printf("Write cache information!\n");
  write_cache_info( cachelogfile, 
                    filename, 
                    host_cache.ways,
                    host_cache.cache_size, 
                    host_cache.block_size,
                    total_HOST_REQ,
                    total_HOST_RD,
                    total_HOST_WR,
                    total_HOST_LA,
                    total_PIMS_RD,
                    total_DRAM_RD,
                    total_THROUGHPUT,
                    host_cache.hits,
                    host_cache.misses );
  printf("Finish simulation!\n");

cleanup:
  /* ----------------------Close file---------------------- */
  if( tracefile != NULL )
  {
    fclose(tracefile);
  }
  if( tracelogfile != NULL )
  {
    fclose(tracelogfile);
  }
  if( cachelogfile != NULL )
  {
    fclose(cachelogfile);
  }

  /* ----------------------Deallocate memory---------------------- */
  printf("Free pagetable-sim!\n");
  if( page_table != NULL )
  {
    free(page_table);
  }

  printf("Free cache-sim!\n");
  if( valid != NULL )
  {
    free(valid);
  }

  if( tag != NULL )
  {
    for( i = 0; i < host_cache.sets; i++ )
    {
      if( tag[i] != NULL )
      {
        free(tag[i]);
      }
    }
    free(tag);
  }

  if( lru != NULL )
  {
    for( i = 0; i < host_cache.sets; i++ )
    {
      if( lru[i] != NULL )
      {
        free(lru[i]);
      }
    }
    free(lru);
  }

  printf("Free stencil memory traces container!\n");
  if( data_cntr_a != NULL )
  {
    printf("--Free data container a!\n");
    free(data_cntr_a);
    printf("--Done!\n");
  }

  if( data_cntr_b != NULL )
  {
    printf("--Free data container b!\n");
    free(data_cntr_b);
    printf("--Done!\n");
  }
  return 0;
}
/* --------------------------------------------- MAPVIRTUALADDR */
extern void mapVirtualaddr(uint64_t virtual_addr,
                           uint64_t ste_entries,
                           uint64_t page_size,
                           pta_node *page_table,
                           uint64_t *physical_addr)
{
  /* vars */
  int i = 0;

  /* Get virtual page and offset */
  int64_t virtual_page = (int64_t)((virtual_addr & VIRTUAL_PAGE_MASK)
                                   >> VIRTUAL_PAGE_SHIFT);
  int64_t offset = (int64_t)(virtual_addr & VIRTUAL_OFFSET_MASK);

#ifdef DEBUGPT
  printf("Virtual page:%"PRIX64"\n", virtual_page);
  printf("Offset:%"PRIX64"\n", offset);
#endif

  /* LRU page replacement algorithm
   *
   * nextEntryIndex: point to the next available entry
   * if nextEntryIndex < ste_entries,
   *    // do not need to consider age
   *    traverse from 0 to nextEntryIndex to find if pagetable hit
   *    if hit,return
   *    else add new entry, nextEntryIndex++
   * else
        // pagetable is full
   *    traverse from 0 to ste_entries to find if a pagetable hit
   *    if hit, return
   *    else age++, find indexOfOldest, replace it to the new entry, return
   *
   */

  if( nextEntryIndex < ste_entries )
  {
    for( i = 0; i < nextEntryIndex; i++ )
    {
      // Page table hit
      if( page_table[i].virtual_page == virtual_page )
      {
        *physical_addr = (uint64_t)( ( page_table[i].page_frame
                                       << VIRTUAL_PAGE_SHIFT) | offset );
        return;
      }
    }
    // Add new entry
    page_table[nextEntryIndex].virtual_page = virtual_page;
    *physical_addr = (uint64_t)( ( page_table[nextEntryIndex].page_frame
                                   << VIRTUAL_PAGE_SHIFT) | offset);
    nextEntryIndex ++;
    return;
  }
  else
  {
    for( i = 0; i < ste_entries; i++)
    {
      // Page table hit
      if( page_table[i].virtual_page == virtual_page )
      {
        *physical_addr = (uint64_t)( ( page_table[i].page_frame
                                       << VIRTUAL_PAGE_SHIFT) | offset );
        return;
      }
      else
      {
        page_table[i].age++;
        if( page_table[i].age > oldestAge )
        {
          oldestAge = page_table[i].age;
          indexOfOldest = i;
        }
      }
    }
    page_table[indexOfOldest].virtual_page = virtual_page;
    *physical_addr = (uint64_t)( ( page_table[indexOfOldest].page_frame
                                   << VIRTUAL_PAGE_SHIFT) | offset );
    return;
  }
}
/* EOF */

// /* --------------------------------------------- host_lru_simulation */
int host_lru_simulation( cache_node *cache, uint64_t address )
{
  uint64_t block_addr = (uint64_t)( address / (cache->block_size) );
  uint64_t index = (uint64_t)( block_addr % cache->sets);
  uint64_t tag_val = (uint64_t)( block_addr >> (uint64_t)log2(cache->sets));

#ifdef DEBUGCACHE
  printf("%s%016" PRIX64 "\n", "Physcial addr: ", address);
  printf("%s%016" PRIX64 "\n", "Block addr: ", block_addr);
  printf("%s%016" PRIX64 "\n", "Index: ", index);
  printf("%s%016" PRIX64 "\n", "Tag: ", tag_val);
#endif

  int hit_flag;
  int found;
  int i;
  int j;
  int lru1;
  int result;         // -1 means miss, 1 means hit

  // Compare tag
  if( valid[index] == 0 )
  {
#ifdef DEBUGCACHE
    printf("Invalid!\n");
#endif
    valid[index] = 1;
    tag[index][0] = tag_val;
    lru[index][0] = 0;

    // Miss!
    cache->misses ++;
    result = -1;
  }
  else if( valid[index] == 1 )
  {
    hit_flag = 0;
    for( i = 0; i < cache->ways; i ++ )
    {
      if( tag[index][i] == tag_val )
      {
#ifdef DEBUGCACHE
    printf("HIT!!\n");
#endif
        // Hit!
        hit_flag = 1;
        result = 1;
        cache->hits ++;
        //update lru
        for( j = 0; j < cache->ways; j ++)
        {
          if( (lru[index][j] != -1) && (lru[index][j] < lru[index][i]) )
          {
            lru[index][j] = lru[index][j] + 1;
          }
        }
        lru[index][i] = 0;
        break;
      }
    }
    if( hit_flag == 0 )
    {
#ifdef DEBUGCACHE
    printf("MISS!!\n");
#endif
      // Miss!
      result = -1;
      cache->misses ++;
      found = 0;
      // find the lru block in cache
      for( i = 0; i < cache->ways; i++ )
      {
        if( lru[index][i] == -1 )
        {
          found = 1;
          tag[index][i] = tag_val;
          // update lru
          for( j = 0; j < i; j++ )
          {
            lru[index][j] = lru[index][j] + 1;
          }
          lru[index][i] = 0;
          break;
        }
      }
      if( found == 0 )
      {
        //all blocks are in use
        // replace the highest lru
        lru1 = 0;
        for( j = 0; j < cache->ways; j++ )
        {
          if( lru[index][j] == (cache->ways - 1) )
          {
            lru1 = j;
            break;
          }
        }
        tag[index][lru1] = tag_val;

        for( j = 0; j < cache->ways; j++ )
        {
          lru[index][j] = lru[index][j] + 1;
        }
        lru[index][lru1] = 0;
      }
    } // Endif hit_flag
  }   // Endelif Valid
  return result;
}
