/*
 * Using Intel Architecture Code Analyzer to
 * analyze the throughput and latency of a loop body
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include "iacaMarks.h"

#define DIM 32
#define ORDER 8

int main(int argc, char* argv[])
{
  int i = 0;
  int j = 0;
  int k = 0;
  int n = 0;
  int dim_x = DIM;
  int dim_y = DIM;
  int dim_z = DIM;
  int tdim_x;
  int tdim_y;
  int tdim_z;
  int sten_order = ORDER;
  int iteration = 2;

  double random_value;

  double *tmp_point;
  double *data_cntr_a;             // data container
  double *data_cntr_b;             // data container
  double *data_cntr_c;             // data container
  double *v_pointer;
  double *u_pointer;
  double *w_pointer;


  uint32_t cntr_size;

  srand( time(NULL));

  /*Get total grid size*/
  tdim_x = dim_x + sten_order;
  tdim_y = dim_y + sten_order;
  tdim_z = dim_z + sten_order;

  /* Get Container Size */
  cntr_size = (uint32_t)((uint32_t)(tdim_x)
                       * (uint32_t)(tdim_y)
                       * (uint32_t)(tdim_z));


  /* Allocate contiguous memory space for storing stencil grid addresses and coefficients */
  data_cntr_a = (double *) malloc( sizeof( double ) * cntr_size);
  data_cntr_b = (double *) malloc( sizeof( double ) * cntr_size);
  data_cntr_c = (double *) malloc( sizeof( double ) * cntr_size);

  /* Init data container */
  for( i = 0; i < cntr_size; i ++)
  {
    random_value = (double)rand()/RAND_MAX;
    data_cntr_a[i] = random_value;
    random_value = (double)rand()/RAND_MAX;
    data_cntr_b[i] = random_value;
    random_value = (double)rand()/RAND_MAX;
    data_cntr_c[i] = random_value;
    // printf("%f\n", data_cntr_a[i]);
    // printf("%f\n", data_cntr_b[i]);
  }

  /* Stencil Computation */
  for( n = 0; n < iteration; n++ )
  {
    for( k = sten_order/2; k < (dim_y+sten_order/2); k++ )
    {
      for( j = sten_order/2; j < (dim_y+sten_order/2); j++ )
      {
        v_pointer = &data_cntr_a[j*tdim_x + k * tdim_x * tdim_y];
        u_pointer = &data_cntr_b[j*tdim_x + k * tdim_x * tdim_y];
        w_pointer = &data_cntr_c[j*tdim_x + k * tdim_x * tdim_y];
        for( i = sten_order/2; i < (dim_x+sten_order/2); i++ )
        {
          IACA_START
          u_pointer[i] = (double)(0.1) * v_pointer[i]
                       + (double)(0.2) * (v_pointer[i + 1] + v_pointer[i - 1]
                                        + v_pointer[i + tdim_x]
                                        + v_pointer[i - tdim_x]
                                        + v_pointer[i + tdim_x*tdim_y]
                                        + v_pointer[i - tdim_x*tdim_y])
                       + (double)(0.3) * (v_pointer[i + 2] + v_pointer[i - 2]
                                        + v_pointer[i + 2 * tdim_x]
                                        + v_pointer[i - 2 * tdim_x]
                                        + v_pointer[i + 2 * tdim_x*tdim_y]
                                        + v_pointer[i - 2 * tdim_x*tdim_y])
                       + (double)(0.3) * (v_pointer[i + 3] + v_pointer[i - 3]
                                        + v_pointer[i + 3 * tdim_x]
                                        + v_pointer[i - 3 * tdim_x]
                                        + v_pointer[i + 3 * tdim_x*tdim_y]
                                        + v_pointer[i - 3 * tdim_x*tdim_y])
                       + (double)(0.4) * (v_pointer[i + 4] + v_pointer[i - 4]
                                        + v_pointer[i + 4 * tdim_x]
                                        + v_pointer[i - 4 * tdim_x]
                                        + v_pointer[i + 4 * tdim_x*tdim_y]
                                        + v_pointer[i - 4 * tdim_x*tdim_y]);
          // u_pointer[i] = (double)(0.1) * v_pointer[i]
          //              + (double)(0.2) * (w_pointer[i + 1])
          //              + (double)(0.3) * (w_pointer[i + 2])
          //              + (double)(0.3) * (w_pointer[i + 3])
          //              + (double)(0.4) * (w_pointer[i + 4]);
        }
      }
    }
    /*switch container pointer*/
    tmp_point = data_cntr_a;
    data_cntr_a = data_cntr_b;
    data_cntr_b = tmp_point;
  }
  IACA_END

  return 0;

}
