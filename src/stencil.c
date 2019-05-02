/*
 * _GRID_GENERATOR_C_
 *
 * FUNCTION TO GENERATOR STENCIL GRID VIRTUAL MEMORY ADDRESSES
 * AND TRANSLATE INTO PHYSICAL ADDRESSES
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

// #define DEBUG

/* Global variables */
uint64_t pta_miss = 0;
uint64_t oldestAge = 0;
uint64_t indexOfOldest = 0;
uint64_t nextEntryIndex = 0;

int *valid;
int **tag;
int **lru;

/* ---------------------------------------------- FUNCTION PROTOTYPES*/

extern int getshiftamount( uint32_t bsize, uint32_t *shiftamt );
extern int getpimsid (int *pimsid, uint64_t addr, uint32_t shiftamt, uint32_t num_vaults);
extern void write_to_file(FILE* fp, char* op, int num_bytes, int procid, uint64_t addr);

extern int geneRandom(int size);
extern void mapVirtualaddr(uint64_t virtual_addr, uint64_t entries, uint64_t page_size,
                    pta_node *page_table, uint64_t *physical_addr);

// Write stencil information
extern void write_sten_info(FILE* fp, char* filename, int dim,
                     int dim_x, int dim_y, int dim_z,
                     uint32_t cntr_size, int sten_order,
                     int sten_coeff, char* data_type);

// Write cache information
extern void write_cache_info( FILE* fp, char* filename, uint64_t ways,
                      uint64_t cache_size, uint64_t block_size,
                      uint64_t hits, uint64_t misses,
                      double hit_rate, double miss_rate );

int run_lru_simulation( cache_node *cache, uint64_t address );

/* Main Function. Takes command line arguments, generates stencil grid addresses*/
int main(int argc, char* argv[])
{
  /* vars */
  int ret = 0;
  int i = 0;
  int j = 0;
  int k = 0;
  int r = 0;
  uint64_t idx = 0;

  /* STENCIL GRID FEATURES
   *
   * b[i] = a[i]
   * Jocobi iteration stencil, read grid <--> write grid
   */
  int dim_x = 0;
  int dim_y = 0;
  int dim_z = 0;
  uint32_t dim = 0;            // dimension of the grid
  uint32_t cntr_size = 0;      // container size
  uint64_t offset = 0x00ll;    // offset between base_a and base_b
  uint64_t stor_size = 0x00ll; // storage size: intger-4, double-8
  char data_type[10];          // stencil data type
  int sten_type = 0;           // stencil type: 0, 1, 2
  int sten_order = 1;          // stencil order
  int sten_ptnum = 1;          // stencil computing elements
  int sten_coeff = 1;           // number of coefficients


  /* HMC */
  uint32_t capacity = 0;    // HMC capacity
  uint32_t bsize = 0;       // HMC blocksize, the less significant bsize bits are ignored
  uint32_t vaults = 0;      // number of vaults
  uint32_t shiftamt = 0;    // based on the blocksize, the consectutive addresses will be distribued across vaults

  /* PIMS */
  int flag = 0;             // PIMS flag, without PIMS, flag = 0; with PIMS, flag = 1;
  int procid;          // 32: request from host, 0-31: request from PIMS

  /* MEMORY TRACE */
  uint64_t base_a = 0x00ll;
  uint64_t base_b = 0x00ll;
  FILE *outfile = NULL;     // save trace file
  FILE *tracelogfile = NULL;     // save trace log
  char filename[1024];
  char tracelog[1024];
  char ops[10];
  int num_bytes = 1;            // number of bytes for each ops

  /* MALLOC MEMORY*/
  uint64_t *data_cntr_a;             // data container
  uint64_t *grid_1d_a;               // pointer container
  uint64_t **grid_2d_a;              // pointer container
  uint64_t ***grid_3d_a;             // pointer container
  uint64_t *data_cntr_b;             // data container
  uint64_t *grid_1d_b;               // pointer container
  uint64_t **grid_2d_b;              // pointer container
  uint64_t ***grid_3d_b;             // pointer container
  uint64_t virtual_addr_a = 0x00ll;  // temporary store
  uint64_t virtual_addr_b = 0x00ll;  // temporary store

  /* PAGE TABLE*/
  uint64_t memSize = 0;                // Main memory size
  uint64_t page_size = PAGESIZE;       // page size
  uint64_t entries = 0;                // number of entries
  pta_node *page_table = NULL;         // page table
  int randframe;                       // generate random page frame

  /* CACHE */
  char cachelog[1024];
  FILE *cachelogfile = NULL;
  // cache_node *cache;
  int cache_size;                     // specified by user
  int num_block;

/*----------------------------------------------------------------------------*/

  while( (ret = getopt( argc, argv, "x:y:z:t:T:O:C:p:c:b:v:a:h")) != -1 )
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
      case 't':
        sprintf(data_type, "%s", optarg);
        break;
      case 'T':
        sten_type = (int)(atoi(optarg));
        break;
      case 'O':
        sten_order = (int)(atoi(optarg));
        break;
      case 'C':
        sten_coeff = (int)(atoi(optarg));
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
        printf("%s%s%s\n", "usage : ", argv[0], " -xyztfh");
        printf(" -x <stencil grid size on dim x>\n");
        printf(" -y <stencil grid size on dim y>\n");
        printf(" -z <stencil grid size on dim z>\n");
        printf(" -t <stencil grid data type: Integer, Double>\n");
        printf(" -T <stencil type: 0(Order varies), 1(2D-9points), 2(3D-27points)>\n");
        printf(" -O <stencil order>\n");
        printf(" -C <stencil number of coefficients>\n");
        printf(" -p <PIMS flag: 0(without PIMS), 1(with PIMS)>\n");
        printf(" -c <HMC capacity: 4, 8>");
        printf(" -b <HMC blocksize: 32, 64, 128, 256>");
        printf(" -v <HMC vaults: 16, 32>");
        printf(" -a <Cache size: 32, 4096, 8192 in KB>");
        printf(" h ...print help\n");
        return 0;
        break;
      default:
          printf("%s%s%s\n", "Unknown option: see ", argv[0], " -xyztTOPcbvh" );
          return -1;
          break;
    }
  }

