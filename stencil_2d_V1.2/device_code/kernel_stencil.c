#include <compiler/m3000.h>
#include <compiler/vsip.h>
#include <hthread_device.h>

#define unlong unsigned long
#define unint unsigned int
#define OFFSET(row, col, ld) ((row) * (ld) + (col))

unlong min(unlong a, unlong b){
	if(a<b) return a;
	return b;
}

__global__ void stencil_scalar(unlong M, unlong N, double *A, double *B){
	const int tid = get_thread_id();
	const int grp_size = get_group_size();
	const unlong tile_size = (M + grp_size - 1) / grp_size;
	const unlong row_start = tid * tile_size;
	unlong row_end = row_start + tile_size;
	if(row_end > M)
		row_end = M;
	// hthread_printf("tid=%d\n", tid);
	// hthread_printf("row_start = %ld\n", row_start);
	// hthread_printf("row_end   = %ld\n", row_end);

	const unint M_S = 32;
	const unint N_S = 32;

	double* spm_A[2];
	double* spm_B[2];
	spm_A[0] = scalar_malloc((M_S+2) * (N_S+2) * sizeof(double));
	spm_A[1] = scalar_malloc((M_S+2) * (N_S+2) * sizeof(double));
	spm_B[0] = scalar_malloc(M_S * N_S * sizeof(double));
	spm_B[1] = scalar_malloc(M_S * N_S * sizeof(double));
	
	int ch_a[2], ch_b[2];

	const int ch0_a = 0;
	const int ch0_b = 2;

	unint M_S_cur = min(M_S, row_end-row_start);
	unint N_S_cur = min(N_S, N);
	unint M_S_next, N_S_next;
	ch_a[0] = dma_p2p_opt(&A[OFFSET(row_start, 0, N+2)], M_S_cur+2, (N_S_cur+2)*sizeof(double), (N-N_S_cur)*sizeof(double),
									spm_A[0], M_S_cur+2, (N_S_cur+2)*sizeof(double), (N_S-N_S_cur)*sizeof(double), 0, 0, ch0_a);
	ch_b[1] = dma_p2p_opt(&B[OFFSET(row_start, 0, N)], 1, 1*sizeof(double), 0,
						   			spm_B[1], 1, 1*sizeof(double), 0, 0, 0, ch0_b+1); // 一次无效传输
	unint cnt_a = 0;
	unint cnt_b = 0;
	for(unlong m_o=row_start; m_o<row_end; m_o+=M_S){
		M_S_next = min(M_S, row_end-(m_o+M_S));
		for(unlong n_o=0; n_o<N; n_o+=N_S){
			unint cnt_a_1 = (cnt_a + 1) % 2;
			unint cnt_b_1 = (cnt_b + 1) % 2;

			if(n_o + N_S < N){
				N_S_next = min(N_S, N-(n_o+N_S));
				ch_a[cnt_a_1] = dma_p2p_opt(&A[OFFSET(m_o, n_o+N_S, N+2)], M_S_cur+2, (N_S_next+2)*sizeof(double), (N-N_S_next)*sizeof(double),
										spm_A[cnt_a_1], M_S_cur+2, (N_S_next+2)*sizeof(double), (N_S-N_S_next)*sizeof(double), 0, 0, ch0_a+cnt_a_1);
			} else if(m_o + M_S < M){
				N_S_next = min(N_S, N);
				ch_a[cnt_a_1] = dma_p2p_opt(&A[OFFSET(m_o+M_S, 0, N+2)], M_S_next+2, (N_S_next+2)*sizeof(double), (N-N_S_next)*sizeof(double),
										spm_A[cnt_a_1], M_S_next+2, (N_S_next+2)*sizeof(double), (N_S-N_S_next)*sizeof(double), 0, 0, ch0_a+cnt_a_1);
			}
			
			dma_wait_p2p(ch_a[cnt_a]);

			for(int m_i=0; m_i<M_S_cur; m_i++){
				for(int n_i=0; n_i<N_S_cur; n_i++){
					spm_B[cnt_b][OFFSET(m_i, n_i, N_S)] = 0.2 * (\
						spm_A[cnt_a][OFFSET(m_i, n_i+1, N_S+2)] + spm_A[cnt_a][OFFSET(m_i+1, n_i+1, N_S+2)] + spm_A[cnt_a][OFFSET(m_i+2, n_i+1, N_S+2)] + \
						spm_A[cnt_a][OFFSET(m_i+1, n_i, N_S+2)] + spm_A[cnt_a][OFFSET(m_i+1, n_i+2, N_S+2)] \
					);
				}
			}
			// hthread_printf("\n");
			// for(int i=0; i<M_S_cur; i++){
			// 	for(int j=0; j<N_S_cur; j++){
			// 		hthread_printf("%f, ", spm_B[cnt_b][OFFSET(i, j, N_S)]);
			// 	}hthread_printf("\n");
			// }

			dma_wait_p2p(ch_b[cnt_b_1]);

			ch_b[cnt_b] = dma_p2p_opt(spm_B[cnt_b], M_S_cur, N_S_cur*sizeof(double), (N_S-N_S_cur)*sizeof(double),
						   &B[OFFSET(m_o, n_o, N)], M_S_cur, N_S_cur*sizeof(double), (N-N_S_cur)*sizeof(double), 0, 0, ch0_b+cnt_b);
			
			cnt_a = cnt_a_1;
			cnt_b = cnt_b_1;
			N_S_cur = N_S_next;						 
		}
		M_S_cur = M_S_next;
	}
	dma_wait_p2p(ch_b[(cnt_b+1)%2]);
	scalar_free(spm_A[0]);
	scalar_free(spm_A[1]);
	scalar_free(spm_B[0]);
	scalar_free(spm_B[1]);
}


