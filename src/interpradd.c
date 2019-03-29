/*
 * _INTERPRADD_C_ *
 *
 * FUNCTIONS TO INTERPRATE THE ELEMENT ADDRESS TO ITS NEIGHBOURS ADDRESSES
 * BASED ON THE STENCIL SCHEMA
 *
 */

 #include <stdio.h>
 #include <stdlib.h>
 #include <unistd.h>

 /* ---------------------------------------------- INTERPRADD*/
 extern int interpradd( uint64_t *addr_arr,
                        long i,
                        long j,
                        long k,
                        long size_x,
                        long size_y,
                        long size_z,
                        long type )
{
  uint64_t *rtn_i = NULL;
  int points;
  /*
   * Make sure memory access are valid
   *
   */
   if( i < 0 || j < 0 || k < 0) {
     return -1;
   }
   else if ( i > (size_x - 1) || j > (size_y - 1) || k > (size_z - 1) ) {
     return -1;
   }

   switch( type ) {
    case 1:
      points = 3;
      rtn_i = malloc( sizeof( int ) * points);
      if( i = 0) {
        *rtn_i = size_x - 1;
        *(rtn_i + 1) = 0;
        *(rtn_i + 2) = 1;
      } else if ( i = size_x - 1) {
        *rtn_i = i - 1;
        *(rtn_i + 1) = i;
        *(rtn_i + 2) = 0;
      } else {
        *rtn_i = i - 1;
        *(rtn_i + 1) = i;
        *(rtn_i + 2) = i + 1;
      }
      break;
    case 2:
      points = 3;
      break;
    case 3:
      points = 3;
      break;
    case 4:
      points = 3;
      break;
    case 5:
      points = 3;
      break;
    case 6:
      points = 3;
      break;
   }

   *addr_arr = *rtn_i;

   return 0;
}

/* EOF */
