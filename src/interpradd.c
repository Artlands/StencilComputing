/*
 * INTERPRADD
 *
 * FUNCTIONS TO INTERPRATE THE ELEMENT ADDRESS TO ITS NEIGHBOURS ADDRESSES
 * BASED ON THE STENCIL SCHEMA
 *
 */

 #include <stdio.h>
 #include <stdlib.h>
 #include <unistd.h>

 /* ---------------------------------------------- INTERPRADD*/
 static int interpradd( int * addr_arr,
                        int i,
                        int j,
                        int k,
                        int size_x,
                        int size_y,
                        int size_z,
                        int type )
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



}
