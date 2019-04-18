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
  /* STENCIL GRID FEATURES */
  int dim_x = 0;
  int dim_y = 0;
  int dim_z = 0;
  char data_t[10]; //stencil data type

  /* MEMORY TRACE FILE*/
  FILE* outfile = NULL;
  char filename[1024];

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
  
}
