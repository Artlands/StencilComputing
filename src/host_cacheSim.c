/*
 * _HOST_CASHESIM_C_
 *
 * FUNCTION TO SIMULATE CACHE ON HOST SIDE
 * NRU reused from Brody's CacheSimulator program
 */

/* Included files*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <inttypes.h>
#include <math.h>
#include "pims.h"

// #define DEBUG

/* ---------------------------------------------- FUNCTION PROTOTYPES*/
extern int read_trace( FILE *infile, trace_node *trace);
extern void write_to_file(FILE* fp, char* op, int num_bytes, int procid, uint64_t addr);

void write_cache_info( FILE* fp, char* filename,
                       char* read_op, uint64_t read_num,
                       char* write_op, uint64_t write_num,
                       uint64_t hits, uint64_t misses,
                       double hit_rate, double miss_rate )
{
  fprintf(fp, "# Trace file path: %s\n", filename);
  fprintf(fp, "# %s:              %llu\n", read_op, read_num);
  fprintf(fp, "# %s:              %llu\n", write_op, write_num);
  fprintf(fp, "# Total hits:      %llu\n", hits);
  fprintf(fp, "# Total misses:    %llu\n", misses);
  fprintf(fp, "# Hits rate:       %f\n", hit_rate);
  fprintf(fp, "# Misses rate:     %f\n", miss_rate);
  fprintf(fp, "#==============================================================================\n");
}

/*
 * Main Function, Takes command line arguments, read physical memory address
 * traces, simulate 'n-way' cache, generate memory access trace if cache miss
 *
 */
