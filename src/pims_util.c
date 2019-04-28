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
