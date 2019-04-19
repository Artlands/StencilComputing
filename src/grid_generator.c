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

/* Main Function. Takes command line arguments, generates stencil grid addresses*/
int main(int argc, char* argv[])
{
  /*vars*/
  int ret = 0;
  int i = 0;
  int j = 0;
  int k = 0;
  int idx;

  /* STENCIL GRID FEATURES */
  int dim_x = 0;
  int dim_y = 0;
  int dim_z = 0;
  int dim = 0;            // dimension of the grid
  int container_size = 0;
  int point_size = 0;     // integer - 4B, double - 8B
  char data_type[10];     // stencil data type


  /* MEMORY TRACE FILE*/
  FILE *outfile = NULL;
  char filename[1024];

  /* MALLOC MEMORY*/
  int *int_data;        // data container
  int *grid_1d_i;       // pointer container
  int **grid_2d_i;      // pointer container
  int ***grid_3d_i;     // pointer container
  double *double_data;  // data container
  double *grid_1d_d;    // pointer container
  double **grid_2d_d;   // pointer container
  double ***grid_3d_d;  // pointer container

  while( (ret = getopt( argc, argv, "x:y:z:t:f:h")) != -1 )
  {
    switch( ret )
    {
      case 'x':
        dim_x = (int)(atoi(optarg));
        break;
      case 'y':
        dim_y = (int)(atoi(optarg));
        break;
      case 'z':
        dim_z = (int)(atoi(optarg));
        break;
      case 't':
        sprintf(data_t, "%s", optarg);
        break;
      case 'f':
        sprintf( filename, "%s", optarg);
        break;
      case 'h':
        printf("%s%s%s\n", "usage : ", argv[0], " -xyztfh");
        printf(" -x <stencil grid size on dim x>\n");
        printf(" -y <stencil grid size on dim y>\n");
        printf(" -z <stencil grid size on dim z>\n");
        printf(" -t <stencil grid data type: Integer, Float, Double>\n");
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

  /* data_type = Integer */
  if( (strncmp(data_type, "i", 1) == 0) || (strncmp(data_type, "I", 1) == 0) )
  {
    point_size = 4;
  }
    /* data_type = Double floating */
  else if ( (strncmp(data_type, "d", 1) == 0) || (strncmp(data_type, "D", 1) == 0) )
  {
    point_size = 8;
  }
  else
  {
    printf("ERROR: Invalid data type\n");
    return -1;
  }

  if( strlen(filename) == 0) {
    printf("ERROR: File name is invalid\n");
    return -1;
  }
  /* ---- End Sanity Check ---- */

  /* Get Container Size */
  if( dim_y == 0 )
  {
    dim = 1;
    container_size = dim_x;
  }
  else if( dim_z == 0 )
  {
    dim = 2;
    container_size = dim_x * dim_y;
  }
  else
  {
    dim = 3;
    container_size = dim_x * dim_y * dim_z;
  }

  /* Allocate contiguous memory for stencil grid*/
  switch(data_type[0])
  {
    case 'i':
    case 'I':
      int_data = malloc( sizeof(int) * container_size );
      if( int_data == NULL)
      {
        printf("Error: Out of memory\n");
        return -1;
      }
      switch(dim)
      {
        case 1:
          grid_1d_i = int_data;
          break;
        case 2:
          grid_2d_i = malloc( sizeof(int *) * dim_x );
          if( grid_2d_i == NULL)
          {
            printf("Error: Out of memory\n");
            return -1;
          }
          for(i = 0; i < dim_x; i++){
            idx = i * dim_y;
            grid_2d_i[i] = &int_data[idx]
          }
        case 3:
          grid_3d_i = malloc( sizeof(int **) * dim_x );
          if( grid_3d_i == NULL )
          {
            printf("Error: Out of memory\n");
            return -1;
          }
          for(i = 0; i < dim_x; i++)
          {
            grid_3d_i[i] = (int **) malloc( sizeof(int *) * dim_y);
            for(j = 0; j < dim_y; j++)
            {
              idx = j * dim_x + i * dim_x * dim_y;
              grid_3d_i[i][j] = &int_data[idx];
            }
          }
          break;
        default:
          printf("Error\n");
          return -1;
          break;
      }
      break;
    case 'd':
    case 'D':
      double_data = malloc( sizeof(double) * container_size);
      break;
    default:
      printf("Error: Unknown data type\n");
      return -1;
      break;
  }

  /* Deallocate memory */
  switch(data_type[0])
  {
    case 'i':
    case 'I':
      free(int_data);
      break;
    case 'd':
    case 'D':
      break;
    default:
      printf("Error: Unknown data type\n");
      return -1;
      break;
  }

  return 0;
}