// 无优化版本
void micro_kernel_cross_reduce_V1(lvector double* src_a, lvector double* dst_b, unlong m, unlong n, unlong N){
	__asm__ __volatile__(
		// %0:src_a (源数据首地址),    %1:dst_b (目的数据首地址),
		// %2:m (目的数据行数),    %3:n (目的数据列数),    %4:N (目的数据buffer区行长度)
		
		// 常量初始化：
		"		VMOVI			0x3FF0000000000000, VR63  ;; VR63 = 1			\t\n"
		"		VMOVI			0x3FC999999999999A, VR62  ;; VR62 = 0.2			\t\n"
		"		VMOVI			0x00, VR61  			;; VR61 = 0.0			\t\n"
		"		SMOVI24         64, R59                 ;; R59 = 64				\t\n"
		"		SMOVI24         512, R61                ;; R61 = 16*4*8			\t\n"
		"		SADD.M1         2, %4, R26              ;; R26 = N+2			\t\n"

		"		SSHFLL          3, R26, R55             ;; R55 = (N+2)*8		\t\n"
		"		SSHFLL          4, R26, R56             ;; R56 = (N+2)*2*8		\t\n"
		"		SSHFLL          5, R26, R57             ;; R57 = (N+2)*4*8		\t\n"
		"		SSHFLL          4, %4, R51              ;; R51 = N*2*8			\t\n"
		"		SSHFLL          5, %4, R52              ;; R52 = N*4*8			\t\n"

		"		SSHFLR          1, R26, R25             ;; R25 = (N+2)*1/2		\t\n"
		"		SADD.M1         R26, R25, R27           ;; R27 = (N+2)*3/2		\t\n"
		"		SSHFLL			1, R27, R28				;; R28 = (N+2)*3		\t\n"
		"		SSHFLR          1, %4, R29              ;; R29 = N * 1/2		\t\n"

		"		SMVAGA.M1       R26, OR2                ;; OR2 = (N+2)			\t\n"
		"		SMVAGA.M1       R25, OR1                ;; OR1 = (N+2)*1/2		\t\n"
		"		SMVAGA.M1       R27, OR3                ;; OR3 = (N+2)*3/2		\t\n"
		"		SMVAGA.M1       R28, OR4                ;; OR4 = (N+2)*3		\t\n"
		"		SMVAGA.M1       R29, OR5                ;; OR5 = N * 1/2		\t\n"

		// 地址初始化：
		"		SADD.M1         8, %0, R42              ;; R42 = src_A + 8		\t\n"
		"		SADD.M1         R55, %0, R43            ;; R43 = src_A + (N+2)*8\t\n"
		"		SADD.M1         R56, R42, R45           ;; R45 = R42 + (N+2)*2*8\t\n"
		"		SADD.M1         16, R43, R44            ;; R44 = R43 + 16		\t\n"
		"		SADD.M1         R51, %1, R46            ;; R46 = dst_B + N*2*8	\t\n"
		
		"		SMVAGA.M1       R42, AR0				\t\n"
		"		SMVAGA.M1       R43, AR1				\t\n"
		"		SMVAGA.M1       R44, AR2				\t\n"
		"		SMVAGA.M1       R45, AR3				\t\n"
		"		SMVAGA.M1       %1, AR5 				\t\n"
		"		SMVAGA.M1       R46, AR6				\t\n"
		"		SMOVI24         0, R1					\t\n"

		// 加载第一批数据：
			// AR1
		"		VLDDW			*AR1++[OR1], VR5:VR4						\t\n"
		"	|	VLDDW			*+AR1[16], VR7:VR6							\t\n"

		"		VLDDW			*AR1++[OR1], VR17:VR16						\t\n"
		"	|	VLDDW			*+AR1[16], VR19:VR18						\t\n"

		"		VLDDW			*AR1++[OR1], VR29:VR28						\t\n"
		"	|	VLDDW			*+AR1[16], VR31:VR30						\t\n"

		"		VLDDW			*AR1--[OR3], VR41:VR40						\t\n"
		"	|	VLDDW			*+AR1[16], VR43:VR42						\t\n"
			// AR0
		"		VLDW			*+AR0[32], VR2								\t\n"
		"	|	VLDW			*+AR0[48], VR3								\t\n"
		"		VLDW			*AR0++[OR2], VR0							\t\n"
		"	|	VLDW			*+AR0[16], VR1								\t\n"

		"		VLDW			*+AR0[32], VR10								\t\n"
		"	|	VLDW			*+AR0[48], VR11								\t\n"
		"		VLDW			*AR0--[OR2], VR8							\t\n"
		"	|	VLDW			*+AR0[16], VR9								\t\n"
			// AR3
		"		VLDW			*+AR3[32], VR22								\t\n"
		"	|	VLDW			*+AR3[48], VR23								\t\n"
		"		VLDW			*AR3++[OR2], VR20							\t\n"
		"	|	VLDW			*+AR3[16], VR21								\t\n"

		"		VLDW			*+AR3[32], VR34								\t\n"
		"	|	VLDW			*+AR3[48], VR35								\t\n"
		"		VLDW			*AR3++[OR2], VR32							\t\n"
		"	|	VLDW			*+AR3[16], VR33								\t\n"

		"		VLDW			*+AR3[32], VR46								\t\n"
		"	|	VLDW			*+AR3[48], VR47								\t\n"
		"		VLDW			*AR3++[OR2], VR44							\t\n"
		"	|	VLDW			*+AR3[16], VR45								\t\n"

		"		VLDW			*+AR3[32], VR54								\t\n"
		"	|	VLDW			*+AR3[48], VR55								\t\n"
		"		VLDW			*AR3--[OR4], VR52							\t\n"
		"	|	VLDW			*+AR3[16], VR53								\t\n"

		// outer loop:
		"		SMOVI24         0, R7                   ;; i=0					\t\n"
		"loop_m_V1:	\t\n"
		"		SADD.M1    		4, R7, R7               ;; i += 4				\t\n"
		"		SLT     		R7, %2, R3              ;; if i<m				\t\n"
		
		// addr_A for next loop_m:
		"		SADD.M1     	R57, R42, R42			;; R42 += (N+2)*4*8		\t\n"
		"		SADD.M1     	R57, R43, R43			;; R43 += (N+2)*4*8		\t\n"
		"		SADD.M1     	R57, R44, R44			;; R44 += (N+2)*4*8		\t\n"
		"		SADD.M1     	R57, R45, R45			;; R45 += (N+2)*4*8		\t\n"

		// inner loop:
		"		SMOVI24     	0, R8                   ;; j=0					\t\n"
		"loop_n_V1:	\t\n"
		"		SADD.M1    		R59, R8, R8             ;; j += 64				\t\n"
		"		SLT     		R8, %3, R0              ;; if j<n				\t\n"
		
		// ---------------------------------------------------------------
        // VSTDW pre data：
			// AR5
		"		VSTDW			VR13:VR12, *AR5++[OR5]						\t\n"
		"	|	VSTDW			VR15:VR14, *+AR5[16]						\t\n"

		"		VSTDW			VR25:VR24, *AR5--[OR5]						\t\n"
		"	|	VSTDW			VR27:VR26, *+AR5[16]						\t\n"
			// AR6
		"		VSTDW			VR37:VR36, *AR6++[OR5]						\t\n"
		"	|	VSTDW			VR39:VR38, *+AR6[16]						\t\n"

		"		VSTDW			VR49:VR48, *AR6--[OR5]						\t\n"
		"	|	VSTDW			VR51:VR50, *+AR6[16]						\t\n"

		// if(j_pre < n)
		"		[R1] SADDA.M1   		R61, AR5, AR5       ;; AR5 += 16*4*8	\t\n"
		"		[R1] SADDA.M1   		R61, AR6, AR6       ;; AR6 += 16*4*8	\t\n"
		// else
		"		[!R1] SMVAGA.M1         %1, AR5             ;; AR5_next = %1	\t\n"
		"		[!R1] SMVAGA.M1         R46, AR6            ;; AR6_next = R46	\t\n"
		// ---------------------------------------------------------------
		// load from AR2
		"		VLDDW			*AR2++[OR1], VR13:VR12						\t\n"
		"	|	VLDDW			*+AR2[16], VR15:VR14						\t\n"

		"		VLDDW			*AR2++[OR1], VR25:VR24						\t\n"
		"	|	VLDDW			*+AR2[16], VR27:VR26						\t\n"

		"		VLDDW			*AR2++[OR1], VR37:VR36						\t\n"
		"	|	VLDDW			*+AR2[16], VR39:VR38						\t\n"

		"		VLDDW			*AR2--[OR3], VR49:VR48						\t\n"
		"	|	VLDDW			*+AR2[16], VR51:VR50						\t\n"
		// 计算本轮的数据:
		"		VFMULAD.M1		VR0, VR63, VR4, VR4			\t\n" // [1]
		"		VFMULAD.M1		VR1, VR63, VR5, VR5			\t\n"
		"		VFMULAD.M1		VR2, VR63, VR6, VR6			\t\n"
		"		VFMULAD.M1		VR3, VR63, VR7, VR7			\t\n"

		"		VFMULAD.M1		VR20, VR63, VR8, VR8 		\t\n" // [2]
		"		VFMULAD.M1		VR21, VR63, VR9, VR9 		\t\n"
		"		VFMULAD.M1		VR22, VR63, VR10, VR10		\t\n"
		"		VFMULAD.M1		VR23, VR63, VR11, VR11		\t\n"

		"		VFMULAD.M1		VR32, VR63, VR16, VR16		\t\n" // [5]
		"		VFMULAD.M1		VR33, VR63, VR17, VR17		\t\n"
		"		VFMULAD.M1		VR34, VR63, VR18, VR18		\t\n"
		"		VFMULAD.M1		VR35, VR63, VR19, VR19		\t\n"

		"		VFMULAD.M1		VR20, VR63, VR28, VR28		\t\n" // [8]
		"		VFMULAD.M1		VR21, VR63, VR29, VR29		\t\n"
		"		VFMULAD.M1		VR22, VR63, VR30, VR30		\t\n"
		"		VFMULAD.M1		VR23, VR63, VR31, VR31		\t\n"

		"		VFMULAD.M1		VR44, VR63, VR32, VR32		\t\n" // [9]
		"		VFMULAD.M1		VR45, VR63, VR33, VR33		\t\n"
		"		VFMULAD.M1		VR46, VR63, VR34, VR34		\t\n"
		"		VFMULAD.M1		VR47, VR63, VR35, VR35		\t\n"

		"		VFMULAD.M1		VR52, VR63, VR40, VR40		\t\n" // [12]
		"		VFMULAD.M1		VR53, VR63, VR41, VR41		\t\n"
		"		VFMULAD.M1		VR54, VR63, VR42, VR42		\t\n"
		"		VFMULAD.M1		VR55, VR63, VR43, VR43		\t\n"
		// --
		"		VFMULAD.M1		VR8 , VR63, VR4, VR4		\t\n" // [3]
		"		VFMULAD.M1		VR9 , VR63, VR5, VR5		\t\n"
		"		VFMULAD.M1		VR10, VR63, VR6, VR6		\t\n"
		"		VFMULAD.M1		VR11, VR63, VR7, VR7		\t\n"
		
		"		VFMULAD.M1		VR8 , VR63, VR16, VR16		\t\n" // [6]
		"		VFMULAD.M1		VR9 , VR63, VR17, VR17		\t\n"
		"		VFMULAD.M1		VR10, VR63, VR18, VR18		\t\n"
		"		VFMULAD.M1		VR11, VR63, VR19, VR19		\t\n"
		
		"		VFMULAD.M1		VR32, VR63, VR28, VR28		\t\n" // [10]
		"		VFMULAD.M1		VR33, VR63, VR29, VR29		\t\n"
		"		VFMULAD.M1		VR34, VR63, VR30, VR30		\t\n"
		"		VFMULAD.M1		VR35, VR63, VR31, VR31		\t\n"
		
		"		VFMULAD.M1		VR32, VR63, VR40, VR40		\t\n" // [13]
		"		VFMULAD.M1		VR33, VR63, VR41, VR41		\t\n"
		"		VFMULAD.M1		VR34, VR63, VR42, VR42		\t\n"
		"		VFMULAD.M1		VR35, VR63, VR43, VR43		\t\n"
		// --
		"		VFMULAD.M1		VR4, VR63, VR12, VR12		\t\n" // [4]
		"		VFMULAD.M1		VR5, VR63, VR13, VR13		\t\n"
		"		VFMULAD.M1		VR6, VR63, VR14, VR14		\t\n"
		"		VFMULAD.M1		VR7, VR63, VR15, VR15		\t\n"

		"		VFMULAD.M1		VR16, VR63, VR24, VR24		\t\n" // [7]
		"		VFMULAD.M1		VR17, VR63, VR25, VR25		\t\n"
		"		VFMULAD.M1		VR18, VR63, VR26, VR26		\t\n"
		"		VFMULAD.M1		VR19, VR63, VR27, VR27		\t\n"

		"		VFMULAD.M1		VR28, VR63, VR36, VR36		\t\n" // [11]
		"		VFMULAD.M1		VR29, VR63, VR37, VR37		\t\n"
		"		VFMULAD.M1		VR30, VR63, VR38, VR38		\t\n"
		"		VFMULAD.M1		VR31, VR63, VR39, VR39		\t\n"

		"		VFMULAD.M1		VR40, VR63, VR48, VR48		\t\n" // [14]
		"		VFMULAD.M1		VR41, VR63, VR49, VR49		\t\n"
		"		VFMULAD.M1		VR42, VR63, VR50, VR50		\t\n"
		"		VFMULAD.M1		VR43, VR63, VR51, VR51		\t\n"
		// --
		"		VFMULAD.M1		VR12, VR62, VR61, VR12		\t\n" // [15]
		"		VFMULAD.M2		VR13, VR62, VR61, VR13		\t\n"
		"		VFMULAD.M3		VR14, VR62, VR61, VR14		\t\n"
		"		VFMULAD.M1		VR15, VR62, VR61, VR15		\t\n"

		"		VFMULAD.M1		VR24, VR62, VR61, VR24		\t\n" // [16]
		"		VFMULAD.M1		VR25, VR62, VR61, VR25		\t\n"
		"		VFMULAD.M1		VR26, VR62, VR61, VR26		\t\n"
		"		VFMULAD.M1		VR27, VR62, VR61, VR27		\t\n"

		"		VFMULAD.M1		VR36, VR62, VR61, VR36		\t\n" // [17]
		"		VFMULAD.M1		VR37, VR62, VR61, VR37		\t\n"
		"		VFMULAD.M1		VR38, VR62, VR61, VR38		\t\n"
		"		VFMULAD.M1		VR39, VR62, VR61, VR39		\t\n"

		"		VFMULAD.M1		VR48, VR62, VR61, VR48		\t\n" // [18]
		"		VFMULAD.M1		VR49, VR62, VR61, VR49		\t\n"
		"		VFMULAD.M1		VR50, VR62, VR61, VR50		\t\n"
		"		VFMULAD.M1		VR51, VR62, VR61, VR51		\t\n"

		// ---------------------------------------------------------------
		// if(j_cur < n)
		"		[R0] SADDA.M1   		R61, AR0, AR0		;; AR0 += 16*4*8	\t\n"
		"		[R0] SADDA.M1   		R61, AR1, AR1		;; AR1 += 16*4*8	\t\n"
		"		[R0] SADDA.M1   		R61, AR2, AR2		;; AR2 += 16*4*8	\t\n"
		"		[R0] SADDA.M1   		R61, AR3, AR3		;; AR3 += 16*4*8	\t\n"
		// else
		"		[!R0] SMVAGA.M1         R42, AR0            ;; AR0_next = R42	\t\n"
		"		[!R0] SMVAGA.M1         R43, AR1            ;; AR1_next = R43	\t\n"
		"		[!R0] SMVAGA.M1         R44, AR2            ;; AR2_next = R44	\t\n"
		"		[!R0] SMVAGA.M1         R45, AR3            ;; AR3_next = R45	\t\n"
		// VLDDW next data (load form AR1,AR0,AR3):
		// 	// AR1
		"		VLDDW			*AR1++[OR1], VR5:VR4						\t\n"
		"	|	VLDDW			*+AR1[16], VR7:VR6							\t\n"

		"		VLDDW			*AR1++[OR1], VR17:VR16						\t\n"
		"	|	VLDDW			*+AR1[16], VR19:VR18						\t\n"

		"		VLDDW			*AR1++[OR1], VR29:VR28						\t\n"
		"	|	VLDDW			*+AR1[16], VR31:VR30						\t\n"

		"		VLDDW			*AR1--[OR3], VR41:VR40						\t\n"
		"	|	VLDDW			*+AR1[16], VR43:VR42						\t\n"
			// AR0
		"		VLDW			*+AR0[32], VR2								\t\n"
		"	|	VLDW			*+AR0[48], VR3								\t\n"
		"		VLDW			*AR0++[OR2], VR0							\t\n"
		"	|	VLDW			*+AR0[16], VR1								\t\n"

		"		VLDW			*+AR0[32], VR10								\t\n"
		"	|	VLDW			*+AR0[48], VR11								\t\n"
		"		VLDW			*AR0--[OR2], VR8							\t\n"
		"	|	VLDW			*+AR0[16], VR9								\t\n"
			// AR3
		"		VLDW			*+AR3[32], VR22								\t\n"
		"	|	VLDW			*+AR3[48], VR23								\t\n"
		"		VLDW			*AR3++[OR2], VR20							\t\n"
		"	|	VLDW			*+AR3[16], VR21								\t\n"

		"		VLDW			*+AR3[32], VR34								\t\n"
		"	|	VLDW			*+AR3[48], VR35								\t\n"
		"		VLDW			*AR3++[OR2], VR32							\t\n"
		"	|	VLDW			*+AR3[16], VR33								\t\n"

		"		VLDW			*+AR3[32], VR46								\t\n"
		"	|	VLDW			*+AR3[48], VR47								\t\n"
		"		VLDW			*AR3++[OR2], VR44							\t\n"
		"	|	VLDW			*+AR3[16], VR45								\t\n"

		"		VLDW			*+AR3[32], VR54								\t\n"
		"	|	VLDW			*+AR3[48], VR55								\t\n"
		"		VLDW			*AR3--[OR4], VR52							\t\n"
		"	|	VLDW			*+AR3[16], VR53								\t\n"
		// ---------------------------------------------------------------
		"		SMOV			R0, R1		\t\n"
		"		[R0] SBR    	loop_n_V1	\t\n" // end inner loop
		"		SNOP			6			\t\n"
		// addr_B for next loop_m:
		"		SADD.M1     	R52, %1, %1  			;; %1 += N*4*8	   		\t\n"
		"		SADD.M1     	R52, R46, R46			;; R46 += N*4*8	   		\t\n"
		
		"		[R3] SBR  	 	loop_m_V1	\t\n" // end outer loop
		"		SNOP			6			\t\n"

		// VSTDW last data:
			// AR5
		"		VSTDW			VR13:VR12, *AR5++[OR5]						\t\n"
		"	|	VSTDW			VR15:VR14, *+AR5[16]						\t\n"

		"		VSTDW			VR25:VR24, *AR5--[OR5]						\t\n"
		"	|	VSTDW			VR27:VR26, *+AR5[16]						\t\n"
		// 	// AR6
		"		VSTDW			VR37:VR36, *AR6++[OR5]						\t\n"
		"	|	VSTDW			VR39:VR38, *+AR6[16]						\t\n"

		"		VSTDW			VR49:VR48, *AR6--[OR5]						\t\n"
		"	|	VSTDW			VR51:VR50, *+AR6[16]						\t\n"

		"		SBR				R63					\t\n"
		"		SNOP			6					\t\n"
    :
    :"r"(src_a), "r"(dst_b), "r"(m), "r"(n), "r"(N)
    );
}


