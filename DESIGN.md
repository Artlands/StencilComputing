# Generate stencil grid address traces
* Dimension [int]: 1D, 2D, 3D
* Size_of_Dimension [int]
* Data_type: single floating[float], double floating[double]

# Build memory traces file of stencil computing
* Iteration_type: Jacobi Iteration, involve 2 same size grid
* Order [int]: (2* Order + 1) <= (Size of each Dimension)
* Boundary Condition: Periodic
  * i.e. the element before the first one is the last element in this dimension. the element after the last on is the first element in this dimension.
* Access_types:
  * (__TYPE_0__): the elements involved in one point computation is calculated based on the order size.
  * (__TYPE_1__)2D 9-points: B\[i][j] = A\[i-1][j] + A\[i][j] + A\[i+1][j] + A\[i][j-1] + A\[i][j+1] + A\[i-1][j-1] + A\[i+1][j-1] + A\[i-1][j+1] + A\[i+1][j+1]
  * (__TYPE_2__)3D 27-points: B\[i]\[j][k] = A\[i]\[j][k] + (A\[i-1]\[j][k] + A\[i+1]\[j][k] + A\[i]\[j-1][k] + A\[i]\[j+1][k] + A\[i]\[j][k-1] + A\[i]\[j][k+1]) + (A\[i-1]\[j][k-1] + A\[i+1]\[j][k-1] + A\[i]\[j-1][k-1] + A\[i]\[j+1][k-1] + A\[i-1]\[j-1][k] + A\[i-1]\[j+1][k] + A\[i+1]\[j-1][k] + A\[i+1]\[j+1][k] + A\[i-1]\[j][k+1] + A\[i+1]\[j][k+1] + A\[i]\[j-1][k+1] + A\[i]\[j+1][k+1]) + (A\[i-1]\[j-1][k-1] + A\[i-1]\[j+1][k-1] + A\[i+1]\[j-1][k-1] + A\[i+1]\[j+1][k-1] + A\[i-1]\[j-1][k+1] + A\[i-1]\[j+1][k+1] + A\[i+1]\[j-1][k+1] + A\[i+1]\[j+1][k+1])
Read stencil grid address traces and build memory traces based on the stencil type

# Cache simulation
* Cache_type
* Memory_trace_file
Read memory trace file; Calculate cache miss and cache hit; Generate memory request traces file

# Build PIMS memory traces file
Read stencil grid address traces, mark these traces as HOST_RD and HOST_WR; Build memory traces based on the stencil type, mark these traces as PIMS_RD

# Cache simulation on PIMS
Read PIMS memory traces file; Apply cache simulation on the PIMS_RD memory requests; Generate memory request traces file
