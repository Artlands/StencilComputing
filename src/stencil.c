/*
 * STENCIL MEMORY TRACES SIMULATION
 *
 * Simulate the memory traces of stencil computing
 *
 * Iteration type : Jacobi Iteration, involve 2 same size arrays: A, B
 *
 * Dimension:1D, 2D, 3D
 *
 * Order: 1
 *
 * Boundary Condition: Periodic,
 * i.e. the element before the first one is the last element in this dimension.
 * the element after the last on is the first element in this dimension.
 *
 * Access types(ignore coefficient):
 * 1D 3-points: B[i] = A[i-1] + A[i] + A[i+1]
 * 1D 5-points: B[i] = A[i-2] + A[i-1] + A[i] + A[i+1] + A[i+2]
 * 2D 5-points: B[i][j] = A[i-1][j] + A[i][j] + A[i+1][j] + A[i][j-1] + A[i][j+1]
 * 2D 9-points: B[i][j] = A[i-1][j] + A[i][j] + A[i+1][j] + A[i][j-1] + A[i][j+1]
 *                        A[i-1][j-1] + A[i+1][j-1] + A[i+1][j-1] + A[i+1][j-+]
 * 3D 7-points: B[i][j][k] = A[i-1][j][k] + A[i][j][k] + A[i+1][j][k] + A[i][j-1][k]
 *                         + A[i][j+1][k] + A[i][j][k-1] + A[i][j][k+1]
 * 3D 27-points: B[i][j][k] = A[i][j][k]
 *                         + (A[i-1][j][k] + A[i+1][j][k] + A[i][j-1][k] + A[i][j+1][k] + A[i][j][k-1] + A[i][j][k+1])
 *                         + (A[i-1][j][k-1] + A[i+1][j][k-1] + A[i][j-1][k-1] + A[i][j+1][k-1]
 *                           +A[i-1][j-1][k] + A[i-1][j+1][k] + A[i+1][j-1][k] + A[i+1][j+1][k]
 *                           +A[i-1][j][k+1] + A[i+1][j][k+1] + A[i][j-1][k+1] + A[i][j+1][k+1])
 *                         + (A[i-1][j-1][k-1] + A[i-1][j+1][k-1] + A[i+1][j-1][k-1] + A[i+1][j+1][k-1]
 *                           +A[i-1][j-1][k+1] + A[i-1][j+1][k+1] + A[i+1][j-1][k+1] + A[i+1][j+1][k+1])
 */

 #include <stdio.h>
 #include <stdlib.h>
 #include <unistd.h>

 /* ---------------------------------------------- FUNCTION PROTOTYPES*/
extern int getshiftamount ( uint32_t num_links,
                            uint32_t capacity,
                            uint32_t bsize,
                            uint32_t *shiftamt );
 /* ---------------------------------------------- GENRANDS*/
static int genrands( uint64_t *addr_a,
                     uint64_t *addr_b,
                     long num_req,
                     uint32_t num_devs,
                     uint32_t capacity,
                     uint32_t shiftamt )
{
  /* vars */
  long i = 0x00l;
  uint64_t base_a = 0x00ll;
  uint64_t base_b = 0x00ll;
  uint64_t offset = 0x00ll;
  /* ---- */

  if( addr_a == NULL) {
    return -1;
  }else if( addr_b == NULL) {
    return -1;
  }

  /*
   * Make sure we have enough capacity
   * one slot stores 1 double float point data, i.e. 64bits or 8Bytes
   */
#define SLOTS_PER_GB 134217728
  if( (long)((long)capacity * (long)num_devs * (long)SLOTS_PER_GB) <
      (long)((num_req) * (long)(3)) ) {
        printf("ERROR: NOT ENOUGH AVAILABLE PHYSICAL STORAGE\n");
        return -1;
      }
  /*
   * Caculate the base of each vector
   *
   */
  offset = (uint64_t) num_req * 0x08ll;

  base_a = (0x00ll) << (uint64_t)(shiftamt);
  base_b = base_a + ( (offset) << (uint64_t)(shiftamt) );

  /*
   * Vectors a, b
   *
   */
  for( i = 0; i < num_req; i++) {
    addr_a[i] = (uint64_t)( base_a + (((uint64_t)(i) * 0x08ll) << (uint64_t)(shiftamt)) );
    addr_b[i] = (uint64_t)( base_a + (((uint64_t)(i) * 0x08ll) << (uint64_t)(shiftamt)) );
  }

  return 0;
}
