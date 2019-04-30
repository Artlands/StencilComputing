/*
 * _PIMS_UTIL_C_  *
 * PIMS UTILITY FUNCTIONS
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <inttypes.h>
#include "pims.h"

/* Global variables */
uint64_t pta_miss;
uint64_t oldestAge;
uint64_t indexOfOldest;
uint64_t nextEntryIndex;

/* --------------------------------------------- READTRACE */

extern int read_trace( FILE *infile, trace_node *trace)
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

/* EOF */

/* --------------------------------------------- WRITETOFILE */

/*
 * Write traces
 * Format: {WR,RD,PIMS_RD}:{NUM_BYTES}:{PROCID}:{0xADDR}
 *
 */
extern void write_to_file(FILE* fp,
                          char* op,
                          int num_bytes,
                          int procid,
                          uint64_t addr)
{
  fprintf(fp, "%s:%d:%d:0x%016"PRIX64"\n", op, num_bytes, procid, addr);
}
/* EOF */

/* --------------------------------------------- GETPIMSID */

extern int getpimsid (uint32_t *pimsid,
                      uint64_t addr,
                      uint32_t shiftamt,
                      uint32_t num_vaults)
{
  if( pimsid == NULL ) {
    return -1;
  }

  switch( num_vaults )
  {
    case 32:
      *pimsid = (uint32_t) ((addr >> shiftamt) & 0x1F);
      break;
    case 16:
      *pimsid = (uint32_t) ((addr >> shiftamt) & 0xF);
      break;
    default:
      return -1;
      break;
  }
  return 0;
}

/* EOF */

/* --------------------------------------------- GETSHIFTAMOUNT */

extern int getshiftamount( uint32_t bsize,
                    			 uint32_t *shiftamt )
{
  switch( bsize )
  {
    case 32 :
      *shiftamt = 5;
      break;
    case 64 :
      *shiftamt = 6;
      break;
    case 128:
     *shiftamt = 7;
      break;
    case 256:
      *shiftamt = 8;
      break;
    default:
      return -1;
      break;
  }
	return 0;
}

/* EOF */

/* --------------------------------------------- GENERANDOM */
extern int geneRandom(int size)
{
  int i, n;
  static int numNums = 0;
  static int *numArr = NULL;

  // Initialize with a specific size.

  if (size >= 0) {
     if (numArr != NULL)
         free (numArr);
     if ((numArr = malloc (sizeof(unsigned) * size)) == NULL)
         return -1;
     for (i = 0; i  < size; i++)
         numArr[i] = i;
     numNums = size;
  }

  // Error if no numbers left in pool.

  if (numNums == 0)
    return -2;

  // Get random number from pool and remove it (rnd in this
  //   case returns a number between 0 and numNums-1 inclusive).

  n = rand() % numNums;
  i = numArr[n];
  numArr[n] = numArr[numNums-1];
  numNums--;
  if (numNums == 0)
  {
     free (numArr);
     numArr = 0;
  }

  return i;
}
/* EOF */

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
  // printf("Virtual page:%"PRIX64"\n", virtual_page);
  // printf("Offset:%"PRIX64"\n", offset);
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
      }
    }
    // Add new entry
    page_table[nextEntryIndex].virtual_page = virtual_page;
    *physical_addr = (uint64_t)( ( page_table[nextEntryIndex].page_frame
                                   << VIRTUAL_PAGE_SHIFT) | offset);
    nextEntryIndex ++;
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
  }
}
/* EOF */

/* --------------------------------------------- WIRTE_STEN_INFO */
extern void write_sten_info(FILE* fp, char* filename, int dim,
                     int dim_x, int dim_y, int dim_z,
                     uint32_t cntr_size, int sten_order,
                     int sten_coeff, char* data_type)
{
  uint32_t inner = 0;
  switch(dim)
  {
    case 1:
      inner = dim_x - sten_order * 2;
      break;
    case 2:
      inner = (dim_x - sten_order * 2) * (dim_y - sten_order * 2);
      break;
    case 3:
      inner = (dim_x - sten_order * 2) * (dim_y - sten_order * 2)
            * (dim_z - sten_order * 2);
      break;
    default:
      break;
  }
  fprintf(fp,"# Trace file path:       %s\n", filename);
  fprintf(fp,"# Stencil Dimension:    %u, X: %d, Y: %d, Z: %d\n",
          dim, dim_x, dim_y, dim_z);
  fprintf(fp,"# Stencil Grid Size:    %u\n", cntr_size);
  fprintf(fp,"# Stencil Inner Size:   %u\n", inner);
  fprintf(fp,"# Stencil Order:        %d\n", sten_order);
  fprintf(fp,"# Stencil Coefficients: %d\n", sten_coeff);
  fprintf(fp,"# Stencil Data Type:    %s\n", data_type);
  fprintf(fp,"#==============================================================================\n");
}
/* EOF */
