/*
 * STENCIL MEMORY TRACES SIMULATION
 *
 * Simulate the memory traces of stencil computing
 *
 */

 #include <stdio.h>
 #include <stdlib.h>
 #include <unistd.h>
 #include <string.h>
 #include <stdint.h>
 #include <inttypes.h>
 #include <sys/types.h>
 #include <sys/stat.h>
 #include <fcntl.h>

 /* ---------------------------------------------- FUNCTION PROTOTYPES*/
extern int getshiftamount( uint32_t bsize,
                           uint32_t *shiftamt );

extern int genrands( uint64_t *addr_a,
                     uint64_t *addr_b,
                     uint64_t num_req,
                     uint32_t num_devs,
                     uint32_t capacity,
                     uint32_t shiftamt );

// extern int interpradd( uint64_t *addr_arr,
//                       long i,
//                       long j,
//                       long k,
//                       long size_x,
//                       long size_y,
//                       long size_z,
//                       long sten_type );
/* ------------------------------------------------------------ MAIN */
/*
 * MAIN
 *
 */
extern int main( int argc, char **argv) {
  /* vars */
  int ret = 0;

  /*HMC*/
  uint32_t bsize = 0;
  uint32_t capacity = 0;
  uint32_t num_devs = 0;
  uint32_t shiftamt = 0;

  /*STENCIL GRID*/
  int sten_type = 0;
  long size_x = 0;
  long size_y = 0;
  long size_z = 0;
  uint64_t num_req;
  // uint64_t *addr_arr = NULL;
  uint64_t *addr_a = NULL;
  uint64_t *addr_b = NULL;

  /*STENCIL COMPUTING*/
  long i = 0;
  long j = 0;
  long k = 0;

  /*MEMORY TRACE FILE*/
  // char trace[25];
  FILE *outfile = NULL;
  char filename[1024];
  /* ---- */

  while( (ret = getopt( argc, argv, "b:c:d:ht:x:y:z:F:")) != -1 ) {
    switch( ret ) {
      case 'b':
        bsize = (uint32_t)(atoi(optarg));
        break;
      case 'c':
        capacity = (uint32_t)(atoi(optarg));
        break;
      case 'd':
        num_devs = (uint32_t)(atoi(optarg));
        break;
      case 'F':
        sprintf( filename, "%s", optarg);
        break;
      case 'h':
        printf("%s%s%s\n","usage : ", argv[0], " -bcdhltxyzF" );
        printf(" -b <block_size>\n");
        printf(" -c <capacity>\n");
        printf(" -d <num_devs>\n");
        printf(" -h ...print help\n");
        printf(" -t <sten_type>\n");
        printf(" -x <size_x>\n");
        printf(" -x <size_y>\n");
        printf(" -z <size_z>\n");
        return 0;
        break;
      case 't':
        sten_type = (int)(atoi(optarg));
        break;
      case 'x':
        size_x = (long)(atoi(optarg));
        break;
      case 'y':
        size_y = (long)(atoi(optarg));
        break;
      case 'z':
        size_z = (long)(atoi(optarg));
        break;
      default:
        printf("%s%s%s\n", "Unknown option: see ", argv[0], " -bcdhltxyz" );
        return -1;
        break;
    }
  }

  /* Sanity check*/
  if (bsize != 32 && bsize != 64 && bsize != 128 && bsize != 256) {
    printf("ERROR: Block size is invalid\n");
    return -1;
  }

  if (capacity != 4 && capacity != 8) {
    printf("ERROR: Capacity size is invalid\n");
    return -1;
  }

  if (num_devs <= 0) {
    printf("ERROR: Number of devices is invalid\n");
    return -1;
  }

  if (sten_type <= 0 || sten_type > 6) {
    printf("ERROR: Stencil type is invalid\n");
    return -1;
  }

  if (size_x == 0) {
    printf("ERROR: Grid size is invalid\n");
    return -1;
  } else if(size_y == 0 && size_z != 0) {
    printf("ERROR: Grid size is invalid\n");
    return -1;
  } else if( size_x < 0 || size_y < 0 || size_z < 0) {
    printf("ERROR: Grid size is invalid\n");
    return -1;
  }

  if( strlen(filename) == 0) {
    printf("ERROR: File name is invalid\n");
    return -1;
  }
  /* ---- */

  outfile = fopen( filename, "w");
  if( outfile == NULL) {
    printf("ERROR: Could not open file %s\n", filename);
    return -1;
  }

  /*
   * Calculate Number of requests
   *
   */
  if( size_y == 0) {
    num_req = (uint64_t)size_x;
  } else if(size_z == 0) {
    num_req = (uint64_t)((uint64_t)size_x * (uint64_t)size_y);
  } else {
    num_req = (uint64_t)((uint64_t)size_x * (uint64_t)size_y * (uint64_t)size_z);
  }

  /*
   * Allocate memory
   *
   */
  addr_a = malloc( sizeof(uint64_t) * num_req);
  if( addr_a == NULL) {
    printf("Failed to allocate memory for addr_a\n");
    return -1;
  }
  addr_b = malloc( sizeof(uint64_t) * num_req);
  if( addr_b == NULL) {
   printf("Failed to allocate memory for addr_b\n");
   free( addr_a );
   return -1;
  }

  /*
   * Get the shift amount based upon the block size
   *
   */
  if( getshiftamount( bsize, &shiftamt) != 0 ) {
    printf("Failed to retrive shift amount\n");
    free( addr_a );
    free( addr_b );
    return -1;
  }

  /*
   * Generate memory address of grid
   *
   */
  if( genrands( addr_a,
                addr_b,
                num_req,
                num_devs,
                capacity,
                shiftamt ) != 0) {
    printf("Failed to generate addressing schema\n");
    free( addr_a );
    free( addr_b );
    return -1;
  }

  /*
   * Generate memory address of stencil computing
   *
   */
  for ( i = 0; i < num_req; i++) {
    // RD:8:0:0x0000000000000000
    // sprintf( trace, "%s%"PRIu64"\n","RD:8:0:",addr_a[i]);
    fprintf( outfile,
             "%s%016" PRIx64 "\n",
             "RD:8:0:",
             (uint64_t)addr_a[i] );
  }

  fclose(outfile);
  return 0;
}
