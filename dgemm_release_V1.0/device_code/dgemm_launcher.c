#include <compiler/m3000.h>
#include <compiler/vsip.h>
#include "hthread_device.h"
#include "dgemm_kernel.h"

#ifndef unlong 
#define unlong unsigned long
#endif

__global__ void launch_dgemm_NN(unlong M, unlong N, unlong K, unlong b_id, double* A, double* B, double* C){
    dgemm_NN(M, N, K, b_id, A, B, C);
}

__global__ void launch_dgemm_NT(unlong M, unlong N, unlong K, unlong b_id, double* A, double* B, double* C){
    dgemm_NT(M, N, K, b_id, A, B, C);
}

__global__ void launch_dgemm_TN(unlong M, unlong N, unlong K, unlong bid_main, unlong bid_T, double* A, double* B, double* C){
    dgemm_TN(M, N, K, bid_main, bid_T, A, B, C);
}

__global__ void launch_dgemm_TT(unlong M, unlong N, unlong K, unlong b_id, double* A, double* B, double* C){
    dgemm_TT(M, N, K, b_id, A, B, C);
}

__global__ void  launch_dgemm_null(unlong M, unlong N, unlong K, unlong b_id, double* A, double* B, double* C){
    dgemm_null(M, N, K, b_id, A, B, C);
}

