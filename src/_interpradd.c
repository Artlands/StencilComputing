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
                        uint64_t *lc_pos,
                        long i,
                        long j,
                        long k,
                        long size_x,
                        long size_y,
                        long size_z,
                        long sten_type )
{
  // reset i-/+1, j-/+1, k-/+1
  long i_rel_n = 0;
  long i_rel_p = 0;
  long j_rel_n = 0;
  long j_rel_p = 0;
  long k_rel_n = 0;
  long k_rel_p = 0;

  if( rd_pos == NULL) {
    return -1;
  }else if( wr_pos == NULL) {
    return -1;
  }

  // reset i-/+1
  if ( i == 0 ) {
    i_rel_n = size_x - 1;
    i_rel_p = 1;
  } else if ( i == (size_x - 1) ) {
    i_rel_n = i - 1;
    i_rel_p = 0;
  } else {
    i_rel_n = i - 1;
    i_rel_p = i + 1;
  }

  // reset j-/+1
  if ( j == 0 ) {
    j_rel_n = size_y - 1;
    j_rel_p = 1;
  } else if ( j == (size_y - 1) ) {
    j_rel_n = j - 1;
    j_rel_p = 0;
  } else {
    j_rel_n = j - 1;
    j_rel_p = j + 1;
  }

  // reset k-/+1
  if ( k == 0 ) {
    k_rel_n = size_z - 1;
    k_rel_p = 1;
  } else if ( k == (size_z - 1) ) {
    k_rel_n = k - 1;
    k_rel_p = 0;
  } else {
    k_rel_n = k - 1;
    k_rel_p = k + 1;
  }

  switch( sten_type ) {
    case 1:
      // 1D 3-points
      // read position
      rd_pos[0] = (uint64_t)(i_rel_n);
      rd_pos[1] = (uint64_t)i;
      rd_pos[2] = (uint64_t)(i_rel_p);

      // write position
      wr_pos[0] = (uint64_t)(i);

      // local position
      lc_pos[0] = (uint64_t)(i);
      break;
    case 2:
      // 1D 5-points
      // reset i-/+2, only for TYPE 2
      i_rel_n = 0;
      i_rel_p = 0;
      if ( i == 0 ) {
        i_rel_n = size_x - 2;
        i_rel_p = 2;
      } else if ( i == (size_x - 2) ) {
        i_rel_n = i - 2;
        i_rel_p = 1;
      } else {
        i_rel_n = i - 2;
        i_rel_p = i + 2;
      }
      // read position
      rd_pos[0] = (uint64_t)(i_rel_n);
      rd_pos[1] = (uint64_t)(i_rel_n + 1);
      rd_pos[2] = (uint64_t)i;
      rd_pos[3] = (uint64_t)(i_rel_p - 1);
      rd_pos[4] = (uint64_t)(i_rel_p);
      // write position
      wr_pos[0] = (uint64_t)(i);

      // local position
      lc_pos[0] = (uint64_t)(i);
      break;
    case 3:
      // 2D 5-points
      // 2D row-major: offset = i_row*NCOLS + i_col = i * size_x + j

      // read position
      // A[i-1][j]
      rd_pos[0] = (uint64_t)(i_rel_n * size_x + j);
      // A[i][j]
      rd_pos[1] = (uint64_t)(i * size_x + j);
      // A[i+1][j]
      rd_pos[2] = (uint64_t)(i_rel_p * size_x + j);
      // A[i][j-1]
      rd_pos[3] = (uint64_t)(i * size_x + j_rel_n);
      // A[i][j+1]
      rd_pos[4] = (uint64_t)(i * size_x + j_rel_p);

      // write position
      wr_pos[0] = (uint64_t)(i * size_x + j);

      // local position
      lc_pos[0] = (uint64_t)(i * size_x + j);
      break;
    case 4:
      // 2D 9-points
      // 2D row-major: offset = i_row*NCOLS + i_col = i * size_x + j

      // read position
      // A[i-1][j]
      rd_pos[0] = (uint64_t)(i_rel_n * size_x + j);
      // A[i][j]
      rd_pos[1] = (uint64_t)(i * size_x + j);
      // A[i+1][j]
      rd_pos[2] = (uint64_t)(i_rel_p * size_x + j);
      // A[i][j-1]
      rd_pos[3] = (uint64_t)(i * size_x + j_rel_n);
      // A[i][j+1]
      rd_pos[4] = (uint64_t)(i * size_x + j_rel_p);
      // A[i-1][j-1]
      rd_pos[5] = (uint64_t)(i_rel_n * size_x + j_rel_n);
      // A[i+1][j-1]
      rd_pos[6] = (uint64_t)(i_rel_p * size_x + j_rel_n);
      // A[i-1][j+1]
      rd_pos[7] = (uint64_t)(i_rel_n * size_x + j_rel_p);
      // A[i+1][j+1]
      rd_pos[8] = (uint64_t)(i_rel_p * size_x + j_rel_p);

      // write position
      wr_pos[0] = (uint64_t)(i);

      // local position
      lc_pos[0] = (uint64_t)(i * size_x + j);
      break;
    case 5:
      // 3D 7-points
      // 3D row-major: offset = n_3 + N_3 * ( n_2 + N_2 * n_1)
      //                      = k + size_z * ( j + size_y * i)

      // read position
      // A[i-1][j][k]
      rd_pos[0] = (uint64_t)(k + size_z * ( j + size_y * i_rel_n ));
      // A[i][j][k]
      rd_pos[1] = (uint64_t)(k + size_z * ( j + size_y * i ));
      // A[i+1][j][k]
      rd_pos[2] = (uint64_t)(k + size_z * ( j + size_y * i_rel_p) );
      // A[i][j-1][k]
      rd_pos[3] = (uint64_t)(k + size_z * ( j_rel_n + size_y * i ));
      // A[i][j+1][k]
      rd_pos[4] = (uint64_t)(k + size_z * ( j_rel_p + size_y * i ));
      // A[i][j][k-1]
      rd_pos[5] = (uint64_t)(k_rel_n + size_z * ( j + size_y * i ));
      // A[i][j][k+1]
      rd_pos[6] = (uint64_t)(k_rel_p + size_z * ( j + size_y * i ));

      // write position
      wr_pos[0] = (uint64_t)(k + size_z * ( j + size_y * i));

      // local position
      lc_pos[0] = (uint64_t)(k + size_z * ( j + size_y * i));
      break;
    case 6:
      // 3D 27-points
      // 3D row-major: offset = n_3 + N_3 * ( n_2 + N_2 * n_1)
      //                      = k + size_z * ( j + size_y * i)

      // read position
      // A[i-1][j][k]
      rd_pos[0] = (uint64_t)(k + size_z * ( j + size_y * i_rel_n ));
      // A[i][j][k]
      rd_pos[1] = (uint64_t)(k + size_z * ( j + size_y * i ));
      // A[i+1][j][k]
      rd_pos[2] = (uint64_t)(k + size_z * ( j + size_y * i_rel_p) );
      // A[i][j-1][k]
      rd_pos[3] = (uint64_t)(k + size_z * ( j_rel_n + size_y * i ));
      // A[i][j+1][k]
      rd_pos[4] = (uint64_t)(k + size_z * ( j_rel_p + size_y * i ));
      // A[i][j][k-1]
      rd_pos[5] = (uint64_t)(k_rel_n + size_z * ( j + size_y * i ));
      // A[i][j][k+1]
      rd_pos[6] = (uint64_t)(k_rel_p + size_z * ( j + size_y * i ));
      // A[i-1][j][k-1]
      rd_pos[7] = (uint64_t)(k_rel_n + size_z * ( j + size_y * i_rel_n ));
      // A[i+1][j][k-1]
      rd_pos[8] = (uint64_t)(k_rel_n + size_z * ( j + size_y * i_rel_p ));
      // A[i][j-1][k-1]
      rd_pos[9] = (uint64_t)(k_rel_n + size_z * ( j_rel_n + size_y * i ));
      // A[i][j+1][k-1]
      rd_pos[10] = (uint64_t)(k_rel_n + size_z * ( j_rel_p + size_y * i ));
      // A[i-1][j-1][k]
      rd_pos[11] = (uint64_t)(k + size_z * ( j_rel_n + size_y * i_rel_n ));
      // A[i-1][j+1][k]
      rd_pos[12] = (uint64_t)(k + size_z * ( j_rel_p + size_y * i_rel_n ));
      // A[i+1][j-1][k]
      rd_pos[13] = (uint64_t)(k + size_z * ( j_rel_n + size_y * i_rel_p ));
      // A[i+1][j+1][k]
      rd_pos[14] = (uint64_t)(k + size_z * ( j_rel_p + size_y * i_rel_p ));
      // A[i-1][j][k+1]
      rd_pos[15] = (uint64_t)(k_rel_p + size_z * ( j + size_y * i_rel_n ));
      // A[i+1][j][k+1]
      rd_pos[16] = (uint64_t)(k_rel_p + size_z * ( j + size_y * i_rel_p ));
      // A[i][j-1][k+1]
      rd_pos[17] = (uint64_t)(k_rel_p + size_z * ( j_rel_n + size_y * i ));
      // A[i][j+1][k+1]
      rd_pos[18] = (uint64_t)(k_rel_p + size_z * ( j_rel_p + size_y * i ));
      // A[i-1][j-1][k-1]
      rd_pos[19] = (uint64_t)(k_rel_n + size_z * ( j_rel_n + size_y * i_rel_n ));
      // A[i-1][j+1][k-1]
      rd_pos[20] = (uint64_t)(k_rel_n + size_z * ( j_rel_p + size_y * i_rel_n ));
      // A[i+1][j-1][k-1]
      rd_pos[21] = (uint64_t)(k_rel_n + size_z * ( j_rel_n + size_y * i_rel_p ));
      // A[i+1][j+1][k-1]
      rd_pos[22] = (uint64_t)(k_rel_n + size_z * ( j_rel_p + size_y * i_rel_p ));
      // A[i-1][j-1][k+1]
      rd_pos[23] = (uint64_t)(k_rel_p + size_z * ( j_rel_n + size_y * i_rel_n ));
      // A[i-1][j+1][k+1]
      rd_pos[24] = (uint64_t)(k_rel_p + size_z * ( j_rel_p + size_y * i_rel_n ));
      // A[i+1][j-1][k+1]
      rd_pos[25] = (uint64_t)(k_rel_p + size_z * ( j_rel_n + size_y * i_rel_p ));
      // A[i+1][j+1][k+1]
      rd_pos[26] = (uint64_t)(k_rel_p + size_z * ( j_rel_p + size_y * i_rel_p ));

      // write position
      wr_pos[0] = (uint64_t)(k + size_z * ( j + size_y * i));

      // local position
      lc_pos[0] = (uint64_t)(k + size_z * ( j + size_y * i));
      break;
    default:
      break;
   }
   return 0;
}

/* EOF */
