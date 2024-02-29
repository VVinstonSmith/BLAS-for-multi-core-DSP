#include <compiler/m3000.h>
#include <compiler/vsip.h>
#include "hthread_device.h"
#include "dgemm_kernel.h"

#ifndef unlong 
#define unlong unsigned long
#endif

__global__ void launch_dgemm_NN(unlong lda, unlong ldb, unlong ldc, unlong M, unlong N, unlong K, unlong b_id, double* A, double* B, double* C){
    dgemm_NN(lda, ldb, ldc, M, N, K, b_id, A, B, C);
}

__global__ void launch_dgemm_NT(unlong lda, unlong ldb, unlong ldc, unlong M, unlong N, unlong K, unlong b_id, double* A, double* B, double* C){
    dgemm_NT(lda, ldb, ldc, M, N, K, b_id, A, B, C);
}

__global__ void launch_dgemm_TN(unlong lda, unlong ldb, unlong ldc, unlong M, unlong N, unlong K, unlong bid_main, unlong bid_T, double* A, double* B, double* C){
    dgemm_TN(lda, ldb, ldc, M, N, K, bid_main, bid_T, A, B, C);
}

__global__ void launch_dgemm_TT(unlong lda, unlong ldb, unlong ldc, unlong M, unlong N, unlong K, unlong b_id, double* A, double* B, double* C){
    dgemm_TT(lda, ldb, ldc, M, N, K, b_id, A, B, C);
}

__global__ void  launch_dgemm_null(unlong lda, unlong ldb, unlong ldc, unlong M, unlong N, unlong K, unlong b_id, double* A, double* B, double* C){
    dgemm_null(lda, ldb, ldc, M, N, K, b_id, A, B, C);
}