// 部分优化版本
void micro_kernel_cross_reduce_V2(lvector double* src_a, lvector double* dst_b, unlong m, unlong n, unlong N){
	__asm__ __volatile__(
		// %0:src_a (源数据首地址),    %1:dst_b (目的数据首地址),
		// %2:m (目的数据行数),    %3:n (目的数据列数),    %4:N (目的数据buffer区行长度)
		
		// 常量初始化：
		"		VMOVI			0x3FF0000000000000, VR63  ;; VR63 = 1			\t\n"
		"		VMOVI			0x3FC999999999999A, VR62  ;; VR62 = 0.2			\t\n"
		"		VMOVI			0x00, VR61  			;; VR61 = 0.0			\t\n"
		"		SMOVI24         64, R59                 ;; R59 = 64				\t\n"
		"		SMOVI24         512, R61                ;; R61 = 16*4*8			\t\n"
		"		SADD.M1         2, %4, R26              ;; R26 = N+2			\t\n"

		"		SSHFLL          3, R26, R55             ;; R55 = (N+2)*8		\t\n"
		"		SSHFLL          4, R26, R56             ;; R56 = (N+2)*2*8		\t\n"
		"		SSHFLL          5, R26, R57             ;; R57 = (N+2)*4*8		\t\n"
		"		SADD.M1			R55, R56, R58			;; R58 = (N+2)*3*8		\t\n"
		"		SSHFLL          4, %4, R51              ;; R51 = N*2*8			\t\n"
		"		SSHFLL          5, %4, R52              ;; R52 = N*4*8			\t\n"

		"		SSHFLR          1, R26, R25             ;; R25 = (N+2)*1/2		\t\n"
		"		SADD.M1         R26, R25, R27           ;; R27 = (N+2)*3/2		\t\n"
		"		SSHFLL			1, R27, R28				;; R28 = (N+2)*3		\t\n"
		"		SSHFLR          1, %4, R29              ;; R29 = N * 1/2		\t\n"

		"		SMVAGA.M1       R26, OR2                ;; OR2 = (N+2)			\t\n"
		"		SMVAGA.M1       R25, OR1                ;; OR1 = (N+2)*1/2		\t\n"
		"		SMVAGA.M1       R27, OR3                ;; OR3 = (N+2)*3/2		\t\n"
		"		SMVAGA.M1       R28, OR4                ;; OR4 = (N+2)*3		\t\n"
		"		SMVAGA.M1       R29, OR5                ;; OR5 = N * 1/2		\t\n"

		// 地址初始化：
		"		SADD.M1         8, %0, R42              ;; R42 = src_A + 8		\t\n"
		"		SADD.M1         R55, %0, R43            ;; R43 = src_A + (N+2)*8\t\n"
		"		SADD.M1         R56, R42, R45           ;; R45 = R42 + (N+2)*2*8\t\n"
		"		SADD.M1         16, R43, R44            ;; R44 = R43 + 16		\t\n"
		"		SADD.M1         R51, %1, R46            ;; R46 = dst_B + N*2*8	\t\n"
		
		"		SMVAGA.M1       R42, AR0				\t\n"
		"		SMVAGA.M1       R43, AR1				\t\n"
		"		SMVAGA.M1       R44, AR2				\t\n"
		"		SMVAGA.M1       R45, AR3				\t\n"
		"		SMVAGA.M1       %1, AR5 				\t\n"
		"		SMVAGA.M1       R46, AR6				\t\n"
		"		SMOVI24         0, R1					\t\n"

		// 加载第一批数据：
			// AR1
		"		VLDDW			*AR1++[OR1], VR5:VR4						\t\n"
		"	|	VLDDW			*+AR1[16], VR7:VR6							\t\n"

		"		VLDDW			*AR1++[OR1], VR17:VR16						\t\n"
		"	|	VLDDW			*+AR1[16], VR19:VR18						\t\n"

		"		VLDDW			*AR1++[OR1], VR29:VR28						\t\n"
		"	|	VLDDW			*+AR1[16], VR31:VR30						\t\n"

		"		VLDDW			*AR1--[OR3], VR41:VR40						\t\n"
		"	|	VLDDW			*+AR1[16], VR43:VR42						\t\n"
			// AR0
		"		VLDW			*+AR0[32], VR2								\t\n"
		"	|	VLDW			*+AR0[48], VR3								\t\n"
		"		VLDW			*AR0++[OR2], VR0							\t\n"
		"	|	VLDW			*+AR0[16], VR1								\t\n"

		"		VLDW			*+AR0[32], VR10								\t\n"
		"	|	VLDW			*+AR0[48], VR11								\t\n"
		"		VLDW			*AR0--[OR2], VR8							\t\n"
		"	|	VLDW			*+AR0[16], VR9								\t\n"
			// AR3
		"		VLDW			*+AR3[32], VR22								\t\n"
		"	|	VLDW			*+AR3[48], VR23								\t\n"
		"		VLDW			*AR3++[OR2], VR20							\t\n"
		"	|	VLDW			*+AR3[16], VR21								\t\n"

		"		VLDW			*+AR3[32], VR34								\t\n"
		"	|	VLDW			*+AR3[48], VR35								\t\n"
		"		VLDW			*AR3++[OR2], VR32							\t\n"
		"	|	VLDW			*+AR3[16], VR33								\t\n"

		"		VLDW			*+AR3[32], VR46								\t\n"
		"	|	VLDW			*+AR3[48], VR47								\t\n"
		"		VLDW			*AR3++[OR2], VR44							\t\n"
		"	|	VLDW			*+AR3[16], VR45								\t\n"

		"		VLDW			*+AR3[32], VR54								\t\n"
		"	|	VLDW			*+AR3[48], VR55								\t\n"
		"		VLDW			*AR3--[OR4], VR52							\t\n"
		"	|	VLDW			*+AR3[16], VR53								\t\n"

		// outer loop:
		"		SMOVI24         0, R7                   ;; i=0					\t\n"
		"loop_m_V2:	\t\n"
		"		SADD.M1    		4, R7, R7               ;; i += 4				\t\n"
		"		SLT     		R7, %2, R3              ;; if i<m				\t\n"
		
		// addr_A for next loop_m:
		"		SADD.M1     	R57, R42, R42			;; R42 += (N+2)*4*8		\t\n"
		"		SADD.M1     	R57, R43, R43			;; R43 += (N+2)*4*8		\t\n"
		"		SADD.M1     	R57, R44, R44			;; R44 += (N+2)*4*8		\t\n"
		"		SADD.M1     	R57, R45, R45			;; R45 += (N+2)*4*8		\t\n"

		// inner loop:
		"		SMOVI24     	0, R8                   ;; j=0					\t\n"
		"loop_n_V2:	\t\n"
		"		SADD.M1    		R59, R8, R8             ;; j += 64				\t\n"
		"		SLT     		R8, %3, R0              ;; if j<n				\t\n"
		
		// ---------------------------------------------------------------
		// VFMULAD fist round
		"		VFMULAD.M1		VR0, VR63, VR4, VR4			\t\n" // [1]
		"	|	VFMULAD.M2		VR1, VR63, VR5, VR5			\t\n"
		"	|	VFMULAD.M3		VR2, VR63, VR6, VR6			\t\n"
		
		"		VFMULAD.M1		VR3, VR63, VR7, VR7			\t\n"
		"	|	VFMULAD.M2		VR20, VR63, VR8, VR8 		\t\n" // [2]
		"	|	VFMULAD.M3		VR21, VR63, VR9, VR9 		\t\n"
		"	|	VSTDW			VR13:VR12, *AR5++[OR5]							\t\n" // store pre to AR5
		"	|	VSTDW			VR15:VR14, *+AR5[16]							\t\n" // store pre to AR5
		
		"		VFMULAD.M1		VR22, VR63, VR10, VR10		\t\n"
		"	|	VFMULAD.M2		VR23, VR63, VR11, VR11		\t\n"
		"	|	VFMULAD.M3		VR32, VR63, VR16, VR16		\t\n" // [5]
		"	|	VSTDW			VR25:VR24, *AR5--[OR5]							\t\n" // store pre to AR5
		"	|	VSTDW			VR27:VR26, *+AR5[16]							\t\n" // store pre to AR5
		
		"		VFMULAD.M1		VR33, VR63, VR17, VR17		\t\n"
		"	|	VFMULAD.M2		VR34, VR63, VR18, VR18		\t\n"
		"	|	VFMULAD.M3		VR35, VR63, VR19, VR19		\t\n"
		"	|	VSTDW			VR37:VR36, *AR6++[OR5]							\t\n" // store pre to AR6
		"	|	VSTDW			VR39:VR38, *+AR6[16]							\t\n" // store pre to AR6

		"		VFMULAD.M1		VR20, VR63, VR28, VR28		\t\n" // [8]
		"	|	VFMULAD.M2		VR21, VR63, VR29, VR29		\t\n"
		"	|	VFMULAD.M3		VR22, VR63, VR30, VR30		\t\n"
		"	|	VSTDW			VR49:VR48, *AR6--[OR5]							\t\n" // store pre to AR6
		"	|	VSTDW			VR51:VR50, *+AR6[16]							\t\n" // store pre to AR6
		
		"		VLDDW			*AR2++[OR1], VR13:VR12							\t\n" // load current from AR2
		"	|	VLDDW			*+AR2[16], VR15:VR14							\t\n" // load current from AR2
		"	|	VFMULAD.M1		VR23, VR63, VR31, VR31		\t\n"
		"	|	VFMULAD.M2		VR44, VR63, VR32, VR32		\t\n" // [9]
		"	|	VFMULAD.M3		VR45, VR63, VR33, VR33		\t\n"
		"	|	[R0] SADDA.M1   		R61, AR1, AR1		;; AR1 += 16*4*8	\t\n" // if(j_cur < n)
		"	|	[!R0] SMVAGA.M2         R43, AR1            ;; AR1_next = R43	\t\n" // else if(j_cur >= n)
		
		"		VLDDW			*AR2++[OR1], VR25:VR24							\t\n" // load current from AR2
		"	|	VLDDW			*+AR2[16], VR27:VR26							\t\n" // load current from AR2
		"	|	VFMULAD.M1		VR46, VR63, VR34, VR34		\t\n"
		"	|	VFMULAD.M2		VR47, VR63, VR35, VR35		\t\n"
		"	|	VFMULAD.M3		VR52, VR63, VR40, VR40		\t\n" // [12]
		"	|	[R0] SADDA.M1   		R61, AR0, AR0		;; AR0 += 16*4*8	\t\n" // if(j_cur < n)
		"	|	[!R0] SMVAGA.M2         R42, AR0            ;; AR0_next = R42	\t\n" // else if(j_cur >= n)

		"		VLDDW			*AR2++[OR1], VR37:VR36							\t\n" // load current from AR2
		"	|	VLDDW			*+AR2[16], VR39:VR38							\t\n" // load current from AR2
		"	|	VFMULAD.M1		VR53, VR63, VR41, VR41		\t\n"
		"	|	VFMULAD.M2		VR54, VR63, VR42, VR42		\t\n"
		"	|	VFMULAD.M3		VR55, VR63, VR43, VR43		\t\n"
		"	|	[R0] SADDA.M1   		R61, AR3, AR3		;; AR3 += 16*4*8	\t\n" // if(j_cur < n)
		"	|	[!R0] SMVAGA.M2         R45, AR3            ;; AR3_next = R45	\t\n" // else if(j_cur >= n)
		
		// VFMULAD second round
		"		VLDDW			*AR2--[OR3], VR49:VR48							\t\n" // load current from AR2
		"	|	VLDDW			*+AR2[16], VR51:VR50							\t\n" // load current from AR2
		"	|	VFMULAD.M1		VR8 , VR63, VR4, VR4		\t\n" // [3]
		"	|	VFMULAD.M2		VR9 , VR63, VR5, VR5		\t\n"
		"	|	VFMULAD.M3		VR10, VR63, VR6, VR6		\t\n"
		"	|	[R1] SADDA.M1   		R61, AR5, AR5       ;; AR5 += 16*4*8	\t\n" // if(j_pre < n)
		"	|	[!R1] SMVAGA.M2         %1, AR5             ;; AR5_next = %1	\t\n" // else if(j_pre >= n)
		
		"		VFMULAD.M1		VR11, VR63, VR7, VR7		\t\n"
		"	|	VFMULAD.M2		VR8 , VR63, VR16, VR16		\t\n" // [6]
		"	|	VFMULAD.M3		VR9 , VR63, VR17, VR17		\t\n"
		"	|	VLDDW			*AR1++[OR1], VR5:VR4							\t\n" // load next from AR1
		"	|	VLDDW			*+AR1[16], VR7:VR6								\t\n" // load next from AR1
		"	|	[R1] SADDA.M1   		R61, AR6, AR6       ;; AR6 += 16*4*8	\t\n" // if(j_pre < n)
		"	|	[!R1] SMVAGA.M2         R46, AR6            ;; AR6_next = R46	\t\n" // else if(j_pre >= n)
		
		"		VFMULAD.M1		VR10, VR63, VR18, VR18		\t\n"
		"	|	VFMULAD.M2		VR11, VR63, VR19, VR19		\t\n"
		"	|	VLDW			*AR0, VR0										\t\n" // load next from AR0
		"	|	VLDW			*+AR0[16], VR1									\t\n" // load next from AR0
		"	|	SADDA.M1		R55, AR0, AR0									\t\n" // AR0 += (N+2)*8
		
		"		VFMULAD.M1		VR32, VR63, VR28, VR28		\t\n" // [10]
		"	|	VFMULAD.M2		VR33, VR63, VR29, VR29		\t\n"
		"	|	VLDW			*+AR0[32], VR2									\t\n" // load next from AR0
		"	|	VLDW			*+AR0[48], VR3									\t\n" // load next from AR0
		
		"		VFMULAD.M1		VR34, VR63, VR30, VR30		\t\n"
		"	|	VFMULAD.M2		VR35, VR63, VR31, VR31		\t\n"
		"	|	VFMULAD.M3		VR32, VR63, VR40, VR40		\t\n" // [13]
		"	|	VLDW			*AR3, VR20										\t\n" // load next from AR3
		"	|	VLDW			*+AR3[16], VR21									\t\n" // load next from AR3
		"	|	SADDA.M1		R55, AR3, AR3									\t\n" // AR3 += (N+2)*8
		
		"		VFMULAD.M1		VR33, VR63, VR41, VR41		\t\n"
		"	|	VFMULAD.M2		VR34, VR63, VR42, VR42		\t\n"
		"	|	VFMULAD.M3		VR35, VR63, VR43, VR43		\t\n"
		"	|	VLDW			*+AR3[32], VR22									\t\n" // load next from AR3
		"	|	VLDW			*+AR3[48], VR23									\t\n" // load next from AR3
		
		// VFMULAD third round
		"		VFMULAD.M1		VR4, VR63, VR12, VR12		\t\n" // [4]
		"	|	VFMULAD.M2		VR5, VR63, VR13, VR13		\t\n"
		"	|	VFMULAD.M3		VR6, VR63, VR14, VR14		\t\n"
		"	|	VLDW			*AR0, VR8										\t\n" // load next from AR0
		"	|	VLDW			*+AR0[16], VR9									\t\n" // load next from AR0
		"	|	SSUBA.M1		R55, AR0, AR0									\t\n" // AR0 -= (N+2)*8
		
		"		VFMULAD.M1		VR7, VR63, VR15, VR15		\t\n"
		"	|	VFMULAD.M2		VR16, VR63, VR24, VR24		\t\n" // [7]
		"	|	VFMULAD.M3		VR17, VR63, VR25, VR25		\t\n"
		"	|	VLDW			*+AR0[32], VR10									\t\n" // load next from AR0
		"	|	VLDW			*+AR0[48], VR11									\t\n" // load next from AR0

		"		VFMULAD.M1		VR18, VR63, VR26, VR26		\t\n"
		"	|	VFMULAD.M2		VR19, VR63, VR27, VR27		\t\n"
		"	|	VLDDW			*AR1++[OR1], VR17:VR16							\t\n" // load next from AR1
		"	|	VLDDW			*+AR1[16], VR19:VR18							\t\n" // load next from AR1
		"	|	[R0] SADDA.M1   		R61, AR2, AR2		;; AR2 += 16*4*8	\t\n" // if(j_cur < n)
		"	|	[!R0] SMVAGA.M2         R44, AR2            ;; AR2_next = R44	\t\n" // else if(j_cur >= n)
		
		"		VFMULAD.M1		VR28, VR63, VR36, VR36		\t\n" // [11]
		"	|	VFMULAD.M2		VR29, VR63, VR37, VR37		\t\n"
		"	|	VLDW			*AR3, VR32										\t\n" // load next from AR3
		"	|	VLDW			*+AR3[16], VR33									\t\n" // load next from AR3
		"	|	SADDA.M1		R55, AR3, AR3									\t\n" // AR3 += (N+2)*8
		
		"		VFMULAD.M1		VR30, VR63, VR38, VR38		\t\n"
		"	|	VFMULAD.M2		VR31, VR63, VR39, VR39		\t\n"
		"	|	VFMULAD.M3		VR40, VR63, VR48, VR48		\t\n" // [14]
		"	|	VLDW			*+AR3[32], VR34									\t\n" // load next from AR3
		"	|	VLDW			*+AR3[48], VR35									\t\n" // load next from AR3

		"		VFMULAD.M1		VR41, VR63, VR49, VR49		\t\n"
		"	|	VFMULAD.M2		VR42, VR63, VR50, VR50		\t\n"
		"	|	VFMULAD.M3		VR43, VR63, VR51, VR51		\t\n"
		"	|	VLDDW			*AR1++[OR1], VR29:VR28							\t\n" // load next from AR1
		"	|	VLDDW			*+AR1[16], VR31:VR30							\t\n" // load next from AR1
		
		// VFMULAD fourth round
		"		VFMULAD.M1		VR12, VR62, VR61, VR12		\t\n" // [15]
		"	|	VFMULAD.M2		VR13, VR62, VR61, VR13		\t\n"
		"	|	VFMULAD.M3		VR14, VR62, VR61, VR14		\t\n"
		"	|	VLDW			*AR3, VR44										\t\n" // load next from AR3
		"	|	VLDW			*+AR3[16], VR45									\t\n" // load next from AR3
		"	|	SADDA.M1		R55, AR3, AR3									\t\n" // AR3 += (N+2)*8
		
		"		VFMULAD.M1		VR15, VR62, VR61, VR15		\t\n"
		"	|	VFMULAD.M2		VR24, VR62, VR61, VR24		\t\n" // [16]
		"	|	VFMULAD.M3		VR25, VR62, VR61, VR25		\t\n"
		"	|	VLDW			*+AR3[32], VR46									\t\n" // load next from AR3
		"	|	VLDW			*+AR3[48], VR47									\t\n" // load next from AR3
		
		"		VFMULAD.M1		VR26, VR62, VR61, VR26		\t\n"
		"	|	VFMULAD.M2		VR27, VR62, VR61, VR27		\t\n"
		"	|	VLDDW			*AR1--[OR3], VR41:VR40							\t\n" // load next from AR0
		"	|	VLDDW			*+AR1[16], VR43:VR42							\t\n" // load next from AR0
		
		"		VFMULAD.M1		VR36, VR62, VR61, VR36		\t\n" // [17]
		"	|	VFMULAD.M2		VR37, VR62, VR61, VR37		\t\n"
		"	|	VLDW			*AR3, VR52										\t\n" // load next from AR3
		"	|	VLDW			*+AR3[16], VR53									\t\n" // load next from AR3
		"	|	SSUBA.M1		R58, AR3, AR3									\t\n" // AR3 -= (N+2)*3*8
		
		"		VFMULAD.M1		VR38, VR62, VR61, VR38		\t\n"
		"	|	VFMULAD.M2		VR39, VR62, VR61, VR39		\t\n"
		"	|	VFMULAD.M3		VR48, VR62, VR61, VR48		\t\n" // [18]
		"	|	VLDW			*+AR3[32], VR54								\t\n" // load next from AR3
		"	|	VLDW			*+AR3[48], VR55								\t\n" // load next from AR3
		
		"		VFMULAD.M1		VR49, VR62, VR61, VR49		\t\n"
		"	|	VFMULAD.M2		VR50, VR62, VR61, VR50		\t\n"
		"	|	VFMULAD.M3		VR51, VR62, VR61, VR51		\t\n"
		
		// ---------------------------------------------------------------
		"		SMOV			R0, R1		\t\n"
		"		[R0] SBR    	loop_n_V2	\t\n" // end inner loop
		"		SNOP			6			\t\n"
		// addr_B for next loop_m:
		"		SADD.M1     	R52, %1, %1  			;; %1 += N*4*8	   		\t\n"
		"		SADD.M1     	R52, R46, R46			;; R46 += N*4*8	   		\t\n"
		
		"		[R3] SBR  	 	loop_m_V2	\t\n" // end outer loop
		"		SNOP			6			\t\n"

		// VSTDW last data:
			// AR5
		"		VSTDW			VR13:VR12, *AR5++[OR5]						\t\n"
		"	|	VSTDW			VR15:VR14, *+AR5[16]						\t\n"

		"		VSTDW			VR25:VR24, *AR5--[OR5]						\t\n"
		"	|	VSTDW			VR27:VR26, *+AR5[16]						\t\n"
		// 	// AR6
		"		VSTDW			VR37:VR36, *AR6++[OR5]						\t\n"
		"	|	VSTDW			VR39:VR38, *+AR6[16]						\t\n"

		"		VSTDW			VR49:VR48, *AR6--[OR5]						\t\n"
		"	|	VSTDW			VR51:VR50, *+AR6[16]						\t\n"

		"		SBR				R63					\t\n"
		"		SNOP			6					\t\n"
    :
    :"r"(src_a), "r"(dst_b), "r"(m), "r"(n), "r"(N)
    );
}



