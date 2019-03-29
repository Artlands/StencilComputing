# StencilComputing-Sim
MEMORY TRACES SIMULATION  FOR STENCIL COMPUTING

* Simulate the memory traces of stencil computing
* Iteration type : Jacobi Iteration, involve 2 same size arrays: A, B
* Dimension:1D, 2D, 3D
* Order: 1
* Boundary Condition: Periodic,
  * i.e. the element before the first one is the last element in this dimension. the element after the last on is the first element in this dimension.
* Access types(ignore coefficient):
  * (__TYPE 1__)1D 3-points: B[i] = A[i-1] + A[i] + A[i+1]
  * (__TYPE 2__)1D 5-points: B[i] = A[i-2] + A[i-1] + A[i] + A[i+1] + A[i+2]
  * (__TYPE 3__)2D 5-points: B\[i][j] = A\[i-1][j] + A\[i][j] + A\[i+1][j] + A\[i][j-1] + A\[i][j+1]
  * (__TYPE 4__)2D 9-points: B\[i][j] = A\[i-1][j] + A\[i][j] + A\[i+1][j] + A\[i][j-1] + A\[i][j+1] + A\[i-1][j-1] + A\[i+1][j-1] + A\[i+1][j-1] + A\[i+1][j-+]
  * (__TYPE 5__)3D 7-points: B\[i]\[j][k] = A\[i-1]\[j][k] + A\[i]\[j][k] + A\[i+1]\[j][k] + A\[i]\[j-1][k] + A\[i]\[j+1][k] + A\[i]\[j][k-1] + A\[i]\[j][k+1]
  * (__TYPE 6__)3D 27-points: B\[i]\[j][k] = A\[i]\[j][k] + (A\[i-1]\[j][k] + A\[i+1]\[j][k] + A\[i]\[j-1][k] + A\[i]\[j+1][k] + A\[i]\[j][k-1] + A\[i]\[j][k+1]) + (A\[i-1]\[j][k-1] + A\[i+1]\[j][k-1] + A\[i]\[j-1][k-1] + A\[i]\[j+1][k-1] + A\[i-1]\[j-1][k] + A\[i-1]\[j+1][k] + A\[i+1]\[j-1][k] + A\[i+1]\[j+1][k] + A\[i-1]\[j][k+1] + A\[i+1]\[j][k+1] + A\[i]\[j-1][k+1] + A\[i]\[j+1][k+1]) + (A\[i-1]\[j-1][k-1] + A\[i-1]\[j+1][k-1] + A\[i+1]\[j-1][k-1] + A\[i+1]\[j+1][k-1] + A\[i-1]\[j-1][k+1] + A\[i-1]\[j+1][k+1] + A\[i+1]\[j-1][k+1] + A\[i+1]\[j+1][k+1])
