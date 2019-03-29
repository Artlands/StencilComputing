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

  if( rd_pos == NULL) {
    return -1;
  }else if( wr_pos == NULL) {
    return -1;
  }
  switch( sten_type ) {
    case 1:
      if( i == 0) {
        rd_pos[0] = (uint64_t)(size_x - 1);
        rd_pos[1] = (uint64_t)0;
        rd_pos[2] = (uint64_t)1;
      } else if ( i == size_x - 1) {
        rd_pos[0] = (uint64_t)(i - 1);
        rd_pos[1] = (uint64_t)i;
        rd_pos[2] = (uint64_t)0;
      } else {
        rd_pos[0] = (uint64_t)(i - 1);
        rd_pos[1] = (uint64_t)i;
        rd_pos[2] = (uint64_t)(i + 1);
      }
      wr_pos[0] = (uint64_t)(i);
      break;
    case 2:
      if( i == 0) {
        rd_pos[0] = (uint64_t)(size_x - 2);
        rd_pos[1] = (uint64_t)(size_x - 1);
        rd_pos[2] = (uint64_t)0;
        rd_pos[3] = (uint64_t)1;
        rd_pos[4] = (uint64_t)2;
      } else if ( i == size_x - 1) {
        rd_pos[0] = (uint64_t)(i - 2);
        rd_pos[1] = (uint64_t)(i - 1);
        rd_pos[2] = (uint64_t)i;
        rd_pos[3] = (uint64_t)0;
        rd_pos[4] = (uint64_t)1;
      } else {
        rd_pos[0] = (uint64_t)(i - 2);
        rd_pos[1] = (uint64_t)(i - 1);
        rd_pos[2] = (uint64_t)i;
        rd_pos[3] = (uint64_t)(i + 1);
        rd_pos[4] = (uint64_t)(i + 2);
      }
      wr_pos[0] = (uint64_t)(i);
      break;
    case 3:
      break;
    case 4:
      break;
    case 5:
      break;
    case 6:
      break;
    default:
      break;
   }
   return 0;
}

/* EOF */
