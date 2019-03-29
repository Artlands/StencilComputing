/*
 * STENCIL MEMORY TRACES SIMULATION
 *
 * Simulate the memory traces of stencil computing
 *
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
                     int size_x,
                     int size_y,
                     int size_z,
                     uint32_t num_devs,
                     uint32_t capacity,
                     uint32_t shiftamt )
{
  /* vars */
  long i = 0x00l;
  long num_req = 0x00l;
  uint64_t base_a = 0x00ll;
  uint64_t base_b = 0x00ll;
  uint64_t offset = 0x00ll;
  /* ---- */

  if (size_x == 0) {
    return -1;
  } else if(size_y == 0 && size_z != 0) {
    return -1;
  }

  if( addr_a == NULL) {
    return -1;
  }else if( addr_b == NULL) {
    return -1;
  }

  if( size_y == 0) {
    num_req = (long)size_x;
  } else if(size_z == 0) {
    num_req = (long)((long)size_x * (long)size_y);
  } else {
    num_req = (long)((long)size_x * (long)size_y * (long)size_z);
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
  offset = (uint64_t)num_req * 0x08ll;

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
