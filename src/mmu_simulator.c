/*
 * _MMU_SIMULATOR_C_
 *
 * FUNCTION TO TRANSLATE VIRTUAL MEMORY ADDRESSES TO PHYSICAL MEMORY ADDRESSES
 *
 * Search the TLB, if a TLB hit occurs, return the frame number else go to the page table
 * mapVirtualaddr function refers to https://github.com/Oakes6/VirtualMemorySimulator
 * /tree/d9826d50530630ec8e1686308a3794eae93e8fc3
 *
 */

/* Included files*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <inttypes.h>

// #define DEBUG

/* Physical Memory Parameters */
#define PAGESIZE 4096
#define GB 1073741824

/* Masks for Virtual address */
#define VIRTUAL_PAGE_MASK 0xFFFFFFFFFFFFF000
#define VIRTUAL_OFFSET_MASK 0x0000000000000FFF
#define VIRTUAL_PAGE_SHIFT 12


/* ---------------------------------------------- DATA STRUCTURE*/

typedef struct pta_node
{
  int64_t virtual_page;
  int64_t page_frame;
  int64_t age;
  int isValid;
}pta_node;

typedef struct trace_node
{
 char op[10];
 int num_bytes;
 int procid;
 uint64_t addr;
}trace_node;

/* ---------------------------------------------- FUNCTION PROTOTYPES*/
static int read_trace( FILE *infile, trace_node *trace)
{
 /* vars */
 char buf[50];
 char *token;
 size_t len = 0;

 /* read a single entry from the trace file */
 if( feof(infile) ){
   return -2;
 }
 if( fgets( buf, 50, infile ) == NULL ){
   return -1;
 }

 /*
  * we have a valid buffer
  * strip the newline and tokenize it
  */

 len = strlen( buf );
 if( buf[len] == '\n' ){
   buf[len] = '\0';
 }

 // if( buf[0] == '#' ){
 //   // not a valid trace
 //   return -1;
 // }

 /* tokenize it */
 token = strtok( buf, ":");
 strcpy(trace->op, token);

 /* num_bytes */
 token = strtok( NULL, ":");
 trace->num_bytes = (int)(atoi(token));

 /* procid */
 token = strtok( NULL, ":");
 trace->procid = (int)(atoi(token));

 /* first part of address = 0x */
 token = strtok( NULL, "x");
 /* last part of address in hex */
 token = strtok( NULL, " ");
 trace->addr = (uint64_t)(strtol( token, NULL, 16 ));

 return 0;
}

static void write_to_file(FILE* fp,
                          char* op,
                          int num_bytes,
                          int procid,
                          uint64_t addr)
{
  fprintf(fp, "%s:%d:%d:0x%016"PRIX64"\n", op, num_bytes, procid, addr);
}

static int mapVirtualaddr(uint64_t virtual_addr,
                          uint64_t entries,
                          uint64_t page_size,
                          uint64_t *pta_miss,
                          pta_node *page_table,
                          uint64_t *physical_addr)
{
  /* vars */
  int i = 0;
  int hitFlag = 0;
  int found = 0;
  uint64_t oldestAge = 0;
  uint64_t indexOfOldest = 0;
  uint64_t nextEntryIndex = 0;

  /* Get virtual page and offset */
  int64_t virtual_page = (int64_t)((virtual_addr & VIRTUAL_PAGE_MASK)
                                   >> VIRTUAL_PAGE_SHIFT);
  int64_t offset = (int64_t)(virtual_addr & VIRTUAL_OFFSET_MASK);

#ifdef DEBUG
  printf("Virtual page:%"PRIX64"\n", virtual_page);
  printf("Offset:%"PRIX64"\n", offset);
#endif

  /* Search the Page table */
  for ( i = 0; i < entries; i++ )
  {
    // Page table hit
    if( (page_table[i].virtual_page == virtual_page) && page_table[i].isValid )
    {
      *physicalAddr = (uint64_t)((page_table[i].page_frame << VIRTUAL_PAGE_SHIFT) | offset);
      hitFlag = 1;
    }
  }
  /* Page table miss*/
  if( hitFlag == 0)
  {
    *pta_miss ++;
    /* LRU */
    // find oldest entry
    for( i = 0; i < nextEntryIndex; i++ )
    {
      if( page_table[i].page_frame == page_frame )
      {
        found = 1;
        page_table[i].age = 0;
        *physicalAddr = (uint64_t)((page_table[i].page_frame << VIRTUAL_PAGE_SHIFT) | offset);
      }
      else
      {
        page_table[i].age ++;
        if( page_table[i].age > oldestAge )
        {
          oldestAge = page_table[i].age;
          indexOfOldest = i;
        }
      }
    }
    // oldest entry not found, either replace the oldest entry or insert a new one
    if( found == 0)
    {
      if( nextEntryIndex <= entries )
      {
        page_table[nextEntryIndex].virtual_page = virtual_page;
        page_table[nextEntryIndex].page_frame = page_frame;
        page_table[nextEntryIndex].age = 0;
        page_table[nextEntryIndex].isValid = 1;
        nextEntryIndex ++;
        *physicalAddr = (uint64_t)((page_table[i].page_frame << VIRTUAL_PAGE_SHIFT) | offset);
      }
      else
      {
        page_table[indexOfOldest].virtual_page = virtual_page;
        page_table[indexOfOldest].page_frame = page_frame;
        page_table[indexOfOldest].age = 0;
        page_table[indexOfOldest].isValid = 1;
        nextEntryIndex = 0;
        *physicalAddr = (uint64_t)((page_table[i].page_frame << VIRTUAL_PAGE_SHIFT) | offset);
      }
    }
  }
  return 0;
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
  uint64_t pta_miss = 0;
  pta_node *page_table = NULL;          // page table

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
    page_table[i].page_frame = -1;
    page_table[i].age = 0;
    page_table[i].isValid = 0;
  }

  printf("Reading memory traces...\n");
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
     ret = mapVirtualaddr(virtual_addr,
                          entries,
                          page_size,
                          &pta_miss,
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
#ifdef DEBUG
     printf("PageTable misses: %llu\n", pta_miss);
#endif

  free(page_table);
  fclose(infile);
  fclose(outfile);

  return 0;
}