// 终极优化版本
// total_cycles = 20 + (m/4)*(n/64)*26 + 6
void micro_kernel_cross_reduce_V3(lvector double* src_a, lvector double* dst_b, unlong m, unlong n, unlong N){
	__asm__ __volatile__(
		// %0:src_a (源数据首地址),    %1:dst_b (目的数据首地址),
		// %2:m (目的数据行数),    %3:n (目的数据列数),    %4:N (目的数据buffer区行长度)

			// [-3]
		"		SADD.M1         2, %4, R26              ;; R26 = N+2			\t\n" // [key variable]
		"	|	SADD.M2         8, %0, R42              ;; R42 = src_A + 8		\t\n" // [key variable]
		"	|	VMOVI			0x00, VR61  			;; VR61 = 0.0			\t\n"
			// [-2]
		"		SSHFLL          4, R26, R56             ;; R56 = (N+2)*2*8		\t\n" // [key variable]
		"	|	SMVAGA.M2       R42, AR0										\t\n" // [key variable]
		"	|	VMOVI			0x00, VR61  			;; VR61 = 0.0			\t\n" // 无效指令 
			// [-1]
		"		SSHFLL          3, R26, R55             ;; R55 = (N+2)*8		\t\n" // [key variable]
		"	|	SADD.M1         R56, R42, R45           ;; R45 = R42 + (N+2)*2*8\t\n" // [key variable]
		"	|	VMOVI			0x00, VR61  			;; VR61 = 0.0			\t\n" // 无效指令 
		// 加载第一批数据：
			// [0]
		"		SSHFLR          1, R26, R25             ;; R25 = (N+2)*1/2		\t\n" // [key variable]
		"	|	SADD.M1         R55, %0, R43            ;; R43 = src_A + (N+2)*8\t\n" // [key variable]
		"	|	SMVAGA.M2       R45, AR3										\t\n" // [key variable]
		"	|	VLDW			*+AR0[0], VR0									\t\n" // load next from AR0
		"	|	VLDW			*+AR0[32], VR2									\t\n" // load next from AR0
			// [1]
		"		SSHFLL          5, R26, R57             ;; R57 = (N+2)*4*8		\t\n"
		"	|	SMVAGA.M1       R25, OR1                ;; OR1 = (N+2)*1/2		\t\n" // [key variable]
		"	|	SMVAGA.M2       R43, AR1										\t\n" // [key variable]
		"	|	VLDW			*+AR0[16], VR1									\t\n" // load next from AR0
		"	|	VLDW			*+AR0[48], VR3									\t\n" // load next from AR0
			// [2]
		"		SSHFLL          4, %4, R51              ;; R51 = N*2*8			\t\n"
		"	|	VLDW			*+AR3[0], VR20									\t\n" // load next from AR3
		"	|	VLDW			*+AR3[32], VR22									\t\n" // load next from AR3
		"	|	SADDA.M1		R55, AR0, AR0									\t\n" // AR0 += (N+2)*8
		"	|	SADDA.M2		R55, AR3, AR3									\t\n" // AR3 += (N+2)*8
			// [3]
		"		SADD.M1         R26, R25, R27           ;; R27 = (N+2)*3/2		\t\n"
		"	|	SADD.M2         16, R43, R44            ;; R44 = R43 + 16		\t\n"
		"	|	VLDW			*+AR3[16], VR21									\t\n" // load next from AR3
		"	|	VLDW			*+AR3[48], VR23									\t\n" // load next from AR3
			// [4]
		"		SADD.M1			R55, R56, R58			;; R58 = (N+2)*3*8		\t\n"
		"	|	SMVAGA.M2       R27, OR3                ;; OR3 = (N+2)*3/2		\t\n"
		"	|	VLDDW			*AR1++[OR1], VR5:VR4							\t\n" // load next from AR1
		"	|	VLDDW			*+AR1[16], VR7:VR6								\t\n" // load next from AR1
			// [5]
		"		VLDW			*+AR0[0], VR8									\t\n" // load next from AR0
		"	|	VLDW			*+AR0[32], VR10									\t\n" // load next from AR0
		"	|	SSUBA.M1		R55, AR0, AR0									\t\n" // AR0 -= (N+2)*8
			// [6]
		"		SSHFLL          5, %4, R52              ;; R52 = N*4*8			\t\n"
		"	|	SADD.M1         R51, %1, R46            ;; R46 = dst_B + N*2*8	\t\n"
		"	|	SMVAGA.M2       R44, AR2										\t\n"
		"	|	VLDW			*+AR0[16], VR9									\t\n" // load next from AR0
		"	|	VLDW			*+AR0[48], VR11									\t\n" // load next from AR0
			// [7]
		"		SSHFLR          1, %4, R29              ;; R29 = N * 1/2		\t\n"
		"	|	SMVAGA.M1       %1, AR5 										\t\n"
		"	|	SMVAGA.M2       R46, AR6										\t\n"
		"	|	VLDDW			*AR1++[OR1], VR17:VR16							\t\n" // load next from AR1
		"	|	VLDDW			*+AR1[16], VR19:VR18							\t\n" // load next from AR1
			// [8]
		"		SMOVI24         0, R7                   ;; i=0					\t\n"
		"	|	VLDW			*+AR3[0], VR32									\t\n" // load next from AR3
		"	|	VLDW			*+AR3[32], VR34									\t\n" // load next from AR3
		"	|	SADDA.M1		R55, AR3, AR3									\t\n" // AR3 += (N+2)*8
		"	|	SMVAGA.M2       R29, OR5                ;; OR5 = N * 1/2		\t\n"
			// [9]
		"		SMOVI24         0, R0					\t\n"
		"	|	VLDW			*+AR3[16], VR33									\t\n" // load next from AR3
		"	|	VLDW			*+AR3[48], VR35									\t\n" // load next from AR3
			// [10]
		"		SMOVI24         0, R1					\t\n"
		"	|	VLDDW			*AR1++[OR1], VR29:VR28							\t\n" // load next from AR1
		"	|	VLDDW			*+AR1[16], VR31:VR30							\t\n" // load next from AR1
			// [11]
		"		SMOVI24         1, R3					\t\n"
		"	|	VMOVI			0x3FF0000000000000, VR63  ;; VR63 = 1			\t\n"
		"	|	VLDW			*+AR3[0], VR44									\t\n" // load next from AR3
		"	|	VLDW			*+AR3[32], VR46									\t\n" // load next from AR3
		"	|	SADDA.M1		R55, AR3, AR3									\t\n" // AR3 += (N+2)*8
			// [12]
		"		SMOVI24         64, R59                 ;; R59 = 64				\t\n"
		"	|	VMOVI			0x3FC999999999999A, VR62  ;; VR62 = 0.2			\t\n"
		"	|	VLDW			*+AR3[16], VR45									\t\n" // load next from AR3
		"	|	VLDW			*+AR3[48], VR47									\t\n" // load next from AR3
			// [13]
		"		SMOVI24         512, R61                ;; R61 = 16*4*8			\t\n"
		"	|	VMOVI			0x00, VR61  			;; VR61 = 0.0			\t\n"
		"	|	VLDDW			*AR1--[OR3], VR41:VR40							\t\n" // load next from AR0
		"	|	VLDDW			*+AR1[16], VR43:VR42							\t\n" // load next from AR0
			// [14]
		"		VLDW			*+AR3[0], VR52									\t\n" // load next from AR3
		"	|	VLDW			*+AR3[32], VR54									\t\n" // load next from AR3
		"	|	SSUBA.M1		R58, AR3, AR3									\t\n" // AR3 -= (N+2)*3*8
			// [15]
		"		VLDW			*+AR3[16], VR53									\t\n" // load next from AR3
		"	|	VLDW			*+AR3[48], VR55									\t\n" // load next from AR3
			// [16]
		"		SNOP			1			\t\n"

		"loop_V3:						\t\n"
		// ================================== MAIN LOOP ==================================	
			// [0] VFMULAD fist round
		"		[!R0] SMOVI24     	0, R8                   ;; j=0				\t\n"
		"	|	[!R0] SADD.M1     	R57, R42, R42			;; R42 += (N+2)*4*8	\t\n"
		"	|	[!R0] SADD.M2     	R57, R43, R43			;; R43 += (N+2)*4*8	\t\n"
		"	|	VFMULAD.M1		VR0, VR63, VR4, VR4			\t\n" // [readme:1]
		"	|	VFMULAD.M2		VR1, VR63, VR5, VR5			\t\n"
		"	|	VFMULAD.M3		VR2, VR63, VR6, VR6			\t\n"
			// [1]
		"		[!R0] SADD.M1     	R57, R44, R44			;; R44 += (N+2)*4*8	\t\n"
		"	|	[!R0] SADD.M2     	R57, R45, R45			;; R45 += (N+2)*4*8	\t\n"
		"	|	VSTDW			VR13:VR12, *AR5++[OR5]							\t\n" // store pre to AR5
		"	|	VSTDW			VR15:VR14, *+AR5[16]							\t\n" // store pre to AR5
		"	|	VFMULAD.M1		VR3, VR63, VR7, VR7			\t\n"
		"	|	VFMULAD.M2		VR20, VR63, VR8, VR8 		\t\n" // [readme:2]
		"	|	VFMULAD.M3		VR21, VR63, VR9, VR9 		\t\n"
			// [2]
		"		SADD.M1    		R59, R8, R8             	;; j += 64			\t\n"
		"	|	[!R0] SADD.M2    	4, R7, R7               ;; i += 4			\t\n"
		"	|	VSTDW			VR25:VR24, *AR5--[OR5]							\t\n" // store pre to AR5
		"	|	VSTDW			VR27:VR26, *+AR5[16]							\t\n" // store pre to AR5
		"	|	VFMULAD.M1		VR22, VR63, VR10, VR10		\t\n"
		"	|	VFMULAD.M2		VR23, VR63, VR11, VR11		\t\n"
		"	|	VFMULAD.M3		VR32, VR63, VR16, VR16		\t\n" // [readme:5]
			// [3]
		"		SLT     		R8, %3, R0              	;; if j<n			\t\n"
		"	|	VLDDW			*AR2++[OR1], VR13:VR12							\t\n" // load current from AR2
		"	|	VLDDW			*+AR2[16], VR15:VR14							\t\n" // load current from AR2
		"	|	VFMULAD.M1		VR33, VR63, VR17, VR17		\t\n"
		"	|	VFMULAD.M2		VR34, VR63, VR18, VR18		\t\n"
		"	|	VFMULAD.M3		VR35, VR63, VR19, VR19		\t\n"
			// [4]
		"		SLT     		R7, %2, R3              	;; if i<m			\t\n"
		"	|	VLDDW			*AR2++[OR1], VR25:VR24							\t\n" // load current from AR2
		"	|	VLDDW			*+AR2[16], VR27:VR26							\t\n" // load current from AR2
		"	|	VFMULAD.M1		VR20, VR63, VR28, VR28		\t\n" // [readme:8]
		"	|	VFMULAD.M2		VR21, VR63, VR29, VR29		\t\n"
		"	|	VFMULAD.M3		VR22, VR63, VR30, VR30		\t\n"
		"	|	[R0] SADDA.M1   		R61, AR1, AR1		;; AR1 += 16*4*8	\t\n" // if(j_cur < n)
		"	|	[!R0] SMVAGA.M2         R43, AR1            ;; AR1_next = R43	\t\n" // else if(j_cur >= n)
			// [5]
		"		VSTDW			VR37:VR36, *AR6++[OR5]							\t\n" // store pre to AR6
		"	|	VSTDW			VR39:VR38, *+AR6[16]							\t\n" // store pre to AR6
		"	|	VFMULAD.M1		VR23, VR63, VR31, VR31		\t\n"
		"	|	VFMULAD.M2		VR44, VR63, VR32, VR32		\t\n" // [readme:9]
		"	|	VFMULAD.M3		VR45, VR63, VR33, VR33		\t\n"
		"	|	[R0] SADDA.M1   		R61, AR0, AR0		;; AR0 += 16*4*8	\t\n" // if(j_cur < n)
		"	|	[!R0] SMVAGA.M2         R42, AR0            ;; AR0_next = R42	\t\n" // else if(j_cur >= n)
			// [6]
		"		VSTDW			VR49:VR48, *AR6--[OR5]							\t\n" // store pre to AR6
		"	|	VSTDW			VR51:VR50, *+AR6[16]							\t\n" // store pre to AR6
		"	|	VFMULAD.M1		VR46, VR63, VR34, VR34		\t\n"
		"	|	VFMULAD.M2		VR47, VR63, VR35, VR35		\t\n"
		"	|	VFMULAD.M3		VR52, VR63, VR40, VR40		\t\n" // [readme:12]
		"	|	[R0] SADDA.M1   		R61, AR3, AR3		;; AR3 += 16*4*8	\t\n" // if(j_cur < n)
		"	|	[!R0] SMVAGA.M2         R45, AR3            ;; AR3_next = R45	\t\n" // else if(j_cur >= n)
			// [7]
		"		VLDDW			*AR2++[OR1], VR37:VR36							\t\n" // load current from AR2
		"	|	VLDDW			*+AR2[16], VR39:VR38							\t\n" // load current from AR2
		"	|	VFMULAD.M1		VR53, VR63, VR41, VR41		\t\n"
		"	|	VFMULAD.M2		VR54, VR63, VR42, VR42		\t\n"
		"	|	VFMULAD.M3		VR55, VR63, VR43, VR43		\t\n"
		"	|	SADD.M1			R0, R3, R4					\t\n" // R4 = R0 + R3
			// [8] VFMULAD second round
		"		VLDDW			*AR2--[OR3], VR49:VR48							\t\n" // load current from AR2
		"	|	VLDDW			*+AR2[16], VR51:VR50							\t\n" // load current from AR2
		"	|	VFMULAD.M1		VR8 , VR63, VR4, VR4		\t\n" // [readme:3]
		"	|	VFMULAD.M2		VR9 , VR63, VR5, VR5		\t\n"
		"	|	VFMULAD.M3		VR10, VR63, VR6, VR6		\t\n"
		"	|	[R1] SADDA.M1   		R61, AR5, AR5       ;; AR5 += 16*4*8	\t\n" // if(j_pre < n)
		"	|	[!R1] SMVAGA.M2         %1, AR5             ;; AR5_next = %1	\t\n" // else if(j_pre >= n)
			// [9]
		"		VFMULAD.M1		VR11, VR63, VR7, VR7		\t\n"
		"	|	VFMULAD.M2		VR8 , VR63, VR16, VR16		\t\n" // [readme:6]
		"	|	VFMULAD.M3		VR9 , VR63, VR17, VR17		\t\n"
		"	|	VLDDW			*AR1++[OR1], VR5:VR4							\t\n" // load next from AR1
		"	|	VLDDW			*+AR1[16], VR7:VR6								\t\n" // load next from AR1
		"	|	[R1] SADDA.M1   		R61, AR6, AR6       ;; AR6 += 16*4*8	\t\n" // if(j_pre < n)
		"	|	[!R1] SMVAGA.M2         R46, AR6            ;; AR6_next = R46	\t\n" // else if(j_pre >= n)
			// [10]
		"		VFMULAD.M1		VR10, VR63, VR18, VR18		\t\n"
		"	|	VFMULAD.M2		VR11, VR63, VR19, VR19		\t\n"
		"	|	VLDW			*+AR0[0], VR0									\t\n" // load next from AR0
		"	|	VLDW			*+AR0[32], VR2									\t\n" // load next from AR0
		"	|	SADDA.M1		R55, AR0, AR0									\t\n" // AR0 += (N+2)*8
			// [11]
		"		VFMULAD.M1		VR32, VR63, VR28, VR28		\t\n" // [readme:10]
		"	|	VFMULAD.M2		VR33, VR63, VR29, VR29		\t\n"
		"	|	VLDW			*+AR0[16], VR1									\t\n" // load next from AR0
		"	|	VLDW			*+AR0[48], VR3									\t\n" // load next from AR0
			// [12]
		"		VFMULAD.M1		VR34, VR63, VR30, VR30		\t\n"
		"	|	VFMULAD.M2		VR35, VR63, VR31, VR31		\t\n"
		"	|	VFMULAD.M3		VR32, VR63, VR40, VR40		\t\n" // [readme:13]
		"	|	VLDW			*+AR3[0], VR20									\t\n" // load next from AR3
		"	|	VLDW			*+AR3[32], VR22									\t\n" // load next from AR3
		"	|	SADDA.M1		R55, AR3, AR3									\t\n" // AR3 += (N+2)*8
			// [13]
		"		VFMULAD.M1		VR33, VR63, VR41, VR41		\t\n"
		"	|	VFMULAD.M2		VR34, VR63, VR42, VR42		\t\n"
		"	|	VFMULAD.M3		VR35, VR63, VR43, VR43		\t\n"
		"	|	VLDW			*+AR3[16], VR21									\t\n" // load next from AR3
		"	|	VLDW			*+AR3[48], VR23									\t\n" // load next from AR3
			// [14] VFMULAD third round
		"		VFMULAD.M1		VR4, VR63, VR12, VR12		\t\n" // [readme:4]
		"	|	VFMULAD.M2		VR5, VR63, VR13, VR13		\t\n"
		"	|	VFMULAD.M3		VR6, VR63, VR14, VR14		\t\n"
		"	|	VLDW			*+AR0[0], VR8									\t\n" // load next from AR0
		"	|	VLDW			*+AR0[32], VR10									\t\n" // load next from AR0
		"	|	SSUBA.M1		R55, AR0, AR0									\t\n" // AR0 -= (N+2)*8
			// [15]
		"		VFMULAD.M1		VR7, VR63, VR15, VR15		\t\n"
		"	|	VFMULAD.M2		VR16, VR63, VR24, VR24		\t\n" // [readme:7]
		"	|	VFMULAD.M3		VR17, VR63, VR25, VR25		\t\n"
		"	|	VLDW			*+AR0[16], VR9									\t\n" // load next from AR0
		"	|	VLDW			*+AR0[48], VR11									\t\n" // load next from AR0
			// [16]
		"		VFMULAD.M1		VR18, VR63, VR26, VR26		\t\n"
		"	|	VFMULAD.M2		VR19, VR63, VR27, VR27		\t\n"
		"	|	VLDDW			*AR1++[OR1], VR17:VR16							\t\n" // load next from AR1
		"	|	VLDDW			*+AR1[16], VR19:VR18							\t\n" // load next from AR1
		"	|	[R0] SADDA.M1   		R61, AR2, AR2		;; AR2 += 16*4*8	\t\n" // if(j_cur < n)
		"	|	[!R0] SMVAGA.M2         R44, AR2            ;; AR2_next = R44	\t\n" // else if(j_cur >= n)
			// [17]
		"		VFMULAD.M1		VR28, VR63, VR36, VR36		\t\n" // [readme:11]
		"	|	VFMULAD.M2		VR29, VR63, VR37, VR37		\t\n"
		"	|	VLDW			*+AR3[0], VR32									\t\n" // load next from AR3
		"	|	VLDW			*+AR3[32], VR34									\t\n" // load next from AR3
		"	|	SADDA.M1		R55, AR3, AR3									\t\n" // AR3 += (N+2)*8
			// [18]
		"		VFMULAD.M1		VR30, VR63, VR38, VR38		\t\n"
		"	|	VFMULAD.M2		VR31, VR63, VR39, VR39		\t\n"
		"	|	VFMULAD.M3		VR40, VR63, VR48, VR48		\t\n" // [readme:14]
		"	|	VLDW			*+AR3[16], VR33									\t\n" // load next from AR3
		"	|	VLDW			*+AR3[48], VR35									\t\n" // load next from AR3
			// [19]
		"		VFMULAD.M1		VR41, VR63, VR49, VR49		\t\n"
		"	|	VFMULAD.M2		VR42, VR63, VR50, VR50		\t\n"
		"	|	VFMULAD.M3		VR43, VR63, VR51, VR51		\t\n"
		"	|	VLDDW			*AR1++[OR1], VR29:VR28							\t\n" // load next from AR1
		"	|	VLDDW			*+AR1[16], VR31:VR30							\t\n" // load next from AR1
		"	|	[R4] SBR  	 	loop_V3						\t\n"
			//  [20] VFMULAD fourth round
		"		VFMULAD.M1		VR12, VR62, VR61, VR12		\t\n" // [readme:15]
		"	|	VFMULAD.M2		VR13, VR62, VR61, VR13		\t\n"
		"	|	VFMULAD.M3		VR14, VR62, VR61, VR14		\t\n"
		"	|	VLDW			*+AR3[0], VR44									\t\n" // load next from AR3
		"	|	VLDW			*+AR3[32], VR46									\t\n" // load next from AR3
		"	|	SADDA.M1		R55, AR3, AR3									\t\n" // AR3 += (N+2)*8
			// [21]
		"		VFMULAD.M1		VR15, VR62, VR61, VR15		\t\n"
		"	|	VFMULAD.M2		VR24, VR62, VR61, VR24		\t\n" // [readme:16]
		"	|	VFMULAD.M3		VR25, VR62, VR61, VR25		\t\n"
		"	|	VLDW			*+AR3[16], VR45									\t\n" // load next from AR3
		"	|	VLDW			*+AR3[48], VR47									\t\n" // load next from AR3
			// [22]
		"		VFMULAD.M1		VR26, VR62, VR61, VR26		\t\n"
		"	|	VFMULAD.M2		VR27, VR62, VR61, VR27		\t\n"
		"	|	VLDDW			*AR1--[OR3], VR41:VR40							\t\n" // load next from AR0
		"	|	VLDDW			*+AR1[16], VR43:VR42							\t\n" // load next from AR0
		"	|	[!R0] SADD.M1     	R52, %1, %1  			;; %1 += N*4*8	   	\t\n" // addr_B for next loop_m
		"	|	[!R0] SADD.M2     	R52, R46, R46			;; R46 += N*4*8	   	\t\n" // addr_B for next loop_m
		"	|	SMOV			R0, R1		\t\n"
			// [23]
		"		VFMULAD.M1		VR36, VR62, VR61, VR36		\t\n" // [readme:17]
		"	|	VFMULAD.M2		VR37, VR62, VR61, VR37		\t\n"
		"	|	VLDW			*+AR3[0], VR52									\t\n" // load next from AR3
		"	|	VLDW			*+AR3[32], VR54									\t\n" // load next from AR3
		"	|	SSUBA.M1		R58, AR3, AR3									\t\n" // AR3 -= (N+2)*3*8
			// [24]
		"		VFMULAD.M1		VR38, VR62, VR61, VR38		\t\n"
		"	|	VFMULAD.M2		VR39, VR62, VR61, VR39		\t\n"
		"	|	VFMULAD.M3		VR48, VR62, VR61, VR48		\t\n" // [readme:18]
		"	|	VLDW			*+AR3[16], VR53									\t\n" // load next from AR3
		"	|	VLDW			*+AR3[48], VR55									\t\n" // load next from AR3
			// [25]
		"		VFMULAD.M1		VR49, VR62, VR61, VR49		\t\n"
		"	|	VFMULAD.M2		VR50, VR62, VR61, VR50		\t\n"
		"	|	VFMULAD.M3		VR51, VR62, VR61, VR51		\t\n"
		"	|	[!R4]	SBR		R63							\t\n"
		// ================================== MAIN LOOP BOTTOM ==================================	
		// VSTDW last data:
		"		SNOP			1			\t\n"

		"		VSTDW			VR13:VR12, *AR5++[OR5]						\t\n"
		"	|	VSTDW			VR15:VR14, *+AR5[16]						\t\n"

		"		VSTDW			VR25:VR24, *AR5--[OR5]						\t\n"
		"	|	VSTDW			VR27:VR26, *+AR5[16]						\t\n"
		
		"		SNOP			1			\t\n"

		"		VSTDW			VR37:VR36, *AR6++[OR5]						\t\n"
		"	|	VSTDW			VR39:VR38, *+AR6[16]						\t\n"

		"		VSTDW			VR49:VR48, *AR6--[OR5]						\t\n"
		"	|	VSTDW			VR51:VR50, *+AR6[16]						\t\n"
    :
    :"r"(src_a), "r"(dst_b), "r"(m), "r"(n), "r"(N)
    );
}


