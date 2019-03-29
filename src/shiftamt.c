/*
 * _SHIFTAMT_C_  *
 * FUNCTIONS TO GET THE APPROPRIATE ADDRESS SHIFT AMOUNT
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

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
