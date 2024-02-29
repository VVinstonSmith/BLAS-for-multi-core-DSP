#ifndef DGEMM_KERNEL_H
#define DGEMM_KERNEL_H

#ifndef unlong 
#define unlong unsigned long
#endif

__global__ void  transpose_A2B(unlong M, unlong N, unlong b_id, double* A, double* B);
// transpose A(M x N) to B(N x M)

__global__ void dgemm_NN(unlong M, unlong N, unlong K, unlong b_id, double* A, double* B, double* C);
// C += A x B
// bid: id of group_barrier

__global__ void dgemm_NT(unlong M, unlong N, unlong K, unlong b_id, double* A, double* B, double* C);
// C += A x B^T

__global__ void dgemm_TN(unlong M, unlong N, unlong K, unlong bid_main, unlong bid_T, double* A, double* B, double* C);
// C += A^T x B
// TN mode uses no less than 5 cores & 2 ids of group_barrier.

__global__ void dgemm_TT(unlong M, unlong N, unlong K, unlong b_id, double* A, double* B, double* C);
// C += A^T x B^T

__global__ void  dgemm_null(unlong M, unlong N, unlong K, unlong b_id, double* A, double* B, double* C);
// do nothing

#endif
