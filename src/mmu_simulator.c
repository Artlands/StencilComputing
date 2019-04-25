/*
 * _MMU_SIMULATOR_C_
 *
 * FUNCTION TO TRANSLATE VIRTUAL MEMORY ADDRESSES TO PHYSICAL MEMORY ADDRESSES
 *
 * mapVirtualAddr function refers to https://github.com/ktno43/Virtual-Memory-Addressing/blob/master/Source.c
 *
 */

/* Included files*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <inttypes.h>

#define DEBUG

/* Physical Memory Parameters */
#define PAGESIZE 4096
#define GB 1073741824

/* Masks for Virtual address */
#define VIRTUAL_PAGE_MASK 0xFFFFFFFFFFFFF000
#define VIRTUAL_OFFSET_MASK 0x0000000000000FFF
#define VIRTUAL_PAGE_SHIFT 12


/* ---------------------------------------------- DATA STRUCTURE*/
struct pageTableNode
{
 int64_t virtualPage;
 int64_t pageFrame;
 int64_t hit;
};

struct traceNode
{
 char op[10];
 int num_bytes;
 int procid;
 uint64_t addr;
};

/* ---------------------------------------------- FUNCTION PROTOTYPES*/
static int read_trace( FILE *infile, struct traceNode *trace)
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

static int mapVirtualAddr(uint64_t virtualAddr,
                          uint32_t entries,
                          struct pageTableNode *pageTable,
                          uint64_t *physicalAddr)
{
  /* vars */
  int i, j, k, m;
  int upMoves;
  int start;
  int prevHit;
  int currHit;

  /* PAGE TABLE */
  int64_t virtualPage = (int64_t)((virtualAddr & VIRTUAL_PAGE_MASK)
                                   >> VIRTUAL_PAGE_SHIFT);
  int64_t offset = (int64_t)(virtualAddr & VIRTUAL_OFFSET_MASK);
  int64_t pageFrame = -1;

#ifdef DEBUG
  printf("Virtual page:%"PRIX64"\n", virtualPage);
  printf("Offset:%"PRIX64"\n", offset);
#endif

  /* check for end of table, unallocated entry or matched entry in table */
  i = 0;
  while( (i < entries) && (pageTable[i].virtualPage != -1) &&
         (pageTable[i].virtualPage != virtualPage) )
  {
    i++;
  }

  /* In case of end of the table, replace entry */
  if( i >= entries )
  {
    pageFrame = pageTable[0].pageFrame;               // previous reference to pageFrame
    for( j = 0; j < entries - 1; j++ )
    {
      pageTable[j] = pageTable[j + 1];                // shift all entries up
    }
    pageTable[entries - 1].virtualPage = virtualPage; // last entry will be the new VP
    pageTable[entries - 1].pageFrame = pageFrame;     // last entry's PF will be the previous PF
    pageTable[entries - 1].hit = 0;                   // set the last entry's counter

    k = 1;
    /* while the current's hit count is < the previous hit count */
    while( pageTable[entries - 1].hit < pageTable[entries - 1 - k].hit)
    {
      k++;
    }
    upMoves = k -1;                            // number of moves is off by 1
    start = entries - 1;                       // Variable for starting position for page table
    for( m = 0; m < upMoves; m++, start--)
    {
      pageTable[start] = pageTable[start - 1]; // Move each entry to the one above
    }
    pageTable[entries - 1 - upMoves].virtualPage = virtualPage;
    pageTable[entries - 1 - upMoves].pageFrame = pageFrame;
    pageTable[entries - 1 - upMoves].hit = 0;
  }
  /* In case of unallocated entry, set entry according to virtual page and page frame */
  else if( pageTable[i].virtualPage == -1 )
  {
    pageTable[i].virtualPage = virtualPage;    // set current index to new VP
    pageTable[i].pageFrame = i;                // set current PF to index because index = PF
    prevHit = pageTable[i].hit;                // previous reference to number of hits

    j = 1;
    /* while the index is a valid index and the current hit count is less than the previous*/
    while( !((i - j) <= -1) && pageTable[i].hit < pageTable[i - j].hit )
    {
      j++;
    }
    upMoves = j - 1;
    k = i;
    start = 0;
    for( start = 0; start < upMoves; start++, k--)
    {
      pageTable[k] = pageTable[k - 1];                  // move each entry to the one above
    }
    pageTable[i - upMoves].virtualPage = virtualPage;   // update the VP at the new location
    pageTable[i - upMoves].pageFrame = i;               // update the PF to the new location
    pageTable[i - upMoves].hit = prevHit;               // update the hit count to the previous hit
  }
  /* In case of hit in page table, calculate physical address, update page table */
  else
  {
    pageFrame = pageTable[i].pageFrame;
    *physicalAddr = (uint64_t)((pageFrame << VIRTUAL_PAGE_SHIFT) | offset);

    /* LFU policy*/
    pageTable[i].hit += 1;
    currHit = pageTable[i].hit;

    k = i;
    /* while a valid entry and the current hit count is greater than the next, swap them */
    while( (k < entries - 1) && (pageTable[i].hit >= pageTable[k + 1].hit) )
    {
      if( pageTable[k + 1].virtualPage == -1 )    // check to see if there is an entry or not
      {
        break;
      }
      pageTable[k] = pageTable[k + 1];            // current reference will be the next reference
      pageTable[k + 1].virtualPage = virtualPage; // next reference's VP will be the current VP
      pageTable[k + 1].pageFrame = pageFrame;     // next reference's PF will be the current PF
      pageTable[k + 1].hit = currHit;             // next reference's hit will be the current hit
      k ++;                                       // keep going until the end of the table or an empty entry
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
  uint32_t capacity = 0;            // HMC capacity
  uint32_t memSize = 0;             // Main memory size
  uint32_t pageSize = PAGESIZE;     // page size
  uint32_t entries = 0;             // number of entries
  struct pageTableNode *pageTable = NULL;  // page table

  /* MEMORY TRACE*/
  char infilename[1024];
  char outfilename[1024];
  FILE *infile = NULL;    // read trace file
  FILE *outfile = NULL;   // save trace file
  struct traceNode trace;
  uint64_t virtualAddr;
  uint64_t physicalAddr;

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
  memSize = capacity * GB;
  entries = memSize / pageSize;
  pageTable = (struct pageTableNode *)malloc(entries * sizeof(struct pageTableNode));

  if( pageTable == NULL )
  {
    printf("Error: Out of memory\n");
    return -1;
  }

  for( i = 0; i < entries; i++)
  {
    pageTable[i].virtualPage = -1;
    pageTable[i].hit = 0;
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
     ret = mapVirtualAddr(trace.addr, entries, pageTable, &physicalAddr);

     if ( ret == 0 ){
       /*write trace*/
       write_to_file(outfile, trace.op, trace.num_bytes, trace.procid, physicalAddr);
     }
     /* read next request from the input file until to the end of file*/
     done = read_trace( infile, &trace );
     while( done != 0 ){
       done = read_trace( infile, &trace);
       if( done == -2) break;
     }
  }

  free(pageTable);
  fclose(infile);
  fclose(outfile);

  return 0;
}
