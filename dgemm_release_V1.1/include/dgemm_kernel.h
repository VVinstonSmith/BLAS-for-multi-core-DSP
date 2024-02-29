#ifndef DGEMM_KERNEL_H
#define DGEMM_KERNEL_H

#ifndef unlong 
#define unlong unsigned long
#endif

__global__ void  transpose_A2B(unlong lda, unlong ldb, unlong M, unlong N, unlong b_id, double* A, double* B);
// transpose A(M x N) to B(N x M)

__global__ void dgemm_NN(unlong lda, unlong ldb, unlong ldc, unlong M, unlong N, unlong K, unlong b_id, double* A, double* B, double* C);
// C += A x B
// bid: id of group_barrier
// A0(Mxlda), A(MxK)
// B0(Kxldb), B(KxN)
// C0(Mxldc), C(MxN)

__global__ void dgemm_NT(unlong lda, unlong ldb, unlong ldc, unlong M, unlong N, unlong K, unlong b_id, double* A, double* B, double* C);
// C += A x B^T
// bid: id of group_barrier
// A0(Mxlda), A(MxK)
// B0(Nxldb), B(NxK)
// C0(Mxldc), C(MxN)

__global__ void dgemm_TN(unlong lda, unlong ldb, unlong ldc, unlong M, unlong N, unlong K, unlong bid_main, unlong bid_T, double* A, double* B, double* C);
// C += A^T x B
// TN mode uses no less than 5 cores & 2 ids of group_barrier.
// A0(Kxlda), A(KxM)
// B0(Kxldb), B(KxN)
// C0(Mxldc), C(MxN)

__global__ void dgemm_TT(unlong lda, unlong ldb, unlong ldc, unlong M, unlong N, unlong K, unlong b_id, double* A, double* B, double* C);
// C += A^T x B^T
// bid: id of group_barrier
// A0(Kxlda), A(KxM)
// B0(Nxldb), B(NxK)
// C0(Mxldc), C(MxN)

__global__ void  dgemm_null(unlong lda, unlong ldb, unlong ldc, unlong M, unlong N, unlong K, unlong b_id, double* A, double* B, double* C);
// do nothing

#endif
