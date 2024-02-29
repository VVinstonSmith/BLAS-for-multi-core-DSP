#include <compiler/m3000.h>
#include <compiler/vsip.h>
#include "hthread_device.h"
#include "sgemm_kernel.h"
// #include "../../mtblas_dev/libmtblas/include/mt_hthread_dev_blas.h"

#ifndef unlong 
#define unlong unsigned long
#endif

float alpha = 5.0;
float beta = 3.0;

__global__ void launch_sgemm_NN(unlong M, unlong N, unlong K, unlong b_id, float* A, float* B, float* C){
    sgemm_NN_qxx(M, N, K, b_id, A, B, C, alpha, beta);
    
    // mt_hthread_sgemm(M, N, K,\
    //                 alpha, A, K,\
    //                 B, N, beta, \
    //                 C, N);
}

__global__ void launch_sgemm_NT(unlong M, unlong N, unlong K, unlong b_id, float* A, float* B, float* C){
    sgemm_NT_qxx(M, N, K, b_id, A, B, C, alpha, beta);
}

__global__ void launch_sgemm_TN(unlong M, unlong N, unlong K, unlong bid_main, unlong bid_T, float* A, float* B, float* C){
    sgemm_TN_qxx(M, N, K, bid_main, bid_T, A, B, C, alpha, beta);
}

__global__ void launch_sgemm_TT(unlong M, unlong N, unlong K, unlong b_id, float* A, float* B, float* C){
    sgemm_TT_qxx(M, N, K, b_id, A, B, C, alpha, beta);
}

__global__ void  launch_sgemm_null(unlong M, unlong N, unlong K, unlong b_id, float* A, float* B, float* C){
    sgemm_null(M, N, K, b_id, A, B, C, alpha, beta);
}

