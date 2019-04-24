/*
 * _MMU_SIMULATOR_C_
 *
 * FUNCTION TO TRANSLATE VIRTUAL MEMORY ADDRESSES TO PHYSICAL MEMORY ADDRESSES
 *
 */

 /* Included files*/
 #include <stdio.h>
 #include <stdlib.h>

 #define 4K_PAGESIZE 4096
 #define 1_GB 1073741824
 #define DEBUG

 /* ---------------------------------------------- DATA STRUCTURE*/
 typedef struct
 {
   uint64_t virtualPage;
   uint64_t pageFrame;
   uint64_t hit;
 }pageTableNode;

 typedef struct
 {
   char op[10];
   int num_bytes;
   int procid;
   uint64_t addr;
 }traceNode;

 /* ---------------------------------------------- FUNCTION PROTOTYPES*/
 static int read_trace( FILE *infile, struct traceNode *trace)
 {
   /* vars */
   char buf[50];
   char *token;
   size_t len = 0;

   /* read a single entry from the trace file */
   if( feof(infile) ){
     return 2;
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

   if( buf[0] == '#' ){
     // not a valid trace
     return -1;
   }

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

 static int mapVirtualAddr(uint64_t virtualAddr,
                           uint32_t pageSize,
                           struct pageTableNode pageTable,
                           uint64_t *physicalAddr)
{
  /* vars */
  int i;

  /* PAGE TABLE */
  uint64_t virtualPage = virtualAddr / pageSize;
  uint64_t offset = virtualAddr % pageSize;
  int64_t pageFrame = -1;

  /* check for end of table, unallocated entry or matched entry in table */
  i = 0;
  while( (i < entries) && (pageTable[i].virtualPage != 0) &&
         (pageTable[i].virtualPage != virtualPage) )
  {
    i++;
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
  int ret = 0;
  int i = 0;

  /* PAGE TABLE*/
  uint32_t capacity = 0;            // HMC capacity
  uint32_t memSize = 0;             // Main memory size
  uint32_t pageSize = 4K_PAGESIZE;  // page size
  uint32_t entries = 0;             // number of entries
  pageTableNode *pageTable = NULL;  // page table

  /* MEMORY TRACE*/
  char infilename[1024];
  char outfilename[1024];
  FILE *infile = NULL;    // read trace file
  FILE *outfile = NULL;   // save trace file
  traceNode trace;
  uint64_t virtualAddr;
  uint64_t offset;
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
  memSize = capacity * 1_GB;
  entries = memSize / pageSize;
  pageTable = (pageTableNode *)malloc(entries * sizeof(pageTableNode));

  if( pageTable == NULL )
  {
    printf("Error: Out of memory\n");
    return -1;
  }

  for( i = 0; i < entries; i++)
  {
    pageTable[i].virtualPage = 0;
    pageTable[i].hit = 0;
  }

  /* read first request from the input file */
  ret = read_trace( infile, &trace );
  while( ret != 0 ){
    ret = read_trace( infile, &trace);
  }

  /* read all traces until to the end of file */
  while( ret == 0 ){
    /*
     * translate virtual memory address to physical memory address
     *
     */
  }

  fclose(infile);
  fclose(outfile);

  return 0;
}
