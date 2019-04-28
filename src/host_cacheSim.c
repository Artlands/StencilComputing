/*
 * _HOST_CASHESIM_C_
 *
 * FUNCTION TO SIMULATE CACHE ON HOST SIDE
 * PLRU reused from Brody's CacheSimulator program
 */

/* Included files*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <inttypes.h>
#include "pims.h"

// #define DEBUG

/* ---------------------------------------------- FUNCTION PROTOTYPES*/
extern int read_trace( FILE *infile, trace_node *trace);
extern tree_node* create_tree(unsigned number_nodes);
extern void hit_update_bst(tree_node* root, unsigned total_ways, unsigned target_way);
extern unsigned find_plru_way(tree_node* root, unsigned total_ways);
extern void write_to_file(FILE* fp, char* op, int num_bytes, int procid, uint64_t addr);

/*
 * Main Function, Takes command line arguments, read physical memory address
 * traces, simulate 'n-way' cache, generate memory access trace if cache miss
 *
 */
int main(int argc, char* argv[])
{
  /* vars */
  int i = 0;
  int j = 0;
  int ret = 0;
  int done = 0;

  /* MEMORY TRACE */
  char infilename[1024];
  char outfilename[1024];
  FILE *infile = NULL;
  FILE *outfile = NULL;
  trace_node trace;

  /* CACHE */
  CACHE current_cache;
  uint64_t tag_field[current_cache.sets][current_cache.ways];
  unsigned valid_field[current_cache.sets][current_cache.ways];
  tree_node *trees[current_cache.sets];

  /* For each address */
  uint64_t block_addr;
  uint64_t index;
  uint64_t tag;
  int processed_flag = 0;
  unsigned replacement;

  while( (ret = getopt( argc, argv, "f:h")) != -1 )
  {
    switch( ret )
    {
      case 'f':
        sprintf(infilename, "%s", optarg);
        break;
      case 'h':
        printf("%s%s%s\n", "usage : ", argv[0], " -fh");
        printf(" -f <Trace file name>\n");
        printf(" h ...print help\n");
        return 0;
        break;
      default:
        printf("%s%s%s\n", "Unknown option: see ", argv[0], " -fh" );
        return -1;
        break;
    }
  }
  /* ---- Sanity Check ----*/
  infile = fopen(infilename, "r");
  if( infile == NULL ){
    printf("ERROR: Cannot open input memory trace file\n");
    return -1;
  }

  outfile = fopen(outfilename, "w");
  if( outfile == NULL ){
    printf("ERROR: Cannot open output memory trace file\n");
    return -1;
  }
  /* ---- End Sanity Check ---- */

  /* Init cache */
  current_cache = {.way = WAYS, .sets = SETS};

  /* Initialization of arrays*/
  for(int i=0; i<current_cache.sets; i++)
  {
    for(int j=0; j<current_cache.ways; j++)
    {
      valid_field[i][j] = 0;
      tag_field[i][j] = 0;
    }
  }

  /* Calls function to create BST trees for each set in the cache *
   * using n-1 nodes where n is the number of ways. Sets pointer  *
   * at appropriate index of tree array                           */
  for(int i=0; i<current_cache.sets; i++)
  {
    trees[i] = create_tree(current_cache.sets-1);
  }

  /* read first request from the input file */
  done = read_trace( infile, &trace );
  while( done != 0 ){
    done = read_trace( infile, &trace);
  }

  /* read all traces until to the end of file */
  while( done == 0 ){
    /* get address infor from input trace */
    block_addr = (uint64_t)( trace.addr >> (unsigned)log2(BLOCK_SIZE) );
    index = (uint64_t)( block_addr % current_cache.sets );
    tag = (uint64_t)( block_addr >> (unsigned)log2(current_cache.sets) );

#ifdef DEBUG
    printf("Memory address: %llu, Sets: %u, Ways: %u, Block address: %llu, Index: %llu, Tag: %llu ",
           trace.addr, current_cache->sets, current_cache->ways, block_addr, index, tag);
#endif
    /* Flag is set to 1 when address is processed */
    processed_flag = 0;

    /* check for hit in appropriate set */
    for( i = 0; i < current_cache.ways; i++ )
    {
      /* If cache space is occupied and tag matches then Hit*/
      if( (valid_field[index][i]) && (tag_field[index][i] == tag) )
      {
        current_cache.hits ++;
        hit_update_bst(trees[index], current_cache.ways, i);
        processed_flag = 1;
        break;
      }
    }
    /* If not a hit then process miss */
    if( processed_flag == 0 )
    {
      current_cache.misses ++;
      /* write this address to output trace file */
      write_to_file(outfile, trace.op, trace.num_bytes, trace.procid, trace.addr);

      /* First look for an unused way in the proper set */
      for( i = 0; i < current_cache.ways; i++ )
      {
        if( valid_field[index][i] == 0 )
        {
          valid_field[index][i] = 1;
          tag_field[index][i] = tag;
          processed_flag = 1;
          break;
        }
      }
      /* If all ways are occupied then traverse bst and replace indicated way*/
      if( processed_flag == 0 )
      {
        replacment = find_plru_way(trees[index], current_cache.ways);
        tag_field[index][replacement] = tag;
      }
    }
    /* read next request from the input file until to the end of file*/
    done = read_trace( infile, &trace );
    while( done != 0 ){
      done = read_trace( infile, &trace);
      if( done == -2) break;
    }
  }
  current_cache->hit_rate = (((double)(current_cache->hits))/((double)(current_cache->hits+current_cache->misses)));
  current_cache->miss_rate = (((double)(current_cache->misses))/((double)(current_cache->hits+current_cache->misses)));

  fclose(infile);
  fclose(outfile);

  return 0;
}
