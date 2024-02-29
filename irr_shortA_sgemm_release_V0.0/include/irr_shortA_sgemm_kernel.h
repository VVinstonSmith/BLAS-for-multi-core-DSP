#ifndef irr_shortA_sgemm_KERNEL_H
#define irr_shortA_sgemm_KERNEL_H

#ifndef unlong 
#define unlong unsigned long
#endif

__global__ void  transpose_A2B(unlong M, unlong N, unlong b_id, float* A, float* B);
// transpose A(M x N) to B(N x M)

__global__ void irr_shortA_sgemm_NN(unlong M, unlong N, unlong K, unlong b_id, float* A, float* B, float* C);
// C += A x B
// bid: id of group_barrier

__global__ void irr_shortA_sgemm_NT(unlong M, unlong N, unlong K, unlong b_id, float* A, float* B, float* C);
// C += A x B^T

__global__ void irr_shortA_sgemm_TN(unlong M, unlong N, unlong K, unlong bid_main, unlong bid_T, float* A, float* B, float* C);
// C += A^T x B
// TN mode uses no less than 5 cores & 2 ids of group_barrier.

__global__ void irr_shortA_sgemm_TT(unlong M, unlong N, unlong K, unlong bid_main, unlong bid_T, float* A, float* B, float* C);
// C += A^T x B^T

__global__ void  irr_shortA_sgemm_null(unlong M, unlong N, unlong K, unlong b_id, float* A, float* B, float* C);
// do nothing

#endif
