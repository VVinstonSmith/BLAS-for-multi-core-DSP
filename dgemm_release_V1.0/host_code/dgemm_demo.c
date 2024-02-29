#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "time.h"
#include <math.h>
#include <sys/time.h>
#include <omp.h>
#include "hthread_host.h"

#define OFFSET(row, col, ld) ((row) * (ld) + (col))

#define NUM_TESTS 1
#define NUM_THREADS 16
#define dataType double
#define CHECK_DATA

/** timer - gettimeofday() **/
double timer(){
	long t_val = 0;
	struct timeval t;
	gettimeofday(&t,NULL);
	t_val = t.tv_sec * 1e+6 + t.tv_usec;
	return (double)t_val;
}

int main(int argc, char **argv){
	const int K_G = 512;
	const int n_cores_for_transpose = 4;
	if (argc < 4) {
		printf("Useage: ~ dev_id{0, 1, 2, 3} side_elements num_threads{1...24}\n");
		return -1;
	}
	int dev_id= atoi(argv[1]);
	long M = atol(argv[2]);
	long K = atol(argv[3]);
	long N = atol(argv[4]);
	int num_threads = atoi(argv[5]);
	printf("M=%ld, K=%ld, N=%ld\n", M, K, N);

	long len_A = M * K;
	long len_B = N * K;
	long len_C = M * N;
	long size_A = len_A * sizeof(dataType);
	long size_B = len_B * sizeof(dataType);
	long size_C = len_C * sizeof(dataType);
	
	hthread_dev_open(dev_id);
	hthread_dat_load(dev_id, "dgemm_launcher.dat");
	
	dataType *A = hthread_malloc(dev_id, size_A, HT_MEM_RO);
	dataType *A_T = hthread_malloc(dev_id, size_A, HT_MEM_RO);
	dataType *B = hthread_malloc(dev_id, size_B, HT_MEM_RO);
	dataType *B_T = hthread_malloc(dev_id, size_B, HT_MEM_RO);
	dataType *C_NN = hthread_malloc(dev_id, size_C, HT_MEM_WO);
	dataType *C_NT = hthread_malloc(dev_id, size_C, HT_MEM_WO);
	dataType *C_TN = hthread_malloc(dev_id, size_C, HT_MEM_WO);
	dataType *C_TT = hthread_malloc(dev_id, size_C, HT_MEM_WO);
	dataType *C_cpu = (dataType *)malloc(size_C);

	srand(time(0));
	printf("preparing data~\n");
	printf("------------------------------------------------------------------------------------------\n");
	#pragma omp parallel for num_threads(NUM_THREADS) collapse(2)
	for(int i=0; i<M; i++){
		for(int j=0; j<K; j++){
			// A[OFFSET(i, j, K)] = A_T[OFFSET(j, i, M)] = (double)(rand()) / RAND_MAX * (-4.0 + 6.0) + 2.0;
			A[OFFSET(i, j, K)] = A_T[OFFSET(j, i, M)] = OFFSET(i, j, K) % 11;
		}
	}

	#pragma omp parallel for num_threads(NUM_THREADS) collapse(2)
	for(int i=0; i<K; i++){
		for(int j=0; j<N; j++){
			// B[OFFSET(i, j, N)] = B_T[OFFSET(j, i, K)] = (double)(rand()) / RAND_MAX * (-4.0 + 6.0) + 2.0;
			B[OFFSET(i, j, N)] = B_T[OFFSET(j, i, K)] = OFFSET(i, j, N) % 7;
		}
	}

	#pragma omp parallel for num_threads(NUM_THREADS) collapse(1)
    for(int i=0; i<M*N; i++){
        C_cpu[i] = C_NN[i] = C_NT[i] = C_TN[i] = C_TT[i] = 0.0;
    }

	/*申请设备端barrier同步单元*/
	int bid_main = hthread_barrier_create(dev_id);
	int bid_T = hthread_barrier_create(dev_id);
	printf("bid_main = %d\n", bid_main);
	printf("bid_T = %d\n", bid_T);

	unsigned long args_GEMM_NN[7];
	args_GEMM_NN[0] = M; args_GEMM_NN[1] = N; args_GEMM_NN[2] = K;
	args_GEMM_NN[3] = bid_main;
	args_GEMM_NN[4] = (unsigned long)A;
	args_GEMM_NN[5] = (unsigned long)B;
	args_GEMM_NN[6] = (unsigned long)C_NN;

	unsigned long args_GEMM_NT[7];
	args_GEMM_NT[0] = M; args_GEMM_NT[1] = N; args_GEMM_NT[2] = K;
	args_GEMM_NT[3] = bid_main;
	args_GEMM_NT[4] = (unsigned long)A;
	args_GEMM_NT[5] = (unsigned long)B_T;
	args_GEMM_NT[6] = (unsigned long)C_NT;

	unsigned long args_GEMM_TN[8];
	args_GEMM_TN[0] = M; args_GEMM_TN[1] = N; args_GEMM_TN[2] = K;
	args_GEMM_TN[3] = bid_main; args_GEMM_TN[4] = bid_T; 
	args_GEMM_TN[5] = (unsigned long)A_T;
	args_GEMM_TN[6] = (unsigned long)B;
	args_GEMM_TN[7] = (unsigned long)C_TN;

	unsigned long args_GEMM_TT[7];
	args_GEMM_TT[0] = M; args_GEMM_TT[1] = N; args_GEMM_TT[2] = K;
	args_GEMM_TT[3] = bid_main;
	args_GEMM_TT[4] = (unsigned long)A_T;
	args_GEMM_TT[5] = (unsigned long)B_T;
	args_GEMM_TT[6] = (unsigned long)C_TT;
	
	//--------------------------------------------------------------------------------------
	/*在指定的DSP簇上创建空闲线程组*/
	int tg_id = hthread_group_create(dev_id, num_threads);
	hthread_group_wait(tg_id);

	double t_start, t_end, t_ov;

	t_start = timer();
	for(int i=0; i<NUM_TESTS; i++){
		hthread_group_exec(tg_id, "launch_dgemm_null", 4, 3, args_GEMM_NN);
		hthread_group_wait(tg_id);
	}
	t_end = timer();
	t_ov = t_end - t_start;

	// dgemm_NN
	t_start = timer();
	for(int i=0; i<NUM_TESTS; i++){
        hthread_group_exec(tg_id, "launch_dgemm_NN", 4, 3, args_GEMM_NN);
		hthread_group_wait(tg_id);
	}
	t_end = timer();
	printf("GEMM_NN elapsed time: %lf seconds, size per array: %lf GB, Perf %lf Gflops (%lf Gflops per core)\n", \
			(t_end - t_start - t_ov)/NUM_TESTS*1e-6, size_C/(1024.0 * 1024.0 * 1024.0), \
			(double)(M * N * K * 2.0) / (((t_end - t_start - t_ov)/NUM_TESTS)*1e3), \
			(double)(M * N * K * 2.0) / (((t_end - t_start - t_ov)/NUM_TESTS)*1e3) / num_threads);

	// dgemm_NT
	t_start = timer();
	for(int i=0; i<NUM_TESTS; i++){
        hthread_group_exec(tg_id, "launch_dgemm_NT", 4, 3, args_GEMM_NT);
		hthread_group_wait(tg_id);
	}
	t_end = timer();
	printf("GEMM_NT elapsed time: %lf seconds, size per array: %lf GB, Perf %lf Gflops (%lf Gflops per core)\n", \
			(t_end - t_start - t_ov)/NUM_TESTS*1e-6, size_C/(1024.0 * 1024.0 * 1024.0), \
			(double)(M * N * K * 2.0) / (((t_end - t_start - t_ov)/NUM_TESTS)*1e3), \
			(double)(M * N * K * 2.0) / (((t_end - t_start - t_ov)/NUM_TESTS)*1e3) / num_threads);

	// dgemm_TT
	t_start = timer();
	for(int i=0; i<NUM_TESTS; i++){
        hthread_group_exec(tg_id, "launch_dgemm_TT", 4, 3, args_GEMM_TT);
		hthread_group_wait(tg_id);
	}
	t_end = timer();
	printf("GEMM_TT elapsed time: %lf seconds, size per array: %lf GB, Perf %lf Gflops (%lf Gflops per core)\n", \
			(t_end - t_start - t_ov)/NUM_TESTS*1e-6, size_C/(1024.0 * 1024.0 * 1024.0), \
			(double)(M * N * K * 2.0) / (((t_end - t_start - t_ov)/NUM_TESTS)*1e3), \
			(double)(M * N * K * 2.0) / (((t_end - t_start - t_ov)/NUM_TESTS)*1e3) / num_threads);
	
	hthread_group_destroy(tg_id);
	//--------------------------------------------------------------------------------------
	/*在指定的DSP簇上创建空闲线程组*/
	tg_id = hthread_group_create(dev_id, num_threads + n_cores_for_transpose);
	hthread_group_wait(tg_id);

	// dgemm_TN
	t_start = timer();
	for(int i=0; i<NUM_TESTS; i++){
		hthread_group_exec(tg_id, "launch_dgemm_TN", 5, 3, args_GEMM_TN);
		hthread_group_wait(tg_id);
	}
	t_end = timer();
	printf("GEMM_TN elapsed time: %lf seconds, size per array: %lf GB, Perf %lf Gflops (%lf Gflops per core)\n", \
			(t_end - t_start - t_ov)/NUM_TESTS*1e-6, size_C/(1024.0 * 1024.0 * 1024.0), \
			(double)(M * N * K * 2.0) / (((t_end - t_start - t_ov)/NUM_TESTS)*1e3), \
			(double)(M * N * K * 2.0) / (((t_end - t_start - t_ov)/NUM_TESTS)*1e3) / num_threads);

	//--------------------------------------------------------------------------------------
	double Sum_C_NN = 0.0, Sum_C_NT = 0.0, Sum_C_TN = 0.0, Sum_C_TT = 0.0;
	#pragma omp parallel for num_threads(NUM_THREADS) reduction(+:Sum_C_NN)
    for(int i=0; i<M*N-1; i++){
        Sum_C_NN += C_NN[i];
	}
	#pragma omp parallel for num_threads(NUM_THREADS) reduction(+:Sum_C_NT)
    for(int i=0; i<M*N-1; i++){
        Sum_C_NT += C_NT[i];
	}
	#pragma omp parallel for num_threads(NUM_THREADS) reduction(+:Sum_C_TN)
    for(int i=0; i<M*N-1; i++){
        Sum_C_TN += C_TN[i];
	}
	#pragma omp parallel for num_threads(NUM_THREADS) reduction(+:Sum_C_TT)
    for(int i=0; i<M*N-1; i++){
        Sum_C_TT += C_TT[i];
	}

#ifdef CHECK_DATA
	for(int t=0; t<NUM_TESTS; t++){
		#pragma omp parallel for num_threads(NUM_THREADS) collapse(2)
		for(int i=0; i<M; i++){
			for(int j=0; j<N; j++){
				for(long k=0; k<K-K%K_G; k+=K_G){
					dataType tmp = 0.0;
					for(long kk=0; kk<K_G; kk++){
						tmp = fma(A[OFFSET(i, k+kk, K)], B[OFFSET(k+kk, j, N)], tmp);
					}
					C_cpu[OFFSET(i, j, N)] += tmp;
				}
				dataType tmp = 0.0;
				for(long k=K-K%K_G; k<K; k++){
					tmp = fma(A[OFFSET(i, k, K)], B[OFFSET(k, j, N)], tmp);
				}
				C_cpu[OFFSET(i, j, N)] += tmp;
			}
		}
	}
	double Sum_cpu = 0.0;
	#pragma omp parallel for num_threads(NUM_THREADS) reduction(+:Sum_cpu)
    for(int i=0; i<M*N-1; i++){
        Sum_cpu += C_cpu[i];
	}

	int error_num_1 = 0, error_num_2 = 0, error_num_3 = 0, error_num_4 = 0;
	printf("------------------------------ NN mode results ------------------------------\n");
	#pragma omp parallel for num_threads(NUM_THREADS) reduction(+:error_num_1)
	for(long i=0; i<len_C; i++){
        if(C_NN[i] != C_cpu[i]){
			// printf("C_NN[%ld] = %lf\tC_cpu[%ld] = %lf\n", i, C_NN[i], i, C_cpu[i]); 
			error_num_1 ++;
		}
	}
	if (error_num_1 > 0){
		printf("Result Error!\n");
        printf("error_num = %d\n", error_num_1);
    }else{
		printf("Success!\n");
	}
	printf("Sum_cpu  = %.6f\n", Sum_cpu);
	printf("Sum_C_NN = %.6f\n", Sum_C_NN);
	printf("------------------------------ NT mode results ------------------------------\n");
	#pragma omp parallel for num_threads(NUM_THREADS) reduction(+:error_num_2)
	for(long i=0; i<len_C; i++){
        if(C_NT[i] != C_cpu[i]){
			// printf("C_NT[%ld] = %lf\tC_cpu[%ld] = %lf\n", i, C_NT[i], i, C_cpu[i]); 
			error_num_2 ++;
		}
	}
	if(error_num_2 > 0){
		printf("Result Error!\n");
        printf("error_num = %d\n", error_num_2);
    }else{
		printf("Success!\n");
	}
	printf("Sum_cpu  = %.6f\n", Sum_cpu);
	printf("Sum_C_NT = %.6f\n", Sum_C_NT);
	printf("------------------------------ TT mode results ------------------------------\n");
	// #pragma omp parallel for num_threads(NUM_THREADS) reduction(+:error_num_4)
	for(long i=0; i<len_C; i++){
        if(C_TT[i] != C_cpu[i]){
			// printf("C_TT[%ld] = %lf\tC_cpu[%ld] = %lf\n", i, C_TT[i], i, C_cpu[i]); 
			error_num_4 ++;
		}
	}
	if(error_num_4 > 0){
		printf("Result Error!\n");
        printf("error_num = %d\n", error_num_4);
    }else{
		printf("Success!\n");
	}
	printf("Sum_cpu  = %.6f\n", Sum_cpu);
	printf("Sum_C_TT = %.6f\n", Sum_C_TT);
	printf("------------------------------ TN mode results ------------------------------\n");
	#pragma omp parallel for num_threads(NUM_THREADS) reduction(+:error_num_3)
	for(long i=0; i<len_C; i++){
        if(C_TN[i] != C_cpu[i]){
			// printf("C_TN[%ld] = %lf\tC_cpu[%ld] = %lf\n", i, C_TN[i], i, C_cpu[i]); 
			error_num_3 ++;
		}
	}
	if(error_num_3 > 0){
		printf("Result Error!\n");
        printf("error_num = %d\n", error_num_3);
    }else{
		printf("Success!\n");
	}
	printf("Sum_cpu  = %.6f\n", Sum_cpu);
	printf("Sum_C_TN = %.6f\n", Sum_C_TN);
#endif
	printf("------------------------------------------------------------------------------------------\n");
	printf("Sum_C_NN  = %.6f\n", Sum_C_NN);
    printf("C_NN[%d] = %f, C_cpu[%d] = %f\n", 0, C_NN[0], 0, C_cpu[0]);
    printf("C_NN[%ld] = %f, C_cpu[%ld] = %f\n", M*N-2, C_NN[M*N-2], M*N-2, C_cpu[M*N-2]);
    printf("C_NN[%ld] = %f, C_cpu[%ld] = %f\n", M*N-1, C_NN[M*N-1], M*N-1, C_cpu[M*N-1]);
	printf("------------------------------------------------------------------------------------------\n");
	printf("Sum_C_NT  = %.6f\n", Sum_C_NT);
	printf("C_NT[%d] = %f, C_cpu[%d] = %f\n", 0, C_NT[0], 0, C_cpu[0]);
    printf("C_NT[%ld] = %f, C_cpu[%ld] = %f\n", M*N-2, C_NT[M*N-2], M*N-2, C_cpu[M*N-2]);
    printf("C_NT[%ld] = %f, C_cpu[%ld] = %f\n", M*N-1, C_NT[M*N-1], M*N-1, C_cpu[M*N-1]);
	printf("------------------------------------------------------------------------------------------\n");
	printf("Sum_C_TT  = %.6f\n", Sum_C_TT);
    printf("C_TT[%d] = %f, C_cpu[%d] = %f\n", 0, C_TT[0], 0, C_cpu[0]);
    printf("C_TT[%ld] = %f, C_cpu[%ld] = %f\n", M*N-2, C_TT[M*N-2], M*N-2, C_cpu[M*N-2]);
    printf("C_TT[%ld] = %f, C_cpu[%ld] = %f\n", M*N-1, C_TT[M*N-1], M*N-1, C_cpu[M*N-1]);
	printf("------------------------------------------------------------------------------------------\n");
	printf("Sum_C_TN  = %.6f\n", Sum_C_TN);
    printf("C_TN[%d] = %f, C_cpu[%d] = %f\n", 0, C_TN[0], 0, C_cpu[0]);
    printf("C_TN[%ld] = %f, C_cpu[%ld] = %f\n", M*N-2, C_TN[M*N-2], M*N-2, C_cpu[M*N-2]);
    printf("C_TN[%ld] = %f, C_cpu[%ld] = %f\n", M*N-1, C_TN[M*N-1], M*N-1, C_cpu[M*N-1]);
	printf("------------------------------------------------------------------------------------------\n");
	
	hthread_barrier_destroy(bid_main);
	hthread_barrier_destroy(bid_T);
	hthread_group_destroy(tg_id);
	hthread_dev_close(dev_id);
	
	hthread_free(A);
	hthread_free(A_T);
	hthread_free(B);
	hthread_free(B_T);
	hthread_free(C_NN);
	hthread_free(C_NT);
	hthread_free(C_TN);
	hthread_free(C_TT);
	free(C_cpu);

	return 0;
}
