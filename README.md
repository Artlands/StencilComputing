## Grid_generator.c [DONE]
### (1) Generate stencil grid address traces
* Dimension: 1D, 2D, 3D
* Size_of_Dimension: [int]
* Data_type: single floating[float], double floating[double]

### (2) Build memory traces file of stencil computing
###### Read stencil grid address traces and build memory traces based on the stencil type
* Iteration_type: Jacobi Iteration, involve 2 same size grid
* Order: [int] (Computing elements in each dimension: 2 * Order + 1)
* Boundary Condition: ghost cells storing boundary conditions
  * i.e. the element before the first one is the last element in this dimension. the element after the last on is the first element in this dimension.
* Access_types:
  * (__TYPE_0__): Order varies, the elements involved in one point computation is calculated based on the order size.
  * (__TYPE_1__): Order = 1, 2D 9-points: B\[i][j] = A\[i-1][j] + A\[i][j] + A\[i+1][j] + A\[i][j-1] + A\[i][j+1] + A\[i-1][j-1] + A\[i+1][j-1] + A\[i-1][j+1] + A\[i+1][j+1]
  * (__TYPE_2__): Order = 1, 3D 27-points: B\[i]\[j][k] = A\[i]\[j][k] + (A\[i-1]\[j][k] + A\[i+1]\[j][k] + A\[i]\[j-1][k] + A\[i]\[j+1][k] + A\[i]\[j][k-1] + A\[i]\[j][k+1]) + (A\[i-1]\[j][k-1] + A\[i+1]\[j][k-1] + A\[i]\[j-1][k-1] + A\[i]\[j+1][k-1] + A\[i-1]\[j-1][k] + A\[i-1]\[j+1][k] + A\[i+1]\[j-1][k] + A\[i+1]\[j+1][k] + A\[i-1]\[j][k+1] + A\[i+1]\[j][k+1] + A\[i]\[j-1][k+1] + A\[i]\[j+1][k+1]) + (A\[i-1]\[j-1][k-1] + A\[i-1]\[j+1][k-1] + A\[i+1]\[j-1][k-1] + A\[i+1]\[j+1][k-1] + A\[i-1]\[j-1][k+1] + A\[i-1]\[j+1][k+1] + A\[i+1]\[j-1][k+1] + A\[i+1]\[j+1][k+1])

### (3) Build PIMS memory traces file
###### Read stencil grid address traces, mark these traces as HOST_RD and HOST_WR; Build memory traces based on the stencil type, mark these traces as PIMS_RD

## Mmu_simulator.c [DONE]
###### Read memory traces and translate into physical memory traces

## Host_cacheSim.c  [DONE]
###### Read memory trace file; Calculate cache miss and cache hit; Generate memory request traces file

## PIMS_cacheSim.c [TODO]
###### Read PIMS memory traces file; Apply cache simulation on the PIMS_RD memory requests; Generate memory request traces file

## HMC-SIM [TODO]
###### Implement CMC, Modify HMC-SIM, Simulate with trace files