int main(int argc, char* argv[])
{
  /* vars */
  uint64_t i = 0;
  uint64_t j = 0;
  uint64_t idx = 0;
  int ret = 0;
  int done = 0;
  char *token;

  /* MEMORY TRACE */
  char infilename[1024];
  char outfilename[1024];
  char cachelog[1024];
  FILE *infile = NULL;
  FILE *outfile = NULL;
  FILE *logfile = NULL;   // save trace log
  trace_node trace;

  /* CACHE */
  uint64_t cache_size;
  uint64_t block_size;
  uint64_t num_block;
  uint64_t ways;
  uint64_t sets;
  uint64_t **tag_field;
  uint64_t **valid_field;
  uint64_t **nru_reference;

  /* For each address */
  int processed_flag = 0;
  uint64_t block_addr;
  uint64_t index;
  uint64_t tag;
  uint64_t replacement;

  /* Cache records*/
  uint64_t hits = 0;
  uint64_t misses = 0;
  uint64_t read_num = 0;
  uint64_t write_num = 0;
  double hit_rate;
  double miss_rate;

  while( (ret = getopt( argc, argv, "c:b:w:f:h")) != -1 )
  {
    switch( ret )
    {
      case 'c':
        cache_size = (uint64_t)(atoi(optarg));
        break;
      case 'b':
        block_size = (uint64_t)(atoi(optarg));
        break;
      case 'w':
        ways = (uint64_t)(atoi(optarg));
        break;
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
  if( cache_size != 32768 && cache_size != 131072 && cache_size != 8388608 )
  {
    printf("ERROR: Cache size is invalid\n");
    return -1;
  }

  if( block_size != 64 && block_size != 128 && block_size != 256 )
  {
    printf("ERROR: Block size is invalid\n");
    return -1;
  }

  if( ways != 8 && ways != 16 && ways != 32 )
  {
    printf("ERROR: Ways is invalid\n");
    return -1;
  }

  infile = fopen(infilename, "r");
  if( infile == NULL )
  {
    printf("ERROR: Cannot open input memory trace file\n");
    return -1;
  }

  token = strtok(infilename, ".");
  sprintf(outfilename, "%s%s", token, ".cached");

  outfile = fopen(outfilename, "w");
  if( outfile == NULL ){
    printf("ERROR: Cannot open output memory trace file\n");
    return -1;
  }

  sprintf(cachelog, "../traces/cache.log");
  logfile = fopen(cachelog, "a");
  if( logfile == NULL )
  {
    printf("ERROR: Cannot open cache log file\n");
    return -1;
  }
/* ---- End Sanity Check ---- */


  /* Initialization of cache*/
  num_block = cache_size/block_size;
  sets = num_block/ways;

#ifdef DEBUG
        printf("# Blocks: %llu\n", num_block);
        printf("Sets: %llu\n", sets);
#endif

  tag_field = (uint64_t **) malloc( sizeof( uint64_t *) * sets);
  valid_field  = (uint64_t **) malloc( sizeof( uint64_t *) * sets);
  nru_reference = (uint64_t **) malloc( sizeof( uint64_t *) * sets);

  if( tag_field == NULL || valid_field == NULL || nru_reference == NULL )
  {
    printf("ERROR: Out of memory\n");
    return -1;
  }

printf("Initialize Cache...\n");
  for( i = 0; i < sets; i++ )
  {
    tag_field[i] = (uint64_t *) malloc( sizeof( uint64_t ) * ways);
    valid_field[i] = (uint64_t *) malloc( sizeof( uint64_t ) * ways);
    nru_reference[i] = (uint64_t *) malloc( sizeof( uint64_t ) * ways);
  }

  for( i = 0; i < sets; i++ )
  {
    for( j = 0; j < ways; j++ )
    {
      tag_field[i][j] = 0;
      valid_field[i][j] = 0;
      nru_reference[i][j] = 0;
    }
  }

printf("Reading memory trace file...\n");
  /* read first request from the input file */
  done = read_trace( infile, &trace );
  while( done != 0 ){
    done = read_trace( infile, &trace);
  }

  /* read all traces until to the end of file */
  while( done == 0 ){
    /* get address infor from input trace */
    block_addr = (uint64_t)( trace.addr >> (uint64_t)log2(block_size) );
    index = (uint64_t)( block_addr % sets );
    tag = (uint64_t)( block_addr >> (uint64_t)log2(sets) );

#ifdef DEBUG
    printf("Memory address: %llu, Sets: %llu, Ways: %llu, Block address: %llu, Index: %llu, Tag: %llu ",
           trace.addr, sets, ways, block_addr, index, tag);
#endif
    /* Flag is set to 1 when address is processed */
    processed_flag = 0;

    /* check for hit in appropriate set */
    for( i = 0; i < ways; i++ )
    {
      /* If cache space is occupied and tag matches then Hit*/
      if( (valid_field[index][i]) && (tag_field[index][i] == tag) )
      {
        hits ++;
        nru_reference[index][i] = 1;
        processed_flag = 1;
#ifdef DEBUG
        printf("Hit!\n");
#endif
        break;
      }
    }
    /* If not a hit then process miss */
    if( processed_flag == 0 )
    {
      misses ++;

      /* Get all read and write operation number*/
      if( strcmp( trace.op, "HOST_RD" ) == 0 ){
        read_num ++;
      }
      if( strcmp( trace.op, "HOST_WR" ) == 0 ){
        write_num ++;
      }

#ifdef DEBUG
        printf("Miss!\n");
#endif
      /* write this address to output trace file */
      write_to_file(outfile, trace.op, trace.num_bytes, trace.procid, trace.addr);

      /* First look for an unused way in the proper set */
      for( i = 0; i < ways; i++ )
      {
        if( valid_field[index][i] == 0 )
        {
          tag_field[index][i] = tag;
          valid_field[index][i] = 1;
          nru_reference[index][i] = 1;
          processed_flag = 1;
          break;
        }
      }
      /* If all ways are occupied then traverse bst and replace indicated way*/
      if( processed_flag == 0 )
      {
        tag_field[index][0] = tag;
        for( j = 1; j < ways; j++ )
        {
          nru_reference[index][j] = 0;
        }
      }
    }
    /* read next request from the input file until to the end of file*/
    done = read_trace( infile, &trace );
    while( done != 0 ){
      done = read_trace( infile, &trace);
      if( done == -2) break;
    }
  }

  printf("Finish!\n");
  hit_rate = (((double)(hits))/((double)( hits + misses )));
  miss_rate = (((double)(misses))/((double)( hits + misses )));

#ifdef DEBUG
  printf("Cache hit rate: %f\n", hit_rate);
  printf("Cache miss rate: %f\n", miss_rate);
#endif
  write_cache_info( logfile, infilename,
                    "HOST_RD", read_num,
                    "HOST_WR", write_num,
                    hits, misses,
                    hit_rate, miss_rate )

cleanup:
  for( i = 0; i < sets; i++ )
  {
    free(tag_field[i]);
    free(valid_field[i]);
    free(nru_reference[i]);
  }

  free(tag_field);
  free(valid_field);
  free(nru_reference);

  fclose(infile);
  fclose(outfile);
  fclose(logfile);

  return 0;
}