#ifdef DEBUG
    printf("Dimension X is %" PRId32"\n", dim_x);
    printf("Dimension Y is %" PRId32"\n", dim_y);
    printf("Dimension Z is %" PRId32"\n", dim_z);
    printf("HMC capacity is %d\n", capacity);
    printf("HMC blocksize is %" PRId32"\n", bsize);
#endif

  /* ---- Sanity Check ---- */
  if ( dim_x == 0 ) {
    printf("ERROR: Grid size is invalid\n");
    return -1;
  } else if( dim_y == 0 && dim_z != 0 ) {
    printf("ERROR: Grid size is invalid\n");
    return -1;
  } else if( dim_x < 0 || dim_y < 0 || dim_z < 0 ) {
    printf("ERROR: Grid size is invalid\n");
    return -1;
  }

  if ( sten_type != 0 && sten_type != 1 && sten_type != 2 ) {
    printf("ERROR: Stencil type is invalid\n");
    return -1;
  }

  if( sten_type == 1 && dim_y == 0) {
    printf("ERROR: Grid size is invalid\n");
    return -1;
  }

  if( (sten_type == 2 && dim_y == 0) || (sten_type == 2 && dim_z == 0)) {
    printf("ERROR: Grid size is invalid\n");
    return -1;
  }

  if ( sten_order < 1 ) {
    printf("ERROR: Stencil order is invalid\n");
    return -1;
  }

  if( sten_coeff <= 0 )
  {
    printf("ERROR: Stencil number of coefficients is invalid\n");
    return -1;
  }

  switch (sten_type) {
    case 0:
      if( sten_coeff > (2 * sten_order + 1) )
      {
        printf("ERROR: Stencil number of coefficients is invalid\n");
        return -1;
      }
      break;
    case 1:
      if( sten_coeff > 9 )
      {
        printf("ERROR: Stencil number of coefficients is invalid\n");
        return -1;
      }
      break;
    case 2:
      if( sten_coeff != 4 )
      {
        printf("ERROR: Stencil number of coefficients is invalid\n");
        return -1;
      }
      break;
    default:
      return -1;
      break;
  }

  if( flag != 0 && flag != 1) {
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

  /* Get Dimension and Container Size */
  if( dim_y == 0 )
  {
    dim = 1;
    cntr_size = (uint32_t)dim_x;
  }
  else if( dim_z == 0 )
  {
    dim = 2;
    cntr_size = (uint32_t)((uint32_t)dim_x * (uint32_t)dim_y);
  }
  else
  {
    dim = 3;
    cntr_size = (uint32_t)((uint32_t)dim_x * (uint32_t)dim_y * (uint32_t)dim_z);
  }

#ifdef DEBUG
        printf("%s%d\n", "Grid Dimension: ", dim);
        printf("%s%u\n", "Grid size: ", cntr_size);
#endif

  switch(dim)
  {
    case 1:
      if( sten_order > (dim_x - 1)/2 )
      {
        printf("ERROR: Stencil order is invalid\n");
        return -1;
      }
      break;
    case 2:
      if( sten_order > (dim_x - 1)/2 || sten_order > (dim_y - 1)/2 )
      {
        printf("ERROR: Stencil order is invalid\n");
        return -1;
      }
      break;
    case 3:
      if( sten_order > (dim_x - 1)/2 || sten_order > (dim_y - 1)/2
          || sten_order > (dim_z - 1)/2 )
      {
        printf("ERROR: Stencil order is invalid\n");
        return -1;
      }
      break;
    default:
      printf("Error: Unknown dimension\n");
      return -1;
      break;
  }
  /* ---- End Sanity Check ---- */

  sprintf(tracelog, "../traces/trace.log");
  tracelogfile = fopen(tracelog, "a");
  if( tracelogfile == NULL )
  {
    printf("ERROR: Cannot open trace log file\n");
    return -1;
  }

  sprintf(cachelog, "../traces/cache.log");
  cachelogfile = fopen(cachelog, "a");
  if( cachelogfile == NULL )
  {
    printf("ERROR: Cannot open cache log file\n");
    return -1;
  }

  printf("%s\n", "Checking HMC capacity... ");
  /*
   * Make sure we have enough HMC capacity
   *
   */
  switch(data_type[0])
  {
    case 'i':
    case 'I':
      stor_size = 0x04ll;
      if( (long)((long)capacity * (long)SLOTS_PER_GB) <
          (long)(cntr_size * 2 * 4) )
      {
        printf("ERROR: NOT ENOUGH AVAILABLE PHYSICAL STORAGE\n");
        return -1;
      }
      break;
    case 'd':
    case 'D':
      stor_size = 0x08ll;
      if( (long)((long)capacity * (long)SLOTS_PER_GB) <
          (long)(cntr_size * 2 * 8) )
      {
        printf("ERROR: NOT ENOUGH AVAILABLE PHYSICAL STORAGE\n");
        return -1;
      }
      break;
    default:
      printf("Error: Unknow data type\n");
      return -1;
      break;
  }

  /*
   * Get the shift amount based upon the block size
   *
   */
  if( getshiftamount( bsize, &shiftamt) != 0 ) {
    printf("ERROR: Failed to retrive shift amount\n");
    return -1;
  }

  /*
   * Init process id, default hostid = vaults
   *
   */
  procid = vaults;
/*----------------------------------------------------------------------------*/
  /*
   * Initialize the pagetable
   *
   */
  memSize = (uint64_t)((uint64_t)capacity * (uint64_t)SLOTS_PER_GB);
  entries = memSize / page_size;
  randframe = geneRandom((int)entries); // 2^21 -1

#ifdef DEBUG
  printf("%s%d\n", "Capacity: ", capacity);
  printf("%s%llu\n", "Memory: ", memSize);
  printf("%s%llu\n", "Entries: ", entries);
  // int randframeT = geneRandom((int)entries); // 2^21 -1
  // for ( i = 0; i < entries; i++ )
  // {
  //   printf("%s%d\n", "Random NUM: ", randframeT);
  //   randframeT = geneRandom(-1);
  // }
#endif
  page_table = (pta_node *)malloc(entries * sizeof(pta_node));
  if( page_table == NULL )
  {
    printf("Error: Out of memory\n");
    return -1;
  }

  /* Generate random page frame to page table */
  for( i = 0; i < entries ; i++ )
  {
    page_table[i].virtual_page = -1;
    page_table[i].page_frame = randframe;
    page_table[i].age = 0;
    randframe = geneRandom(-1);
  }
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
  /*
   * Initialize the cache
   *
   */
  num_block = cache_size/BLOCKSIZE;
  cache_node cache = {.cache_size = cache_size,
                      .block_size = BLOCKSIZE,
                      .ways = WAYS,
                      .sets = num_block/WAYS,
                      .hits = 0,
                      .misses = 0};

#ifdef DEBUG
  printf("# Blocks: %d\n", num_block);
#endif

  valid  = (int *) malloc( sizeof( int ) * cache.sets);
  tag = (int **) malloc( sizeof( int *) * cache.sets);
  lru = (int **) malloc( sizeof( int *) * cache.sets);
  // dirty = (int **) malloc( sizeof( int *) * cache.sets);

  for( i = 0; i < cache.sets; i++ )
  {
    tag[i] = (int *) malloc( sizeof( int ) * cache.ways);
    lru[i] = (int *) malloc( sizeof( int ) * cache.ways);
    // dirty[i] = (int *) malloc( sizeof( int ) * cache.ways);
  }

  printf("Initialize Cache...\n");
  for( i = 0; i < cache.sets; i++ )
  {
    for( j = 0; j < cache.ways; j++ )
    {
      tag[i][j] = -1;
      lru[i][j] = -1;
      // dirty[i][j] = 0;
    }
    valid[i] = 0;
  }
/*----------------------------------------------------------------------------*/

  /*
   * Caculate the base of each vector
   *
   */
  offset = (uint64_t)(stor_size * (cntr_size + 1));

  /* Manually select the start address, it can be any arbitrary address */
  base_a = 0x0000000072CE2090;
  base_b = base_a + 0x0000000000E59068 + offset;

  printf("%s\n", "Allocating memory space... ");

  /* Allocate contiguous memory space for storing stencil grid addresses*/
  data_cntr_a = (uint64_t *) malloc( sizeof( uint64_t ) * cntr_size);
  data_cntr_b = (uint64_t *) malloc( sizeof( uint64_t ) * cntr_size);

  if( data_cntr_a == NULL || data_cntr_b == NULL)
  {
    printf("Error: Out of memory\n");
    return -1;
  }

  switch(dim)
  {
    case 1:
      grid_1d_a = data_cntr_a;
      grid_1d_b = data_cntr_b;
      break;
    case 2:
      grid_2d_a = (uint64_t **) malloc( sizeof(uint64_t *) * dim_x );
      grid_2d_b = (uint64_t **) malloc( sizeof(uint64_t *) * dim_x );
      if( grid_2d_a == NULL || grid_2d_b == NULL )
      {
        printf("Error: Out of memory\n");
        free(data_cntr_a);
        free(data_cntr_b);
        return -1;
      }
      for(i = 0; i < dim_x; i++)
      {
        idx = i * dim_y;
        grid_2d_a[i] = &data_cntr_a[idx];
        grid_2d_b[i] = &data_cntr_b[idx];
      }
      break;
    case 3:
      grid_3d_a = (uint64_t ***) malloc( sizeof(uint64_t **) * dim_x );
      grid_3d_b = (uint64_t ***) malloc( sizeof(uint64_t **) * dim_x );
      if( grid_3d_a == NULL ||  grid_3d_b == NULL )
      {
        printf("Error: Out of memory\n");
        free(data_cntr_a);
        free(data_cntr_b);
        return -1;
      }
      for(i = 0; i < dim_x; i++)
      {
        grid_3d_a[i] = (uint64_t **) malloc( sizeof(uint64_t *) * dim_y);
        grid_3d_b[i] = (uint64_t **) malloc( sizeof(uint64_t *) * dim_y);
        for(j = 0; j < dim_y; j++)
        {
          idx = dim_z * (j + dim_y * i);
          grid_3d_a[i][j] = &data_cntr_a[idx];
          grid_3d_b[i][j] = &data_cntr_b[idx];
        }
      }
      break;
    default:
      return -1;
      break;
  }
  /* End Allocation */

  printf("%s\n", "Generating addresses... ");

  /*
   * Generate memory addresses, store them in data container
   *
   */
/*----------------------------------------------------------------------------*/
  switch(dim)
  {
    case 1:
      for ( i = 0; i < dim_x; i++ )
      {
        virtual_addr_a = (uint64_t)( base_a + ((uint64_t)(i) * stor_size));
        virtual_addr_b = (uint64_t)( base_b + ((uint64_t)(i) * stor_size));
        mapVirtualaddr(virtual_addr_a,
                       entries,
                       page_size,
                       page_table,
                       &grid_1d_a[i]);
        mapVirtualaddr(virtual_addr_b,
                       entries,
                       page_size,
                       page_table,
                       &grid_1d_b[i]);
#ifdef DEBUG
        // printf("%s%016" PRIX64 "\n", "Virtual addr: ", virtual_addr_a);
        // printf("%s%016" PRIX64 "\n", "Physical addr: ", grid_1d_a[i]);
#endif
      }
      break;
    case 2:
      for( i = 0; i < dim_x; i++)
      {
        for( j = 0; j < dim_y; j++)
        {
          virtual_addr_a = (uint64_t)( base_a + (uint64_t)((j + i * dim_y)
                                        * stor_size) );
          virtual_addr_b = (uint64_t)( base_b + (uint64_t)((j + i * dim_y)
                                        * stor_size) );
          mapVirtualaddr(virtual_addr_a,
                         entries,
                         page_size,
                         page_table,
                         &grid_2d_a[i][j]);
          mapVirtualaddr(virtual_addr_b,
                         entries,
                         page_size,
                         page_table,
                         &grid_2d_b[i][j]);
#ifdef DEBUG
          // printf("%s%016" PRIX64 "\n", "Virtual addr: ", virtual_addr_a);
          // printf("%s%016" PRIX64 "\n", "Physical addr: ", grid_2d_a[i][j]);
#endif
        }
      }
      break;
    case 3:
      for( i = 0; i < dim_x; i++)
      {
        for( j = 0; j < dim_y; j++)
        {
          for( k = 0; k < dim_z; k++)
          {
            virtual_addr_a = (uint64_t)( base_a + (uint64_t)((k + (j + i
                                             * dim_y) * dim_z) * stor_size) );
            virtual_addr_b = (uint64_t)( base_b + (uint64_t)((k + (j + i
                                             * dim_y) * dim_z) * stor_size) );
            mapVirtualaddr(virtual_addr_a,
                           entries,
                           page_size,
                           page_table,
                           &grid_3d_a[i][j][k]);
            mapVirtualaddr(virtual_addr_b,
                           entries,
                           page_size,
                           page_table,
                           &grid_3d_b[i][j][k]);
#ifdef DEBUG
            // printf("%s%016" PRIX64 "\n", "Virtual addr: ", virtual_addr_a);
            // printf("%s%016" PRIX64 "\n", "Physical addr: ", grid_3d_a[i][j][k]);
#endif
          }
        }
      }
      break;
    default:
      return -1;
      break;
  }
  /* End Allocation */

  /*
   * Generate memory traces based on the stencil type, order and if PIMS involves
   *
   */

  /* 3D-27 points */
  if( sten_type == 2)
  {
    if( flag == 1)
    {
      sprintf(filename, "../traces/PIMS-3D-27points-X%dY%dZ%d",
              dim_x, dim_y, dim_z);
      num_bytes = sten_coeff * (int)stor_size;
    }
    else
    {
      sprintf(filename, "../traces/3D-27points-X%dY%dZ%d",
              dim_x, dim_y, dim_z);
      num_bytes = (int)stor_size;
    }

    outfile = fopen(filename, "w");
    if( outfile == NULL ) {
      printf("ERROR: Cannot open trace file\n");
      goto cleanup;
    }

    // Write Stencil information
    write_sten_info(tracelogfile, filename, dim, dim_x, dim_y, dim_z,
                    cntr_size, sten_order, sten_coeff, data_type);
    fclose(tracelogfile);

    // Read from grid a, wirte to grid b
    for( i = 1; i < (dim_x-1); i++ )
    {
      for( j = 1; j < (dim_y-1); j++ )
      {
        for( k = 1; k < (dim_z-1); k++ )
        {
          memset(ops, 0, sizeof(ops));
          /*
           * Read operation from HOST
           * Length varies
           * if PIMS, num_bytes = sten_coeff * stor_size
           * else, num_bytes = stor_size
           *
           */
          sprintf(ops, "HOST_RD");
          write_to_file(outfile, ops, num_bytes, vaults, grid_3d_a[i][j][k]);

          if( flag == 1 )
          {
            if( getpimsid( &procid, grid_3d_a[i][j][k], shiftamt, vaults) !=0 )
            {
              printf("ERROR: Failed to retrive PIMS id\n");
              goto cleanup;
            }
            memset(ops, 0, sizeof(ops));
            /*
             * Read operation from PIMS
             * Length always stor_size
             *
             */
            sprintf(ops, "PIMS_RD");
            write_to_file(outfile, ops, stor_size, procid, grid_3d_a[i][j][k]);
          }
          else
          {
            memset(ops, 0, sizeof(ops));
            // Read operation from HOST
            sprintf(ops, "HOST_RD");
          }
          write_to_file(outfile, ops, stor_size, procid, grid_3d_a[i-1][j][k]);
          write_to_file(outfile, ops, stor_size, procid, grid_3d_a[i+1][j][k]);
          write_to_file(outfile, ops, stor_size, procid, grid_3d_a[i][j-1][k]);
          write_to_file(outfile, ops, stor_size, procid, grid_3d_a[i][j+1][k]);
          write_to_file(outfile, ops, stor_size, procid, grid_3d_a[i][j][k-1]);
          write_to_file(outfile, ops, stor_size, procid, grid_3d_a[i][j][k+1]);

          write_to_file(outfile, ops, stor_size, procid, grid_3d_a[i-1][j][k-1]);
          write_to_file(outfile, ops, stor_size, procid, grid_3d_a[i+1][j][k-1]);
          write_to_file(outfile, ops, stor_size, procid, grid_3d_a[i][j-1][k-1]);
          write_to_file(outfile, ops, stor_size, procid, grid_3d_a[i][j+1][k-1]);
          write_to_file(outfile, ops, stor_size, procid, grid_3d_a[i-1][j-1][k]);
          write_to_file(outfile, ops, stor_size, procid, grid_3d_a[i-1][j+1][k]);
          write_to_file(outfile, ops, stor_size, procid, grid_3d_a[i+1][j-1][k]);
          write_to_file(outfile, ops, stor_size, procid, grid_3d_a[i+1][j+1][k]);
          write_to_file(outfile, ops, stor_size, procid, grid_3d_a[i-1][j][k+1]);
          write_to_file(outfile, ops, stor_size, procid, grid_3d_a[i+1][j][k+1]);
          write_to_file(outfile, ops, stor_size, procid, grid_3d_a[i][j-1][k+1]);
          write_to_file(outfile, ops, stor_size, procid, grid_3d_a[i][j+1][k+1]);

          write_to_file(outfile, ops, stor_size, procid, grid_3d_a[i-1][j-1][k-1]);
          write_to_file(outfile, ops, stor_size, procid, grid_3d_a[i-1][j+1][k-1]);
          write_to_file(outfile, ops, stor_size, procid, grid_3d_a[i+1][j-1][k-1]);
          write_to_file(outfile, ops, stor_size, procid, grid_3d_a[i+1][j+1][k-1]);
          write_to_file(outfile, ops, stor_size, procid, grid_3d_a[i-1][j-1][k+1]);
          write_to_file(outfile, ops, stor_size, procid, grid_3d_a[i-1][j+1][k+1]);
          write_to_file(outfile, ops, stor_size, procid, grid_3d_a[i+1][j-1][k+1]);
          write_to_file(outfile, ops, stor_size, procid, grid_3d_a[i+1][j+1][k+1]);

          memset(ops, 0, sizeof(ops));
          /*
           * Write operation from HOST
           * Length always stor_size
           *
           */
          sprintf(ops, "HOST_WR");
          write_to_file(outfile, ops, stor_size, vaults, grid_3d_b[i][j][k]);
        }
      }
    }
  }

  /* 2D-9 points */
  if( sten_type == 1)
  {
    if( flag == 1 )
    {
      sprintf(filename, "../traces/PIMS-2D-9points-X%dY%d",
              dim_x, dim_y);
      num_bytes = sten_coeff * (int)stor_size;
    }
    else
    {
      sprintf(filename, "../traces/2D-9points-X%dY%d",
              dim_x, dim_y);
      num_bytes = (int)stor_size;
    }

    outfile = fopen(filename, "w");
    if( outfile == NULL ) {
      printf("ERROR: Cannot open trace file\n");
      goto cleanup;
    }

    // Write Stencil information
    write_sten_info(tracelogfile, filename, dim, dim_x, dim_y, dim_z,
                    cntr_size, sten_order, sten_coeff, data_type);
    fclose(tracelogfile);

    // Read grid a, wirte grid b
    for( i = 1; i < (dim_x-1); i++ )
    {
      for( j = 1; j< (dim_y-1); j++ )
      {
        memset(ops, 0, sizeof(ops));
        /*
         * Read operation from HOST
         * Length varies
         * if PIMS, num_bytes = sten_coeff * stor_size
         * else, num_bytes = stor_size
         *
         */
        sprintf(ops, "HOST_RD");
        write_to_file(outfile, ops, num_bytes, vaults, grid_2d_a[i][j]);

        if( flag == 1)
        {
          if( getpimsid( &procid, grid_2d_a[i][j], shiftamt, vaults) != 0 )
          {
            printf("ERROR: Failed to retrive PIMS id\n");
            goto cleanup;
          }
          memset(ops, 0, sizeof(ops));
          /*
           * Read operation from PIMS
           * Length always stor_size
           *
           */
          sprintf(ops, "PIMS_RD");
          write_to_file(outfile, ops, stor_size, procid, grid_2d_a[i][j]);
        }
        else
        {
          memset(ops, 0, sizeof(ops));
          // Read operation from HOST
          sprintf(ops, "HOST_RD");
        }

        write_to_file(outfile, ops, stor_size, procid, grid_2d_a[i-1][j]);
        write_to_file(outfile, ops, stor_size, procid, grid_2d_a[i+1][j]);
        write_to_file(outfile, ops, stor_size, procid, grid_2d_a[i][j-1]);
        write_to_file(outfile, ops, stor_size, procid, grid_2d_a[i][j+1]);
        write_to_file(outfile, ops, stor_size, procid, grid_2d_a[i-1][j-1]);
        write_to_file(outfile, ops, stor_size, procid, grid_2d_a[i+1][j-1]);
        write_to_file(outfile, ops, stor_size, procid, grid_2d_a[i-1][j+1]);
        write_to_file(outfile, ops, stor_size, procid, grid_2d_a[i+1][j+1]);

        memset(ops, 0, sizeof(ops));
        /*
         * Write operation from HOST
         * Length always stor_size
         *
         */
        sprintf(ops, "HOST_WR");
        write_to_file(outfile, ops, stor_size, vaults, grid_2d_b[i][j]);
      }
    }
  }

  /* Order varies */
  if( sten_type == 0)
  {
    sten_ptnum = 2 * sten_order * dim + 1;
    if( flag == 1 )
    {
      sprintf(filename, "../traces/PIMS-%dD-%dpoints-O%dX%dY%dZ%d",
              dim, sten_ptnum, sten_order, dim_x, dim_y, dim_z);
      num_bytes = sten_coeff * (int)stor_size;
    }
    else
    {
      sprintf(filename, "../traces/%dD-%dpoints-O%dX%dY%dZ%d",
              dim, sten_ptnum, sten_order, dim_x, dim_y, dim_z);
      num_bytes = (int)stor_size;
    }

    outfile = fopen(filename, "w");
    if( outfile == NULL ) {
      printf("ERROR: Cannot open trace file\n");
      goto cleanup;
    }

    // Write Stencil information
    write_sten_info(tracelogfile, filename, dim, dim_x, dim_y, dim_z,
                    cntr_size, sten_order, sten_coeff, data_type);
    fclose(tracelogfile);

    switch(dim)
    {
      case 1:
        // Read grid a, wirte grid b
        for( i = sten_order; i < (dim_x-sten_order); i++ )
        {
          memset(ops, 0, sizeof(ops));
          // Read operation from HOST
          sprintf(ops, "HOST_RD");

          ret = run_lru_simulation( &cache, grid_1d_a[i] );
          if( ret == -1 )
          {
            // Central point
            write_to_file(outfile, ops, num_bytes, vaults, grid_1d_a[i]);
          }

#ifdef DEBUG
          // printf("%s:%d:%d:0x%016" PRIX64 "\n", ops, num_bytes, vaults, grid_1d_a[i]);
#endif

          if( flag == 1)
          {
            if( getpimsid( &procid, grid_1d_a[i], shiftamt, vaults )!= 0 )
            {
              printf("ERROR: Failed to retrive PIMS id\n");
              goto cleanup;
            }
            memset(ops, 0, sizeof(ops));
            /*
             * Read operation from PIMS
             * Length always stor_size
             *
             */
            sprintf(ops, "PIMS_RD");
            write_to_file(outfile, ops, stor_size, procid, grid_1d_a[i]);
#ifdef DEBUG
          // printf("%s:%d:%d:0x%016" PRIX64 "\n", ops, num_bytes, procid, grid_1d_a[i]);
#endif
          }
          else
          {
            memset(ops, 0, sizeof(ops));
            // Read operation from HOST
            sprintf(ops, "HOST_RD");
          }

          // Orders points
          for( r = 1; r <= sten_order; r++)
          {
            ret = run_lru_simulation( &cache, grid_1d_a[i-r] );
            if( ret == -1 )
            {
              write_to_file(outfile, ops, stor_size, procid, grid_1d_a[i-r]);
            }
            ret = run_lru_simulation( &cache, grid_1d_a[i+r] );
            if( ret == -1 )
            {
              write_to_file(outfile, ops, stor_size, procid, grid_1d_a[i+r]);
            }

#ifdef DEBUG
          // printf("%s:%d:%d:0x%016" PRIX64 "\n", ops, num_bytes, procid, grid_1d_a[i-r]);
          // printf("%s:%d:%d:0x%016" PRIX64 "\n", ops, num_bytes, procid, grid_1d_a[i+r]);
#endif
          }

          memset(ops, 0, sizeof(ops));
          /*
           * Write operation from HOST
           * Length always stor_size
           * Allways record write operation
           */
          sprintf(ops, "HOST_WR");
          write_to_file(outfile, ops, stor_size, vaults, grid_1d_b[i]);
        }

        //--------------------------------------------------------------------
        break;
      case 2:
        // Read grid a, wirte grid b
        for( i = sten_order; i < (dim_x-sten_order); i++ )
        {
          for( j = sten_order; j < (dim_y-sten_order); j++ )
          {
            memset(ops, 0, sizeof(ops));
            // Read operation from HOST
            sprintf(ops, "HOST_RD");

            // Central point
            ret = run_lru_simulation( &cache, grid_2d_a[i][j] );
            if( ret == -1 )
            {
              write_to_file(outfile, ops, num_bytes, vaults, grid_2d_a[i][j]);
            }


            if( flag == 1 )
            {
              if( getpimsid( &procid, grid_2d_a[i][j], shiftamt, vaults ) != 0)
              {
                printf("ERROR: Failed to retrive PIMS id\n");
                goto cleanup;
              }
              memset(ops, 0, sizeof(ops));
              /*
               * Read operation from PIMS
               * Length always stor_size
               *
               */
              sprintf(ops, "PIMS_RD");
              write_to_file(outfile, ops, stor_size, procid, grid_2d_a[i][j]);
            }
            else
            {
              memset(ops, 0, sizeof(ops));
              // Read operation from HOST
              sprintf(ops, "HOST_RD");
            }

            // Orders points
            for( r = 1; r <= sten_order; r++ )
            {
              ret = run_lru_simulation( &cache,grid_2d_a[i-r][j] );
              if( ret == -1 )
              {
                write_to_file(outfile, ops, num_bytes, vaults, grid_2d_a[i-r][j]);
              }

              ret = run_lru_simulation( &cache, grid_2d_a[i+r][j] );
              if( ret == -1 )
              {
                write_to_file(outfile, ops, num_bytes, vaults, grid_2d_a[i+r][j]);
              }

              ret = run_lru_simulation( &cache, grid_2d_a[i][j-r] );
              if( ret == -1 )
              {
                write_to_file(outfile, ops, num_bytes, vaults, grid_2d_a[i][j-r]);
              }

              ret = run_lru_simulation( &cache, grid_2d_a[i][j+r] );
              if( ret == -1 )
              {
                write_to_file(outfile, ops, num_bytes, vaults, grid_2d_a[i][j+r]);
              }

              // write_to_file(outfile, ops, stor_size, procid, grid_2d_a[i-r][j]);
              // write_to_file(outfile, ops, stor_size, procid, grid_2d_a[i+r][j]);
              // write_to_file(outfile, ops, stor_size, procid, grid_2d_a[i][j-r]);
              // write_to_file(outfile, ops, stor_size, procid, grid_2d_a[i][j+r]);
            }

            memset(ops, 0, sizeof(ops));
            /*
             * Write operation from HOST
             * Length always stor_size
             *
             */
            sprintf(ops, "HOST_WR");
            write_to_file(outfile, ops, stor_size, vaults, grid_2d_b[i][j]);
          }
        }
        break;
      case 3:
        // Read grid a, wirte grid b
        for( i = sten_order; i < (dim_x-sten_order); i++ )
        {
          for( j = sten_order; j < (dim_y-sten_order); j++)
          {
            for( k = sten_order; k < (dim_z-sten_order); k++)
            {
              memset(ops, 0, sizeof(ops));
              // Read operation from HOST
              sprintf(ops, "HOST_RD");
              // Central point
              ret = run_lru_simulation( &cache, grid_3d_a[i][j][k] );
              if( ret == -1 )
              {
                write_to_file(outfile, ops, num_bytes, vaults,grid_3d_a[i][j][k]);
              }

              if( flag == 1)
              {
                if( getpimsid( &procid, grid_3d_a[i][j][k], shiftamt, vaults) !=0 )
                {
                  printf("ERROR: Failed to retrive PIMS id\n");
                  goto cleanup;
                }
                memset(ops, 0, sizeof(ops));
                /*
                 * Read operation from PIMS
                 * Length always stor_size
                 *
                 */
                sprintf(ops, "PIMS_RD");
                write_to_file(outfile, ops, stor_size, procid, grid_3d_a[i][j][k]);
              }
              else
              {
                memset(ops, 0, sizeof(ops));
                // Read operation from HOST
                sprintf(ops, "HOST_RD");
              }

              // Orders points
              for ( r = 1; r <= sten_order; r++ )
              {
                ret = run_lru_simulation( &cache, grid_3d_a[i-r][j][k] );
                if( ret == -1 )
                {
                  write_to_file(outfile, ops, num_bytes, vaults, grid_3d_a[i-r][j][k]);
                }

                ret = run_lru_simulation( &cache, grid_3d_a[i+r][j][k] );
                if( ret == -1 )
                {
                  write_to_file(outfile, ops, num_bytes, vaults, grid_3d_a[i+r][j][k]);
                }

                ret = run_lru_simulation( &cache, grid_3d_a[i][j-r][k] );
                if( ret == -1 )
                {
                  write_to_file(outfile, ops, num_bytes, vaults, grid_3d_a[i][j-r][k]);
                }

                ret = run_lru_simulation( &cache, grid_3d_a[i][j+r][k] );
                if( ret == -1 )
                {
                  write_to_file(outfile, ops, num_bytes, vaults, grid_3d_a[i][j+r][k]);
                }

                ret = run_lru_simulation( &cache, grid_3d_a[i][j][k-r] );
                if( ret == -1 )
                {
                  write_to_file(outfile, ops, num_bytes, vaults, grid_3d_a[i][j][k-r]);
                }

                ret = run_lru_simulation( &cache, grid_3d_a[i][j][k+r] );
                if( ret == -1 )
                {
                  write_to_file(outfile, ops, num_bytes, vaults, grid_3d_a[i][j][k+r]);
                }
                // write_to_file(outfile, ops, stor_size, procid, grid_3d_a[i-r][j][k]);
                // write_to_file(outfile, ops, stor_size, procid, grid_3d_a[i+r][j][k]);
                // write_to_file(outfile, ops, stor_size, procid, grid_3d_a[i][j-r][k]);
                // write_to_file(outfile, ops, stor_size, procid, grid_3d_a[i][j+r][k]);
                // write_to_file(outfile, ops, stor_size, procid, grid_3d_a[i][j][k-r]);
                // write_to_file(outfile, ops, stor_size, procid, grid_3d_a[i][j][k+r]);
              }

              memset(ops, 0, sizeof(ops));
              // Write operation from HOST
              sprintf(ops, "HOST_WR");
              write_to_file(outfile, ops, stor_size, vaults, grid_3d_b[i][j][k]);
            }
          }
        }
        break;
      default:
        return -1;
        break;
    }

  }

  cache.hit_rate = (((double)(cache.hits))/((double)( cache.hits + cache.misses )));
  cache.miss_rate = (((double)(cache.misses))/((double)( cache.hits + cache.misses )));

  printf("Write cache information!\n");
  write_cache_info( cachelogfile, filename, cache.ways,
                    cache.cache_size, cache.block_size,
                    cache.hits, cache.misses,
                    cache.hit_rate, cache.miss_rate );
  fclose(cachelogfile);
  printf("Finish!\n");
cleanup:
  /* Close file */
  if( outfile != NULL )
  {
    fclose(outfile);
  }

  /* Deallocate memory */
  free(page_table);

  for( i = 0; i < cache.sets; i++ )
  {
    free(tag[i]);
    free(lru[i]);
  }

  free(tag);
  free(lru);
  free(valid);

  switch(dim)
  {
    case 1:
      break;
    case 2:
      free(grid_2d_a);
      free(grid_2d_b);
      break;
    case 3:
      for(i = 0; i < dim_x; i++)
      {
        free(grid_3d_a[i]);
        free(grid_3d_b[i]);
      }
      free(grid_3d_a);
      free(grid_3d_b);
      break;
    default:
      return -1;
      break;
  }
  free(data_cntr_a);
  free(data_cntr_b);
  /* End Deallocation */

  return 0;
}
/* --------------------------------------------- MAPVIRTUALADDR */
extern void mapVirtualaddr(uint64_t virtual_addr,
                           uint64_t entries,
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

#ifdef DEBUG
  printf("Virtual page:%"PRIX64"\n", virtual_page);
  printf("Offset:%"PRIX64"\n", offset);
#endif

  /* LRU page replacement algorithm
   *
   * nextEntryIndex: point to the next available entry
   * if nextEntryIndex < entries,
   *    // do not need to consider age
   *    traverse from 0 to nextEntryIndex to find if pagetable hit
   *    if hit,return
   *    else add new entry, nextEntryIndex++
   * else
        // pagetable is full
   *    traverse from 0 to entries to find if a pagetable hit
   *    if hit, return
   *    else age++, find indexOfOldest, replace it to the new entry, return
   *
   */

  if( nextEntryIndex < entries )
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
    for( i = 0; i < entries; i++)
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
    // page table miss, find indexOfOldest
    pta_miss ++;
    page_table[indexOfOldest].virtual_page = virtual_page;
    *physical_addr = (uint64_t)( ( page_table[indexOfOldest].page_frame
                                   << VIRTUAL_PAGE_SHIFT) | offset );
    return;
  }
}
/* EOF */

/* --------------------------------------------- run_lru_simulation */
int run_lru_simulation( cache_node *cache, uint64_t address )
{
  uint64_t block_addr = (uint64_t)(address >> (uint64_t)log2(cache->block_size) );
  uint64_t index = (uint64_t)( block_addr % cache->sets);
  uint64_t tag_val = (uint64_t)( block_addr >> (uint64_t)log2(cache->sets));

  int hit_flag;
  int found;
  int i;
  int j;
  int lru1;
  int result;         // -1 means miss, 1 means hit

  // Compare tag
  if( valid[index] == 0 )
  {
    valid[index] = 1;
    tag[index][0] = tag_val;
    lru[index][0] = 0;
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
        hit_flag = 1;
        cache->hits ++;
        result = 1;
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
      cache->misses ++;
      result = -1;
      found = 0;
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
        for( j = 0; j < cache->sets; j++ )
        {
          if( lru[index][j] == (cache->sets - 1) )
          {
            lru1 = j;
            break;
          }
        }
        tag[index][lru1] = tag_val;

        for( j = 0; j < cache->sets; j++ )
        {
          lru[index][j] = lru[index][j] + 1;
        }
        lru[index][lru1] = 0;
      }
    }
  }
  return result;
}
