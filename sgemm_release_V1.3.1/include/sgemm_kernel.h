#ifndef SGEMM_KERNEL_H
#define SGEMM_KERNEL_H

#ifndef unlong 
#define unlong unsigned long
#endif

__global__ void  transpose_A2B_float32(unlong M, unlong N, unlong b_id, float* A, float* B);
// transpose A(M x N) to B(N x M)

__global__ void sgemm_NN_qxx(unlong M, unlong N, unlong K, unlong b_id, float* A, float* B, float* C, float alpha, float beta);
// C += A x B
// bid: id of group_barrier

__global__ void sgemm_NT_qxx(unlong M, unlong N, unlong K, unlong b_id, float* A, float* B, float* C, float alpha, float beta);
// C += A x B^T

__global__ void sgemm_TN_qxx(unlong M, unlong N, unlong K, unlong bid_main, unlong bid_T, float* A, float* B, float* C, float alpha, float beta);
// C += A^T x B
// TN mode uses no less than 5 cores & 2 ids of group_barrier.

__global__ void sgemm_TT_qxx(unlong M, unlong N, unlong K, unlong b_id, float* A, float* B, float* C, float alpha, float beta);
// C += A^T x B^T

__global__ void  sgemm_null(unlong M, unlong N, unlong K, unlong b_id, float* A, float* B, float* C, float alpha, float beta);
// do nothing

#endif
