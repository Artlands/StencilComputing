/*
 * _MMU_SIMULATOR_C_
 *
 * FUNCTION TO TRANSLATE VIRTUAL MEMORY ADDRESSES TO PHYSICAL MEMORY ADDRESSES
 *
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

/* Global variables */
uint64_t pta_miss;
uint64_t oldestAge;
uint64_t indexOfOldest;
uint64_t nextEntryIndex;

/* ---------------------------------------------- FUNCTION PROTOTYPES*/
extern int read_trace( FILE *infile, trace_node *trace);
extern void write_to_file(FILE* fp, char* op, int num_bytes, int procid, uint64_t addr);

static int mapVirtualaddr(uint64_t virtual_addr,
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
        *physical_addr = (uint64_t)( (i << VIRTUAL_PAGE_SHIFT) | offset );
        return 0;
      }
    }
    // page table miss, add new entry
    pta_miss ++;
    page_table[nextEntryIndex].virtual_page = virtual_page;
    *physical_addr = (uint64_t)( (nextEntryIndex << VIRTUAL_PAGE_SHIFT) | offset);
    nextEntryIndex ++;
    return 0;
  }
  else
  {
    for( i = 0; i < entries; i++)
    {
      // Page table hit
      if( page_table[i].virtual_page == virtual_page )
      {
        *physical_addr = (uint64_t)( (i << VIRTUAL_PAGE_SHIFT) | offset );
        return 0;
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
    *physical_addr = (uint64_t)((indexOfOldest << VIRTUAL_PAGE_SHIFT) | offset);
    return 0;
  }
}

/*
 * Main Function, Takes command line arguments, read virtual memory address
 * traces and translate into physical memory address traces
 *
 */
int main(int argc, char* argv[])
{
  /* vars */
  int i = 0;
  int ret = 0;
  int done = 0;

  /* PAGE TABLE*/
  uint64_t capacity = 0;               // HMC capacity
  uint64_t memSize = 0;                // Main memory size
  uint64_t page_size = PAGESIZE;       // page size
  uint64_t entries = 0;                // number of entries
  pta_node *page_table = NULL;         // page table
  pta_miss = 0;
  oldestAge = 0;
  indexOfOldest = 0;
  nextEntryIndex = 0;

  /* MEMORY TRACE*/
  char infilename[1024];
  char outfilename[1024];
  FILE *infile = NULL;    // read trace file
  FILE *outfile = NULL;   // save trace file
  trace_node trace;
  uint64_t virtual_addr = 0x00ull;
  uint64_t physical_addr = 0x00ull;

  while( (ret = getopt( argc, argv, "c:f:h")) != -1 )
  {
    switch( ret )
    {
      case 'c':
        capacity = (uint32_t)(atoi(optarg));
        break;
      case 'f':
        sprintf(infilename, "%s", optarg);
        break;
      case 'h':
        printf("%s%s%s\n", "usage : ", argv[0], " -cfh");
        printf(" -c <Memory capacity>\n");
        printf(" -f <Trace file name>\n");
        printf(" h ...print help\n");
        return 0;
        break;
      default:
        printf("%s%s%s\n", "Unknown option: see ", argv[0], " -cfh" );
        return -1;
        break;
    }
  }

  /* ---- Sanity Check ---- */
  if ( capacity != 4 && capacity != 8 ) {
    printf("ERROR: Capacity is invalid\n");
    return -1;
  }

  sprintf(outfilename, "%s%s", infilename, "phy");

  infile = fopen(infilename, "r");
  if( infile == NULL ){
    printf("ERROR: Cannot open virtual memory trace file\n");
    return -1;
  }

  outfile = fopen(outfilename, "w");
  if( outfile == NULL ){
    printf("ERROR: Cannot open physical memory trace file\n");
    return -1;
  }
  /* ---- End Sanity Check ---- */

  /* Initialization */
  memSize = (uint64_t)(capacity * GB);
  entries = memSize / page_size;

#ifdef DEBUG
  printf("Page size: %llu\n", page_size );
  printf("Memory size: %llu\n", memSize );
  printf("Entries: %llu\n", entries);
#endif

  /* Init Page table */
  page_table = (pta_node *)malloc(entries * sizeof(pta_node));
  if( page_table == NULL )
  {
    printf("Error: Out of memory\n");
    return -1;
  }
  for( i = 0; i < entries; i++ )
  {
    page_table[i].virtual_page = -1;
    page_table[i].age = 0;
  }

  /* read first request from the input file */
  done = read_trace( infile, &trace );
  while( done != 0 ){
    done = read_trace( infile, &trace);
  }

  /* read all traces until to the end of file */
  while( done == 0 ){
     /* translate virtual memory address to physical memory address */
#ifdef DEBUG
     printf("Operation: %s\n", trace.op );
     printf("Num of bytes: %d\n", trace.num_bytes );
     printf("Process ID: %d\n", trace.procid );
     printf("Address: 0x%016"PRIX64"\n", trace.addr );
#endif
     ret = mapVirtualaddr(trace.addr,
                          entries,
                          page_size,
                          page_table,
                          &physical_addr);

     if ( ret == 0 ){
       /*write trace*/
       write_to_file(outfile, trace.op, trace.num_bytes, trace.procid, physical_addr);
     }
     /* read next request from the input file until to the end of file*/
     done = read_trace( infile, &trace );
     while( done != 0 ){
       done = read_trace( infile, &trace);
       if( done == -2) break;
     }
  }

  printf("PageTable misses: %llu\n", pta_miss);

  free(page_table);
  fclose(infile);
  fclose(outfile);

  return 0;
}
