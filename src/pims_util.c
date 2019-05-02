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

/* --------------------------------------------- WIRTE_CACHE_INFO */
extern void write_cache_info( FILE* fp, char* filename, uint64_t ways,
                       uint64_t cache_size, uint64_t block_size,
                       uint64_t load_hits, uint64_t load_misses,
                       uint64_t store_hits, uint64_t store_misses,
                       double hit_rate, double miss_rate )
{
  fprintf(fp, "# Trace file path: %s\n", filename);
  fprintf(fp, "# Cache ways:      %" PRId64 "\n", ways);
  fprintf(fp, "# Cache size:      %" PRId64 "\n", cache_size);
  fprintf(fp, "# Block size:      %" PRId64 "\n", block_size);
  fprintf(fp, "# Load hits:       %" PRId64 "\n", load_hits);
  fprintf(fp, "# Load misses:     %" PRId64 "\n", load_misses);
  fprintf(fp, "# Store hits:      %" PRId64 "\n", store_hits);
  fprintf(fp, "# Store misses:    %" PRId64 "\n", store_misses);
  fprintf(fp, "# Hits rate:       %f\n", hit_rate);
  fprintf(fp, "# Misses rate:     %f\n", miss_rate);
  fprintf(fp, "#==============================================================================\n");
}
/* EOF */
