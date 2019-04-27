/*
 * _HOST_CASHESIM_C_
 *
 * FUNCTION TO SIMULATE CACHE ON HOST SIDE
 * PLRU reused from Brody's CacheSimulator program and Dr. Chen's cachesim program
 *
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

/* Dr. Chen's function to convert a hex address to the appropriate value */
static uint64_t convert_address(char memory_addr[])
{
    uint64_t binary = 0;
    int i = 0;

    while (memory_addr[i] != '\n')
    {
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
/*
 * Main Function, Takes command line arguments, read physical memory address
 * traces, simulate 'n-way' cache, generate memory access trace if cache miss
 *
 */
int main(int argc, char* argv[])
{
  /* vars */
  int ret = 0;

  /* MEMORY TRACE */
  char infilename[1024];
  char outfilename[1024];
  FILE *infile = NULL;
  FILE *outfile = NULL;
  trace_node trace;
  uint64_t physical_addr = 0x00ull;

  /* cache */
  cache_node current_cache;

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
  current_cache = {.way = WAY, .sets = NUM_BLOCKS/WAY};

  return 0;
}
