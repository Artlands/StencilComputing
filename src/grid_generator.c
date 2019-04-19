/*
 * _GRID_GENERATOR_C_
 *
 * FUNCTION TO GENERATOR STENCIL GRID ADDRESSES
 *
 */

/* Included Files*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <inttypes.h>

#define SLOTS_PER_GB 134217728
#define DEBUG

/* ---------------------------------------------- FUNCTION PROTOTYPES*/
extern int getshiftamount( uint32_t bsize,
                           uint32_t *shiftamt );

/* Main Function. Takes command line arguments, generates stencil grid addresses*/
int main(int argc, char* argv[])
{
  /*vars*/
  int ret = 0;
  int i = 0;
  int j = 0;
  int k = 0;
  int idx = 0;

  /* STENCIL GRID FEATURES
   *
   * b[i] = a[i]
   * Jocobi iteration stencil, read grid <--> write grid
   */
  uint32_t dim_x = 0;
  uint32_t dim_y = 0;
  uint32_t dim_z = 0;
  uint32_t dim = 0;            // dimension of the grid
  uint32_t cntr_size = 0;      // container size
  uint64_t offset = 0x00ll;    // offset between base_a and base_b
  uint64_t stor_size = 0x00ll; // storage size: intger-4, double-8
  uint32_t shiftamt = 0;
  char data_type[10];          // stencil data type

  /* HMC */
  uint32_t capacity = 4;    // HMC capacity
  uint32_t bsize = 0;       // HMC blocksize, the less significant bsize bits are ignored

  /* MEMORY TRACE */
  uint64_t base_a = 0x00ll;
  uint64_t base_b = 0x00ll;
  FILE *outfile = NULL;
  char filename[1024];

  /* MALLOC MEMORY*/
  uint64_t *data_cntr_a;       // data container
  uint64_t *grid_1d_a;         // pointer container
  uint64_t **grid_2d_a;        // pointer container
  uint64_t ***grid_3d_a;       // pointer container
  uint64_t *data_cntr_b;       // data container
  uint64_t *grid_1d_b;         // pointer container
  uint64_t **grid_2d_b;        // pointer container
  uint64_t ***grid_3d_b;       // pointer container

  while( (ret = getopt( argc, argv, "x:y:z:t:f:h")) != -1 )
  {
    switch( ret )
    {
      case 'x':
        dim_x = (uint32_t)(atoi(optarg));
        break;
      case 'y':
        dim_y = (uint32_t)(atoi(optarg));
        break;
      case 'z':
        dim_z = (uint32_t)(atoi(optarg));
        break;
      case 't':
        sprintf(data_type, "%s", optarg);
        break;
      case 'c':
        capacity = (uint32_t)(atoi(optarg));
        break;
      case 'b':
        bsize = (uint32_t)(atoi(optarg));
        break;
      case 'f':
        sprintf( filename, "%s", optarg);
        break;
      case 'h':
        printf("%s%s%s\n", "usage : ", argv[0], " -xyztfh");
        printf(" -x <stencil grid size on dim x>\n");
        printf(" -y <stencil grid size on dim y>\n");
        printf(" -z <stencil grid size on dim z>\n");
        printf(" -t <stencil grid data type: Integer, Double>\n");
        printf(" -c <HMC capacity: 4, 8>");
        printf(" -b <HMC blocksize: 32, 64, 128, 256>");
        printf(" -f <output trace file name>\n");
        printf(" h ...print help\n");
        return 0;
        break;
      default:
          printf("%s%s%s\n", "Unknown option: see ", argv[0], " -xyztfh" );
          return -1;
          break;
    }
  }

  /* ---- Sanity Check ---- */
  if (dim_x == 0) {
    printf("ERROR: Grid size is invalid\n");
    return -1;
  } else if(dim_y == 0 && dim_z != 0) {
    printf("ERROR: Grid size is invalid\n");
    return -1;
  } else if( dim_x < 0 || dim_y < 0 || dim_z < 0) {
    printf("ERROR: Grid size is invalid\n");
    return -1;
  }

  if (capacity != 4 && capacity != 8) {
    printf("ERROR: Capacity is invalid\n");
    return -1;
  }

  if (bsize != 32 && bsize != 64 && bsize != 128 && bsize != 256) {
    printf("ERROR: Block size is invalid\n");
    return -1;
  }
  /* ---- End Sanity Check ---- */

  /* Get Dimension and Container Size */
  if( dim_y == 0 )
  {
    dim = 1;
    cntr_size = dim_x;
  }
  else if( dim_z == 0 )
  {
    dim = 2;
    cntr_size = dim_x * dim_y;
  }
  else
  {
    dim = 3;
    cntr_size = dim_x * dim_y * dim_z;
  }

  /*
   * Make sure we have enough HMC capacity
   *
   */
  switch(data_type[0])
  {
    case 'i':
    case 'I':
      stor_size = 0x04ll;
      if( (long)((long)capacity * (long)SLOTS_PER_GB) <
          (long)(cntr_size * 2 * 4) )
      {
        printf("ERROR: NOT ENOUGH AVAILABLE PHYSICAL STORAGE\n");
        return -1;
      }
      break;
    case 'd':
    case 'D':
      stor_size = 0x08ll;
      if( (long)((long)capacity * (long)SLOTS_PER_GB) <
          (long)(cntr_size * 2 * 8) )
      {
        printf("ERROR: NOT ENOUGH AVAILABLE PHYSICAL STORAGE\n");
        return -1;
      }
      break;
    default:
      printf("Error: Unknow data type\n");
      return -1;
      break;
  }

  /*
   * Get the shift amount based upon the block size
   *
   */
  if( getshiftamount( bsize, &shiftamt) != 0 ) {
    printf("ERROR: Failed to retrive shift amount\n");
    return -1;
  }

  /*
   * Caculate the base of each vector
   *
   */
  offset = (uint64_t)(cntr_size * (stor_size + 1));

  base_a = (0xAFll) << (uint64_t)(shiftamt);
  base_b = base_a + ( (offset) << (uint64_t)(shiftamt) );

  /* Allocate contiguous memory space for storing stencil grid addresses*/
  data_cntr_a = (uint64_t *) malloc( sizeof( uint64_t ) * cntr_size);
  data_cntr_b = (uint64_t *) malloc( sizeof( uint64_t ) * cntr_size);
  if( data_cntr_a == NULL || data_cntr_b == NULL)
  {
    printf("Error: Out of memory\n");
    return -1;
  }

  switch(dim)
  {
    case 1:
      grid_1d_a = data_cntr_a;
      grid_1d_b = data_cntr_b;
      break;
    case 2:
      grid_2d_a = (uint64_t **) malloc( sizeof(uint64_t *) * dim_x );
      grid_2d_b = (uint64_t **) malloc( sizeof(uint64_t *) * dim_x );
      if( grid_2d_a == NULL || grid_2d_b == NULL )
      {
        printf("Error: Out of memory\n");
        return -1;
      }
      for(i = 0; i < dim_x; i++)
      {
        idx = i * dim_y;
        grid_2d_a[i] = &data_cntr_a[idx];
        grid_2d_b[i] = &data_cntr_b[idx];
      }
      break;
    case 3:
      grid_3d_a = (uint64_t ***) malloc( sizeof(uint64_t **) * dim_x );
      grid_3d_b = (uint64_t ***) malloc( sizeof(uint64_t **) * dim_x );
      if( grid_3d_a == NULL ||  grid_3d_b == NULL )
      {
        printf("Error: Out of memory\n");
        return -1;
      }
      for(i = 0; i < dim_x; i++)
      {
        grid_3d_a[i] = (uint64_t **) malloc( sizeof(uint64_t *) * dim_y);
        grid_3d_b[i] = (uint64_t **) malloc( sizeof(uint64_t *) * dim_y);
        for(j = 0; j < dim_y; j++)
        {
          idx = j * dim_x + i * dim_x * dim_y;
          grid_3d_a[i][j] = &data_cntr_a[idx];
          grid_3d_b[i][j] = &data_cntr_b[idx];
        }
      }
      break;
    default:
      return -1;
      break;
  }
  /* End Allocation */

  /* Generate memory addresses, store them in data container */
  switch(dim)
  {
    case 1:
      for ( i = 0; i < dim_x; i++ )
      {
        grid_1d_a[i] = (uint64_t)( base_a + (((uint64_t)(i) * stor_size)
                                   << (uint64_t)(shiftamt)) );
        grid_1d_b[i] = (uint64_t)( base_b + (((uint64_t)(i) * stor_size)
                                   << (uint64_t)(shiftamt)) );
#ifdef DEBUG
        printf("%s%016" PRIX64 "\n", "A grid address: ", grid_1d_a[i]);
        printf("%s%016" PRIX64 "\n", "B grid address: ", grid_1d_b[i]);
#endif
      }
      break;
    case 2:
      for( i = 0; i < dim_x; i++)
      {
        for( j = 0; j < dim_y; j++)
        {
          grid_2d_a[i][j] = (uint64_t)( base_a + (((uint64_t)(j + i * dim_y)
                                        * stor_size) << (uint64_t)(shiftamt)) );
          grid_2d_b[i][j] = (uint64_t)( base_b + (((uint64_t)(j + i * dim_y)
                                        * stor_size) << (uint64_t)(shiftamt)) );
#ifdef DEBUG
          printf("%s%016" PRIX64 "\n", "A grid address: ", grid_2d_a[i][j]);
          printf("%s%016" PRIX64 "\n", "B grid address: ", grid_2d_b[i][j]);
#endif
        }
      }
      break;
    case 3:
      for( i = 0; i < dim_x; i++)
      {
        for( j = 0; j < dim_y; j++)
        {
          for( k = 0; k < dim_z; k++)
          {
            grid_3d_a[i][j][k] = (uint64_t)( base_a + (((uint64_t)(k + (j + i
                                             * dim_y) * dim_z) * stor_size)
                                             << (uint64_t)(shiftamt)) );
            grid_3d_b[i][j][k] = (uint64_t)( base_b + (((uint64_t)(k + (j + i
                                             * dim_y) * dim_z) * stor_size)
                                             << (uint64_t)(shiftamt)) );
#ifdef DEBUG
           printf("%s%016" PRIX64 "\n", "A grid address: ", grid_3d_a[i][j][k]);
           printf("%s%016" PRIX64 "\n", "B grid address: ", grid_3d_b[i][j][k]);
#endif
          }
        }
      }
      break;
    default:
      return -1;
      break;
  }
  /* End Allocation */

cleanup:
  /* Deallocate memory */
  switch(dim)
  {
    case 1:
      break;
    case 2:
      free(grid_2d_a);
      free(grid_2d_b);
      break;
    case 3:
      for(i = 0; i < dim_x; i++)
      {
        free(grid_3d_a[i]);
        free(grid_3d_b[i]);
      }
      free(grid_3d_a);
      free(grid_3d_b);
      break;
    default:
      printf("Error: Unknown dimension\n");
      return -1;
      break;
  }
  free(data_cntr_a);
  free(data_cntr_b);
  /* End Deallocation */

  return 0;
}
