#include <compiler/m3000.h>
#include <compiler/vsip.h>
#include "hthread_device.h"
#include "irr_thinB_dgemm_kernel.h"

#ifndef unlong 
#define unlong unsigned long
#endif

__global__ void launch_irr_thinB_dgemm_TT(unlong M, unlong N, unlong K, unlong b_id, double* A, double* B, double* C){
    irr_thinB_dgemm_TT_dbuf(M, N, K, b_id, A, B, C);
}

__global__ void launch_irr_thinB_dgemm_NT(unlong M, unlong N, unlong K, unlong b_id, double* A, double* B, double* C){
    irr_thinB_dgemm_NT_dbuf(M, N, K, b_id, A, B, C);
}

__global__ void launch_irr_thinB_dgemm_TN(unlong M, unlong N, unlong K, unlong bid_main, unlong bid_T, double* A, double* B, double* C){
    irr_thinB_dgemm_TN_dbuf(M, N, K, bid_main, bid_T, A, B, C);
}

__global__ void launch_irr_thinB_dgemm_NN(unlong M, unlong N, unlong K, unlong bid_main, unlong bid_T, double* A, double* B, double* C){
    irr_thinB_dgemm_NN_dbuf(M, N, K, bid_main, bid_T, A, B, C);
}

__global__ void  launch_irr_thinB_dgemm_null(unlong M, unlong N, unlong K, unlong b_id, double* A, double* B, double* C){
    irr_thinB_dgemm_null(M, N, K, b_id, A, B, C);
}

