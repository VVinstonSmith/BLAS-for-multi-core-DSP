#include <compiler/m3000.h>
#include <compiler/vsip.h>
#include "hthread_device.h"
#include "irr_shortA_sgemm_kernel.h"

#ifndef unlong 
#define unlong unsigned long
#endif

__global__ void launch_irr_shortA_sgemm_NN(unlong M, unlong N, unlong K, unlong b_id, float* A, float* B, float* C){
    irr_shortA_sgemm_NN(M, N, K, b_id, A, B, C);
}

__global__ void launch_irr_shortA_sgemm_NT(unlong M, unlong N, unlong K, unlong b_id, float* A, float* B, float* C){
    irr_shortA_sgemm_NT(M, N, K, b_id, A, B, C);
}

__global__ void launch_irr_shortA_sgemm_TN(unlong M, unlong N, unlong K, unlong bid_main, unlong bid_T, float* A, float* B, float* C){
    irr_shortA_sgemm_TN(M, N, K, bid_main, bid_T, A, B, C);
}

__global__ void launch_irr_shortA_sgemm_TT(unlong M, unlong N, unlong K, unlong bid_main, unlong bid_T, float* A, float* B, float* C){
    irr_shortA_sgemm_TT(M, N, K, bid_main, bid_T, A, B, C);
}

__global__ void  launch_irr_shortA_sgemm_null(unlong M, unlong N, unlong K, unlong b_id, float* A, float* B, float* C){
    irr_shortA_sgemm_null(M, N, K, b_id, A, B, C);
}

