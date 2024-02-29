#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "time.h"
#include <sys/time.h>
#include <omp.h>
#include <hthread_host.h>

#define OFFSET(row, col, ld) ((row) * (ld) + (col))

#define NUM_THREADS 16
#define NUM_ITER 50

#define CHECK_DATA

/**
  timer - gettimeofday()
 **/
double timer(){
	long t_val = 0;
	struct timeval t;
	gettimeofday(&t,NULL);
	t_val = t.tv_sec * 1e+6 + t.tv_usec;
	return (double)t_val;
}

void switch_pointer(double* ptr_A, double* ptr_B){
	double* ptr_tmp;
	ptr_tmp = ptr_A;
	ptr_A = ptr_B;
	ptr_B = ptr_tmp;
}

void reference_stencil(const int M, const int N, double *A, double *B){
	double top, left, right, down, middle;
	#pragma omp parallel for num_threads(NUM_THREADS) collapse(2)
	for(int row=0; row<M; row++){
		for(int col=0; col<N; col++){
			top = A[(row) * (N+2) + col + 1];
			left = A[(row+1)* (N+2) + col];
			right = A[(row+1)* (N+2) + col + 2];
			down = A[(row+2)* (N+2) + col + 1];
			middle = A[(row+1) * (N+2) + col + 1];
			B[row*N+col] = (top + left + right + down +middle) * 0.2;
		}
	}
}

int main(int argc, char **argv){

	if(argc < 4){
		printf("Useage: ~ N Num_cores cluster_id\n");
	}

	long M = atol(argv[1]);
	long N = atol(argv[2]);
	int num_threads = atoi(argv[3]);
	int cluster_id = atoi(argv[4]);

	long size_A = (M+2) * (N+2) * sizeof(double);

	hthread_dev_open(cluster_id);
	hthread_dat_load(cluster_id, "kernel_stencil.dat");

	double *A = hthread_malloc(cluster_id, size_A, HT_MEM_WO);
	double *B = hthread_malloc(cluster_id, size_A, HT_MEM_WO);
	double *A_cpu = (double *)malloc(size_A);
	double *B_cpu = (double *)malloc(size_A);

	printf("data preparing~\n");
	memset(A, 0, size_A);
	memset(B, 0, size_A);
	memset(A_cpu, 0, size_A);
	memset(B_cpu, 0, size_A);
	#pragma omp parallel for num_threads(NUM_THREADS) collapse(2)
	for(long i=1; i<M+1; i++){
		for(long j=1; j<N+1; j++){
			// A_cpu[OFFSET(i, j, N+2)] = A[OFFSET(i, j, N+2)] = 5;
			// A_cpu[OFFSET(i, j, N+2)] = A[OFFSET(i, j, N+2)] = OFFSET(i, j, N+2) % 13;
			A_cpu[OFFSET(i, j, N+2)] = A[OFFSET(i, j, N+2)] = 1.0 * rand()/RAND_MAX;
		}
	}

	unsigned long args[4];
	args[0] = M;
	args[1] = N;
	args[2] = (unsigned long)A;
	args[3] = (unsigned long)B;

	int tg_id = hthread_group_create(cluster_id, num_threads);
	hthread_group_wait(tg_id);	

	printf("DSP computing~\n");
	double t_start, t_end, t_ov;

	t_start = timer();
	for(int it=0; it<NUM_ITER; it++){
		hthread_group_exec(tg_id, "stencil_null", 2, 2, args);
		hthread_group_wait(tg_id);
	}
	t_end = timer();
	t_ov = t_end - t_start;

	t_start = timer();
	for(int it=0; it<NUM_ITER; it++){
		// hthread_group_exec(tg_id, "stencil_scalar", 2, 2, args);
		hthread_group_exec(tg_id, "stencil_vector", 2, 2, args);
		switch_pointer(A, B);
		hthread_group_wait(tg_id);
	}
	t_end = timer();

	printf("stencil 2d elapsed time: %lf seconds(%lf seconds per iteration), size per array: %lf GB, Perf %lf grids/msec\n", \
			(t_end - t_start - t_ov)*1e-6, \
			(t_end - t_start - t_ov)/NUM_ITER*1e-6, size_A/(1024.0 * 1024.0 * 1024.0), \
			(double)(M * N) / ((t_end - t_start - t_ov)/NUM_ITER*1e-3) );

#ifdef CHECK_DATA
	printf("CPU computing~\n");
	for(int it=0; it<NUM_ITER; it++){
		reference_stencil(M, N, A_cpu, B_cpu);
		switch_pointer(A_cpu, B_cpu);
	}
#endif

#ifdef CHECK_DATA
	int error_num = 0;
	#pragma omp parallel for num_threads(NUM_THREADS) collapse(1)
	for(long i = 0; i<M*N; i++){
		// printf("[%ld] : (compute) %f ?= %f (reference)\n", i, A[i], A_cpu[i]);
		// printf("[%ld] : %f \n", i, A[i]);
		if(fabs(A[i] - A_cpu[i]) > 1e-8){
			error_num++;
			// printf("[%ld] : (compute) %f != %f (reference)\n", i, A[i], A_cpu[i]);
		}
	}
	if(error_num > 0){
		printf("Result Error!\n");
		printf("error_num: %d\n", error_num);
	}else{
		printf("Success!\n");
	}
#endif

	int idx_0 = OFFSET(1, 1, N+2);
	int idx_1 = OFFSET(1, 1, N+2) + 1;
	int idx_2 = OFFSET(M, N, N+2) - 1;
	int idx_3 = OFFSET(M, N, N+2);
	printf("A[%d] = %f,  A_cpu[%d] = %f\n", idx_0, A[idx_0], idx_0, A_cpu[idx_0]);
	printf("A[%d] = %f,  A_cpu[%d] = %f\n", idx_1, A[idx_1], idx_1, A_cpu[idx_1]);
	printf("A[%d] = %f,  A_cpu[%d] = %f\n", idx_2, A[idx_2], idx_2, A_cpu[idx_2]);
	printf("A[%d] = %f,  A_cpu[%d] = %f\n", idx_3, A[idx_3], idx_3, A_cpu[idx_3]);

	hthread_group_destroy(tg_id);
	hthread_dev_close(cluster_id);

	hthread_free(A);
	hthread_free(B);
	free(A_cpu);
	free(B_cpu);

	return 0;
}
