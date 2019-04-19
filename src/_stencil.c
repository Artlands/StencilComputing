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
 #include "pims_struct.h"

 #define DEBUG

/* ---------------------------------------------- FUNCTION PROTOTYPES*/
extern int getshiftamount( uint32_t bsize,
                           uint32_t *shiftamt );

extern int genrands( uint64_t *addr_a,
                     uint64_t *addr_b,
                     uint64_t num_req,
                     uint32_t num_devs,
                     uint32_t capacity,
                     uint32_t shiftamt );

extern int interpradd( uint64_t *rd_pos,
                       uint64_t *wr_pos,
                       uint64_t *lc_pos,
                       long i,
                       long j,
                       long k,
                       long size_x,
                       long size_y,
                       long size_z,
                       long sten_type );

extern int getpimsid( uint32_t *pimsid,
                      uint64_t addr,
                      uint32_t shiftamt,
                      uint32_t num_vaults);

extern int joinrqst( uint64_t addr,
                     uint32_t shiftamt,
                     uint32_t num_vaults,
                     uint32_t buf_size,
                     struct pimsbuf *pims_buf,
                     uint32_t cur_id,
                     int flag,
                     uint64_t *pim_rd_count_tmp,
                     FILE *outfile);

/* ------------------------------------------------------------ FREEBUF*/
static void freebuf( struct pimsbuf *pimsbuf, uint32_t num_vaults) {
  uint32_t i = 0;
  for( i = 0; i < num_vaults; i++) {
    free(pimsbuf[i].in_addr);
    free(pimsbuf[i].out_addr);
  }
}
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
  uint32_t num_vaults = 0;
  uint32_t shiftamt = 0;

  /*STENCIL GRID*/
  int sten_type = 0;
  long size_x = 0x00l;
  long size_y = 0x00l;
  long size_z = 0x00l;
  uint64_t num_req;
  // uint64_t *addr_arr = NULL;
  uint64_t *addr_a = NULL;
  uint64_t *addr_b = NULL;

  /*STENCIL COMPUTING*/
  long i = 0x00l;
  long j = 0x00l;
  long k = 0x00l;
  uint64_t rd_idx = 0x00ll;
  uint64_t wr_idx = 0x00ll;

  /*MEMORY TRACE FILE*/
  FILE *outfile = NULL;
  char filename[1024];
  int points_size = 0;
  int p = 0;
  uint64_t *rd_pos = NULL;
  uint64_t *wr_pos = NULL;
  uint64_t *lc_pos = NULL;

  /*PIMS*/
  int flag = 0;                   // Indicate whether the send requests from CPU are done
  uint32_t *pimsid = NULL;
  uint32_t cur_id = 0x00l;
  uint32_t buf_size = 0x00l;
  uint64_t pims_addr = 0x00ll;
  struct pimsbuf *pims_buf = NULL;
  uint64_t row_rd_count = 0x00ll;
  uint64_t pim_rd_count = 0x00ll;
  uint64_t *pim_rd_count_tmp = NULL;
  uint64_t wr_count = 0x00ll;
  /* ---- */

  while( (ret = getopt( argc, argv, "b:c:d:f:ht:v:x:y:z:F:")) != -1 ) {
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
      case 'f':
        buf_size = (uint32_t)(atoi(optarg));
        break;
      case 'F':
        sprintf( filename, "%s", optarg);
        break;
      case 'h':
        printf("%s%s%s\n","usage : ", argv[0], " -bcdhltvxyzF" );
        printf(" -b <block_size>\n");
        printf(" -c <capacity>\n");
        printf(" -d <num_devs>\n");
        printf(" -f <buf_size>\n");
        printf(" -h ...print help\n");
        printf(" -t <sten_type>\n");
        printf(" -v <num_vaults>\n");
        printf(" -x <size_x>\n");
        printf(" -x <size_y>\n");
        printf(" -z <size_z>\n");
        printf(" -F <output trace file>\n");
        return 0;
        break;
      case 't':
        sten_type = (int)(atoi(optarg));
        break;
      case 'v':
        num_vaults = (uint32_t)(atoi(optarg));
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
   goto cleanup;
  }

  /*
   * Get the shift amount based upon the block size
   *
   */
  if( getshiftamount( bsize, &shiftamt) != 0 ) {
    printf("Failed to retrive shift amount\n");
    goto cleanup;
  }

#ifdef DEBUG
  printf("Shift amount: %u\n", shiftamt);
