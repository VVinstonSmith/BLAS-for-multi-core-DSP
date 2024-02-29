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
	const long M_delta = 64;
	const long K_delta = 128;
	const long N_delta = 256;

	if (argc < 4) {
		printf("Useage: ~ dev_id{0, 1, 2, 3} side_elements num_threads{1...24}\n");
		return -1;
	}
	const int dev_id= atoi(argv[1]);
	const long M = atol(argv[2]);
	const long K = atol(argv[3]);
	const long N = atol(argv[4]);
	const int num_threads = atoi(argv[5]);
	printf("M=%ld, K=%ld, N=%ld\n", M, K, N);

	const long M_0 = M + M_delta;
	const long K_0 = K + K_delta;
	const long N_0 = N + N_delta;

	const long len_A = M_0 * K_0;
	const long len_B = N_0 * K_0;
	const long len_C = M_0 * N_0;
	const long size_A = len_A * sizeof(dataType);
	const long size_B = len_B * sizeof(dataType);
	const long size_C = len_C * sizeof(dataType);
	
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
	for(int i=0; i<M_0; i++){
		for(int j=0; j<K_0; j++){
			A[OFFSET(i, j, K_0)] = A_T[OFFSET(j, i, M_0)] = (double)(rand()) / RAND_MAX * (-4.0 + 6.0) + 2.0;
			// A[OFFSET(i, j, K_0)] = A_T[OFFSET(j, i, M_0)] = OFFSET(i, j, K_0) % 11;
		}
	}

	#pragma omp parallel for num_threads(NUM_THREADS) collapse(2)
	for(int i=0; i<K_0; i++){
		for(int j=0; j<N_0; j++){
			B[OFFSET(i, j, N_0)] = B_T[OFFSET(j, i, K_0)] = (double)(rand()) / RAND_MAX * (-4.0 + 6.0) + 2.0;
			// B[OFFSET(i, j, N_0)] = B_T[OFFSET(j, i, K_0)] = OFFSET(i, j, N_0) % 7;
		}
	}

	#pragma omp parallel for num_threads(NUM_THREADS) collapse(2)
	for(int i=0; i<M; i++){
		for(int j=0; j<N; j++){
			int index = OFFSET(i, j, N_0);
        	C_cpu[index] = C_NN[index] = C_NT[index] = C_TN[index] = C_TT[index] = 0.0;
		}
    }

	/*申请设备端barrier同步单元*/
	int bid_main = hthread_barrier_create(dev_id);
	int bid_T = hthread_barrier_create(dev_id);
	printf("bid_main = %d\n", bid_main);
	printf("bid_T = %d\n", bid_T);

	unsigned long args_GEMM_NN[10];
	args_GEMM_NN[0] = K_0; args_GEMM_NN[1] = N_0; args_GEMM_NN[2] = N_0;
	args_GEMM_NN[3] = M; args_GEMM_NN[4] = N; args_GEMM_NN[5] = K;
	args_GEMM_NN[6] = bid_main;
	args_GEMM_NN[7] = (unsigned long)A;
	args_GEMM_NN[8] = (unsigned long)B;
	args_GEMM_NN[9] = (unsigned long)C_NN;

	unsigned long args_GEMM_NT[10];
	args_GEMM_NT[0] = K_0; args_GEMM_NT[1] = K_0; args_GEMM_NT[2] = N_0;
	args_GEMM_NT[3] = M; args_GEMM_NT[4] = N; args_GEMM_NT[5] = K;
	args_GEMM_NT[6] = bid_main;
	args_GEMM_NT[7] = (unsigned long)A;
	args_GEMM_NT[8] = (unsigned long)B_T;
	args_GEMM_NT[9] = (unsigned long)C_NT;

	unsigned long args_GEMM_TN[11];
	args_GEMM_TN[0] = M_0; args_GEMM_TN[1] = N_0; args_GEMM_TN[2] = N_0;
	args_GEMM_TN[3] = M; args_GEMM_TN[4] = N; args_GEMM_TN[5] = K;
	args_GEMM_TN[6] = bid_main; args_GEMM_TN[7] = bid_T; 
	args_GEMM_TN[8] = (unsigned long)A_T;
	args_GEMM_TN[9] = (unsigned long)B;
	args_GEMM_TN[10] = (unsigned long)C_TN;

	unsigned long args_GEMM_TT[10];
	args_GEMM_TT[0] = M_0; args_GEMM_TT[1] = K_0; args_GEMM_TT[2] = N_0;
	args_GEMM_TT[3] = M; args_GEMM_TT[4] = N; args_GEMM_TT[5] = K;
	args_GEMM_TT[6] = bid_main;
	args_GEMM_TT[7] = (unsigned long)A_T;
	args_GEMM_TT[8] = (unsigned long)B_T;
	args_GEMM_TT[9] = (unsigned long)C_TT;
	
	//--------------------------------------------------------------------------------------
	/*在指定的DSP簇上创建空闲线程组*/
	int tg_id = hthread_group_create(dev_id, num_threads);
	hthread_group_wait(tg_id);

	double t_start, t_end, t_ov;

	t_start = timer();
	for(int i=0; i<NUM_TESTS; i++){
		hthread_group_exec(tg_id, "launch_dgemm_null", 7, 3, args_GEMM_NN);
		hthread_group_wait(tg_id);
	}
	t_end = timer();
	t_ov = t_end - t_start;

	// dgemm_NN
	t_start = timer();
	for(int i=0; i<NUM_TESTS; i++){
        hthread_group_exec(tg_id, "launch_dgemm_NN", 7, 3, args_GEMM_NN);
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
        hthread_group_exec(tg_id, "launch_dgemm_NT", 7, 3, args_GEMM_NT);
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
        hthread_group_exec(tg_id, "launch_dgemm_TT", 7, 3, args_GEMM_TT);
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
		hthread_group_exec(tg_id, "launch_dgemm_TN", 8, 3, args_GEMM_TN);
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
	for(int i=0; i<M; i++){
		for(int j=0; j<N; j++){
			int index = OFFSET(i, j, N_0);
			Sum_C_NN += C_NN[index];
		}
    }
	#pragma omp parallel for num_threads(NUM_THREADS) reduction(+:Sum_C_NT)
	for(int i=0; i<M; i++){
		for(int j=0; j<N; j++){
			int index = OFFSET(i, j, N_0);
			Sum_C_NT += C_NT[index];
		}
    }
	#pragma omp parallel for num_threads(NUM_THREADS) reduction(+:Sum_C_TN)
	for(int i=0; i<M; i++){
		for(int j=0; j<N; j++){
			int index = OFFSET(i, j, N_0);
			Sum_C_TN += C_TN[index];
		}
    }
	#pragma omp parallel for num_threads(NUM_THREADS) reduction(+:Sum_C_TT)
	for(int i=0; i<M; i++){
		for(int j=0; j<N; j++){
			int index = OFFSET(i, j, N_0);
			Sum_C_TT += C_TT[index];
		}
    }

#ifdef CHECK_DATA
	for(int t=0; t<NUM_TESTS; t++){
		#pragma omp parallel for num_threads(NUM_THREADS) collapse(2)
		for(int i=0; i<M; i++){
			for(int j=0; j<N; j++){
				for(long k=0; k<K-K%K_G; k+=K_G){
					dataType tmp = 0.0;
					for(long kk=0; kk<K_G; kk++){
						tmp = fma(A[OFFSET(i, k+kk, K_0)], B[OFFSET(k+kk, j, N_0)], tmp);
					}
					C_cpu[OFFSET(i, j, N_0)] += tmp;
				}
				dataType tmp = 0.0;
				for(long k=K-K%K_G; k<K; k++){
					tmp = fma(A[OFFSET(i, k, K_0)], B[OFFSET(k, j, N_0)], tmp);
				}
				C_cpu[OFFSET(i, j, N_0)] += tmp;
			}
		}
	}
	double Sum_cpu = 0.0;
	#pragma omp parallel for num_threads(NUM_THREADS) reduction(+:Sum_cpu)
	for(int i=0; i<M; i++){
		for(int j=0; j<N; j++){
			int index = OFFSET(i, j, N_0);
			Sum_cpu += C_cpu[index];
		}
	}

	int error_num_1 = 0, error_num_2 = 0, error_num_3 = 0, error_num_4 = 0;
	printf("------------------------------ NN mode results ------------------------------\n");
	#pragma omp parallel for num_threads(NUM_THREADS) collapse(2)
	for(int i=0; i<M; i++){
		for(int j=0; j<N; j++){
			int index = OFFSET(i, j, N_0);
			if(C_NN[index] != C_cpu[index]){
				// printf("C_NN[%d] = %lf\tC_cpu[%d] = %lf\n", index, C_NN[index], index, C_cpu[index]); 
				error_num_1 ++;
			}
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
	#pragma omp parallel for num_threads(NUM_THREADS) collapse(2)
	for(int i=0; i<M; i++){
		for(int j=0; j<N; j++){
			int index = OFFSET(i, j, N_0);
			if(C_NT[index] != C_cpu[index]){
				// printf("C_NT[%d] = %lf\tC_cpu[%d] = %lf\n", index, C_NT[index], index, C_cpu[index]); 
				error_num_1 ++;
			}
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
	#pragma omp parallel for num_threads(NUM_THREADS) collapse(2)
	for(int i=0; i<M; i++){
		for(int j=0; j<N; j++){
			int index = OFFSET(i, j, N_0);
			if(C_TT[index] != C_cpu[index]){
				// printf("C_TT[%d] = %lf\tC_cpu[%d] = %lf\n", index, C_TT[index], index, C_cpu[index]); 
				error_num_1 ++;
			}
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
	#pragma omp parallel for num_threads(NUM_THREADS) collapse(2)
	for(int i=0; i<M; i++){
		for(int j=0; j<N; j++){
			int index = OFFSET(i, j, N_0);
			if(C_TN[index] != C_cpu[index]){
				// printf("C_TN[%d] = %lf\tC_cpu[%d] = %lf\n", index, C_TN[index], index, C_cpu[index]); 
				error_num_1 ++;
			}
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
	long index_1 = OFFSET(M-1, N-1, N_0) - 2, index_2 = OFFSET(M-1, N-1, N_0) - 1;
	printf("------------------------------------------------------------------------------------------\n");
	printf("Sum_C_NN  = %.6f\n", Sum_C_NN);
    printf("C_NN[%d] = %f, C_cpu[%d] = %f\n", 0, C_NN[0], 0, C_cpu[0]);
    printf("C_NN[%ld] = %f, C_cpu[%ld] = %f\n", index_1, C_NN[index_1], index_1, C_cpu[index_1]);
    printf("C_NN[%ld] = %f, C_cpu[%ld] = %f\n", index_2, C_NN[index_2], index_2, C_cpu[index_2]);
	printf("------------------------------------------------------------------------------------------\n");
	printf("Sum_C_NT  = %.6f\n", Sum_C_NT);
	printf("C_NT[%d] = %f, C_cpu[%d] = %f\n", 0, C_NT[0], 0, C_cpu[0]);
    printf("C_NT[%ld] = %f, C_cpu[%ld] = %f\n", index_1, C_NT[index_1], index_1, C_cpu[index_1]);
    printf("C_NT[%ld] = %f, C_cpu[%ld] = %f\n", index_2, C_NN[index_2], index_2, C_cpu[index_2]);
	printf("------------------------------------------------------------------------------------------\n");
	printf("Sum_C_TT  = %.6f\n", Sum_C_TT);
    printf("C_TT[%d] = %f, C_cpu[%d] = %f\n", 0, C_TT[0], 0, C_cpu[0]);
    printf("C_TT[%ld] = %f, C_cpu[%ld] = %f\n", index_1, C_TT[index_1], index_1, C_cpu[index_1]);
    printf("C_TT[%ld] = %f, C_cpu[%ld] = %f\n", index_2, C_NN[index_2], index_2, C_cpu[index_2]);
	printf("------------------------------------------------------------------------------------------\n");
	printf("Sum_C_TN  = %.6f\n", Sum_C_TN);
    printf("C_TN[%d] = %f, C_cpu[%d] = %f\n", 0, C_TN[0], 0, C_cpu[0]);
    printf("C_TN[%ld] = %f, C_cpu[%ld] = %f\n", index_1, C_TN[index_1], index_1, C_cpu[index_1]);
    printf("C_TN[%ld] = %f, C_cpu[%ld] = %f\n", index_2, C_NN[index_2], index_2, C_cpu[index_2]);
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
