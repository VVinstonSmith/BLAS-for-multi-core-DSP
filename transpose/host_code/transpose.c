#include <stdio.h>
#include <stdlib.h>
#include "time.h"
#include <math.h>
#include <sys/time.h>
#include <omp.h>
#include "hthread_host.h"

#define NUM_TESTS 1
#define NUM_THREADS 16
#define dataType float
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

int main(int argc, char **argv){

	if (argc < 4) {
		printf("Useage: ~ dev_id{0, 1, 2, 3} side_elements num_threads{1...24}\n");
		return -1;
	}
	int dev_id= atoi(argv[1]);
	long M = atol(argv[2]);
	long N = atol(argv[3]);
	int num_threads = atoi(argv[4]);
	printf("M=%ld, N=%ld\n", M, N);

	long len_A = M * N;
	long size_A = len_A * sizeof(dataType);
	
	hthread_dev_open(dev_id);
	hthread_dat_load(dev_id, "kernel.dat");
	
	dataType *A = hthread_malloc(dev_id, size_A, HT_MEM_RO);
	dataType *B = hthread_malloc(dev_id, size_A, HT_MEM_RO);
	dataType *B_ref = malloc(size_A);

	// srand(time(0));
	printf("preparing data~\n");
	#pragma omp parallel for num_threads(NUM_THREADS) collapse(1)
	for(long i=0;i<len_A;i++){
		// A[i] = i;
		// A[i] = 1;
		// A[i] = 1.0 * rand()/RAND_MAX;
        A[i] = i % 7;
        // A[i] = (i % 7) + 0.01*i;
    }
	// memset(B_ref, 0, sizeof(dataType)); 
	// memset(B, 0, sizeof(dataType));
	#pragma omp parallel for num_threads(NUM_THREADS) collapse(1)
	for(long i=0;i<len_A;i++){
		// B[i] = i;
		// B_ref[i] = i;
		B[i] = 0.0;
		B_ref[i] = 0.0;
		// B[i] = i%13;
		// B_ref[i] = i%13;
    }

	/*申请设备端barrier同步单元*/
	int b_id = hthread_barrier_create(dev_id);
	// printf("B_id = %d\n", b_id);

	unsigned long args[5];
	args[0] = M; args[1] = N;
	args[2] = b_id;
	args[3] = (unsigned long)A;
	args[4] = (unsigned long)B;
	
	/*在指定的DSP簇上创建空闲线程组*/
	int tg_id = hthread_group_create(dev_id, num_threads);
	hthread_group_wait(tg_id);

	double t_start, t_end, t_ov;
	/* 测出数据传输的耗时 */
	t_start = timer();
	for(int i=0; i<NUM_TESTS; i++){
#if datatype == float
		hthread_group_exec(tg_id, "transpose_float_null", 3, 2, args);
#else
		hthread_group_exec(tg_id, "transpose_null", 3, 2, args);
#endif
		hthread_group_wait(tg_id);
	}
	t_end = timer();
	t_ov = t_end - t_start;

	printf("start computing~\n");
	t_start = timer();
	for(int i=0; i<NUM_TESTS; i++){
#if datatype == float
		hthread_group_exec(tg_id, "transpose_float_32x32_dbuf", 3, 2, args);
#else
		hthread_group_exec(tg_id, "transpose_16x16_dbuf", 3, 2, args);
#endif
		hthread_group_wait(tg_id);
	}
	t_end = timer();
	printf("Transpose elapsed time: %lf seconds, size per array: %lf GB, Perf %lf G elements/sec (%lf G elements/sec per core)\n", \
			(t_end - t_start - t_ov)/NUM_TESTS*1e-6, size_A/(1024.0 * 1024.0 * 1024.0), \
			(double)(M * N) / (((t_end - t_start - t_ov)/NUM_TESTS)*1e3), \
			(double)(M * N) / (((t_end - t_start - t_ov)/NUM_TESTS)*1e3) / num_threads);

#ifdef CHECK_DATA
	#pragma omp parallel for num_threads(NUM_THREADS) collapse(2)
	for(int i=0; i<M; i++){
		for(int j=0; j<N; j++){
			B_ref[j*M + i] += A[i*N + j];
		}
	}

	int error_num = 0;
	// #pragma omp parallel for num_threads(NUM_THREADS) reduction(+:error_num)
	for(long i=0;i<len_A;i++) 	{
        if ( B[i] != B_ref[i] ) {
			printf("B[%ld] = %lf\tB_ref[%ld] = %lf\n", i, B[i], i, B_ref[i]); 
			// printf("B[%ld][%ld] = %lf\tB_ref[%ld][%ld] = %lf\n", i/N, i%N, B[i], i/N, i%N, B_ref[i]); 
			error_num ++;
		}
	}

	if (error_num > 0){
		printf("Result Error!\n");
        printf("error_num = %d\n", error_num);
    }else 
		printf("Success!\n");

	double Sum_Bref = 0.0;
	#pragma omp parallel for num_threads(NUM_THREADS) reduction(+:Sum_Bref)
    for(int i=0;i<M*N-1;i++){
        Sum_Bref += B_ref[i];
	}
	printf("Sum_Bref = %.6f\n", Sum_Bref);
#endif

	double Sum_B = 0.0;
	#pragma omp parallel for num_threads(NUM_THREADS) reduction(+:Sum_B)
    for(int i=0;i<M*N-1;i++){
        Sum_B += B[i];
	}
	printf("Sum_B    = %.6f\n", Sum_B);

    const int index = 0;
    printf("B[%d] = %f, B_ref[%d] = %f\n", index, B[index], index, B_ref[index]);
    printf("B[%ld] = %f, B_ref[%ld] = %f\n", M*N-2, B[M*N-2], M*N-2, B_ref[M*N-2]);
    printf("B[%ld] = %f, B_ref[%ld] = %f\n", M*N-1, B[M*N-1], M*N-1, B_ref[M*N-1]);

	hthread_barrier_destroy(b_id);
	hthread_group_destroy(tg_id);
	hthread_dev_close(dev_id);
	
	hthread_free(A);
	hthread_free(B);
	free(B_ref);

	return 0;
}
