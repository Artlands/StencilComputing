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
 extern int interpradd( uint64_t *rd_pos,
                        uint64_t *wr_pos,
                        long i,
                        long j,
                        long k,
                        long size_x,
                        long size_y,
                        long size_z,
                        long sten_type )
{
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

   switch( sten_type ) {
    case 1:
      if( i = 0) {
       *rd_pos = size_x - 1;
        *(rd_pos + 1) = 0;
        *(rd_pos + 2) = 1;
      } else if ( i = size_x - 1) {
        *rd_pos = i - 1;
        *(rd_pos + 1) = i;
        *(rd_pos + 2) = 0;
      } else {
        *rd_pos = i - 1;
        *(rd_pos + 1) = i;
        *(rd_pos + 2) = i + 1;
      }
      *wr_pos = i;
      break;
    case 2:
      break;
    case 3:
      break;
    case 4:
      break;
    case 5:
      break;
    case 6:
      break;
   }
   return 0;
}

/* EOF */