#endif
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
    goto cleanup;
  }

  /*
   * Get points size, allocate memory for storing rd_pos
   *
   */
  switch( sten_type ) {
    case 1:
      points_size = 3;
      break;
    case 2:
      points_size = 5;
      break;
    case 3:
      points_size = 5;
      break;
    case 4:
      points_size = 9;
      break;
    case 5:
      points_size = 7;
      break;
    case 6:
      points_size = 27;
      break;
    default:
      break;
  }

  rd_pos = malloc( sizeof( uint64_t ) * points_size);
  if( rd_pos == NULL ) {
    printf("Failed to allocation memory for storing rd_pos\n");
    goto cleanup;
  }

  wr_pos = malloc( sizeof( uint64_t ) * 1);
  if( wr_pos == NULL ) {
    printf("Failed to allocation memory for storing rd_pos\n");
    goto cleanup;
  }

  lc_pos = malloc( sizeof( uint64_t ) * 1);
  if( lc_pos == NULL ) {
    printf("Failed to allocation memory for storing lc_pos\n");
    goto cleanup;
  }

  pimsid = malloc( sizeof( uint32_t ) * 1);
  if( pimsid == NULL ) {
    printf("Failed to allocation memory for pimsid\n");
    goto cleanup;
  }

  pims_buf = malloc( sizeof( struct pimsbuf ) * num_vaults);
  if( pims_buf == NULL ) {
    printf("Failed to allocation memory for pims_buf\n");
    goto cleanup;
  }
  pim_rd_count_tmp = malloc( sizeof( uint64_t ) * 1);
  if( pim_rd_count_tmp == NULL ) {
    printf("Failed to allocation memory for storing pim read requests counts\n");
    goto cleanup;
  }

  /*
   * Initialize pims_buf
   *
   */
  for( i = 0; i < num_vaults; i++) {
    pims_buf[i].pimsid = i;
    pims_buf[i].in_size = 0x00l;
    pims_buf[i].out_size = 0x00l;
    pims_buf[i].in_addr = NULL;
    pims_buf[i].out_addr = NULL;
    pims_buf[i].in_addr = malloc( sizeof( uint64_t ) * buf_size);
    if( pims_buf[i].in_addr == NULL ) {
      printf("Failed to allocation memory for each buffer\n");
      goto cleanup;
    }
    pims_buf[i].out_addr = malloc( sizeof( struct flit ) * num_vaults);
    if( pims_buf[i].out_addr == NULL ) {
      printf("Failed to allocation memory for each buffer\n");
      goto cleanup;
    }
    for( j = 0; j < num_vaults; j++) {
      pims_buf[i].out_addr[j].flitid = j;
      pims_buf[i].out_addr[j].size = 0;
      for ( k = 0; k < 16; k++) {
        pims_buf[i].out_addr[j].addr[k] = 0x00ll;
      }
    }
  }

  /*
   * Generate memory address of stencil computing
   *
   */
  switch( sten_type ) {
    case 1:
    case 2:
      for ( i = 0; i < size_x; i++) {
        if( i == (size_x - 1)) {
          flag = 1;
        }
        // Interpration
        if( interpradd( rd_pos,
                        wr_pos,
                        lc_pos,
                        i,
                        j,
                        k,
                        size_x,
                        size_y,
                        size_z,
                        sten_type) != 0) {
          printf("Failed to interprate the address\n");
          goto cleanup;
        }
        // Get pimsid
        pims_addr = (uint64_t)addr_a[lc_pos[0]];
        if( getpimsid( pimsid,
                       pims_addr,
                       shiftamt,
                       num_vaults) != 0 ) {
          printf("Failed to get PIMS id\n");
        }
        cur_id = *pimsid;

        for( p = 0; p < points_size; p++) {
          row_rd_count ++;
          rd_idx = rd_pos[p];
          if ( joinrqst( (uint64_t)addr_a[rd_idx],
                         shiftamt,
                         num_vaults,
                         buf_size,
                         pims_buf,
                         cur_id,
                         flag,
                         pim_rd_count_tmp,
                         outfile) != 0) {
            printf("Failed to coalesce the address\n");
          }
        }
        wr_count ++;

#ifdef DEBUG
        printf("PIMS Current id: %d\n",cur_id);
        printf(  "%s%016" PRIX64 "\n",
                 "Current address:",
                 pims_addr);
        fprintf( outfile,
                 "%s%016" PRIX64 "\n",
                 "RD:8:0:0x",
                 (uint64_t)pims_addr );
#endif
      }
      pim_rd_count = *pim_rd_count_tmp;
      printf("ROW READ REQUESTS COUNT: %" PRIu64 "\n",row_rd_count);
      printf("PIM READ REQUESTS COUNT: %" PRIu64 "\n",pim_rd_count);
      printf("WRITE REQUESTS COUNT: %" PRIu64 "\n",row_rd_count);
      break;
    case 3:
    case 4:
      for ( i = 0; i < size_x; i++) {
        for (j = 0; j < size_y; j ++) {
          // Function
        }
      }
      break;
    case 5:
    case 6:
      for ( i = 0; i < size_x; i++) {
        for (j = 0; j < size_y; j++) {
          for ( k = 0; k < size_z; k++) {
            // Function
          }
        }
      }
      break;
    default:
      break;
 }

cleanup:
  free( addr_a );
  free( addr_b );
  free( rd_pos );
  free( wr_pos );
  free( lc_pos );
  free( pimsid );
  freebuf( pims_buf, num_vaults );
  free( pims_buf );
  fclose(outfile);
  return 0;
}