__global__ void stencil_vector(unlong M, unlong N, double *A, double *B){
	const int tid = get_thread_id();
	const int grp_size = get_group_size();
	// --------------------------- blocking params ---------------------------
	const unlong M_A = 128;
	const unlong N_A = 128;
	// --------------------------- blocking params ---------------------------
	const unlong row_start = tid * M_A;
	const unlong row_step = grp_size * M_A;

	lvector double* spm_A[2];
	lvector double* spm_B[2];
	spm_A[0] = vector_malloc((M_A+2) * (N_A+2) * sizeof(double));
	spm_A[1] = vector_malloc((M_A+2) * (N_A+2) * sizeof(double));
	spm_B[0] = vector_malloc(M_A * N_A * sizeof(double));
	spm_B[1] = vector_malloc(M_A * N_A * sizeof(double));
	
	int ch_a[2], ch_b[2];

	const int ch0_a = 0;
	const int ch0_b = 2;

	unlong M_A_cur = min(M_A, M-row_start);
	unlong N_A_cur = min(N_A, N);
	unlong M_A_next, N_A_next;
	ch_a[0] = dma_p2p_opt(&A[OFFSET(row_start, 0, N+2)], M_A_cur+2, (N_A_cur+2)*sizeof(double), (N-N_A_cur)*sizeof(double),
									spm_A[0], M_A_cur+2, (N_A_cur+2)*sizeof(double), (N_A-N_A_cur)*sizeof(double), 0, 0, ch0_a);
	ch_b[1] = dma_p2p_opt(&B[OFFSET(row_start, 0, N+2)], 1, 1*sizeof(double), 0,
						   			spm_B[1], 1, 1*sizeof(double), 0, 0, 0, ch0_b+1); // 一次无效传输
	unint cnt_a = 0;
	unint cnt_b = 0;
	for(unlong m_o=row_start; m_o<M; m_o+=row_step){
		M_A_next = min(M_A, M-(m_o+row_step));
		for(unlong n_o=0; n_o<N; n_o+=N_A){
			unint cnt_a_1 = (cnt_a + 1) % 2;
			unint cnt_b_1 = (cnt_b + 1) % 2;

			if(n_o + N_A < N){
				N_A_next = min(N_A, N-(n_o+N_A));
				ch_a[cnt_a_1] = dma_p2p_opt(&A[OFFSET(m_o, n_o+N_A, N+2)], M_A_cur+2, (N_A_next+2)*sizeof(double), (N-N_A_next)*sizeof(double),
										spm_A[cnt_a_1], M_A_cur+2, (N_A_next+2)*sizeof(double), (N_A-N_A_next)*sizeof(double), 0, 0, ch0_a+cnt_a_1);
			} else if(m_o + M_A < M){
				N_A_next = min(N_A, N);
				ch_a[cnt_a_1] = dma_p2p_opt(&A[OFFSET(m_o+M_A, 0, N+2)], M_A_next+2, (N_A_next+2)*sizeof(double), (N-N_A_next)*sizeof(double),
										spm_A[cnt_a_1], M_A_next+2, (N_A_next+2)*sizeof(double), (N_A-N_A_next)*sizeof(double), 0, 0, ch0_a+cnt_a_1);
			}
			
			dma_wait_p2p(ch_a[cnt_a]);

			// micro_kernel_cross_reduce_V1(spm_A[cnt_a], spm_B[cnt_b], M_A_cur, N_A_cur, N_A);
			// micro_kernel_cross_reduce_V2(spm_A[cnt_a], spm_B[cnt_b], M_A_cur, N_A_cur, N_A);
			micro_kernel_cross_reduce_V3(spm_A[cnt_a], spm_B[cnt_b], M_A_cur, N_A_cur, N_A);

			ch_b[cnt_b] = dma_p2p_opt(spm_B[cnt_b], M_A_cur, N_A_cur*sizeof(double), (N_A-N_A_cur)*sizeof(double),
						   &B[OFFSET(m_o+1, n_o+1, N+2)], M_A_cur, N_A_cur*sizeof(double), (N+2-N_A_cur)*sizeof(double), 0, 0, ch0_b+cnt_b);

			dma_wait_p2p(ch_b[cnt_b_1]);
			
			cnt_a = cnt_a_1;
			cnt_b = cnt_b_1;
			N_A_cur = N_A_next;						 
		}
		M_A_cur = M_A_next;
	}
	dma_wait_p2p(ch_b[(cnt_b+1)%2]);
	vector_free(spm_A[0]);
	vector_free(spm_A[1]);
	vector_free(spm_B[0]);
	vector_free(spm_B[1]);
}


__global__ void stencil_null(unlong M, unlong N, double *A, double *B){
	const int tid = get_thread_id();
	const int grp_size = get_group_size();
	const unlong tile_size = (M + grp_size - 1) / grp_size;
	const unlong row_start = tid * tile_size;
	unlong row_end = row_start + tile_size;
	if(row_end > M)
		row_end = M;
	
	const unlong M_A = 128;
	const unlong N_A = 128;

	lvector double* spm_A[2];
	lvector double* spm_B[2];
	spm_A[0] = vector_malloc((M_A+2) * (N_A+2) * sizeof(double));
	spm_A[1] = vector_malloc((M_A+2) * (N_A+2) * sizeof(double));
	spm_B[0] = vector_malloc(M_A * N_A * sizeof(double));
	spm_B[1] = vector_malloc(M_A * N_A * sizeof(double));

	vector_free(spm_A[0]);
	vector_free(spm_A[1]);
	vector_free(spm_B[0]);
	vector_free(spm_B[1]);
}