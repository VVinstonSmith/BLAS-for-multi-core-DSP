#include <compiler/m3000.h>
#include <compiler/vsip.h>
#include "hthread_device.h"
#define unlong unsigned long
#define unint unsigned int
#define indexType long

#define OFFSET(row, col, ld) ((row) * (ld) + (col))

#define unroll_inner_loop
// #define CNACEL_DMA

__gsm__ double gsm_mem[1024*128*6];

unlong min(unlong x, unlong y){
    if(x<y) return x;
    return y;
}


void micro_kernel_16x16(lvector double* src_a, lvector double* dst_b, lvector double* spm_D,
                        const unlong m, const unlong n){
    __asm__ __volatile__(// m:%3     n:%4
        // --------------------------- step 1 ---------------------------
        // [-3]
        "       SSHFLL          1, %4, R52      ;; R52 = 2*n                    \t\n"
        "   |   SMVAGA.M1       %0, AR0         ;; AR0 = src_A                  \t\n" 
        "   |   SMVAGA.M2       %4, OR1         ;; OR1 = n                      \t\n"
        // [-2]
        "       SSHFLL          5, %4, R54      ;; R54 = 4*(8*n)                \t\n"
        "   |   SADD.M1         %4, R52, R53    ;; R53 = 3*n                    \t\n"     
        "   |   SMVAGA.M2       R52, OR2        ;; OR2 = 2*n                    \t\n"
        // [-1]
        "       SSHFLL          6, %4, R56      ;; R56 = 8*(8*n)                \t\n"
        "   |   SMVAGA.M1       R53, OR3        ;; OR3 = 3*n                    \t\n"
        "   |   SADDA.M2        R54, AR0, AR0                                   \t\n"
        // [0]
        "       SADD.M1         R56, %0, R56    ;; R56 = src_A + 8*(8*n)        \t\n"
        "   |   VLDW            *AR0, VR0                   \t\n"
        "   |   VLDW            *+AR0[OR1], VR4             \t\n"
        // [1]
        "       SMVAGA.M1       R56, AR1        ;; AR1 = src_A + 8*(8*n)        \t\n"
        "   |   VLDW            *+AR0[OR2], VR8             \t\n"
        "   |   VLDW            *+AR0[OR3], VR12            \t\n"
        // [2]
        "       VLDW            *AR0, VR1                   \t\n"
        "   |   VLDW            *+AR0[OR1], VR5             \t\n"
        // [3]
        "       SADDA.M1        R54, AR1, AR1                                   \t\n"
        "   |   VLDW            *+AR0[OR2], VR9             \t\n"
        "   |   VLDW            *+AR0[OR3], VR13            \t\n"
        // [4]
        "       VLDW            *AR1, VR2                   \t\n"
        "   |   VLDW            *+AR1[OR1], VR6             \t\n"
        // [5]
        "       VLDW            *+AR1[OR2], VR10            \t\n"
        "   |   VLDW            *+AR1[OR3], VR14            \t\n"
        // [6]
        "       VLDW            *AR1, VR3                   \t\n"
        "   |   VLDW            *+AR1[OR1], VR7             \t\n"
        // [7]
        "       VLDW            *+AR1[OR2], VR11            \t\n"
        "   |   VLDW            *+AR1[OR3], VR15            \t\n"
        "   |   SMVAGA.M1       %2, AR4         ;; AR4 = spm_D                  \t\n"
        // [7 + 1~7]
        "       SNOP            7       \t\n"
        // --------------------------- step 2 ---------------------------
        "       VSTDW0M16       VR1:VR0, *AR4               \t\n"
        "   |   VSTDW1M16       VR3:VR2, *+AR4[16]          \t\n"
        "       VSTDW0M16       VR5:VR4, *+AR4[32]          \t\n"
        "   |   VSTDW1M16       VR7:VR6, *+AR4[48]          \t\n"
        "       VSTDW0M16       VR9:VR8, *+AR4[64]          \t\n"
        "   |   VSTDW1M16       VR11:VR10, *+AR4[80]        \t\n"
        "       VSTDW0M16       VR13:VR12, *+AR4[96]        \t\n"
        "   |   VSTDW1M16       VR15:VR14, *+AR4[112]       \t\n"
        // --------------------------- step 3 ---------------------------
        // [0]
        "       VLDW            *AR4, VR0                   \t\n"
        "   |   VLDW            *+AR4[16], VR4              \t\n"
        // [1]
        "       VLDW            *+AR4[64], VR1              \t\n"
        "   |   VLDW            *+AR4[80], VR5              \t\n"
        // [2]
        "       VLDW            *+AR4[128], VR2             \t\n"
        "   |   VLDW            *+AR4[144], VR6             \t\n"
        // [3]
        "       VLDW            *+AR4[192], VR3             \t\n"
        "   |   VLDW            *+AR4[208], VR7             \t\n"
        // [4]
        "       VLDW            *+AR4[32], VR8              \t\n"
        "   |   VLDW            *+AR4[48], VR12             \t\n"
        // [5]
        "       VLDW            *+AR4[96], VR9              \t\n"
        "   |   VLDW            *+AR4[112], VR13            \t\n"
        // [6]
        "       VLDW            *+AR4[160], VR10            \t\n"
        "   |   VLDW            *+AR4[176], VR14            \t\n"
        // [7]
        "       VLDW            *+AR4[224], VR11            \t\n"
        "   |   VLDW            *+AR4[240], VR15            \t\n"
        // [7 + 1~4]
        "       SNOP            4       \t\n"
        // --------------------------- step 4, 5 ---------------------------
        // [0]
        "       VSTDW0M16       VR1:VR0, *AR4               \t\n"
        "   |   VSTDW1M16       VR3:VR2, *+AR4[16]          \t\n"
        // [1]
        "       VLDDW           *AR4, VR1:VR0               \t\n"
        "   |   VLDDW           *+AR4[16], VR3:VR2          \t\n"
        // [2]
        "       VSTDW0M16       VR5:VR4, *+AR4[32]          \t\n"
        "   |   VSTDW1M16       VR7:VR6, *+AR4[48]          \t\n"
        // [3]
        "       VLDDW           *+AR4[32], VR5:VR4          \t\n"
        "   |   VLDDW           *+AR4[48], VR7:VR6          \t\n"
        // [4]
        "       VSTDW0M16       VR9:VR8, *+AR4[64]          \t\n"
        "   |   VSTDW1M16       VR11:VR10, *+AR4[80]        \t\n"
        // [5]
        "       VLDDW           *+AR4[64], VR9:VR8          \t\n"
        "   |   VLDDW           *+AR4[80], VR11:VR10        \t\n"
        // [6]
        "       VSTDW0M16       VR13:VR12, *+AR4[96]        \t\n"
        "   |   VSTDW1M16       VR15:VR14, *+AR4[112]       \t\n"
        // [7]
        "       VLDDW           *+AR4[96], VR13:VR12        \t\n"
        "   |   VLDDW           *+AR4[112], VR15:VR14       \t\n"
        "   |   SSHFLL          1, %3, R44      ;; R44 = 2*m                    \t\n"
        "   |   SMVAGA.M1       %1, AR2         ;; AR2 = src_B                  \t\n"
        "   |   SMVAGA.M2       %3, OR5         ;; OR5 = m                      \t\n"
        // [7+1]
        "       SSHFLL          5, %3, R46      ;; R46 = 4*(8*m)                \t\n"
        "   |   SADD.M1         %3, R44, R45    ;; R45 = 3*m                    \t\n"
        "   |   SMVAGA.M2       R44, OR6        ;; OR6 = 2*m                    \t\n"
        // [7+2]
        "       SSHFLL          6, %3, R47      ;; R47 = 8*(8*m)                \t\n"
        "   |   SMVAGA.M1       R45, OR7        ;; OR7 = 3*m                    \t\n"
        "   |   SADDA.M2        R46, AR2, AR2                                   \t\n"
        // --------------------------- step 6 ---------------------------
        // [0]
        "       SADD.M1         R47, %1, R47    ;; R47 = src_B + 8*(8*m)        \t\n"
        "   |   VSTW            VR0, *AR2                   \t\n"
        "   |   VSTW            VR1, *+AR2[OR5]             \t\n"
        // [1]
        "       SMVAGA.M1       R47, AR3        ;; AR3 = src_B + 8*(8*m)        \t\n"
        "   |   VSTW            VR2, *+AR2[OR6]             \t\n"
        "   |   VSTW            VR3, *+AR2[OR7]             \t\n"
        "   |   SBR             R63         \t\n"
        // [2]
        "       VSTW            VR4, *AR2                   \t\n"
        "   |   VSTW            VR5, *+AR2[OR5]             \t\n"
        // [3]
        "       SADDA.M1        R46, AR3, AR3                                   \t\n"
        "   |   VSTW            VR6, *+AR2[OR6]             \t\n"
        "   |   VSTW            VR7, *+AR2[OR7]             \t\n"
        // [4]
        "       VSTW            VR8, *AR3                   \t\n"
        "   |   VSTW            VR9, *+AR3[OR5]             \t\n"
        // [5]
        "       VSTW            VR10, *+AR3[OR6]            \t\n"
        "   |   VSTW            VR11, *+AR3[OR7]            \t\n"
        // [6]
        "       VSTW            VR12, *AR3                  \t\n"
        "   |   VSTW            VR13, *+AR3[OR5]            \t\n"
        // [7]
        "       VSTW            VR14, *+AR3[OR6]            \t\n"
        "   |   VSTW            VR15, *+AR3[OR7]            \t\n"       
    :
    :"r"(src_a), "r"(dst_b), "r"(spm_D), "r"(m), "r"(n)
    );
}


void micro_kernel_16x16_add(lvector double* src_a, lvector double* dst_b, lvector double* spm_D,
                        const unlong m, const unlong n){
    __asm__ __volatile__(// m:%3     n:%4
        // --------------------------- step 1 ---------------------------
        // [-3]
        "       SSHFLL          1, %4, R52      ;; R52 = 2*n                    \t\n"
        "   |   SMVAGA.M1       %0, AR0         ;; AR0 = src_A                  \t\n" 
        "   |   SMVAGA.M2       %4, OR1         ;; OR1 = n                      \t\n"
        // [-2]      
        "       SSHFLL          5, %4, R54      ;; R54 = 4*(8*n)                \t\n"
        "   |   SADD.M1         %4, R52, R53    ;; R53 = 3*n                    \t\n"     
        "   |   SMVAGA.M2       R52, OR2        ;; OR2 = 2*n                    \t\n"
        // [-1]
        "       SSHFLL          6, %4, R56      ;; R56 = 8*(8*n)                \t\n"
        "   |   SMVAGA.M1       R53, OR3        ;; OR3 = 3*n                    \t\n"
        "   |   SADDA.M2        R54, AR0, AR0                                   \t\n"
        // [0]
        "       SSHFLL          1, %3, R44      ;; R44 = 2*m                    \t\n"
        "   |   SMVAGA.M1       %1, AR2         ;; AR2 = src_B                  \t\n"
        "   |   SADD.M2         R56, %0, R56    ;; R56 = src_A + 8*(8*n)        \t\n"
        "   |   VLDW            *AR0, VR0                   \t\n"
        "   |   VLDW            *+AR0[OR1], VR4             \t\n"
        // [1]
        "       SMVAGA.M1       R56, AR1        ;; AR1 = src_A + 8*(8*n)        \t\n"
        "   |   SMVAGA.M2       %3, OR5         ;; OR5 = m                      \t\n"
        "   |   VLDW            *+AR0[OR2], VR8             \t\n"
        "   |   VLDW            *+AR0[OR3], VR12            \t\n"
        // [2]
        "       SSHFLL          5, %3, R46      ;; R46 = 4*(8*m)                \t\n"
        "   |   SADD.M1         %3, R44, R45    ;; R45 = 3*m                    \t\n"
        "   |   SMVAGA.M2       R44, OR6        ;; OR6 = 2*m                    \t\n"
        "   |   VLDW            *AR0, VR1                   \t\n"
        "   |   VLDW            *+AR0[OR1], VR5             \t\n"
        // [3]
        "       SSHFLL          6, %3, R47      ;; R47 = 8*(8*m)                \t\n"
        "   |   SMVAGA.M1       R45, OR7        ;; OR7 = 3*m                    \t\n"
        "   |   VLDW            *+AR0[OR2], VR9             \t\n"
        "   |   VLDW            *+AR0[OR3], VR13            \t\n"
        "   |   SADDA.M2        R54, AR1, AR1                                   \t\n"
        // [4]
        "       SADD.M1         R47, %1, R47    ;; R47 = src_B + 8*(8*m)        \t\n"
        "   |   VLDW            *AR1, VR2                   \t\n"
        "   |   VLDW            *+AR1[OR1], VR6             \t\n"
        // [5]
        "       SMVAGA.M1       R47, AR3        ;; AR3 = src_B + 8*(8*m)        \t\n"
        "   |   SMVAGA.M2       %2, AR4         ;; AR4 = spm_D                  \t\n"
        "   |   VLDW            *+AR1[OR2], VR10            \t\n"
        "   |   VLDW            *+AR1[OR3], VR14            \t\n"
        // [6]
        "       VLDW            *AR1, VR3                   \t\n"
        "   |   VLDW            *+AR1[OR1], VR7             \t\n"
        // [7]
        "       VLDW            *+AR1[OR2], VR11            \t\n"
        "   |   VLDW            *+AR1[OR3], VR15            \t\n"
        "   |   SADDA.M1        R46, AR2, AR2               \t\n"
        // [7+1]
        "       VLDW            *AR2, VR32                  \t\n"
        "   |   VLDW            *+AR2[OR5], VR33            \t\n"
        // [7+2]
        "       VLDW            *+AR2[OR6], VR34            \t\n"
        "   |   VLDW            *+AR2[OR7], VR35            \t\n"
        // [7+3]
        "       VLDW            *AR2, VR36                  \t\n"
        "   |   VLDW            *+AR2[OR5], VR37            \t\n"
        "   |   SSUBA.M2        R46, AR2, AR2               \t\n"
        // [7+4]
        "       VLDW            *+AR2[OR6], VR38            \t\n"
        "   |   VLDW            *+AR2[OR7], VR39            \t\n"
        "   |   SADDA.M1        R46, AR3, AR3               \t\n"
        // [7+5]
        "       VLDW            *AR3, VR40                  \t\n"
        "   |   VLDW            *+AR3[OR5], VR41            \t\n"
        // [7+6]
        "       VLDW            *+AR3[OR6], VR42            \t\n"
        "   |   VLDW            *+AR3[OR7], VR43            \t\n"
        // [7+7]
        "       SNOP            1       \t\n"
        // --------------------------- step 2 ---------------------------
        "       VSTDW0M16       VR1:VR0, *AR4               \t\n"
        "   |   VSTDW1M16       VR3:VR2, *+AR4[16]          \t\n"
        "       VSTDW0M16       VR5:VR4, *+AR4[32]          \t\n"
        "   |   VSTDW1M16       VR7:VR6, *+AR4[48]          \t\n"
        "       VSTDW0M16       VR9:VR8, *+AR4[64]          \t\n"
        "   |   VSTDW1M16       VR11:VR10, *+AR4[80]        \t\n"
        "       VSTDW0M16       VR13:VR12, *+AR4[96]        \t\n"
        "   |   VSTDW1M16       VR15:VR14, *+AR4[112]       \t\n"
        // --------------------------- step 3 ---------------------------
        // [0]
        "       VLDW            *AR4, VR0                   \t\n"
        "   |   VLDW            *+AR4[16], VR4              \t\n"
        // [1]
        "       VLDW            *+AR4[64], VR1              \t\n"
        "   |   VLDW            *+AR4[80], VR5              \t\n"
        // [2]
        "       VLDW            *+AR4[128], VR2             \t\n"
        "   |   VLDW            *+AR4[144], VR6             \t\n"
        // [3]
        "       VLDW            *+AR4[192], VR3             \t\n"
        "   |   VLDW            *+AR4[208], VR7             \t\n"
        // [4]
        "       VLDW            *+AR4[32], VR8              \t\n"
        "   |   VLDW            *+AR4[48], VR12             \t\n"
        // [5]
        "       VLDW            *+AR4[96], VR9              \t\n"
        "   |   VLDW            *+AR4[112], VR13            \t\n"
        // [6]
        "       VLDW            *+AR4[160], VR10            \t\n"
        "   |   VLDW            *+AR4[176], VR14            \t\n"
        // [7]
        "       VLDW            *+AR4[224], VR11            \t\n"
        "   |   VLDW            *+AR4[240], VR15            \t\n"
        // [7+1]
        "       VLDW            *AR3, VR44                  \t\n"
        "   |   VLDW            *+AR3[OR5], VR45            \t\n"
        "   |   SSUBA.M2        R46, AR3, AR3               \t\n"
        // [7+2]
        "       VLDW            *+AR3[OR6], VR46            \t\n"
        "   |   VLDW            *+AR3[OR7], VR47            \t\n"
        // [7+3,4]
        "       SNOP            2       \t\n"
        // --------------------------- step 4, 5 ---------------------------
        // [0]
        "       VSTDW0M16       VR1:VR0, *AR4               \t\n"
        "   |   VSTDW1M16       VR3:VR2, *+AR4[16]          \t\n"
        // [1]
        "       VLDDW           *AR4, VR1:VR0               \t\n"
        "   |   VLDDW           *+AR4[16], VR3:VR2          \t\n"
        // [2]
        "       VSTDW0M16       VR5:VR4, *+AR4[32]          \t\n"
        "   |   VSTDW1M16       VR7:VR6, *+AR4[48]          \t\n"
        // [3]
        "       VLDDW           *+AR4[32], VR5:VR4          \t\n"
        "   |   VLDDW           *+AR4[48], VR7:VR6          \t\n"
        // [4]
        "       VSTDW0M16       VR9:VR8, *+AR4[64]          \t\n"
        "   |   VSTDW1M16       VR11:VR10, *+AR4[80]        \t\n"
        // [5]
        "       VLDDW           *+AR4[64], VR9:VR8          \t\n"
        "   |   VLDDW           *+AR4[80], VR11:VR10        \t\n"
        // [6]
        "       VSTDW0M16       VR13:VR12, *+AR4[96]        \t\n"
        "   |   VSTDW1M16       VR15:VR14, *+AR4[112]       \t\n"
        // [7]
        "       VLDDW           *+AR4[96], VR13:VR12        \t\n"
        "   |   VLDDW           *+AR4[112], VR15:VR14       \t\n"
        "   |   VMOVI    	    0x3ff0000000000000,VR63	    ;; VR63 = 1 \t\n" 
        // [7+1,2]
        "       SNOP            2       \t\n"
        // --------------------------- step 5.9 ---------------------------
        // [0]
        "       VFMULAD.M1	    VR32, VR63, VR0, VR0        \t\n"
        "   |   VFMULAD.M2	    VR33, VR63, VR1, VR1        \t\n"
        // [1]
        "       VFMULAD.M1	    VR34, VR63, VR2, VR2        \t\n"
        "   |   VFMULAD.M2	    VR35, VR63, VR3, VR3        \t\n"
        // [2]
        "       VFMULAD.M1	    VR36, VR63, VR4, VR4        \t\n"
        "   |   VFMULAD.M2	    VR37, VR63, VR5, VR5        \t\n"
        // [3]
        "       VFMULAD.M1	    VR38, VR63, VR6, VR6        \t\n"
        "   |   VFMULAD.M2	    VR39, VR63, VR7, VR7        \t\n"
        // [4]
        "       VFMULAD.M1	    VR40, VR63, VR8, VR8        \t\n"
        "   |   VFMULAD.M2	    VR41, VR63, VR9, VR9        \t\n"
        // [5]
        "       VFMULAD.M1	    VR42, VR63, VR10, VR10      \t\n"
        "   |   VFMULAD.M2	    VR43, VR63, VR11, VR11      \t\n"
        "   |   SADDA.M2        R46, AR2, AR2               \t\n"
        // --------------------------- step 6 ---------------------------
        // [0]
        "       VSTW            VR0, *AR2                   \t\n"
        "   |   VSTW            VR1, *+AR2[OR5]             \t\n"
        "   |   VFMULAD.M1	    VR44, VR63, VR12, VR12      \t\n"
        "   |   VFMULAD.M2	    VR45, VR63, VR13, VR13      \t\n"
        // [1]
        "       VSTW            VR2, *+AR2[OR6]             \t\n"
        "   |   VSTW            VR3, *+AR2[OR7]             \t\n"
        "   |   VFMULAD.M1	    VR46, VR63, VR14, VR14      \t\n"
        "   |   VFMULAD.M2	    VR47, VR63, VR15, VR15      \t\n"
        "   |   SBR             R63         \t\n"
        // [2]
        "       VSTW            VR4, *AR2                   \t\n"
        "   |   VSTW            VR5, *+AR2[OR5]             \t\n"
        // [3]
        "       SADDA.M1        R46, AR3, AR3               \t\n"
        "   |   VSTW            VR6, *+AR2[OR6]             \t\n"
        "   |   VSTW            VR7, *+AR2[OR7]             \t\n"
        // [4]
        "       VSTW            VR8, *AR3                   \t\n"
        "   |   VSTW            VR9, *+AR3[OR5]             \t\n"
        // [5]
        "       VSTW            VR10, *+AR3[OR6]            \t\n"
        "   |   VSTW            VR11, *+AR3[OR7]            \t\n"
        // [6]
        "       VSTW            VR12, *AR3                  \t\n"
        "   |   VSTW            VR13, *+AR3[OR5]            \t\n"
        // [7]
        "       VSTW            VR14, *+AR3[OR6]            \t\n"
        "   |   VSTW            VR15, *+AR3[OR7]            \t\n"
    :
    :"r"(src_a), "r"(dst_b), "r"(spm_D), "r"(m), "r"(n)
    );
}


void micro_kernel_16x16x2(lvector double* src_a, lvector double* dst_b, lvector double* spm_D,
                        const unlong m, const unlong n){
    __asm__ __volatile__(// m:%3     n:%4
        // --------------------------- step 1 ---------------------------
        // cycles: 3 + 16 + 2
        // [-3]
        "       SSHFLL          1, %4, R52      ;; R52 = 2*n                    \t\n"
        "   |   SMVAGA.M1       %0, AR0         ;; AR0 = src_A                  \t\n" 
        "   |   SMVAGA.M2       %4, OR1         ;; OR1 = n                      \t\n"
        // [-2]
        "       SSHFLL          5, %4, R54      ;; R54 = 4*(8*n)                \t\n"
        "   |   SADD.M1         %4, R52, R53    ;; R53 = 3*n                    \t\n"     
        "   |   SMVAGA.M2       R52, OR2        ;; OR2 = 2*n                    \t\n"
        // [-1]
        "       SSHFLL          6, %4, R56      ;; R56 = 8*(8*n)                \t\n"
        "   |   SMVAGA.M1       R53, OR3        ;; OR3 = 3*n                    \t\n"
        "   |   SADDA.M2        R54, AR0, AR0                                   \t\n"
        // [0]
        "       SMOVI24         128, R61        ;; R61 = 16*8                   \t\n" // ==
        "   |   SADD.M1         R56, %0, R56    ;; R56 = src_A + 8*(8*n)        \t\n"
        "   |   VLDW            *AR0, VR0                   \t\n"
        "   |   VLDW            *+AR0[OR1], VR4             \t\n"
        // [1]
        "       SMVAGA.M1       R56, AR1        ;; AR1 = src_A + 8*(8*n)        \t\n"
        "   |   SADD.M2         R61, %0, R50    ;; R50 = src_A + 16*8           \t\n" // ==
        "   |   VLDW            *+AR0[OR2], VR8             \t\n"
        "   |   VLDW            *+AR0[OR3], VR12            \t\n"
        // [2]
        "       SADD.M1         R61, R56, R51   ;; R51 = src_A + 16*8 + 8*(8*n) \t\n" // ==
        "   |   SMVAGA.M2       R50, AR2        ;; AR2 = src_A + 16*8           \t\n" // ==
        "   |   VLDW            *AR0, VR1                   \t\n"
        "   |   VLDW            *+AR0[OR1], VR5             \t\n"
        // [3]
        "       SADDA.M1        R54, AR1, AR1                                   \t\n"
        "   |   SMVAGA.M2       R51, AR3        ;; AR3 = src_A + 16*8 + 8*(8*n) \t\n" // ==
        "   |   VLDW            *+AR0[OR2], VR9             \t\n"
        "   |   VLDW            *+AR0[OR3], VR13            \t\n"
        // [4]
        "       SADD.M1         R54, R50, R57   ;; R51 = src_A+16*8 + 4*(8*n)   \t\n" // ==
        "   |   SADD.M2         R54, R51, R58   ;; R51 = src_A+16*8 + 12*(8*n)  \t\n" // ==
        "   |   VLDW            *AR1, VR2                   \t\n"
        "   |   VLDW            *+AR1[OR1], VR6             \t\n"
        // [5]
        "       SMVAGA.M1       R57, AR6        ;; AR6 = src_A+16*8 + 4*(8*n)   \t\n" // ==
        "   |   SMVAGA.M2       R58, AR7        ;; AR6 = src_A+16*8 + 12*(8*n)  \t\n" // ==
        "   |   VLDW            *+AR1[OR2], VR10            \t\n"
        "   |   VLDW            *+AR1[OR3], VR14            \t\n"
        // [6]
        "       VLDW            *AR1, VR3                   \t\n"
        "   |   VLDW            *+AR1[OR1], VR7             \t\n"
        // [7]
        "       VLDW            *+AR1[OR2], VR11            \t\n"
        "   |   VLDW            *+AR1[OR3], VR15            \t\n"
        "   |   SMVAGA.M1       %2, AR4         ;; AR4 = spm_D                  \t\n"
        // [8]
        "       VLDW            *AR2, VR16                  \t\n" // ==
        "   |   VLDW            *+AR2[OR1], VR20            \t\n" // ==
        // [9]
        "       VLDW            *AR6, VR17                  \t\n" // ==
        "   |   VLDW            *+AR6[OR1], VR21            \t\n" // ==
        // [10]
        "       VLDW            *AR3, VR18                  \t\n" // ==
        "   |   VLDW            *+AR3[OR1], VR22            \t\n" // ==
        // [11]
        "       VLDW            *AR7, VR19                  \t\n" // ==
        "   |   VLDW            *+AR7[OR1], VR23            \t\n" // ==
        // [12]
        "       VLDW            *+AR2[OR2], VR24            \t\n" // ==
        "   |   VLDW            *+AR2[OR3], VR28            \t\n" // ==
        "   |   SMOVI24         2048, R60       ;; R60 = 256*8                  \t\n" // ==
        // [13]
        "       VLDW            *+AR6[OR2], VR25            \t\n" // ==
        "   |   VLDW            *+AR6[OR3], VR29            \t\n" // ==
        "   |   SADD.M1         R60, %2, R59    ;; R59 = spm_D + 256*8          \t\n" // ==
        // [14]
        "       VLDW            *+AR3[OR2], VR26            \t\n" // ==
        "   |   VLDW            *+AR3[OR3], VR30            \t\n" // ==
        "   |   SMVAGA.M1       R59, AR5        ;; AR5 = spm_D + 256*8          \t\n" // ==
        // [15]
        "       VLDW            *+AR7[OR2], VR27            \t\n" // ==
        "   |   VLDW            *+AR7[OR3], VR31            \t\n" // ==
        // [15 + 1,2]
        "   snop                2           \t\n"
        // --------------------------- step 2 ---------------------------
        // cycles: 8
        "       VSTDW0M16       VR1:VR0, *AR4               \t\n"
        "   |   VSTDW1M16       VR3:VR2, *+AR4[16]          \t\n"
        "       VSTDW0M16       VR5:VR4, *+AR4[32]          \t\n"
        "   |   VSTDW1M16       VR7:VR6, *+AR4[48]          \t\n"
        "       VSTDW0M16       VR9:VR8, *+AR4[64]          \t\n"
        "   |   VSTDW1M16       VR11:VR10, *+AR4[80]        \t\n"
        "       VSTDW0M16       VR13:VR12, *+AR4[96]        \t\n"
        "   |   VSTDW1M16       VR15:VR14, *+AR4[112]       \t\n"
        "       VSTDW0M16       VR17:VR16, *AR5             \t\n" // ==
        "   |   VSTDW1M16       VR19:VR18, *+AR5[16]        \t\n" // ==
        "       VSTDW0M16       VR21:VR20, *+AR5[32]        \t\n" // ==
        "   |   VSTDW1M16       VR23:VR22, *+AR5[48]        \t\n" // ==
        "       VSTDW0M16       VR25:VR24, *+AR5[64]        \t\n" // ==
        "   |   VSTDW1M16       VR27:VR26, *+AR5[80]        \t\n" // ==
        "       VSTDW0M16       VR29:VR28, *+AR5[96]        \t\n" // ==
        "   |   VSTDW1M16       VR31:VR30, *+AR5[112]       \t\n" // ==
        // --------------------------- step 3 ---------------------------
        // cycles: 16
        "       VLDW            *AR4, VR0                   \t\n"
        "   |   VLDW            *+AR4[16], VR4              \t\n"
        "       VLDW            *+AR4[64], VR1              \t\n"
        "   |   VLDW            *+AR4[80], VR5              \t\n"
        "       VLDW            *+AR4[128], VR2             \t\n"
        "   |   VLDW            *+AR4[144], VR6             \t\n"
        "       VLDW            *+AR4[192], VR3             \t\n"
        "   |   VLDW            *+AR4[208], VR7             \t\n"
        "       VLDW            *+AR4[32], VR8              \t\n"
        "   |   VLDW            *+AR4[48], VR12             \t\n"
        "       VLDW            *+AR4[96], VR9              \t\n"
        "   |   VLDW            *+AR4[112], VR13            \t\n"
        "       VLDW            *+AR4[160], VR10            \t\n"
        "   |   VLDW            *+AR4[176], VR14            \t\n"
        "       VLDW            *+AR4[224], VR11            \t\n"
        "   |   VLDW            *+AR4[240], VR15            \t\n"
        "       VLDW            *AR5, VR16                  \t\n" // ==
        "   |   VLDW            *+AR5[16], VR20             \t\n" // ==
        "       VLDW            *+AR5[64], VR17             \t\n" // ==
        "   |   VLDW            *+AR5[80], VR21             \t\n" // ==
        "       VLDW            *+AR5[128], VR18            \t\n" // ==
        "   |   VLDW            *+AR5[144], VR22            \t\n" // ==
        "       VLDW            *+AR5[192], VR19            \t\n" // ==
        "   |   VLDW            *+AR5[208], VR23            \t\n" // ==
        "       VLDW            *+AR5[32], VR24             \t\n" // ==
        "   |   VLDW            *+AR5[48], VR28             \t\n" // ==
        "       VLDW            *+AR5[96], VR25             \t\n" // ==
        "   |   VLDW            *+AR5[112], VR29            \t\n" // ==
        "       VLDW            *+AR5[160], VR26            \t\n" // ==
        "   |   VLDW            *+AR5[176], VR30            \t\n" // ==
        "       VLDW            *+AR5[224], VR27            \t\n" // ==
        "   |   VLDW            *+AR5[240], VR31            \t\n" // ==
        // --------------------------- step 4, 5 ---------------------------
        // [0]
        "       VSTDW0M16       VR1:VR0, *AR4               \t\n"
        "   |   VSTDW1M16       VR3:VR2, *+AR4[16]          \t\n"
        // [1]
        "       VLDDW           *AR4, VR1:VR0               \t\n"
        "   |   VLDDW           *+AR4[16], VR3:VR2          \t\n"
        // [2]
        "       VSTDW0M16       VR5:VR4, *+AR4[32]          \t\n"
        "   |   VSTDW1M16       VR7:VR6, *+AR4[48]          \t\n"
        // [3]
        "       VLDDW           *+AR4[32], VR5:VR4          \t\n"
        "   |   VLDDW           *+AR4[48], VR7:VR6          \t\n"
        // [4]
        "       VSTDW0M16       VR9:VR8, *+AR4[64]          \t\n"
        "   |   VSTDW1M16       VR11:VR10, *+AR4[80]        \t\n"
        // [5]
        "       VLDDW           *+AR4[64], VR9:VR8          \t\n"
        "   |   VLDDW           *+AR4[80], VR11:VR10        \t\n"
        // [6]
        "       VSTDW0M16       VR13:VR12, *+AR4[96]        \t\n"
        "   |   VSTDW1M16       VR15:VR14, *+AR4[112]       \t\n"
        // [7]
        "       VLDDW           *+AR4[96], VR13:VR12        \t\n"
        "   |   VLDDW           *+AR4[112], VR15:VR14       \t\n"
        // [8]
        "       VSTDW0M16       VR17:VR16, *AR5             \t\n" // ==
        "   |   VSTDW1M16       VR19:VR18, *+AR5[16]        \t\n" // ==
        "   |   SSHFLL          1, %3, R44      ;; R44 = 2*m                    \t\n"
        "   |   SMVAGA.M1       %1, AR0         ;; AR0 = src_B                  \t\n"
        "   |   SMVAGA.M2       %3, OR5         ;; OR5 = m                      \t\n"
        // [9]
        "       VLDDW           *AR5, VR17:VR16             \t\n" // ==
        "   |   VLDDW           *+AR5[16], VR19:VR18        \t\n" // ==
        "   |   SSHFLL          5, %3, R46      ;; R46 = 4*(8*m)                \t\n"
        "   |   SADD.M1         %3, R44, R45    ;; R45 = 3*m                    \t\n"
        "   |   SMVAGA.M2       R44, OR6        ;; OR6 = 2*m                    \t\n"
        // [10]
        "       VSTDW0M16       VR21:VR20, *+AR5[32]        \t\n" // ==
        "   |   VSTDW1M16       VR23:VR22, *+AR5[48]        \t\n" // ==
        "   |   SSHFLL          6, %3, R47      ;; R47 = 8*(8*m)                \t\n"
        "   |   SMVAGA.M1       R45, OR7        ;; OR7 = 3*m                    \t\n"
        // [11]
        "       VLDDW           *+AR5[32], VR21:VR20        \t\n" // ==
        "   |   VLDDW           *+AR5[48], VR23:VR22        \t\n" // ==
        "   |   SADD.M1         R47, %1, R48    ;; R48 = src_B + 8*(8*m)        \t\n"
        // [12]
        "       VSTDW0M16       VR25:VR24, *+AR5[64]        \t\n" // ==
        "   |   VSTDW1M16       VR27:VR26, *+AR5[80]        \t\n" // ==
        "   |   SMVAGA.M1       R48, AR1        ;; AR1 = src_B + 8*(8*m)        \t\n"
        // [13]
        "       VLDDW           *+AR5[64], VR25:VR24        \t\n" // ==
        "   |   VLDDW           *+AR5[80], VR27:VR26        \t\n" // ==
        "   |   SADD.M1         R47, R48, R49   ;; R49 = src_B + 16*(8*m)       \t\n" // ==
        // [14]
        "       VSTDW0M16       VR29:VR28, *+AR5[96]        \t\n" // ==
        "   |   VSTDW1M16       VR31:VR30, *+AR5[112]       \t\n" // ==
        "   |   SADD.M1         R47, R49, R50   ;; R50 = src_B + 24*(8*m)       \t\n" // ==
        "   |   SMVAGA.M2       R49, AR2        ;; AR2 = src_B + 16*(8*m)       \t\n" // ==
        // [15]
        "       VLDDW           *+AR5[96], VR29:VR28        \t\n" // ==
        "   |   VLDDW           *+AR5[112], VR31:VR30       \t\n" // ==
        "   |   SMVAGA.M1       R50, AR3        ;; AR3 = src_B + 24*(8*m)       \t\n" // ==
        "   |   SADDA.M2        R46, AR0, AR0                                   \t\n"
        // --------------------------- step 6 ---------------------------
        // [0]
        "       VSTW            VR0, *AR0                   \t\n"
        "   |   VSTW            VR1, *+AR0[OR5]             \t\n"
        // [1]
        "       VSTW            VR2, *+AR0[OR6]             \t\n"
        "   |   VSTW            VR3, *+AR0[OR7]             \t\n"
        // [2]
        "       VSTW            VR4, *AR0                   \t\n"
        "   |   VSTW            VR5, *+AR0[OR5]             \t\n"
        // [3]
        "       SADDA.M1        R46, AR1, AR1                                   \t\n"
        "   |   VSTW            VR6, *+AR0[OR6]             \t\n"
        "   |   VSTW            VR7, *+AR0[OR7]             \t\n"
        // [4]
        "       VSTW            VR8, *AR1                   \t\n"
        "   |   VSTW            VR9, *+AR1[OR5]             \t\n"
        // [5]
        "       VSTW            VR10, *+AR1[OR6]            \t\n"
        "   |   VSTW            VR11, *+AR1[OR7]            \t\n"
        // [6]
        "       VSTW            VR12, *AR1                  \t\n"
        "   |   VSTW            VR13, *+AR1[OR5]            \t\n"
        // [7]
        "       VSTW            VR14, *+AR1[OR6]            \t\n"
        "   |   VSTW            VR15, *+AR1[OR7]            \t\n"
        "   |   SADDA.M1        R46, AR2, AR2                                   \t\n" // ==
        // [8]
        "       VSTW            VR16, *AR2                  \t\n" // ==
        "   |   VSTW            VR17, *+AR2[OR5]            \t\n" // ==
        // [9]
        "       VSTW            VR18, *+AR2[OR6]            \t\n" // ==
        "   |   VSTW            VR19, *+AR2[OR7]            \t\n" // ==
        "   |   SBR             R63         \t\n"
        // [10]
        "       VSTW            VR20, *AR2                  \t\n" // ==
        "   |   VSTW            VR21, *+AR2[OR5]            \t\n" // ==
        // [11]
        "       VSTW            VR22, *+AR2[OR6]            \t\n" // ==
        "   |   VSTW            VR23, *+AR2[OR7]            \t\n" // ==
        "   |   SADDA.M1        R46, AR3, AR3                                   \t\n" // ==
        // [12]
        "       VSTW            VR24, *AR3                  \t\n" // ==
        "   |   VSTW            VR25, *+AR3[OR5]            \t\n" // ==
        // [13]
        "       VSTW            VR26, *+AR3[OR6]            \t\n" // ==
        "   |   VSTW            VR27, *+AR3[OR7]            \t\n" // ==
        // [14]
        "       VSTW            VR28, *AR3                  \t\n" // ==
        "   |   VSTW            VR29, *+AR3[OR5]            \t\n" // ==
        // [15]
        "       VSTW            VR30, *+AR3[OR6]            \t\n" // ==
        "   |   VSTW            VR31, *+AR3[OR7]            \t\n" // ==
    :
    :"r"(src_a), "r"(dst_b), "r"(spm_D), "r"(m), "r"(n)
    );
}


void micro_kernel_16x16x2_add(lvector double* src_a, lvector double* dst_b, lvector double* spm_D,
                        const unlong m, const unlong n){
    __asm__ __volatile__(// m:%3     n:%4
        // --------------------------- step 1 ---------------------------
        // cycles: 3 + 16 + 2
        // [-3]
        "       SSHFLL          1, %4, R52      ;; R52 = 2*n                    \t\n"
        "   |   SMVAGA.M1       %0, AR0         ;; AR0 = src_A                  \t\n" 
        "   |   SMVAGA.M2       %4, OR1         ;; OR1 = n                      \t\n"
        // [-2]
        "       SSHFLL          5, %4, R54      ;; R54 = 4*(8*n)                \t\n"
        "   |   SADD.M1         %4, R52, R53    ;; R53 = 3*n                    \t\n"     
        "   |   SMVAGA.M2       R52, OR2        ;; OR2 = 2*n                    \t\n"
        // [-1]
        "       SSHFLL          6, %4, R56      ;; R56 = 8*(8*n)                \t\n"
        "   |   SMVAGA.M1       R53, OR3        ;; OR3 = 3*n                    \t\n"
        "   |   SADDA.M2        R54, AR0, AR0                                   \t\n"
        // [0]
        "       SMOVI24         128, R61        ;; R61 = 16*8                   \t\n" // ==
        "   |   SADD.M1         R56, %0, R56    ;; R56 = src_A + 8*(8*n)        \t\n"
        "   |   VLDW            *AR0, VR0                   \t\n"
        "   |   VLDW            *+AR0[OR1], VR4             \t\n"
        // [1]
        "       SMVAGA.M1       R56, AR1        ;; AR1 = src_A + 8*(8*n)        \t\n"
        "   |   SADD.M2         R61, %0, R50    ;; R50 = src_A + 16*8           \t\n" // ==
        "   |   VLDW            *+AR0[OR2], VR8             \t\n"
        "   |   VLDW            *+AR0[OR3], VR12            \t\n"
        // [2]
        "       SADD.M1         R61, R56, R51   ;; R51 = src_A + 16*8 + 8*(8*n) \t\n" // ==
        "   |   SMVAGA.M2       R50, AR2        ;; AR2 = src_A + 16*8           \t\n" // ==
        "   |   VLDW            *AR0, VR1                   \t\n"
        "   |   VLDW            *+AR0[OR1], VR5             \t\n"
        // [3]
        "       SADDA.M1        R54, AR1, AR1                                   \t\n"
        "   |   SMVAGA.M2       R51, AR3        ;; AR3 = src_A + 16*8 + 8*(8*n) \t\n" // ==
        "   |   VLDW            *+AR0[OR2], VR9             \t\n"
        "   |   VLDW            *+AR0[OR3], VR13            \t\n"
        // [4]
        "       SADD.M1         R54, R50, R57   ;; R51 = src_A+16*8 + 4*(8*n)   \t\n" // ==
        "   |   SADD.M2         R54, R51, R58   ;; R51 = src_A+16*8 + 12*(8*n)  \t\n" // ==
        "   |   VLDW            *AR1, VR2                   \t\n"
        "   |   VLDW            *+AR1[OR1], VR6             \t\n"
        // [5]
        "       SMVAGA.M1       R57, AR6        ;; AR6 = src_A+16*8 + 4*(8*n)   \t\n" // ==
        "   |   SMVAGA.M2       R58, AR7        ;; AR6 = src_A+16*8 + 12*(8*n)  \t\n" // ==
        "   |   VLDW            *+AR1[OR2], VR10            \t\n"
        "   |   VLDW            *+AR1[OR3], VR14            \t\n"
        // [6]
        "       SSHFLL          1, %3, R44      ;; R44 = 2*m                    \t\n"
        "   |   SMVAGA.M1       %1, AR0         ;; AR0 = src_B                  \t\n"
        "   |   SMVAGA.M2       %3, OR5         ;; OR5 = m                      \t\n"
        "   |   VLDW            *AR1, VR3                   \t\n"
        "   |   VLDW            *+AR1[OR1], VR7             \t\n"
        // [7]
        "       SSHFLL          5, %3, R46      ;; R46 = 4*(8*m)                \t\n"
        "   |   SADD.M1         %3, R44, R45    ;; R45 = 3*m                    \t\n"
        "   |   SMVAGA.M2       R44, OR6        ;; OR6 = 2*m                    \t\n"
        "   |   VLDW            *+AR1[OR2], VR11            \t\n"
        "   |   VLDW            *+AR1[OR3], VR15            \t\n"
        // [8]
        "       SSHFLL          6, %3, R47      ;; R47 = 8*(8*m)                \t\n"
        "   |   SMVAGA.M1       R45, OR7        ;; OR7 = 3*m                    \t\n"
        "   |   VLDW            *AR2, VR16                  \t\n" // ==
        "   |   VLDW            *+AR2[OR1], VR20            \t\n" // ==
        // [9]
        "       SADD.M1         R47, %1, R48    ;; R48 = src_B + 8*(8*m)        \t\n"
        "   |   VLDW            *AR6, VR17                  \t\n" // ==
        "   |   VLDW            *+AR6[OR1], VR21            \t\n" // ==
        // [10]
        "       SADD.M1         R47, R48, R49   ;; R49 = src_B + 16*(8*m)       \t\n" // ==
        "   |   SMVAGA.M2       R48, AR1        ;; AR1 = src_B + 8*(8*m)        \t\n"
        "   |   VLDW            *AR3, VR18                  \t\n" // ==
        "   |   VLDW            *+AR3[OR1], VR22            \t\n" // ==
        // [11]
        "       VLDW            *AR7, VR19                  \t\n" // ==
        "   |   VLDW            *+AR7[OR1], VR23            \t\n" // ==
        // [12]
        "       SMOVI24         2048, R60       ;; R60 = 256*8                  \t\n" // ==
        "   |   SADD.M1         R47, R49, R50   ;; R50 = src_B + 24*(8*m)       \t\n" // ==
        "   |   SMVAGA.M2       R49, AR2        ;; AR2 = src_B + 16*(8*m)       \t\n" // ==
        "   |   VLDW            *+AR2[OR2], VR24            \t\n" // ==
        "   |   VLDW            *+AR2[OR3], VR28            \t\n" // ==
        // [13]
        "       SADD.M1         R60, %2, R59    ;; R59 = spm_D + 256*8          \t\n" // ==
        "   |   VLDW            *+AR6[OR2], VR25            \t\n" // ==
        "   |   VLDW            *+AR6[OR3], VR29            \t\n" // ==
        // [14]
        "       SMVAGA.M1       R59, AR5        ;; AR5 = spm_D + 256*8          \t\n" // ==
        "   |   SMVAGA.M2       R50, AR3        ;; AR3 = src_B + 24*(8*m)       \t\n" // ==
        "   |   VLDW            *+AR3[OR2], VR26            \t\n" // ==
        "   |   VLDW            *+AR3[OR3], VR30            \t\n" // ==
        // [15]
        "       SMVAGA.M1       %2, AR4         ;; AR4 = spm_D                  \t\n"
        "   |   VLDW            *+AR7[OR2], VR27            \t\n" // ==
        "   |   VLDW            *+AR7[OR3], VR31            \t\n" // ==
        "   |   SADDA.M2        R46, AR0, AR0               \t\n"
        // [16]
        "       VLDW            *AR0, VR32                  \t\n"
        "   |   VLDW            *+AR0[OR5], VR33            \t\n"
        // [17]
        "       VLDW            *+AR0[OR6], VR34            \t\n"
        "   |   VLDW            *+AR0[OR7], VR35            \t\n"
        // --------------------------- step 2 ---------------------------
        // cycles: 8
        "       VSTDW0M16       VR1:VR0, *AR4               \t\n"
        "   |   VSTDW1M16       VR3:VR2, *+AR4[16]          \t\n"
        "       VSTDW0M16       VR5:VR4, *+AR4[32]          \t\n"
        "   |   VSTDW1M16       VR7:VR6, *+AR4[48]          \t\n"
        "       VSTDW0M16       VR9:VR8, *+AR4[64]          \t\n"
        "   |   VSTDW1M16       VR11:VR10, *+AR4[80]        \t\n"
        "       VSTDW0M16       VR13:VR12, *+AR4[96]        \t\n"
        "   |   VSTDW1M16       VR15:VR14, *+AR4[112]       \t\n"
        "       VSTDW0M16       VR17:VR16, *AR5             \t\n" // ==
        "   |   VSTDW1M16       VR19:VR18, *+AR5[16]        \t\n" // ==
        "       VSTDW0M16       VR21:VR20, *+AR5[32]        \t\n" // ==
        "   |   VSTDW1M16       VR23:VR22, *+AR5[48]        \t\n" // ==
        "       VSTDW0M16       VR25:VR24, *+AR5[64]        \t\n" // ==
        "   |   VSTDW1M16       VR27:VR26, *+AR5[80]        \t\n" // ==
        "       VSTDW0M16       VR29:VR28, *+AR5[96]        \t\n" // ==
        "   |   VSTDW1M16       VR31:VR30, *+AR5[112]       \t\n" // ==
        // --------------------------- step 3 ---------------------------
        // cycles: 16 + 2
        // [0 ~ 15]
        "       VLDW            *AR4, VR0                   \t\n"
        "   |   VLDW            *+AR4[16], VR4              \t\n"
        "       VLDW            *+AR4[64], VR1              \t\n"
        "   |   VLDW            *+AR4[80], VR5              \t\n"
        "       VLDW            *+AR4[128], VR2             \t\n"
        "   |   VLDW            *+AR4[144], VR6             \t\n"
        "       VLDW            *+AR4[192], VR3             \t\n"
        "   |   VLDW            *+AR4[208], VR7             \t\n"
        "       VLDW            *+AR4[32], VR8              \t\n"
        "   |   VLDW            *+AR4[48], VR12             \t\n"
        "       VLDW            *+AR4[96], VR9              \t\n"
        "   |   VLDW            *+AR4[112], VR13            \t\n"
        "       VLDW            *+AR4[160], VR10            \t\n"
        "   |   VLDW            *+AR4[176], VR14            \t\n"
        "       VLDW            *+AR4[224], VR11            \t\n"
        "   |   VLDW            *+AR4[240], VR15            \t\n"
        "       VLDW            *AR5, VR16                  \t\n" // ==
        "   |   VLDW            *+AR5[16], VR20             \t\n" // ==
        "       VLDW            *+AR5[64], VR17             \t\n" // ==
        "   |   VLDW            *+AR5[80], VR21             \t\n" // ==
        "       VLDW            *+AR5[128], VR18            \t\n" // ==
        "   |   VLDW            *+AR5[144], VR22            \t\n" // ==
        "       VLDW            *+AR5[192], VR19            \t\n" // ==
        "   |   VLDW            *+AR5[208], VR23            \t\n" // ==
        "       VLDW            *+AR5[32], VR24             \t\n" // ==
        "   |   VLDW            *+AR5[48], VR28             \t\n" // ==
        "       VLDW            *+AR5[96], VR25             \t\n" // ==
        "   |   VLDW            *+AR5[112], VR29            \t\n" // ==
        "       VLDW            *+AR5[160], VR26            \t\n" // ==
        "   |   VLDW            *+AR5[176], VR30            \t\n" // ==
        "       VLDW            *+AR5[224], VR27            \t\n" // ==
        "   |   VLDW            *+AR5[240], VR31            \t\n" // ==
        // [15 + 1,2]
        "       VLDW            *AR0, VR36                  \t\n"
        "   |   VLDW            *+AR0[OR5], VR37            \t\n"
        "   |   SSUBA.M2        R46, AR0, AR0               \t\n"
        "       VLDW            *+AR0[OR6], VR38            \t\n"
        "   |   VLDW            *+AR0[OR7], VR39            \t\n"
       // --------------------------- step 4 ---------------------------
       // cycles: 8
        "       VSTDW0M16       VR1:VR0, *AR4               \t\n"
        "   |   VSTDW1M16       VR3:VR2, *+AR4[16]          \t\n"
        "       VSTDW0M16       VR5:VR4, *+AR4[32]          \t\n"
        "   |   VSTDW1M16       VR7:VR6, *+AR4[48]          \t\n"
        "       VSTDW0M16       VR9:VR8, *+AR4[64]          \t\n"
        "   |   VSTDW1M16       VR11:VR10, *+AR4[80]        \t\n"
        "       VSTDW0M16       VR13:VR12, *+AR4[96]        \t\n"
        "   |   VSTDW1M16       VR15:VR14, *+AR4[112]       \t\n"
        "       VSTDW0M16       VR17:VR16, *AR5             \t\n" // ==
        "   |   VSTDW1M16       VR19:VR18, *+AR5[16]        \t\n" // ==
        "       VSTDW0M16       VR21:VR20, *+AR5[32]        \t\n" // ==
        "   |   VSTDW1M16       VR23:VR22, *+AR5[48]        \t\n" // ==
        "       VSTDW0M16       VR25:VR24, *+AR5[64]        \t\n" // ==
        "   |   VSTDW1M16       VR27:VR26, *+AR5[80]        \t\n" // ==
        "       VSTDW0M16       VR29:VR28, *+AR5[96]        \t\n" // ==
        "   |   VSTDW1M16       VR31:VR30, *+AR5[112]       \t\n" // ==
        // --------------------------- step 5 ---------------------------
        // [0]
        "       VLDDW           *AR4, VR1:VR0               \t\n" // VR32,33,34,35
        "   |   VLDDW           *+AR4[16], VR3:VR2          \t\n"
        // [1]
        "       VLDDW           *+AR4[32], VR5:VR4          \t\n" // VR36,37,38,39
        "   |   VLDDW           *+AR4[48], VR7:VR6          \t\n"
        // [2]
        "       VLDDW           *+AR4[64], VR9:VR8          \t\n" // VR40,41,42,43
        "   |   VLDDW           *+AR4[80], VR11:VR10        \t\n"
        "   |   SADDA.M1        R46, AR1, AR1               \t\n"
        // [3]
        "       VLDW            *AR1, VR40                  \t\n"
        "   |   VLDW            *+AR1[OR5], VR41            \t\n"
        // [4]
        "       VLDW            *+AR1[OR6], VR42            \t\n"
        "   |   VLDW            *+AR1[OR7], VR43            \t\n"
        // [5]
        "       VLDDW           *+AR4[96], VR13:VR12        \t\n" // VR44,45,46,47
        "   |   VLDDW           *+AR4[112], VR15:VR14       \t\n"
        // [6]
        "       VLDW            *AR1, VR44                  \t\n"
        "   |   VLDW            *+AR1[OR5], VR45            \t\n"
        "   |   SSUBA.M2        R46, AR1, AR1               \t\n"
        // [7]
        "       VLDW            *+AR1[OR6], VR46            \t\n"
        "   |   VLDW            *+AR1[OR7], VR47            \t\n"
        // [8]
        "       VLDDW           *AR5, VR17:VR16             \t\n" // VR48,49,50,51
        "   |   VLDDW           *+AR5[16], VR19:VR18        \t\n"
        "   |   SADDA.M1        R46, AR2, AR2               \t\n"
        "   |   VMOVI           0x3ff0000000000000,VR63	    ;; VR63 = 1     \t\n" 
        // [9]
        "       VLDW            *AR2, VR48                  \t\n"
        "   |   VLDW            *+AR2[OR5], VR49            \t\n"
        "   |   VFMULAD.M1	    VR32, VR63, VR0, VR0        \t\n"
        "   |   VFMULAD.M2	    VR33, VR63, VR1, VR1        \t\n"
        "   |   VFMULAD.M3	    VR34, VR63, VR2, VR2        \t\n"
        // [10]
        "       VLDW            *+AR2[OR6], VR50            \t\n"
        "   |   VLDW            *+AR2[OR7], VR51            \t\n"
        "   |   VFMULAD.M1	    VR35, VR63, VR3, VR3        \t\n"
        "   |   VFMULAD.M2	    VR36, VR63, VR4, VR4        \t\n"
        "   |   VFMULAD.M3	    VR37, VR63, VR5, VR5        \t\n"
        // [11]
        "       VLDDW           *+AR5[32], VR21:VR20        \t\n" // VR52,53,54,55
        "   |   VLDDW           *+AR5[48], VR23:VR22        \t\n"
        "   |   VFMULAD.M1	    VR38, VR63, VR6, VR6        \t\n"
        "   |   VFMULAD.M2	    VR39, VR63, VR7, VR7        \t\n"
        // [12]
        "       VLDW            *AR2, VR52                  \t\n"
        "   |   VLDW            *+AR2[OR5], VR53            \t\n"
        "   |   SSUBA.M2        R46, AR2, AR2               \t\n"
        "   |   VFMULAD.M1	    VR40, VR63, VR8, VR8        \t\n"
        "   |   VFMULAD.M2	    VR41, VR63, VR9, VR9        \t\n"
        // [13]
        "       VLDW            *+AR2[OR6], VR54            \t\n"
        "   |   VLDW            *+AR2[OR7], VR55            \t\n"
        "   |   VFMULAD.M1	    VR42, VR63, VR10, VR10      \t\n"
        "   |   VFMULAD.M2	    VR43, VR63, VR11, VR11      \t\n"
        // [14]
        "       VLDDW           *+AR5[64], VR25:VR24        \t\n" // VR56,57,58,59
        "   |   VLDDW           *+AR5[80], VR27:VR26        \t\n"
        "   |   SADDA.M1        R46, AR3, AR3               \t\n"
        // [15]
        "       VLDW            *AR3, VR56                  \t\n"
        "   |   VLDW            *+AR3[OR5], VR57            \t\n"
        "   |   VFMULAD.M1	    VR44, VR63, VR12, VR12      \t\n"
        "   |   VFMULAD.M2	    VR45, VR63, VR13, VR13      \t\n"
        // [16]
        "       VLDW            *+AR3[OR6], VR58            \t\n"
        "   |   VLDW            *+AR3[OR7], VR59            \t\n"
        "   |   VFMULAD.M1	    VR46, VR63, VR14, VR14      \t\n"
        "   |   VFMULAD.M2	    VR47, VR63, VR15, VR15      \t\n"
        // [17]
        "       VLDDW           *+AR5[96], VR29:VR28        \t\n" // VR60,61,62,63
        "   |   VLDDW           *+AR5[112], VR31:VR30       \t\n"
        // [18]
        "       VLDW            *AR3, VR60                  \t\n"
        "   |   VLDW            *+AR3[OR5], VR61            \t\n"
        "   |   SSUBA.M2        R46, AR3, AR3               \t\n"
        "   |   VMOVI           0x3ff0000000000000,VR33	    ;; VR33 = 1     \t\n" 
        "   |   VFMULAD.M1	    VR48, VR63, VR16, VR16      \t\n"
        "   |   VFMULAD.M2	    VR49, VR63, VR17, VR17      \t\n"
        // [19]
        "       VLDW            *+AR3[OR6], VR62            \t\n"
        "   |   VLDW            *+AR3[OR7], VR63            \t\n"
        "   |   VFMULAD.M1	    VR50, VR33, VR18, VR18      \t\n"
        "   |   VFMULAD.M2	    VR51, VR33, VR19, VR19      \t\n"
        "   |   SADDA.M1        R46, AR0, AR0                                   \t\n"
        // --------------------------- step 6 ---------------------------
        // [0]
        "       VSTW            VR0, *AR0                   \t\n"
        "   |   VSTW            VR1, *+AR0[OR5]             \t\n"
        // [1]
        "       VSTW            VR2, *+AR0[OR6]             \t\n"
        "   |   VSTW            VR3, *+AR0[OR7]             \t\n"
        // [2]
        "       VSTW            VR4, *AR0                   \t\n"
        "   |   VSTW            VR5, *+AR0[OR5]             \t\n"
        // [3]
        "       VSTW            VR6, *+AR0[OR6]             \t\n"
        "   |   VSTW            VR7, *+AR0[OR7]             \t\n"
        "   |   SADDA.M1        R46, AR1, AR1                                   \t\n"
        // [4]
        "       VSTW            VR8, *AR1                   \t\n"
        "   |   VSTW            VR9, *+AR1[OR5]             \t\n"
        "   |   VFMULAD.M1	    VR52, VR33, VR20, VR20      \t\n"
        "   |   VFMULAD.M2	    VR53, VR33, VR21, VR21      \t\n"
        // [5]
        "       VSTW            VR10, *+AR1[OR6]            \t\n"
        "   |   VSTW            VR11, *+AR1[OR7]            \t\n"
        "   |   VFMULAD.M1	    VR54, VR33, VR22, VR22      \t\n"
        "   |   VFMULAD.M2	    VR55, VR33, VR23, VR23      \t\n"
        // [6]
        "       VSTW            VR12, *AR1                  \t\n"
        "   |   VSTW            VR13, *+AR1[OR5]            \t\n"
        "   |   VFMULAD.M1	    VR56, VR33, VR24, VR24      \t\n"
        "   |   VFMULAD.M2	    VR57, VR33, VR25, VR25      \t\n"
        // [7]
        "       VSTW            VR14, *+AR1[OR6]            \t\n"
        "   |   VSTW            VR15, *+AR1[OR7]            \t\n"
        "   |   SADDA.M1        R46, AR2, AR2                                   \t\n" // ==
        "   |   VFMULAD.M1	    VR58, VR33, VR26, VR26      \t\n"
        "   |   VFMULAD.M2	    VR59, VR33, VR27, VR27      \t\n"
        // [8]
        "       VSTW            VR16, *AR2                  \t\n" // ==
        "   |   VSTW            VR17, *+AR2[OR5]            \t\n" // ==
        "   |   VFMULAD.M1	    VR60, VR33, VR28, VR28      \t\n"
        "   |   VFMULAD.M2	    VR61, VR33, VR29, VR29      \t\n"
        // [9]
        "       VSTW            VR18, *+AR2[OR6]            \t\n" // ==
        "   |   VSTW            VR19, *+AR2[OR7]            \t\n" // ==
        "   |   VFMULAD.M1	    VR62, VR33, VR30, VR30      \t\n"
        "   |   VFMULAD.M2	    VR63, VR33, VR31, VR31      \t\n"
        "   |   SBR             R63         \t\n"
        // [10]
        "       VSTW            VR20, *AR2                  \t\n" // ==
        "   |   VSTW            VR21, *+AR2[OR5]            \t\n" // ==
        // [11]
        "       VSTW            VR22, *+AR2[OR6]            \t\n" // ==
        "   |   VSTW            VR23, *+AR2[OR7]            \t\n" // ==
        "   |   SADDA.M1        R46, AR3, AR3                                   \t\n" // ==
        // [12]
        "       VSTW            VR24, *AR3                  \t\n" // ==
        "   |   VSTW            VR25, *+AR3[OR5]            \t\n" // ==
        // [13]
        "       VSTW            VR26, *+AR3[OR6]            \t\n" // ==
        "   |   VSTW            VR27, *+AR3[OR7]            \t\n" // ==
        // [14]
        "       VSTW            VR28, *AR3                  \t\n" // ==
        "   |   VSTW            VR29, *+AR3[OR5]            \t\n" // ==
        // [15]
        "       VSTW            VR30, *+AR3[OR6]            \t\n" // ==
        "   |   VSTW            VR31, *+AR3[OR7]            \t\n" // ==
    :
    :"r"(src_a), "r"(dst_b), "r"(spm_D), "r"(m), "r"(n)
    );
}


__global__ void  transpose_A2B(unlong M, unlong N, unlong b_id, double* A, double* B){
    const int max_core = 4;
    const int grp_size = min(get_group_size(), max_core);
    const int tid = get_thread_id();
    if(tid >= grp_size)
        return;    
    const unlong h_a = 16;
    const unlong w_a = 16;
    const unlong w_2_a = w_a << 1;
    // --------- blocking parameters ---------
    const unlong M_A = 128;
    const unlong N_A = 128;
    // ---------------------------------------
    const unlong c_start = tid * N_A;
    const unlong c_step = grp_size * N_A;
    bool row_syn = false;
    unint p2pmask = 0x00;
    if(grp_size != 1){
        row_syn = true;
        for(int i=0; i<grp_size; i++){
            p2pmask <<= 1;
            p2pmask |= 1;
        }
    } 
    // unlong t_0, t_1, t_2, t_3;
    lvector double* spm_A[2];
    lvector double* spm_B[2];
    spm_A[0] = vector_malloc(M_A * N_A * sizeof(double));
    spm_A[1] = vector_malloc(M_A * N_A * sizeof(double));
    spm_B[0] = vector_malloc(M_A * N_A * sizeof(double));
    spm_B[1] = vector_malloc(M_A * N_A * sizeof(double));
    lvector double* spm_D = vector_malloc(512 * sizeof(double));

    int ch_al[2], ch_bs[2];
    
    const int ch0_al = 0;
    const int ch0_bs = 2;

    unint cnt_a = 0;
    unint cnt_d = 0;
    unint cnt_b = 0;
    unlong M_A_cur = min(M_A, M);
    unlong N_A_cur = min(N_A, N-c_start);
#ifndef CNACEL_DMA
    ch_al[0] = dma_p2p_opt(&A[OFFSET(0, c_start, N)], M_A_cur, N_A_cur*sizeof(double), (N-N_A_cur)*sizeof(double),
                            spm_A[0], M_A_cur, N_A_cur*sizeof(double), (N_A-N_A_cur)*sizeof(double), row_syn, p2pmask, ch0_al);
    ch_bs[1] = dma_p2p_opt(B, 1, 1*sizeof(double), 0, // 发起一次无效传输
                            spm_B[1], 1, 1*sizeof(double), 0, 0, 0, ch0_bs+1);
#endif
    for(unlong r=0; r<M; r+=M_A){
        unlong M_A_next;
        for(unlong c=c_start; c<N; c+=c_step){
            // -------------------------- next round indexes --------------------------
            unlong N_A_next;
            unint cnt_a_1 = (cnt_a + 1) % 2;
            unint cnt_b_1 = (cnt_b + 1) % 2;
            // ------------------------------------------------------------------------
            if(c + c_step < N){
                N_A_next = min(N_A, N-(c+c_step));
        #ifndef CNACEL_DMA
                ch_al[cnt_a_1] = dma_p2p_opt(&A[OFFSET(r, c+c_step, N)], M_A_cur, N_A_next*sizeof(double), (N-N_A_next)*sizeof(double),
                            spm_A[cnt_a_1], M_A_cur, N_A_next*sizeof(double), (N_A-N_A_next)*sizeof(double), row_syn, p2pmask, ch0_al+cnt_a_1);
        #endif
            }else if(r + M_A < M){
                N_A_next = min(N_A, N-c_start);
                M_A_next = min(M_A, M-(r+M_A));
        #ifndef CNACEL_DMA
                ch_al[cnt_a_1] = dma_p2p_opt(&A[OFFSET(r+M_A, c_start, N)], M_A_next, N_A_next*sizeof(double), (N-N_A_next)*sizeof(double),
                            spm_A[cnt_a_1], M_A_next, N_A_next*sizeof(double), (N_A-N_A_next)*sizeof(double), row_syn, p2pmask, ch0_al+cnt_a_1);
        #endif
            }
        #ifndef CNACEL_DMA
            dma_wait_p2p(ch_al[cnt_a]);
        #endif
            for(unlong m=0; m<M_A_cur; m+=h_a){
                unlong n = 0;
                while(n + w_a < N_A_cur){
                    // t_0 = get_clk();
                    micro_kernel_16x16x2(&spm_A[cnt_a][OFFSET(m, n>>4, N_A>>4)], &spm_B[cnt_b][OFFSET(n, m>>4, M_A>>4)], spm_D, M_A, N_A);
                    // t_1 = get_clk();
                    n += w_2_a;
                }
                if(n < N_A_cur){
                    // t_2 = get_clk();
                    micro_kernel_16x16(&spm_A[cnt_a][OFFSET(m, n>>4, N_A>>4)], &spm_B[cnt_b][OFFSET(n, m>>4, M_A>>4)], spm_D, M_A, N_A);
                    // t_3 = get_clk();
                }
            }
        #ifndef CNACEL_DMA
            ch_bs[cnt_b] = dma_p2p_opt(spm_B[cnt_b], N_A_cur, M_A_cur*sizeof(double), (M_A-M_A_cur)*sizeof(double),
                            &B[OFFSET(c, r, M)], N_A_cur, M_A_cur*sizeof(double), (M-M_A_cur)*sizeof(double), row_syn, p2pmask, ch0_bs+cnt_b);

            dma_wait_p2p(ch_bs[cnt_b_1]);
        #endif
            // -------------------------- renew indexes --------------------------
            cnt_a = cnt_a_1;
            cnt_b = cnt_b_1;
            N_A_cur = N_A_next;
        }
        M_A_cur = M_A_next;
    }
#ifndef CNACEL_DMA
    dma_wait_p2p(ch_bs[(cnt_b+1)%2]);
#endif
    // hthread_printf("t_1 - t_0 = %ld\n", t_1 - t_0);
    // hthread_printf("t_3 - t_2 = %ld\n", t_3 - t_2);
    vector_free(spm_A[0]);
    vector_free(spm_A[1]);
    vector_free(spm_B[0]);
    vector_free(spm_B[1]);
    vector_free(spm_D);
}


void micro_kernel_asm_r6c64(double* src_a, lvector double* src_b, lvector double* dst_c, 
                        const indexType kb, const indexType k_offset, const indexType n_offset){
    __asm__ __volatile__(
        // A一行的字数: k_offset
        // A两行的字数: k_offset*2
        // A三行的字节数: k_offset*3*8
            // [-3]
        "       SSHFLL          3, %4, R7                   ;; R7 = k_offset*8      \t\n"
	    "   |	SMVAGA.M1	    %1, AR4    			        ;; AR4 = src_B          \t\n"
        "   |   SMVAGA.M2       %4, OR9                     ;; OR9 = k_offset       \t\n"
        "   |   VMOVI.M1 	    0x00, VR0				    ;; 初始化C寄存器         \t\n"
	    "   |	VMOVI.M2 	    0x3ff0000000000000,VR63	    ;; VR63 = 0             \t\n" 
            // [-2]
        "       SSHFLR          1, %4, R15                  ;; R15 = k_offset/2     \t\n"
        "   |   SADD.M1         R7, R7, R9                  ;; R9 = 2*k_offset*8    \t\n"
        "   |	SMVAGA.M2	    %0, AR10   			        ;; AR10 = src_A         \t\n"
        "   |   VMOV.M1		    VR0, VR1				    ;; 初始化C寄存器         \t\n"
	    "   |	VMOV.M2		    VR0, VR2				    ;; 初始化C寄存器         \t\n"
	    "   |	VMOV.M3		    VR0, VR3				    ;; 初始化C寄存器         \t\n"
            // [-1]
        "       SSHFLR          1, %5, R42                  ;; BC_buffer一行多少双字 \t\n"
        "   |   SMVAGA.M1       R15, OR8                    ;; OR8 = k_offset/2     \t\n"
        "   |   VMOV.M1		    VR0, VR4				    ;; 初始化C寄存器         \t\n"
	    "   |	VMOV.M2		    VR0, VR5				    ;; 初始化C寄存器         \t\n"
	    "   |	VMOV.M3		    VR0, VR6				    ;; 初始化C寄存器         \t\n"
            // [5]
        "       SSHFLL          3, %3, R5                   ;; R5 = kb*8            \t\n"
        "   |   SADD.M1         R7, R9, R11                 ;; R11 = 3*k_offset*8   \t\n"
        "   |   SMVAGA.M2       R42, OR2                    ;; BC_buffer一行多少双字 \t\n"
        "   |   SLDDW		    *AR10, R51:R50              \t\n"
        "   |   VMOV.M1		    VR0, VR7				    ;; 初始化C寄存器         \t\n"
	    "   |	VMOV.M2		    VR0, VR8				    ;; 初始化C寄存器         \t\n"
	    "   |	VMOV.M3		    VR0, VR9				    ;; 初始化C寄存器         \t\n"
            // [6]
        "       SADD.M1         R11, %0, R11                ;; R11 = src_a + 3*k_offset*8 \t\n"
        "   |   SLDDW		    *+AR10[OR8], R53:R52        \t\n"
        "   |   SADDA.M2        16, AR10, AR10              \t\n"
        "   |   VMOV.M1		    VR0, VR10				    ;; 初始化C寄存器         \t\n"
	    "   |	VMOV.M2		    VR0, VR11				    ;; 初始化C寄存器         \t\n"
	    "   |	VMOV.M3		    VR0, VR12				    ;; 初始化C寄存器         \t\n"
            // [7]
        "       SMVAGA.M1       R11, AR13                   ;; AR13 = src_a+3*k_offset*8  \t\n"
        "   |   SADD.M2		    R5, %0, R21			        ;; R21 = src_A + kb*8   \t\n"
        "   |   SLDDW		    *+AR10[OR9], R55:R54        \t\n"
        "   |   VLDDW 		    *AR4++[OR2], VR25:VR24		;; load next_B[1][0,1]  \t\n"
        "   |   VLDDW 		    *+AR4[16], VR27:VR26		;; load next_B[1][2,3]  \t\n"
        "   |   VMOV.M1		    VR0, VR13				    ;; 初始化C寄存器         \t\n"
	    "   |	VMOV.M2		    VR0, VR14				    ;; 初始化C寄存器         \t\n"
	    "   |	VMOV.M3		    VR0, VR15				    ;; 初始化C寄存器         \t\n"
            // [8]
        "       SSUB.M1		    16, R21, R21			    ;; R21 = src_A+(kb-2)*8 \t\n"
        "   |   SMVAAG.M2	    AR10, %0		            \t\n"
        "   |   VMOV.M1		    VR0, VR16				    ;; 初始化C寄存器         \t\n"
	    "   |	VMOV.M2		    VR0, VR17				    ;; 初始化C寄存器         \t\n"
	    "   |	VMOV.M3		    VR0, VR18				    ;; 初始化C寄存器         \t\n"
            // [9]
        "   	SMVAGA.M1	    %2, AR5				        ;; AR5 = dst_C          \t\n"
        "   |   SMVAGA.M2	    %2, AR6				        ;; AR6 = dst_C          \t\n"
        "   |   SLDDW		    *AR13, R57:R56              \t\n"
        "   |   VMOV.M1		    VR0, VR19				    ;; 初始化C寄存器         \t\n"
	    "   |	VMOV.M2		    VR0, VR20				    ;; 初始化C寄存器         \t\n"
	    "   |	VMOV.M3		    VR0, VR21				    ;; 初始化C寄存器         \t\n"
            // [10]
        "       SLDDW		    *+AR13[OR8], R59:R58        \t\n"
        "   |   SADDA.M2        16, AR13, AR13              \t\n"
        "   |   VMOV.M1		    VR0, VR22				    ;; 初始化C寄存器         \t\n"
	    "   |	VMOV.M2		    VR0, VR23				    ;; 初始化C寄存器         \t\n"
            // [11]
        "       SLDDW		    *+AR13[OR9], R61:R60        \t\n"
        "   |   VLDDW 		    *AR5++[OR2], VR45:VR44		;; load mat_C[0][0,1]   \t\n"
        "   |   VLDDW           *+AR5[16], VR47:VR46        ;; load mat_C[0][2,3]   \t\n"
            // [12]     
        "       SVBCAST.M1 	    R50, VR32                   \t\n"
        "   |   VLDDW 		    *AR5++[OR2], VR49:VR48		;; load mat_C[1][0,1]   \t\n"
        "   |   VLDDW           *+AR5[16], VR51:VR50        ;; load mat_C[1][2,3]   \t\n"
            // [13]
        "       SVBCAST.M1 	    R52, VR34                   \t\n"
        "   |   VLDDW 		    *AR5++[OR2], VR53:VR52		;; load mat_C[2][0,1]   \t\n"
        "   |   VLDDW           *+AR5[16], VR55:VR54        ;; load mat_C[2][2,3]   \t\n"
            // [14]
        "       SVBCAST.M1 	    R54, VR36                   \t\n"
        "   |   VLDDW 		    *AR4++[OR2], VR29:VR28		;; load next_B[1][0,1]  \t\n"
        "   |   VLDDW 		    *+AR4[16], VR31:VR30		;; load next_B[1][2,3]  \t\n"
            // [15]
        "       VLDDW 		    *AR5++[OR2], VR57:VR56		;; load mat_C[3][0,1]   \t\n"
        "   |   VLDDW           *+AR5[16], VR59:VR58        ;; load mat_C[3][2,3]   \t\n"

        "loop_k_r6c64:       \t\n"
//------------------------------------------ main loop start ------------------------------------------
            // [0]
        "   	SLT		        %0, R21, R0                 \t\n"
        "   |	VFMULAD.M1	    VR32, VR24, VR0, VR0        \t\n"
	    "   |	VFMULAD.M2	    VR32, VR25, VR1, VR1        \t\n"
	    "   |	VFMULAD.M3	    VR32, VR26, VR2, VR2        \t\n"
        "   |   SVBCAST.M1 	    R56, VR38                   \t\n"
            // [1]
        "       VFMULAD.M1	    VR32, VR27, VR3, VR3        \t\n"
	    "   |	VFMULAD.M2	    VR34, VR24, VR4, VR4        \t\n"
	    "   |	VFMULAD.M3	    VR34, VR25, VR5, VR5        \t\n"
        "   |   SVBCAST.M1 	    R58, VR40                   \t\n"
            // [2]
        "       VFMULAD.M1	    VR34, VR26, VR6, VR6        \t\n"
	    "   |	VFMULAD.M2	    VR34, VR27, VR7, VR7        \t\n"
	    "   |	VFMULAD.M3	    VR36, VR24, VR8, VR8        \t\n"
        "   |   SVBCAST.M1 	    R60, VR42                   \t\n"
            // [3]
        "       VFMULAD.M1	    VR36, VR25, VR9, VR9        \t\n"
	    "   |	VFMULAD.M2	    VR36, VR26, VR10, VR10      \t\n"
	    "   |	VFMULAD.M3	    VR36, VR27, VR11, VR11      \t\n"
            // [4]
        "       VFMULAD.M1	    VR38, VR24, VR12, VR12      \t\n"
	    "   |	VFMULAD.M2	    VR38, VR25, VR13, VR13      \t\n"
	    "   |	VFMULAD.M3	    VR38, VR26, VR14, VR14      \t\n"
        "   |   SVBCAST.M1 	    R51, VR33                   \t\n"
            // [5]
        "       VFMULAD.M1	    VR38, VR27, VR15, VR15      \t\n"
	    "   |	VFMULAD.M2	    VR40, VR24, VR16, VR16      \t\n"
	    "   |	VFMULAD.M3	    VR40, VR25, VR17, VR17      \t\n"
        "   |	SLDDW		    *AR10, R51:R50              \t\n"
        "   |   SVBCAST.M1 	    R53, VR35                   \t\n"
            // [6]
        "       VFMULAD.M1	    VR40, VR26, VR18, VR18      \t\n"
	    "   |	VFMULAD.M2	    VR40, VR27, VR19, VR19      \t\n"
	    "   |	VFMULAD.M3	    VR42, VR24, VR20, VR20      \t\n"
        "   |	SLDDW		    *+AR10[OR8], R53:R52        \t\n"
        "   |   SVBCAST.M1 	    R55, VR37                   \t\n"
        "   |   SADDA.M2        16, AR10, AR10              \t\n"
        "   |   VLDDW 		    *AR4++[OR2], VR25:VR24		\t\n"
        "   |   VLDDW 		    *+AR4[16], VR27:VR26		\t\n"
            // [7]
        "       VFMULAD.M1	    VR42, VR25, VR21, VR21      \t\n"
	    "   |	VFMULAD.M2	    VR42, VR26, VR22, VR22      \t\n"
	    "   |	VFMULAD.M3	    VR42, VR27, VR23, VR23      \t\n"
        "   |	SLDDW		    *+AR10[OR9], R55:R54        \t\n"
            // [8]
        "       VFMULAD.M1	    VR33, VR28, VR0, VR0        \t\n"
	    "   |	VFMULAD.M2	    VR33, VR29, VR1, VR1        \t\n"
	    "   |	VFMULAD.M3	    VR33, VR30, VR2, VR2        \t\n"
        "   |   SVBCAST.M1 	    R57, VR39                   \t\n"
        "   |   SMVAAG.M2	    AR10, %0		            \t\n"
            // [9]
        "       VFMULAD.M1	    VR33, VR31, VR3, VR3        \t\n"
	    "   |	VFMULAD.M2	    VR35, VR28, VR4, VR4        \t\n"
	    "   |	VFMULAD.M3	    VR35, VR29, VR5, VR5        \t\n"
        "   |	SLDDW		    *AR13, R57:R56              \t\n"
        "   |   SVBCAST.M1 	    R59, VR41                   \t\n"
        "   |   [R0]	SBR		loop_k_r6c64                \t\n"
            // [10]
        "       VFMULAD.M1	    VR35, VR30, VR6, VR6        \t\n"
	    "   |	VFMULAD.M2	    VR35, VR31, VR7, VR7        \t\n"
	    "   |	VFMULAD.M3	    VR37, VR28, VR8, VR8        \t\n"
        "   |	SLDDW		    *+AR13[OR8], R59:R58        \t\n"
        "   |   SVBCAST.M1 	    R61, VR43                   \t\n"
        "   |   SADDA.M2        16, AR13, AR13              \t\n"
            // [11]
        "       VFMULAD.M1	    VR37, VR29, VR9, VR9        \t\n"
	    "   |	VFMULAD.M2	    VR37, VR30, VR10, VR10      \t\n"
	    "   |	VFMULAD.M3	    VR37, VR31, VR11, VR11      \t\n"
        "   |	SLDDW		    *+AR13[OR9], R61:R60        \t\n"
            // [12]
        "       VFMULAD.M1	    VR39, VR28, VR12, VR12      \t\n"
	    "   |	VFMULAD.M2	    VR39, VR29, VR13, VR13      \t\n"
	    "   |	VFMULAD.M3	    VR39, VR30, VR14, VR14      \t\n"
        "   |   SVBCAST.M1 	    R50, VR32                   \t\n"
            // [13]
        "       VFMULAD.M1	    VR39, VR31, VR15, VR15      \t\n"
	    "   |	VFMULAD.M2	    VR41, VR28, VR16, VR16      \t\n"
	    "   |	VFMULAD.M3	    VR41, VR29, VR17, VR17      \t\n"
        "   |   SVBCAST.M1 	    R52, VR34                   \t\n"
            // [14]
        "       VFMULAD.M1	    VR41, VR30, VR18, VR18      \t\n"
	    "   |	VFMULAD.M2	    VR41, VR31, VR19, VR19      \t\n"
	    "   |	VFMULAD.M3	    VR43, VR28, VR20, VR20      \t\n"
        "   |   SVBCAST.M1 	    R54, VR36                   \t\n"
        "   |   VLDDW 		    *AR4++[OR2], VR29:VR28		\t\n"
        "   |   VLDDW 		    *+AR4[16], VR31:VR30		\t\n"
            // [15]
        "       VFMULAD.M1	    VR43, VR29, VR21, VR21      \t\n"
	    "   |	VFMULAD.M2	    VR43, VR30, VR22, VR22      \t\n"
	    "   |	VFMULAD.M3	    VR43, VR31, VR23, VR23      \t\n"
//------------------------------------------ last iteration ------------------------------------------
            // [0]
        "       VFMULAD.M1	    VR32, VR24, VR0, VR0        \t\n"
	    "   |	VFMULAD.M2	    VR32, VR25, VR1, VR1        \t\n"
	    "   |	VFMULAD.M3	    VR32, VR26, VR2, VR2        \t\n"
        "   |   SVBCAST.M1 	    R56, VR38                   \t\n"
            // [1]
        "       VFMULAD.M1	    VR32, VR27, VR3, VR3        \t\n"
	    "   |	VFMULAD.M2	    VR34, VR24, VR4, VR4        \t\n"
	    "   |	VFMULAD.M3	    VR34, VR25, VR5, VR5        \t\n"
        "   |   SVBCAST.M1 	    R58, VR40                   \t\n"
            // [2]
        "       VFMULAD.M1	    VR34, VR26, VR6, VR6        \t\n"
	    "   |	VFMULAD.M2	    VR34, VR27, VR7, VR7        \t\n"
	    "   |	VFMULAD.M3	    VR36, VR24, VR8, VR8        \t\n"
        "   |   SVBCAST.M1 	    R60, VR42                   \t\n"
            // [3]
        "       VFMULAD.M1	    VR36, VR25, VR9, VR9        \t\n"
	    "   |	VFMULAD.M2	    VR36, VR26, VR10, VR10      \t\n"
	    "   |	VFMULAD.M3	    VR36, VR27, VR11, VR11      \t\n"
            // [4]
        "       VFMULAD.M1	    VR38, VR24, VR12, VR12      \t\n"
	    "   |	VFMULAD.M2	    VR38, VR25, VR13, VR13      \t\n"
	    "   |	VFMULAD.M3	    VR38, VR26, VR14, VR14      \t\n"
        "   |   SVBCAST.M1 	    R51, VR33                   \t\n"
            // [5]
        "       VFMULAD.M1	    VR38, VR27, VR15, VR15      \t\n"
	    "   |	VFMULAD.M2	    VR40, VR24, VR16, VR16      \t\n"
	    "   |	VFMULAD.M3	    VR40, VR25, VR17, VR17      \t\n"
        "   |   SVBCAST.M1 	    R53, VR35                   \t\n"
            // [6]
        "       VFMULAD.M1	    VR40, VR26, VR18, VR18      \t\n"
	    "   |	VFMULAD.M2	    VR40, VR27, VR19, VR19      \t\n"
	    "   |	VFMULAD.M3	    VR42, VR24, VR20, VR20      \t\n"
        "   |   SVBCAST.M1 	    R55, VR37                   \t\n"
            // [7]
        "       VFMULAD.M1	    VR42, VR25, VR21, VR21      \t\n"
	    "   |	VFMULAD.M2	    VR42, VR26, VR22, VR22      \t\n"
	    "   |	VFMULAD.M3	    VR42, VR27, VR23, VR23      \t\n"
            // [8]
        "       VFMULAD.M1	    VR33, VR28, VR0, VR0        \t\n"
	    "   |	VFMULAD.M2	    VR33, VR29, VR1, VR1        \t\n"
	    "   |	VFMULAD.M3	    VR33, VR30, VR2, VR2        \t\n"
        "   |   SVBCAST.M1 	    R57, VR39                   \t\n"
            // [9]
        "       VFMULAD.M1	    VR33, VR31, VR3, VR3        \t\n"
	    "   |	VFMULAD.M2	    VR35, VR28, VR4, VR4        \t\n"
	    "   |	VFMULAD.M3	    VR35, VR29, VR5, VR5        \t\n"
        "   |   SVBCAST.M1 	    R59, VR41                   \t\n"
            // [10]
        "       VFMULAD.M1	    VR35, VR30, VR6, VR6        \t\n"
	    "   |	VFMULAD.M2	    VR35, VR31, VR7, VR7        \t\n"
	    "   |	VFMULAD.M3	    VR37, VR28, VR8, VR8        \t\n"
        "   |   SVBCAST.M1 	    R61, VR43                   \t\n"
            // [11]
        "       VFMULAD.M1	    VR37, VR29, VR9, VR9        \t\n"
	    "   |	VFMULAD.M2	    VR37, VR30, VR10, VR10      \t\n"
	    "   |	VFMULAD.M3	    VR37, VR31, VR11, VR11      \t\n"
            // [12]
        "       VFMULAD.M1	    VR39, VR28, VR12, VR12      \t\n"
	    "   |	VFMULAD.M2	    VR39, VR29, VR13, VR13      \t\n"
	    "   |	VFMULAD.M3	    VR39, VR30, VR14, VR14      \t\n"
        "   |   VLDDW 		    *AR5++[OR2], VR25:VR24		;; load mat_C[4][0,1]   \t\n"
        "   |   VLDDW           *+AR5[16], VR27:VR26        ;; load mat_C[4][2,3]   \t\n"
            // [13]
        "       VFMULAD.M1	    VR39, VR31, VR15, VR15      \t\n"
	    "   |	VFMULAD.M2	    VR41, VR28, VR16, VR16      \t\n"
	    "   |	VFMULAD.M3	    VR41, VR29, VR17, VR17      \t\n"
        "   |   VLDDW 		    *AR5++[OR2], VR33:VR32		;; load mat_C[5][0,1]   \t\n"
        "   |   VLDDW           *+AR5[16], VR35:VR34        ;; load mat_C[5][2,3]   \t\n"
            // [14]
        "       VFMULAD.M1	    VR41, VR30, VR18, VR18      \t\n"
	    "   |	VFMULAD.M2	    VR41, VR31, VR19, VR19      \t\n"
	    "   |	VFMULAD.M3	    VR43, VR28, VR20, VR20      \t\n"
            // [15]
        "       VFMULAD.M1	    VR43, VR29, VR21, VR21      \t\n"
	    "   |	VFMULAD.M2	    VR43, VR30, VR22, VR22      \t\n"
	    "   |	VFMULAD.M3	    VR43, VR31, VR23, VR23      \t\n"
//------------------------------------------ main loop end ------------------------------------------
	    // write-back:
            // [0]
	    "   	VFMULAD.M1	VR0, VR63, VR44, VR0            \t\n"
	    "   |	VFMULAD.M2	VR1, VR63, VR45, VR1            \t\n"
	    "   |	VFMULAD.M3	VR2, VR63, VR46, VR2            \t\n"
            // [1]
	    "   	VFMULAD.M1	VR3, VR63, VR47, VR3            \t\n"
	    "   |	VFMULAD.M2	VR4, VR63, VR48, VR4            \t\n"
	    "   |	VFMULAD.M3	VR5, VR63, VR49, VR5            \t\n"
	        // [2]
	    "   	VFMULAD.M1	VR6, VR63, VR50, VR6            \t\n"
	    "   |	VFMULAD.M2	VR7, VR63, VR51, VR7            \t\n"
	    "   |	VFMULAD.M3	VR8, VR63, VR52, VR8            \t\n"
	        // [3]
	    "   	VFMULAD.M1	VR9, VR63, VR53, VR9            \t\n"
	    "   |	VFMULAD.M2	VR10, VR63, VR54, VR10          \t\n"
	    "   |	VFMULAD.M3	VR11, VR63, VR55, VR11          \t\n"
	        // [4]
	    "   	VFMULAD.M1	VR12, VR63, VR56, VR12          \t\n"
	    "   |	VFMULAD.M2	VR13, VR63, VR57, VR13          \t\n"
	    "   |	VFMULAD.M3	VR14, VR63, VR58, VR14          \t\n"
	        // [5]
	    "   	VFMULAD.M1	VR15, VR63, VR59, VR15          \t\n"
	    "   |	VFMULAD.M2	VR16, VR63, VR24, VR16          \t\n"
	    "   |	VFMULAD.M3	VR17, VR63, VR25, VR17          \t\n"
            // [6]
	    "   	VFMULAD.M1	VR18, VR63, VR26, VR18          \t\n"
	    "   |	VFMULAD.M2	VR19, VR63, VR27, VR19          \t\n"
	    "   |	VFMULAD.M3	VR20, VR63, VR32, VR20          \t\n"
            // [7]
        "   	VFMULAD.M1	VR21, VR63, VR33, VR21          \t\n"
	    "   |	VFMULAD.M2	VR22, VR63, VR34, VR22          \t\n"
	    "   |	VFMULAD.M3	VR23, VR63, VR35, VR23          \t\n"
        "   |   VSTDW       VR1:VR0, *AR6++[OR2]            \r\n"
        "   |   VSTDW       VR3:VR2, *+AR6[16]              \r\n"
        "   |   SBR		    R63                             \t\n"
            // [8]
        "       VSTDW       VR5:VR4, *AR6++[OR2]            \r\n"
        "   |   VSTDW       VR7:VR6, *+AR6[16]              \r\n"
            // [9]
        "       VSTDW       VR9:VR8, *AR6++[OR2]            \r\n"
        "   |   VSTDW       VR11:VR10, *+AR6[16]            \r\n"
            // [10]
        "       SNOP        1       \t\n"
            // [11]
        "       VSTDW       VR13:VR12, *AR6++[OR2]          \r\n"
        "   |   VSTDW       VR15:VR14, *+AR6[16]            \r\n"
            // [12]
        "       VSTDW       VR17:VR16, *AR6++[OR2]          \r\n"
        "   |   VSTDW       VR19:VR18, *+AR6[16]            \r\n"
            // [13]    
        "       VSTDW       VR21:VR20, *AR6++[OR2]          \r\n"
        "   |   VSTDW       VR23:VR22, *+AR6[16]            \r\n"
    :
    :"r"(src_a), "r"(src_b), "r"(dst_c), "r"(kb), "r"(k_offset), "r"(n_offset)
    );
}

void micro_kernel_asm_r6c64_only_write(double* src_a, lvector double* src_b, lvector double* dst_c, 
                        const indexType kb, const indexType k_offset, const indexType n_offset){
    __asm__ __volatile__(
        // A一行的字数: k_offset
        // A两行的字数: k_offset*2
        // A三行的字节数: k_offset*3*8
            // [-3]
        "       SSHFLL          3, %4, R7                   ;; R7 = k_offset*8      \t\n"
	    "   |	SMVAGA.M1	    %1, AR4    			        ;; AR4 = src_B          \t\n"
        "   |   SMVAGA.M2       %4, OR9                     ;; OR9 = k_offset       \t\n"
        "   |   VMOVI.M1 	    0x00, VR0				    ;; 初始化C寄存器         \t\n"
            // [-2]
        "       SSHFLR          1, %4, R15                  ;; R15 = k_offset/2     \t\n"
        "   |   SADD.M1         R7, R7, R9                  ;; R9 = 2*k_offset*8    \t\n"
        "   |	SMVAGA.M2	    %0, AR10   			        ;; AR10 = src_A         \t\n"
        "   |   VMOV.M1		    VR0, VR1				    ;; 初始化C寄存器         \t\n"
	    "   |	VMOV.M2		    VR0, VR2				    ;; 初始化C寄存器         \t\n"
	    "   |	VMOV.M3		    VR0, VR3				    ;; 初始化C寄存器         \t\n"
            // [-1]
        "       SSHFLR          1, %5, R42                  ;; BC_buffer一行多少双字 \t\n"
        "   |   SMVAGA.M1       R15, OR8                    ;; OR8 = k_offset/2     \t\n"
        "   |   VMOV.M1		    VR0, VR4				    ;; 初始化C寄存器         \t\n"
	    "   |	VMOV.M2		    VR0, VR5				    ;; 初始化C寄存器         \t\n"
	    "   |	VMOV.M3		    VR0, VR6				    ;; 初始化C寄存器         \t\n"
            // [5]
        "       SSHFLL          3, %3, R5                   ;; R5 = kb*8            \t\n"
        "   |   SADD.M1         R7, R9, R11                 ;; R11 = 3*k_offset*8   \t\n"
        "   |   SMVAGA.M2       R42, OR2                    ;; BC_buffer一行多少双字 \t\n"
        "   |   SLDDW		    *AR10, R51:R50              \t\n"
        "   |   VMOV.M1		    VR0, VR7				    ;; 初始化C寄存器         \t\n"
	    "   |	VMOV.M2		    VR0, VR8				    ;; 初始化C寄存器         \t\n"
	    "   |	VMOV.M3		    VR0, VR9				    ;; 初始化C寄存器         \t\n"
            // [6]
        "       SADD.M1         R11, %0, R11                ;; R11 = src_a + 3*k_offset*8 \t\n"
        "   |   SLDDW		    *+AR10[OR8], R53:R52        \t\n"
        "   |   SADDA.M2        16, AR10, AR10              \t\n"
        "   |   VMOV.M1		    VR0, VR10				    ;; 初始化C寄存器         \t\n"
	    "   |	VMOV.M2		    VR0, VR11				    ;; 初始化C寄存器         \t\n"
	    "   |	VMOV.M3		    VR0, VR12				    ;; 初始化C寄存器         \t\n"
            // [7]
        "       SMVAGA.M1       R11, AR13                   ;; AR13 = src_a+3*k_offset*8  \t\n"
        "   |   SADD.M2		    R5, %0, R21			        ;; R21 = src_A + kb*8   \t\n"
        "   |   SLDDW		    *+AR10[OR9], R55:R54        \t\n"
        "   |   VLDDW 		    *AR4++[OR2], VR25:VR24		;; load next_B[1][0,1]  \t\n"
        "   |   VLDDW 		    *+AR4[16], VR27:VR26		;; load next_B[1][2,3]  \t\n"
        "   |   VMOV.M1		    VR0, VR13				    ;; 初始化C寄存器         \t\n"
	    "   |	VMOV.M2		    VR0, VR14				    ;; 初始化C寄存器         \t\n"
	    "   |	VMOV.M3		    VR0, VR15				    ;; 初始化C寄存器         \t\n"
            // [8]
        "       SMVAAG.M2	    AR10, %0		            \t\n"
        "   |   VMOV.M1		    VR0, VR16				    ;; 初始化C寄存器         \t\n"
	    "   |	VMOV.M2		    VR0, VR17				    ;; 初始化C寄存器         \t\n"
	    "   |	VMOV.M3		    VR0, VR18				    ;; 初始化C寄存器         \t\n"
            // [9]
        "       SMVAGA.M2	    %2, AR6				        ;; AR6 = dst_C          \t\n"
        "   |   SLDDW		    *AR13, R57:R56              \t\n"
        "   |   VMOV.M1		    VR0, VR19				    ;; 初始化C寄存器         \t\n"
	    "   |	VMOV.M2		    VR0, VR20				    ;; 初始化C寄存器         \t\n"
	    "   |	VMOV.M3		    VR0, VR21				    ;; 初始化C寄存器         \t\n"
            // [10]
        "       SLDDW		    *+AR13[OR8], R59:R58        \t\n"
        "   |   SADDA.M2        16, AR13, AR13              \t\n"
        "   |   VMOV.M1		    VR0, VR22				    ;; 初始化C寄存器         \t\n"
	    "   |	VMOV.M2		    VR0, VR23				    ;; 初始化C寄存器         \t\n"
            // [11]
        "       SLDDW		    *+AR13[OR9], R61:R60        \t\n"
            // [12]     
        "       SVBCAST.M1 	    R50, VR32                   \t\n"
            // [13]
        "       SVBCAST.M1 	    R52, VR34                   \t\n"
            // [14]
        "       SVBCAST.M1 	    R54, VR36                   \t\n"
        "   |   VLDDW 		    *AR4++[OR2], VR29:VR28		;; load next_B[1][0,1]  \t\n"
        "   |   VLDDW 		    *+AR4[16], VR31:VR30		;; load next_B[1][2,3]  \t\n"
            // [15]
        "       SNOP            1                           \t\n"

        "loop_k_r6c64_only_write:       \t\n"
//------------------------------------------ main loop start ------------------------------------------
            // [0]
        "       VFMULAD.M1	    VR32, VR24, VR0, VR0        \t\n"
	    "   |	VFMULAD.M2	    VR32, VR25, VR1, VR1        \t\n"
	    "   |	VFMULAD.M3	    VR32, VR26, VR2, VR2        \t\n"
        "   |   SVBCAST.M1 	    R56, VR38                   \t\n"
            // [1]
        "   	SLT		        %0, R21, R0                 \t\n"
        "   |   VFMULAD.M1	    VR32, VR27, VR3, VR3        \t\n"
	    "   |	VFMULAD.M2	    VR34, VR24, VR4, VR4        \t\n"
	    "   |	VFMULAD.M3	    VR34, VR25, VR5, VR5        \t\n"
        "   |   SVBCAST.M1 	    R58, VR40                   \t\n"
            // [2]
        "       VFMULAD.M1	    VR34, VR26, VR6, VR6        \t\n"
	    "   |	VFMULAD.M2	    VR34, VR27, VR7, VR7        \t\n"
	    "   |	VFMULAD.M3	    VR36, VR24, VR8, VR8        \t\n"
        "   |   SVBCAST.M1 	    R60, VR42                   \t\n"
            // [3]
        "       VFMULAD.M1	    VR36, VR25, VR9, VR9        \t\n"
	    "   |	VFMULAD.M2	    VR36, VR26, VR10, VR10      \t\n"
	    "   |	VFMULAD.M3	    VR36, VR27, VR11, VR11      \t\n"
            // [4]
        "       VFMULAD.M1	    VR38, VR24, VR12, VR12      \t\n"
	    "   |	VFMULAD.M2	    VR38, VR25, VR13, VR13      \t\n"
	    "   |	VFMULAD.M3	    VR38, VR26, VR14, VR14      \t\n"
        "   |   SVBCAST.M1 	    R51, VR33                   \t\n"
            // [5]
        "       VFMULAD.M1	    VR38, VR27, VR15, VR15      \t\n"
	    "   |	VFMULAD.M2	    VR40, VR24, VR16, VR16      \t\n"
	    "   |	VFMULAD.M3	    VR40, VR25, VR17, VR17      \t\n"
        "   |	SLDDW		    *AR10, R51:R50              \t\n"
        "   |   SVBCAST.M1 	    R53, VR35                   \t\n"
            // [6]
        "       VFMULAD.M1	    VR40, VR26, VR18, VR18      \t\n"
	    "   |	VFMULAD.M2	    VR40, VR27, VR19, VR19      \t\n"
	    "   |	VFMULAD.M3	    VR42, VR24, VR20, VR20      \t\n"
        "   |	SLDDW		    *+AR10[OR8], R53:R52        \t\n"
        "   |   SVBCAST.M1 	    R55, VR37                   \t\n"
        "   |   SADDA.M2        16, AR10, AR10              \t\n"
        "   |   VLDDW 		    *AR4++[OR2], VR25:VR24		\t\n"
        "   |   VLDDW 		    *+AR4[16], VR27:VR26		\t\n"
            // [7]
        "       VFMULAD.M1	    VR42, VR25, VR21, VR21      \t\n"
	    "   |	VFMULAD.M2	    VR42, VR26, VR22, VR22      \t\n"
	    "   |	VFMULAD.M3	    VR42, VR27, VR23, VR23      \t\n"
        "   |	SLDDW		    *+AR10[OR9], R55:R54        \t\n"
            // [8]
        "       VFMULAD.M1	    VR33, VR28, VR0, VR0        \t\n"
	    "   |	VFMULAD.M2	    VR33, VR29, VR1, VR1        \t\n"
	    "   |	VFMULAD.M3	    VR33, VR30, VR2, VR2        \t\n"
        "   |   SVBCAST.M1 	    R57, VR39                   \t\n"
        "   |   SMVAAG.M2	    AR10, %0		            \t\n"
            // [9]
        "       VFMULAD.M1	    VR33, VR31, VR3, VR3        \t\n"
	    "   |	VFMULAD.M2	    VR35, VR28, VR4, VR4        \t\n"
	    "   |	VFMULAD.M3	    VR35, VR29, VR5, VR5        \t\n"
        "   |	SLDDW		    *AR13, R57:R56              \t\n"
        "   |   SVBCAST.M1 	    R59, VR41                   \t\n"
        "   |   [R0]	SBR		loop_k_r6c64_only_write                \t\n"
            // [10]
        "       VFMULAD.M1	    VR35, VR30, VR6, VR6        \t\n"
	    "   |	VFMULAD.M2	    VR35, VR31, VR7, VR7        \t\n"
	    "   |	VFMULAD.M3	    VR37, VR28, VR8, VR8        \t\n"
        "   |	SLDDW		    *+AR13[OR8], R59:R58        \t\n"
        "   |   SVBCAST.M1 	    R61, VR43                   \t\n"
        "   |   SADDA.M2        16, AR13, AR13              \t\n"
            // [11]
        "       VFMULAD.M1	    VR37, VR29, VR9, VR9        \t\n"
	    "   |	VFMULAD.M2	    VR37, VR30, VR10, VR10      \t\n"
	    "   |	VFMULAD.M3	    VR37, VR31, VR11, VR11      \t\n"
        "   |	SLDDW		    *+AR13[OR9], R61:R60        \t\n"
            // [12]
        "       VFMULAD.M1	    VR39, VR28, VR12, VR12      \t\n"
	    "   |	VFMULAD.M2	    VR39, VR29, VR13, VR13      \t\n"
	    "   |	VFMULAD.M3	    VR39, VR30, VR14, VR14      \t\n"
        "   |   SVBCAST.M1 	    R50, VR32                   \t\n"
            // [13]
        "       VFMULAD.M1	    VR39, VR31, VR15, VR15      \t\n"
	    "   |	VFMULAD.M2	    VR41, VR28, VR16, VR16      \t\n"
	    "   |	VFMULAD.M3	    VR41, VR29, VR17, VR17      \t\n"
        "   |   SVBCAST.M1 	    R52, VR34                   \t\n"
            // [14]
        "       VFMULAD.M1	    VR41, VR30, VR18, VR18      \t\n"
	    "   |	VFMULAD.M2	    VR41, VR31, VR19, VR19      \t\n"
	    "   |	VFMULAD.M3	    VR43, VR28, VR20, VR20      \t\n"
        "   |   SVBCAST.M1 	    R54, VR36                   \t\n"
        "   |   VLDDW 		    *AR4++[OR2], VR29:VR28		\t\n"
        "   |   VLDDW 		    *+AR4[16], VR31:VR30		\t\n"
            // [15]
        "       VFMULAD.M1	    VR43, VR29, VR21, VR21      \t\n"
	    "   |	VFMULAD.M2	    VR43, VR30, VR22, VR22      \t\n"
	    "   |	VFMULAD.M3	    VR43, VR31, VR23, VR23      \t\n"
        "   |   [!R0]   SBR     R63                         \t\n"
//------------------------------------------ main loop end ------------------------------------------
	    // write-back:
            // [0]
        "       VSTDW       VR1:VR0, *AR6++[OR2]            \r\n"
        "   |   VSTDW       VR3:VR2, *+AR6[16]              \r\n"
            // [1]
        "       VSTDW       VR5:VR4, *AR6++[OR2]            \r\n"
        "   |   VSTDW       VR7:VR6, *+AR6[16]              \r\n"
            // [2]
        "       VSTDW       VR9:VR8, *AR6++[OR2]            \r\n"
        "   |   VSTDW       VR11:VR10, *+AR6[16]            \r\n"
            // [3]
        "       VSTDW       VR13:VR12, *AR6++[OR2]          \r\n"
        "   |   VSTDW       VR15:VR14, *+AR6[16]            \r\n"
            // [4]
        "       VSTDW       VR17:VR16, *AR6++[OR2]          \r\n"
        "   |   VSTDW       VR19:VR18, *+AR6[16]            \r\n"
            // [5]    
        "       VSTDW       VR21:VR20, *AR6++[OR2]          \r\n"
        "   |   VSTDW       VR23:VR22, *+AR6[16]            \r\n"
    :
    :"r"(src_a), "r"(src_b), "r"(dst_c), "r"(kb), "r"(k_offset), "r"(n_offset)
    );
}


// traverse order: N_G -> K_G -> M_A -> K_A -> N_S
// to be ensured: N_G == N_A
__global__ void irr_thinB_dgemm_TT_dbuf(unlong M, unlong N, unlong K, unlong b_id, double* A, double* B, double* C){
    const int tid = get_thread_id();
    const int grp_size = get_group_size();
    // --------------------------- blocking params ---------------------------
    const indexType N_G = 192;
    const indexType K_G = 2048;
    const indexType M_A = 64;
    const indexType K_A = 512;
    const indexType N_A = N_G;
    const indexType N_S = 6;
    // --------------------------- blocking params ---------------------------
    const indexType NK_G = N_G * K_G;
    const indexType m_start = tid * M_A;
    const indexType m_step = grp_size * M_A;
    bool row_syn = false;
    unint p2pmask_most = 0x00;
    unint p2pmask_last = 0x00;
    if(grp_size != 1){
        row_syn = true;
        long M_last = M % m_step;
        if(M_last==0)
            M_last = m_step;
        int ncore_last = (M_last + M_A - 1) / M_A;
        int i=0;
        for(; i<ncore_last; i++){
            p2pmask_most <<= 1;
            p2pmask_most |= 1;
        }
        p2pmask_last = p2pmask_most;
        if(M != M_last){
            for(; i<grp_size; i++){
                p2pmask_most <<= 1;
                p2pmask_most |= 1;
            }
        }
    }
    unint p2pmask_cur = p2pmask_most;
    
    double* spm_B[2];
    lvector double* spm_A[2];
    lvector double* spm_C[2];
    spm_B[0] = scalar_malloc(N_S * K_A *sizeof(double));
    spm_B[1] = scalar_malloc(N_S * K_A *sizeof(double));
    spm_A[0] = vector_malloc(K_A * M_A * sizeof(double));
    spm_A[1] = vector_malloc(K_A * M_A * sizeof(double));
    spm_C[0] = vector_malloc(N_A * M_A * sizeof(double));
    spm_C[1] = vector_malloc(N_A * M_A * sizeof(double));
    lvector double* spm_D = vector_malloc(512 * sizeof(double));

    int ch_gsm[2], ch_bl[2], ch_al[2], ch_cl[2], ch_cs;

    const int ch0_gsm = 8;
    const int ch0_al = 2;
    const int ch0_cl = 4; // high level: 4, 5
    const int ch0_bl = 0; // high level: 0, 1 
    const int ch0_cs = 6; // high level: 6, 7

    const unlong prir = 0b0000000011;
    set_prir(prir);

    long N_G_cur = min(N_G, N);
    long K_G_cur = min(K_G, K);
    long M_A_cur = min(M_A, M-m_start);
    long K_A_cur = min(K_A, K);
    long N_G_next, K_G_next, M_A_next, K_A_next;

    // preload B^T to gsm_mem
    if(tid==0){
        ch_gsm[0] = dma_p2p_opt(&B[OFFSET(0, 0, K)], N_G_cur, K_G_cur*sizeof(double), (K-K_G_cur)*sizeof(double),
                            gsm_mem, N_G_cur, K_G_cur*sizeof(double), (K_G-K_G_cur)*sizeof(double), 0, 0, ch0_gsm);
    }
    if(m_start < M){
        // 启动一次无效传输
        ch_cs = dma_p2p_opt(C, 1, 1*sizeof(double), 0,
                            spm_C[1], 1, 1*sizeof(double), 0, 0, 0, ch0_cs);
        // preload spm_A
        ch_al[0] = dma_p2p_opt(&A[OFFSET(0, m_start, M)], K_A_cur, M_A_cur*sizeof(double), (M-M_A_cur)*sizeof(double),
                            spm_A[0], K_A_cur, M_A_cur*sizeof(double), (M_A-M_A_cur)*sizeof(double), row_syn, p2pmask_cur, ch0_al);
        // preload spm_C
        ch_cl[0] = dma_p2p_opt(&C[OFFSET(m_start, 0, N)], M_A_cur, N_G_cur*sizeof(double), (N-N_G_cur)*sizeof(double),
                            spm_C[0], M_A_cur, N_G_cur*sizeof(double), (N_A-N_G_cur)*sizeof(double), row_syn, p2pmask_cur, ch0_cl);
    }
    int cnt_gsm = 0;
    int cnt_a = 0;
    int cnt_c = 0;
    for(long n_o=0; n_o<N; n_o+=N_G){
        N_G_next = N_G_cur;
        for(long k_o=0; k_o<K; k_o+=K_G){
            int cnt_gsm_1 = (cnt_gsm + 1) % 2;
            int n_o_next, k_o_next;
            if(k_o + K_G < K){
                n_o_next = n_o;
                k_o_next = k_o + K_G;
            }else if(n_o + N_G < N){
                n_o_next = n_o + N_G;
                k_o_next = 0;
            }else{
                n_o_next = -1;
            }

            if(tid==0){
                // preload B_next to gsm_mem
                if(k_o + K_G < K){ // not row end
                    K_G_next = min(K_G, K-(k_o+K_G));
                    ch_gsm[cnt_gsm_1] = dma_p2p_opt(&B[OFFSET(n_o, k_o_next, K)], N_G_next, K_G_next*sizeof(double), (K-K_G_next)*sizeof(double),
                                            gsm_mem + NK_G*cnt_gsm_1, N_G_next, K_G_next*sizeof(double), (K_G-K_G_next)*sizeof(double), 0, 0, ch0_gsm+cnt_gsm_1);
                }else if(n_o + N_G < N){ // k + K_G >= K
                    K_G_next = min(K_G, K);
                    N_G_next = min(N_G, N-(n_o+N_G));
                    ch_gsm[cnt_gsm_1] = dma_p2p_opt(&B[OFFSET(n_o_next, 0, K)], N_G_next, K_G_next*sizeof(double), (K-K_G_next)*sizeof(double),
                                            gsm_mem + NK_G*cnt_gsm_1, N_G_next, K_G_next*sizeof(double), (K_G-K_G_next)*sizeof(double), 0, 0, ch0_gsm+cnt_gsm_1);
                }
                // wait for B_cur loaded to gsm_mem
                dma_wait_p2p(ch_gsm[cnt_gsm]);
            } else{
                if(k_o + K_G < K){
                    K_G_next = min(K_G, K-(k_o+K_G));
                }else if(n_o + N_G < N){
                    K_G_next = min(K_G, K);
                    N_G_next = min(N_G, N-(n_o+N_G));
                }
            }
            group_barrier(b_id);

            // preload gsm_mem to sm_B
            if(m_start < M){
                ch_bl[0] = dma_p2p_opt(gsm_mem + NK_G*cnt_gsm, N_S, K_A_cur*sizeof(double), (K_G-K_A_cur)*sizeof(double),
                                    spm_B[0], N_S, K_A_cur*sizeof(double), (K_A-K_A_cur)*sizeof(double), 0, 0, ch0_bl);
            }
            unint cnt_b = 0;

            long m_tail = m_step;
            for(long m_o=m_start; m_o<M; m_o+=m_step){
                unint p2pmask_next;
                if(m_o+m_step < M && m_tail+m_step >= M){ // current is not the last, next is the last
                    p2pmask_next = p2pmask_last;
                }else{
                    p2pmask_next = p2pmask_most;
                }
                m_tail += m_step;
                // compute C index
                unint cnt_c_1 = (cnt_c + 1) % 2;
                // compute m_o_next
                int m_o_next;
                if(m_o + m_step < M){ // m_o没到头
                    m_o_next = m_o + m_step;
                    M_A_next = min(M_A, M-(m_o+m_step));
                }else if(n_o_next == -1){ // m_o到头了，n_o,k_o也到头了
                    m_o_next = -1;
                }else{ // m_o到头了，n_o,k_o还没到头
                    m_o_next = m_start;
                    M_A_next = min(M_A, M-m_start);
                }

                // [0] wait for C stored
                dma_wait_p2p(ch_cs); 

                // [1] compute spm_C[cnt_c_1] 
                // ------------------ first iteration of k_i ------------------
                long k_i = 0, k_i_next;
                    // compute A index      
                    int cnt_a_1 = (cnt_a + 1) % 2;
                    // load spm_A[cnt_a_1]
                    if(k_i + K_A < K_G_cur){ // k_i没到头
                        k_i_next = k_i + K_A;
                        K_A_next = min(K_A, K_G_cur-(k_i+K_A));
                        ch_al[cnt_a_1] = dma_p2p_opt(&A[OFFSET(k_o+k_i+K_A, m_o, M)], K_A_next, M_A_cur*sizeof(double), (M-M_A_cur)*sizeof(double),
                                                spm_A[cnt_a_1], K_A_next, M_A_cur*sizeof(double), (M_A-M_A_cur)*sizeof(double), row_syn, p2pmask_cur, ch0_al+cnt_a_1);
                    }else if(m_o + m_step < M){ // k_i到头了，m_o没到头
                        k_i_next = 0;
                        K_A_next = min(K_A, K_G_cur);
                        ch_al[cnt_a_1] = dma_p2p_opt(&A[OFFSET(k_o, m_o+m_step, M)], K_A_next, M_A_next*sizeof(double), (M-M_A_next)*sizeof(double),
                                                spm_A[cnt_a_1], K_A_next, M_A_next*sizeof(double), (M_A-M_A_next)*sizeof(double), row_syn, p2pmask_next, ch0_al+cnt_a_1);
                    }else if(n_o_next != -1){ // k_i到头了，m_o到头了，n_o,k_o还没到头
                        k_i_next = 0;
                        K_A_next = min(K_A, K_G_next);
                        ch_al[cnt_a_1] = dma_p2p_opt(&A[OFFSET(k_o_next, m_start, M)], K_A_next, M_A_next*sizeof(double), (M-M_A_next)*sizeof(double),
                                                spm_A[cnt_a_1], K_A_next, M_A_next*sizeof(double), (M_A-M_A_next)*sizeof(double), row_syn, p2pmask_next, ch0_al+cnt_a_1);
                    }
                    // wait for spm_A[cnt_a]
                    dma_wait_p2p(ch_al[cnt_a]);
                    // inner loop
                    for(int n_b=0; n_b<N_G_cur; n_b+=2*N_S){
                        // preload B to sm
                        ch_bl[1] = dma_p2p_opt(gsm_mem + NK_G*cnt_gsm + (n_b+N_S)*K_G + k_i, N_S, K_A_cur*sizeof(double), (K_G-K_A_cur)*sizeof(double),
                                            spm_B[1], N_S, K_A_cur*sizeof(double), (K_A-K_A_cur)*sizeof(double), 0, 0, ch0_bl+1);
                        dma_wait_p2p(ch_bl[0]);
                        micro_kernel_asm_r6c64_only_write(spm_B[0], spm_A[cnt_a], spm_C[cnt_c_1] + n_b*(M_A>>4), K_A_cur, K_A, M_A);
                        // preload B to sm
                        if(n_b + 2*N_S < N_G_cur){ // n_b没到头
                            ch_bl[0] = dma_p2p_opt(gsm_mem + NK_G*cnt_gsm + (n_b+2*N_S)*K_G + k_i, N_S, K_A_cur*sizeof(double), (K_G-K_A_cur)*sizeof(double),
                                                spm_B[0], N_S, K_A_cur*sizeof(double), (K_A-K_A_cur)*sizeof(double), 0, 0, ch0_bl+0);
                        }else if(k_i + K_A < K_G_cur || m_o + m_step < M){ // n_b到头了，k_i或m_o没到头
                            ch_bl[0] = dma_p2p_opt(gsm_mem + NK_G*cnt_gsm + k_i_next, N_S, K_A_next*sizeof(double), (K_G-K_A_next)*sizeof(double),
                                                spm_B[0], N_S, K_A_next*sizeof(double), (K_A-K_A_next)*sizeof(double), 0, 0, ch0_bl+0);
                        }
                        dma_wait_p2p(ch_bl[1]);
                        micro_kernel_asm_r6c64_only_write(spm_B[1], spm_A[cnt_a], spm_C[cnt_c_1] + (n_b+N_S)*(M_A>>4), K_A_cur, K_A, M_A);
                    }
                    // renew index B
                    cnt_a = cnt_a_1;
                    K_A_cur = K_A_next;
                // ------------------ subsequent iterations of k_i ------------------
                for(k_i=K_A; k_i<K_G_cur; k_i+=K_A){    
                    // compute A index      
                    cnt_a_1 = (cnt_a + 1) % 2;
                    // load spm_A[cnt_a_1]
                    if(k_i + K_A < K_G_cur){ // k_i没到头
                        k_i_next = k_i + K_A;
                        K_A_next = min(K_A, K_G_cur-(k_i+K_A));
                        ch_al[cnt_a_1] = dma_p2p_opt(&A[OFFSET(k_o+k_i+K_A, m_o, M)], K_A_next, M_A_cur*sizeof(double), (M-M_A_cur)*sizeof(double),
                                                spm_A[cnt_a_1], K_A_next, M_A_cur*sizeof(double), (M_A-M_A_cur)*sizeof(double), row_syn, p2pmask_cur, ch0_al+cnt_a_1);
                    }else if(m_o + m_step < M){ // k_i到头了，m_o没到头
                        k_i_next = 0;
                        K_A_next = min(K_A, K_G_cur);
                        ch_al[cnt_a_1] = dma_p2p_opt(&A[OFFSET(k_o, m_o+m_step, M)], K_A_next, M_A_next*sizeof(double), (M-M_A_next)*sizeof(double),
                                                spm_A[cnt_a_1], K_A_next, M_A_next*sizeof(double), (M_A-M_A_next)*sizeof(double), row_syn, p2pmask_next, ch0_al+cnt_a_1);
                    }else if(n_o_next != -1){ // k_i到头了，m_o到头了，n_o,k_o还没到头
                        k_i_next = 0;
                        K_A_next = min(K_A, K_G_next);
                        ch_al[cnt_a_1] = dma_p2p_opt(&A[OFFSET(k_o_next, m_start, M)], K_A_next, M_A_next*sizeof(double), (M-M_A_next)*sizeof(double),
                                                spm_A[cnt_a_1], K_A_next, M_A_next*sizeof(double), (M_A-M_A_next)*sizeof(double), row_syn, p2pmask_next, ch0_al+cnt_a_1);
                    }
                    // wait for spm_A[cnt_a]
                    dma_wait_p2p(ch_al[cnt_a]);
                    // inner loop
                    for(int n_b=0; n_b<N_G_cur; n_b+=2*N_S){
                        // preload B to sm
                        ch_bl[1] = dma_p2p_opt(gsm_mem + NK_G*cnt_gsm + (n_b+N_S)*K_G + k_i, N_S, K_A_cur*sizeof(double), (K_G-K_A_cur)*sizeof(double),
                                            spm_B[1], N_S, K_A_cur*sizeof(double), (K_A-K_A_cur)*sizeof(double), 0, 0, ch0_bl+1);
                        dma_wait_p2p(ch_bl[0]);
                        micro_kernel_asm_r6c64(spm_B[0], spm_A[cnt_a], spm_C[cnt_c_1] + n_b*(M_A>>4), K_A_cur, K_A, M_A);
                        // preload B to sm
                        if(n_b + 2*N_S < N_G_cur){ // n_b没到头
                            ch_bl[0] = dma_p2p_opt(gsm_mem + NK_G*cnt_gsm + (n_b+2*N_S)*K_G + k_i, N_S, K_A_cur*sizeof(double), (K_G-K_A_cur)*sizeof(double),
                                                spm_B[0], N_S, K_A_cur*sizeof(double), (K_A-K_A_cur)*sizeof(double), 0, 0, ch0_bl+0);
                        }else if(k_i + K_A < K_G_cur || m_o + m_step < M){ // n_b到头了，k_i或m_o没到头
                            ch_bl[0] = dma_p2p_opt(gsm_mem + NK_G*cnt_gsm + k_i_next, N_S, K_A_next*sizeof(double), (K_G-K_A_next)*sizeof(double),
                                                spm_B[0], N_S, K_A_next*sizeof(double), (K_A-K_A_next)*sizeof(double), 0, 0, ch0_bl+0);
                        }
                        dma_wait_p2p(ch_bl[1]);
                        micro_kernel_asm_r6c64(spm_B[1], spm_A[cnt_a], spm_C[cnt_c_1] + (n_b+N_S)*(M_A>>4), K_A_cur, K_A, M_A);
                    }
                    // renew index B
                    cnt_a = cnt_a_1;
                    K_A_cur = K_A_next;
                }
                // ------------------ end loop of k_i ------------------

                // [2] wait for C loaded to spm_C[cnt_c]
                dma_wait_p2p(ch_cl[cnt_c]);

                // [3] transpose spm_C[cnt_c_1] to spm_C[cnt_c]
                for(int n_a=0; n_a<N_G_cur; n_a+=16){
                    for(int m_a=0; m_a<M_A_cur; m_a+=32){
                        micro_kernel_16x16x2_add(&spm_C[cnt_c_1][OFFSET(n_a, m_a>>4, M_A>>4)], &spm_C[cnt_c][OFFSET(m_a, n_a>>4, N_A>>4)], spm_D, N_A, M_A);
                    }
                }

                // [4] store spm_C[cnt_c] to C
                ch_cs = dma_p2p_opt(spm_C[cnt_c], M_A_cur, N_G_cur*sizeof(double), (N_G-N_G_cur)*sizeof(double),
                            &C[OFFSET(m_o, n_o, N)], M_A_cur, N_G_cur*sizeof(double), (N-N_G_cur)*sizeof(double), row_syn, p2pmask_cur, ch0_cs);

                // [5] preload C to spm_C[cnt_c_1]
                if(m_o + m_step < M){ // m_o没到头
                    ch_cl[cnt_c_1] = dma_p2p_opt(&C[OFFSET(m_o_next, n_o, N)], M_A_next, N_G_cur*sizeof(double), (N-N_G_cur)*sizeof(double),
                                        spm_C[cnt_c_1], M_A_next, N_G_cur*sizeof(double), (N_A-N_G_cur)*sizeof(double), row_syn, p2pmask_next, ch0_cl+cnt_c_1);
                }else if(m_o_next != -1){ // m_o到头了，n_o,k_o没到头
                    ch_cl[cnt_c_1] = dma_p2p_opt(&C[OFFSET(m_o_next, n_o_next, N)], M_A_next, N_G_next*sizeof(double), (N-N_G_next)*sizeof(double),
                                        spm_C[cnt_c_1], M_A_next, N_G_next*sizeof(double), (N_A-N_G_next)*sizeof(double), row_syn, p2pmask_next, ch0_cl+cnt_c_1);
                }

                // renew index C
                cnt_c = cnt_c_1;
                p2pmask_cur = p2pmask_next;
                M_A_cur = M_A_next;
            }
            cnt_gsm = cnt_gsm_1;
            K_G_cur = K_G_next;
            group_barrier(b_id);
        }
        N_G_cur = N_G_next;
    }
    // [0] wait for C stored
        dma_wait_p2p(ch_cs); 
    scalar_free(spm_B[0]);
    scalar_free(spm_B[1]);
    vector_free(spm_A[0]);
    vector_free(spm_A[1]);
    vector_free(spm_C[0]);
    vector_free(spm_C[1]);
    vector_free(spm_D);
}

// traverse order: N_G -> K_G -> M_A -> K_A -> N_S
// to be ensured: N_G == N_A
__global__ void irr_thinB_dgemm_NT_dbuf(unlong M, unlong N, unlong K, unlong b_id, double* A, double* B, double* C){
    const int tid = get_thread_id();
    const int grp_size = get_group_size();
    // --------------------------- blocking params ---------------------------
    const indexType N_G = 192;
    const indexType K_G = 2048;
    const indexType M_A = 64;
    const indexType K_A = 512;
    const indexType N_A = N_G;
    const indexType N_S = 6;
    // --------------------------- blocking params ---------------------------
    const indexType NK_G = N_G * K_G;
    const indexType m_start = tid * M_A;
    const indexType m_step = grp_size * M_A;
    bool row_syn = false;
    unint p2pmask_most = 0x00;
    unint p2pmask_last = 0x00;
    if(grp_size != 1){
        row_syn = true;
        long M_last = M % m_step;
        if(M_last==0)
            M_last = m_step;
        int ncore_last = (M_last + M_A - 1) / M_A;
        int i=0;
        for(; i<ncore_last; i++){
            p2pmask_most <<= 1;
            p2pmask_most |= 1;
        }
        p2pmask_last = p2pmask_most;
        if(M != M_last){
            for(; i<grp_size; i++){
                p2pmask_most <<= 1;
                p2pmask_most |= 1;
            }
        }
    }
    unint p2pmask_cur = p2pmask_most;
    
    double* spm_B[2];
    lvector double* spm_A[2];
    lvector double* spm_C[2];
    spm_B[0] = scalar_malloc(N_S * K_A *sizeof(double));
    spm_B[1] = scalar_malloc(N_S * K_A *sizeof(double));
    spm_A[0] = vector_malloc(K_A * M_A * sizeof(double));
    spm_A[1] = vector_malloc(K_A * M_A * sizeof(double));
    spm_C[0] = vector_malloc(N_A * M_A * sizeof(double));
    spm_C[1] = vector_malloc(N_A * M_A * sizeof(double));
    lvector double* spm_D = vector_malloc(512 * sizeof(double));

    int ch_gsm[2], ch_bl[2], ch_al, ch_cl[2], ch_cs;

    const int ch0_gsm = 8;
    const int ch0_al = 2;
    const int ch0_cl = 4; // high level: 4, 5
    const int ch0_bl = 0; // high level: 0, 1 
    const int ch0_cs = 6; // high level: 6, 7

    const unlong prir = 0b0000000011;
    set_prir(prir);

    long N_G_cur = min(N_G, N);
    long K_G_cur = min(K_G, K);
    long M_A_cur = min(M_A, M-m_start);
    long K_A_cur = min(K_A, K);
    long N_G_next, K_G_next, M_A_next, K_A_next;

    // preload B^T to gsm_mem
    if(tid==0){
        ch_gsm[0] = dma_p2p_opt(&B[OFFSET(0, 0, K)], N_G_cur, K_G_cur*sizeof(double), (K-K_G_cur)*sizeof(double),
                            gsm_mem, N_G_cur, K_G_cur*sizeof(double), (K_G-K_G_cur)*sizeof(double), 0, 0, ch0_gsm);
    }
    if(m_start < M){
        // 启动一次无效传输
        ch_cs = dma_p2p_opt(C, 1, 1*sizeof(double), 0,
                            spm_C[1], 1, 1*sizeof(double), 0, 0, 0, ch0_cs);
        // preload spm_A
        ch_al = dma_p2p_opt(&A[OFFSET(m_start, 0, K)], M_A_cur, K_A_cur*sizeof(double), (K-K_A_cur)*sizeof(double),
                            spm_A[0], M_A_cur, K_A_cur*sizeof(double), (K_A-K_A_cur)*sizeof(double), row_syn, p2pmask_cur, ch0_al);
        // preload spm_C
        ch_cl[0] = dma_p2p_opt(&C[OFFSET(m_start, 0, N)], M_A_cur, N_G_cur*sizeof(double), (N-N_G_cur)*sizeof(double),
                            spm_C[0], M_A_cur, N_G_cur*sizeof(double), (N_A-N_G_cur)*sizeof(double), row_syn, p2pmask_cur, ch0_cl);
    }
    int cnt_gsm = 0;
    int cnt_c = 0;
    for(long n_o=0; n_o<N; n_o+=N_G){
        N_G_next = N_G_cur;
        for(long k_o=0; k_o<K; k_o+=K_G){
            int cnt_gsm_1 = (cnt_gsm + 1) % 2;
            int n_o_next, k_o_next;
            if(k_o + K_G < K){
                n_o_next = n_o;
                k_o_next = k_o + K_G;
            }else if(n_o + N_G < N){
                n_o_next = n_o + N_G;
                k_o_next = 0;
            }else{
                n_o_next = -1;
            }

            if(tid==0){
                // preload B_next to gsm_mem
                if(k_o + K_G < K){ // not row end
                    K_G_next = min(K_G, K-(k_o+K_G));
                    ch_gsm[cnt_gsm_1] = dma_p2p_opt(&B[OFFSET(n_o, k_o_next, K)], N_G_next, K_G_next*sizeof(double), (K-K_G_next)*sizeof(double),
                                            gsm_mem + NK_G*cnt_gsm_1, N_G_next, K_G_next*sizeof(double), (K_G-K_G_next)*sizeof(double), 0, 0, ch0_gsm+cnt_gsm_1);
                }else if(n_o + N_G < N){ // k + K_G >= K
                    K_G_next = min(K_G, K);
                    N_G_next = min(N_G, N-(n_o+N_G));
                    ch_gsm[cnt_gsm_1] = dma_p2p_opt(&B[OFFSET(n_o_next, 0, K)], N_G_next, K_G_next*sizeof(double), (K-K_G_next)*sizeof(double),
                                            gsm_mem + NK_G*cnt_gsm_1, N_G_next, K_G_next*sizeof(double), (K_G-K_G_next)*sizeof(double), 0, 0, ch0_gsm+cnt_gsm_1);
                }
                // wait for B_cur loaded to gsm_mem
                dma_wait_p2p(ch_gsm[cnt_gsm]);
            } else{
                if(k_o + K_G < K){
                    K_G_next = min(K_G, K-(k_o+K_G));
                }else if(n_o + N_G < N){
                    K_G_next = min(K_G, K);
                    N_G_next = min(N_G, N-(n_o+N_G));
                }
            }
            group_barrier(b_id);

            // preload gsm_mem to sm_B
            if(m_start < M){
                ch_bl[0] = dma_p2p_opt(gsm_mem + NK_G*cnt_gsm, N_S, K_A_cur*sizeof(double), (K_G-K_A_cur)*sizeof(double),
                                    spm_B[0], N_S, K_A_cur*sizeof(double), (K_A-K_A_cur)*sizeof(double), 0, 0, ch0_bl);
            }
            unint cnt_b = 0;

            long m_tail = m_step;
            for(long m_o=m_start; m_o<M; m_o+=m_step){
                unint p2pmask_next;
                if(m_o+m_step < M && m_tail+m_step >= M){ // current is not the last, next is the last
                    p2pmask_next = p2pmask_last;
                }else{
                    p2pmask_next = p2pmask_most;
                }
                m_tail += m_step;
                // compute C index
                unint cnt_c_1 = (cnt_c + 1) % 2;
                // compute m_o_next
                int m_o_next;
                if(m_o + m_step < M){ // m_o没到头
                    m_o_next = m_o + m_step;
                    M_A_next = min(M_A, M-(m_o+m_step));
                }else if(n_o_next == -1){ // m_o到头了，n_o,k_o也到头了
                    m_o_next = -1;
                }else{ // m_o到头了，n_o,k_o还没到头
                    m_o_next = m_start;
                    M_A_next = min(M_A, M-m_start);
                }

                // [0] wait for C stored
                dma_wait_p2p(ch_cs); 

                // [1] compute spm_C[cnt_c_1] 
                // ------------------ first iteration of k_i ------------------
                long k_i = 0, k_i_next;
                    // wait for spm_A[0]
                    dma_wait_p2p(ch_al);
                    // transpsoe spm_A[0] to spm_A[1]
                    for(int m_a=0; m_a<M_A_cur; m_a+=16){
                        for(int k_a=0; k_a<K_A_cur; k_a+=32){
                            micro_kernel_16x16x2(spm_A[0] + m_a*(K_A>>4) + (k_a>>4), spm_A[1] + k_a*(M_A>>4) + (m_a>>4), spm_D, M_A, K_A);
                        }
                    }
                    // preload next A_tile to spm_A[0]
                    if(k_i + K_A < K_G_cur){ // k_i没到头
                        k_i_next = k_i + K_A;
                        K_A_next = min(K_A, K_G_cur-(k_i+K_A));
                        ch_al = dma_p2p_opt(&A[OFFSET(m_o, k_o+k_i+K_A, K)], M_A_cur, K_A_next*sizeof(double), (K-K_A_next)*sizeof(double),
                                        spm_A[0], M_A_cur, K_A_next*sizeof(double), (K_A-K_A_next)*sizeof(double), row_syn, p2pmask_cur, ch0_al);
                    }else if(m_o + m_step < M){ // k_i到头了，m_o没到头
                        k_i_next = 0;
                        K_A_next = min(K_A, K_G_cur);
                        ch_al = dma_p2p_opt(&A[OFFSET(m_o+m_step, k_o, K)], M_A_next, K_A_next*sizeof(double), (K-K_A_next)*sizeof(double),
                                        spm_A[0], M_A_next, K_A_next*sizeof(double), (K_A-K_A_next)*sizeof(double), row_syn, p2pmask_next, ch0_al);
                    }else if(n_o_next != -1){ // k_i到头了，m_o到头了，n_o,k_o还没到头
                        k_i_next = 0;
                        K_A_next = min(K_A, K_G_next);
                        ch_al = dma_p2p_opt(&A[OFFSET(m_start, k_o_next, K)], M_A_next, K_A_next*sizeof(double), (K-K_A_next)*sizeof(double),
                                        spm_A[0], M_A_next, K_A_next*sizeof(double), (K_A-K_A_next)*sizeof(double), row_syn, p2pmask_next, ch0_al);
                    }
                    // inner loop
                    for(int n_b=0; n_b<N_G_cur; n_b+=2*N_S){
                        // preload B to sm
                        ch_bl[1] = dma_p2p_opt(gsm_mem + NK_G*cnt_gsm + (n_b+N_S)*K_G + k_i, N_S, K_A_cur*sizeof(double), (K_G-K_A_cur)*sizeof(double),
                                            spm_B[1], N_S, K_A_cur*sizeof(double), (K_A-K_A_cur)*sizeof(double), 0, 0, ch0_bl+1);
                        dma_wait_p2p(ch_bl[0]);
                        micro_kernel_asm_r6c64_only_write(spm_B[0], spm_A[1], spm_C[cnt_c_1] + n_b*(M_A>>4), K_A_cur, K_A, M_A);
                        // preload B to sm
                        if(n_b + 2*N_S < N_G_cur){ // n_b没到头
                            ch_bl[0] = dma_p2p_opt(gsm_mem + NK_G*cnt_gsm + (n_b+2*N_S)*K_G + k_i, N_S, K_A_cur*sizeof(double), (K_G-K_A_cur)*sizeof(double),
                                                spm_B[0], N_S, K_A_cur*sizeof(double), (K_A-K_A_cur)*sizeof(double), 0, 0, ch0_bl+0);
                        }else if(k_i + K_A < K_G_cur || m_o + m_step < M){ // n_b到头了，k_i或m_o没到头
                            ch_bl[0] = dma_p2p_opt(gsm_mem + NK_G*cnt_gsm + k_i_next, N_S, K_A_next*sizeof(double), (K_G-K_A_next)*sizeof(double),
                                                spm_B[0], N_S, K_A_next*sizeof(double), (K_A-K_A_next)*sizeof(double), 0, 0, ch0_bl+0);
                        }
                        dma_wait_p2p(ch_bl[1]);
                        micro_kernel_asm_r6c64_only_write(spm_B[1], spm_A[1], spm_C[cnt_c_1] + (n_b+N_S)*(M_A>>4), K_A_cur, K_A, M_A);
                    }
                    K_A_cur = K_A_next;
                // ------------------ subsequent iterations of k_i ------------------
                for(k_i=K_A; k_i<K_G_cur; k_i+=K_A){    
                    // wait for spm_A[0]
                    dma_wait_p2p(ch_al);
                    // transpsoe spm_A[0] to spm_A[1]
                    for(int m_a=0; m_a<M_A_cur; m_a+=16){
                        for(int k_a=0; k_a<K_A_cur; k_a+=32){
                            micro_kernel_16x16x2(spm_A[0] + m_a*(K_A>>4) + (k_a>>4), spm_A[1] + k_a*(M_A>>4) + (m_a>>4), spm_D, M_A, K_A);
                        }
                    }
                    // preload next A_tile to spm_A[0]
                    if(k_i + K_A < K_G_cur){ // k_i没到头
                        k_i_next = k_i + K_A;
                        K_A_next = min(K_A, K_G_cur-(k_i+K_A));
                        ch_al = dma_p2p_opt(&A[OFFSET(m_o, k_o+k_i+K_A, K)], M_A_cur, K_A_next*sizeof(double), (K-K_A_next)*sizeof(double),
                                        spm_A[0], M_A_cur, K_A_next*sizeof(double), (K_A-K_A_next)*sizeof(double), row_syn, p2pmask_cur, ch0_al);
                    }else if(m_o + m_step < M){ // k_i到头了，m_o没到头
                        k_i_next = 0;
                        K_A_next = min(K_A, K_G_cur);
                        ch_al = dma_p2p_opt(&A[OFFSET(m_o+m_step, k_o, K)], M_A_next, K_A_next*sizeof(double), (K-K_A_next)*sizeof(double),
                                        spm_A[0], M_A_next, K_A_next*sizeof(double), (K_A-K_A_next)*sizeof(double), row_syn, p2pmask_next, ch0_al);
                    }else if(n_o_next != -1){ // k_i到头了，m_o到头了，n_o,k_o还没到头
                        k_i_next = 0;
                        K_A_next = min(K_A, K_G_next);
                        ch_al = dma_p2p_opt(&A[OFFSET(m_start, k_o_next, K)], M_A_next, K_A_next*sizeof(double), (K-K_A_next)*sizeof(double),
                                        spm_A[0], M_A_next, K_A_next*sizeof(double), (K_A-K_A_next)*sizeof(double), row_syn, p2pmask_next, ch0_al);
                    }
                    // inner loop
                    for(int n_b=0; n_b<N_G_cur; n_b+=2*N_S){
                        // preload B to sm
                        ch_bl[1] = dma_p2p_opt(gsm_mem + NK_G*cnt_gsm + (n_b+N_S)*K_G + k_i, N_S, K_A_cur*sizeof(double), (K_G-K_A_cur)*sizeof(double),
                                            spm_B[1], N_S, K_A_cur*sizeof(double), (K_A-K_A_cur)*sizeof(double), 0, 0, ch0_bl+1);
                        dma_wait_p2p(ch_bl[0]);
                        micro_kernel_asm_r6c64(spm_B[0], spm_A[1], spm_C[cnt_c_1] + n_b*(M_A>>4), K_A_cur, K_A, M_A);
                        // preload B to sm
                        if(n_b + 2*N_S < N_G_cur){ // n_b没到头
                            ch_bl[0] = dma_p2p_opt(gsm_mem + NK_G*cnt_gsm + (n_b+2*N_S)*K_G + k_i, N_S, K_A_cur*sizeof(double), (K_G-K_A_cur)*sizeof(double),
                                                spm_B[0], N_S, K_A_cur*sizeof(double), (K_A-K_A_cur)*sizeof(double), 0, 0, ch0_bl+0);
                        }else if(k_i + K_A < K_G_cur || m_o + m_step < M){ // n_b到头了，k_i或m_o没到头
                            ch_bl[0] = dma_p2p_opt(gsm_mem + NK_G*cnt_gsm + k_i_next, N_S, K_A_next*sizeof(double), (K_G-K_A_next)*sizeof(double),
                                                spm_B[0], N_S, K_A_next*sizeof(double), (K_A-K_A_next)*sizeof(double), 0, 0, ch0_bl+0);
                        }
                        dma_wait_p2p(ch_bl[1]);
                        micro_kernel_asm_r6c64(spm_B[1], spm_A[1], spm_C[cnt_c_1] + (n_b+N_S)*(M_A>>4), K_A_cur, K_A, M_A);
                    }
                    K_A_cur = K_A_next;
                }
                // ------------------ end loop of k_i ------------------

                // [2] wait for C loaded to spm_C[cnt_c]
                dma_wait_p2p(ch_cl[cnt_c]);

                // [3] transpose spm_C[cnt_c_1] to spm_C[cnt_c]
                for(int n_a=0; n_a<N_G_cur; n_a+=16){
                    for(int m_a=0; m_a<M_A_cur; m_a+=32){
                        micro_kernel_16x16x2_add(&spm_C[cnt_c_1][OFFSET(n_a, m_a>>4, M_A>>4)], &spm_C[cnt_c][OFFSET(m_a, n_a>>4, N_A>>4)], spm_D, N_A, M_A);
                    }
                }

                // [4] store spm_C[cnt_c] to C
                ch_cs = dma_p2p_opt(spm_C[cnt_c], M_A_cur, N_G_cur*sizeof(double), (N_G-N_G_cur)*sizeof(double),
                            &C[OFFSET(m_o, n_o, N)], M_A_cur, N_G_cur*sizeof(double), (N-N_G_cur)*sizeof(double), row_syn, p2pmask_cur, ch0_cs);

                // [5] preload C to spm_C[cnt_c_1]
                if(m_o + m_step < M){ // m_o没到头
                    ch_cl[cnt_c_1] = dma_p2p_opt(&C[OFFSET(m_o_next, n_o, N)], M_A_next, N_G_cur*sizeof(double), (N-N_G_cur)*sizeof(double),
                                        spm_C[cnt_c_1], M_A_next, N_G_cur*sizeof(double), (N_A-N_G_cur)*sizeof(double), row_syn, p2pmask_next, ch0_cl+cnt_c_1);
                }else if(m_o_next != -1){ // m_o到头了，n_o,k_o没到头
                    ch_cl[cnt_c_1] = dma_p2p_opt(&C[OFFSET(m_o_next, n_o_next, N)], M_A_next, N_G_next*sizeof(double), (N-N_G_next)*sizeof(double),
                                        spm_C[cnt_c_1], M_A_next, N_G_next*sizeof(double), (N_A-N_G_next)*sizeof(double), row_syn, p2pmask_next, ch0_cl+cnt_c_1);
                }

                // renew index C
                cnt_c = cnt_c_1;
                p2pmask_cur = p2pmask_next;
                M_A_cur = M_A_next;
            }
            cnt_gsm = cnt_gsm_1;
            K_G_cur = K_G_next;
            group_barrier(b_id);
        }
        N_G_cur = N_G_next;
    }
    // [0] wait for C stored
        dma_wait_p2p(ch_cs); 
    scalar_free(spm_B[0]);
    scalar_free(spm_B[1]);
    vector_free(spm_A[0]);
    vector_free(spm_A[1]);
    vector_free(spm_C[0]);
    vector_free(spm_C[1]);
    vector_free(spm_D);
}


void transpose_gsm_16x16(unlong N, unlong K, unlong bid_main, unlong bid_T, int n_threads, double* B){ // n_threads:多少个核参与转置操作
    const int tid = get_thread_id();
    const int tid_0 = get_group_size() - n_threads;
    // --------- GEMM blocking parameters ---------
    const unlong K_G = 2048;
    const unlong N_G = 192;
    // --------- transpose blocking parameters ---------
    const unlong M_A = 64;
    const unlong N_A = 192;
    const unlong h_a = 16;
    const unlong w_a = 16;
    const unlong w_2_a = w_a << 1;
    // ---------------------------------------
    const indexType NK_G = N_G * K_G;
    const indexType m_start = (tid-tid_0) * M_A;
    const indexType m_step = n_threads * M_A;
    // ---------------------------------------
    lvector double* spm_A[2];
    lvector double* spm_B[2];
    spm_A[0] = vector_malloc(M_A * N_A * sizeof(double));
    spm_A[1] = vector_malloc(M_A * N_A * sizeof(double));
    spm_B[0] = vector_malloc(M_A * N_A * sizeof(double));
    spm_B[1] = vector_malloc(M_A * N_A * sizeof(double));
    lvector double* spm_D = vector_malloc(512 * sizeof(double));

    int ch_gsm, ch_al[2], ch_bs[2];
    const int ch0_gsm = 0;
    const int ch0_al = 2;
    const int ch0_bs = 4;

    for(long n_o=0; n_o<N; n_o+=N_G){
        long N_G_cur = min(N_G, N-n_o);
        for(long k_o=0; k_o<K; k_o+=K_G){
            long K_G_cur = min(K_G, K-k_o);
            // load A from ddr to gsm
            if(tid == tid_0){
                ch_gsm = dma_p2p_opt(&B[OFFSET(k_o, n_o, N)], K_G_cur, N_G_cur*sizeof(double), (N-N_G_cur)*sizeof(double),
                                gsm_mem, K_G_cur, N_G_cur*sizeof(double), (N_G-N_G_cur)*sizeof(double), 0, 0, ch0_gsm);
                dma_wait_p2p(ch_gsm);
            }
            core_barrier(bid_T, n_threads);
            // ----------------------------------- main loop -----------------------------------
            unint cnt_a = 0;
            unint cnt_b = 0;
            unlong M_A_cur = min(M_A, K_G_cur-m_start);
            unlong N_A_cur = min(N_A, N_G_cur);
            if(m_start < K_G_cur){
                ch_al[0] = dma_p2p_opt(gsm_mem + OFFSET(m_start, 0, N_G), M_A_cur, N_A_cur*sizeof(double), (N_G-N_A_cur)*sizeof(double),
                                        spm_A[0], M_A_cur, N_A_cur*sizeof(double), (N_A-N_A_cur)*sizeof(double), 0, 0, ch0_al);
                ch_bs[1] = dma_p2p_opt(gsm_mem, 1, 1*sizeof(double), 0, // 发起一次无效传输
                                        spm_B[1], 1, 1*sizeof(double), 0, 0, 0, ch0_bs+1);
            }

            // gsm_1 using finished
            if(k_o != 0 || n_o != 0)
                group_barrier(bid_main);

            for(unlong m_i=m_start; m_i<K_G_cur; m_i+=m_step){
                unlong M_A_next;
                for(unlong n_i=0; n_i<N_G_cur; n_i+=N_A){
                    // -------------------------- next round indexes --------------------------
                    unlong N_A_next;
                    unint cnt_a_1 = (cnt_a + 1) % 2;
                    unint cnt_b_1 = (cnt_b + 1) % 2;
                    // ------------------------------------------------------------------------
                    // preload next input tile
                    if(n_i + N_A < N_G_cur){ // n_i没到头
                        N_A_next = min(N_A, N_G_cur-(n_i+N_A));
                        ch_al[cnt_a_1] = dma_p2p_opt(gsm_mem + OFFSET(m_i, n_i+N_A, N_G), M_A_cur, N_A_next*sizeof(double), (N_G-N_A_next)*sizeof(double),
                                    spm_A[cnt_a_1], M_A_cur, N_A_next*sizeof(double), (N_A-N_A_next)*sizeof(double), 0, 0, ch0_al+cnt_a_1);
                    }else if(m_i + m_step < K_G_cur){ // n_i到头了，m_i没到头
                        N_A_next = min(N_A, N_G_cur);
                        M_A_next = min(M_A, K_G_cur-(m_i+m_step));
                        ch_al[cnt_a_1] = dma_p2p_opt(gsm_mem + OFFSET(m_i+m_step, 0, N_G), M_A_next, N_A_next*sizeof(double), (N_G-N_A_next)*sizeof(double),
                                    spm_A[cnt_a_1], M_A_next, N_A_next*sizeof(double), (N_A-N_A_next)*sizeof(double), 0, 0, ch0_al+cnt_a_1);
                    }
                    // wait for current input tile
                    dma_wait_p2p(ch_al[cnt_a]);
                    // transpose spm_A[cnt_a] into spm_B[cnt_b]
                    for(unlong m=0; m<M_A_cur; m+=h_a){
                        unlong n = 0;
                        while(n + w_a < N_A_cur){
                            micro_kernel_16x16x2(&spm_A[cnt_a][OFFSET(m, n>>4, N_A>>4)], &spm_B[cnt_b][OFFSET(n, m>>4, M_A>>4)], spm_D, M_A, N_A);
                            n += w_2_a;
                        }
                        if(n < N_A_cur){
                            micro_kernel_16x16(&spm_A[cnt_a][OFFSET(m, n>>4, N_A>>4)], &spm_B[cnt_b][OFFSET(n, m>>4, M_A>>4)], spm_D, M_A, N_A);
                        }
                    }
                    // store spm_B[cnt_b] back to gsm[2]
                    ch_bs[cnt_b] = dma_p2p_opt(spm_B[cnt_b], N_A_cur, M_A_cur*sizeof(double), (M_A-M_A_cur)*sizeof(double),
                                    gsm_mem + NK_G + OFFSET(n_i, m_i, K_G), N_A_cur, M_A_cur*sizeof(double), (K_G-M_A_cur)*sizeof(double), 0, 0, ch0_bs+cnt_b);
                    // wait for previous output tile
                    dma_wait_p2p(ch_bs[cnt_b_1]);
                    // -------------------------- renew indexes --------------------------
                    cnt_a = cnt_a_1;
                    cnt_b = cnt_b_1;
                    N_A_cur = N_A_next;
                }
                M_A_cur = M_A_next;
            }
            if(m_start < K_G_cur){
                dma_wait_p2p(ch_bs[(cnt_b+1)%2]);
            }
            // ----------------------------------- main loop end -----------------------------------
            // gsm_1 loaded
            group_barrier(bid_main); 
        }    
    }
    // final match
    group_barrier(bid_main);
    vector_free(spm_A[0]);
    vector_free(spm_A[1]);
    vector_free(spm_B[0]);
    vector_free(spm_B[1]);
    vector_free(spm_D);
}

// traverse order: N_G -> K_G -> M_A -> K_A -> N_S
// to be ensured: N_G == N_A
__global__ void irr_thinB_dgemm_TN_dbuf_mainbody(unlong M, unlong N, unlong K, unlong b_id, const int grp_size,
                                double* A, double* B, double* C){
    const int tid = get_thread_id();
    // --------------------------- blocking params ---------------------------
    const indexType N_G = 192;
    const indexType K_G = 2048;
    const indexType M_A = 64;
    const indexType K_A = 512;
    const indexType N_A = N_G;
    const indexType N_S = 6;
    // --------------------------- blocking params ---------------------------
    const indexType NK_G = N_G * K_G;
    const indexType m_start = tid * M_A;
    const indexType m_step = grp_size * M_A;
    bool row_syn = false;
    unint p2pmask_most = 0x00;
    unint p2pmask_last = 0x00;
    if(grp_size != 1){
        row_syn = true;
        long M_last = M % m_step;
        if(M_last==0)
            M_last = m_step;
        int ncore_last = (M_last + M_A - 1) / M_A;
        int i=0;
        for(; i<ncore_last; i++){
            p2pmask_most <<= 1;
            p2pmask_most |= 1;
        }
        p2pmask_last = p2pmask_most;
        if(M != M_last){
            for(; i<grp_size; i++){
                p2pmask_most <<= 1;
                p2pmask_most |= 1;
            }
        }
    }
    unint p2pmask_cur = p2pmask_most;
    
    double* spm_B[2];
    lvector double* spm_A[2];
    lvector double* spm_C[2];
    spm_B[0] = scalar_malloc(N_S * K_A *sizeof(double));
    spm_B[1] = scalar_malloc(N_S * K_A *sizeof(double));
    spm_A[0] = vector_malloc(K_A * M_A * sizeof(double));
    spm_A[1] = vector_malloc(K_A * M_A * sizeof(double));
    spm_C[0] = vector_malloc(N_A * M_A * sizeof(double));
    spm_C[1] = vector_malloc(N_A * M_A * sizeof(double));
    lvector double* spm_D = vector_malloc(512 * sizeof(double));

    int ch_bl[2], ch_al[2], ch_cl[2], ch_cs;

    const int ch0_al = 2;
    const int ch0_cl = 4; // high level: 4, 5
    const int ch0_bl = 0; // high level: 0, 1 
    const int ch0_cs = 6; // high level: 6, 7

    const unlong prir = 0b0000000011;
    set_prir(prir);

    long N_G_cur = min(N_G, N);
    long K_G_cur = min(K_G, K);
    long M_A_cur = min(M_A, M-m_start);
    long K_A_cur = min(K_A, K);
    long N_G_next, K_G_next, M_A_next, K_A_next;

    if(m_start < M){
        // 启动一次无效传输
        ch_cs = dma_p2p_opt(C, 1, 1*sizeof(double), 0,
                            spm_C[1], 1, 1*sizeof(double), 0, 0, 0, ch0_cs);
        // preload spm_A
        ch_al[0] = dma_p2p_opt(&A[OFFSET(0, m_start, M)], K_A_cur, M_A_cur*sizeof(double), (M-M_A_cur)*sizeof(double),
                            spm_A[0], K_A_cur, M_A_cur*sizeof(double), (M_A-M_A_cur)*sizeof(double), row_syn, p2pmask_cur, ch0_al);
        // preload spm_C
        ch_cl[0] = dma_p2p_opt(&C[OFFSET(m_start, 0, N)], M_A_cur, N_G_cur*sizeof(double), (N-N_G_cur)*sizeof(double),
                            spm_C[0], M_A_cur, N_G_cur*sizeof(double), (N_A-N_G_cur)*sizeof(double), row_syn, p2pmask_cur, ch0_cl);
    }
    int cnt_a = 0;
    int cnt_c = 0;
    for(long n_o=0; n_o<N; n_o+=N_G){
        N_G_next = N_G_cur;
        for(long k_o=0; k_o<K; k_o+=K_G){
            int n_o_next, k_o_next;
            if(k_o + K_G < K){
                n_o_next = n_o;
                k_o_next = k_o + K_G;
                K_G_next = min(K_G, K-(k_o+K_G));
            }else if(n_o + N_G < N){
                n_o_next = n_o + N_G;
                k_o_next = 0;
                K_G_next = min(K_G, K);
                N_G_next = min(N_G, N-(n_o+N_G));
            }else{
                n_o_next = -1;
            }
            group_barrier(b_id);

            // preload gsm_mem to sm_B
            if(m_start < M){
                ch_bl[0] = dma_p2p_opt(gsm_mem + NK_G, N_S, K_A_cur*sizeof(double), (K_G-K_A_cur)*sizeof(double),
                                    spm_B[0], N_S, K_A_cur*sizeof(double), (K_A-K_A_cur)*sizeof(double), 0, 0, ch0_bl);
            }
            unint cnt_b = 0;

            long m_tail = m_step;
            for(long m_o=m_start; m_o<M; m_o+=m_step){
                unint p2pmask_next;
                if(m_o+m_step < M && m_tail+m_step >= M){ // current is not the last, next is the last
                    p2pmask_next = p2pmask_last;
                }else{
                    p2pmask_next = p2pmask_most;
                }
                m_tail += m_step;
                // compute C index
                unint cnt_c_1 = (cnt_c + 1) % 2;
                // compute m_o_next
                int m_o_next;
                if(m_o + m_step < M){ // m_o没到头
                    m_o_next = m_o + m_step;
                    M_A_next = min(M_A, M-(m_o+m_step));
                }else if(n_o_next == -1){ // m_o到头了，n_o,k_o也到头了
                    m_o_next = -1;
                }else{ // m_o到头了，n_o,k_o还没到头
                    m_o_next = m_start;
                    M_A_next = min(M_A, M-m_start);
                }

                // [0] wait for C stored
                dma_wait_p2p(ch_cs); 

                // [1] compute spm_C[cnt_c_1] 
                // ------------------ first iteration of k_i ------------------
                long k_i = 0, k_i_next;
                    // compute A index      
                    int cnt_a_1 = (cnt_a + 1) % 2;
                    // load spm_A[cnt_a_1]
                    if(k_i + K_A < K_G_cur){ // k_i没到头
                        k_i_next = k_i + K_A;
                        K_A_next = min(K_A, K_G_cur-(k_i+K_A));
                        ch_al[cnt_a_1] = dma_p2p_opt(&A[OFFSET(k_o+k_i+K_A, m_o, M)], K_A_next, M_A_cur*sizeof(double), (M-M_A_cur)*sizeof(double),
                                                spm_A[cnt_a_1], K_A_next, M_A_cur*sizeof(double), (M_A-M_A_cur)*sizeof(double), row_syn, p2pmask_cur, ch0_al+cnt_a_1);
                    }else if(m_o + m_step < M){ // k_i到头了，m_o没到头
                        k_i_next = 0;
                        K_A_next = min(K_A, K_G_cur);
                        ch_al[cnt_a_1] = dma_p2p_opt(&A[OFFSET(k_o, m_o+m_step, M)], K_A_next, M_A_next*sizeof(double), (M-M_A_next)*sizeof(double),
                                                spm_A[cnt_a_1], K_A_next, M_A_next*sizeof(double), (M_A-M_A_next)*sizeof(double), row_syn, p2pmask_next, ch0_al+cnt_a_1);
                    }else if(n_o_next != -1){ // k_i到头了，m_o到头了，n_o,k_o还没到头
                        k_i_next = 0;
                        K_A_next = min(K_A, K_G_next);
                        ch_al[cnt_a_1] = dma_p2p_opt(&A[OFFSET(k_o_next, m_start, M)], K_A_next, M_A_next*sizeof(double), (M-M_A_next)*sizeof(double),
                                                spm_A[cnt_a_1], K_A_next, M_A_next*sizeof(double), (M_A-M_A_next)*sizeof(double), row_syn, p2pmask_next, ch0_al+cnt_a_1);
                    }
                    // wait for spm_A[cnt_a]
                    dma_wait_p2p(ch_al[cnt_a]);
                    // inner loop
                    for(int n_b=0; n_b<N_G_cur; n_b+=2*N_S){
                        // preload B to sm
                        ch_bl[1] = dma_p2p_opt(gsm_mem + NK_G + (n_b+N_S)*K_G + k_i, N_S, K_A_cur*sizeof(double), (K_G-K_A_cur)*sizeof(double),
                                            spm_B[1], N_S, K_A_cur*sizeof(double), (K_A-K_A_cur)*sizeof(double), 0, 0, ch0_bl+1);
                        dma_wait_p2p(ch_bl[0]);
                        micro_kernel_asm_r6c64_only_write(spm_B[0], spm_A[cnt_a], spm_C[cnt_c_1] + n_b*(M_A>>4), K_A_cur, K_A, M_A);
                        // preload B to sm
                        if(n_b + 2*N_S < N_G_cur){ // n_b没到头
                            ch_bl[0] = dma_p2p_opt(gsm_mem + NK_G + (n_b+2*N_S)*K_G + k_i, N_S, K_A_cur*sizeof(double), (K_G-K_A_cur)*sizeof(double),
                                                spm_B[0], N_S, K_A_cur*sizeof(double), (K_A-K_A_cur)*sizeof(double), 0, 0, ch0_bl+0);
                        }else if(k_i + K_A < K_G_cur || m_o + m_step < M){ // n_b到头了，k_i或m_o没到头
                            ch_bl[0] = dma_p2p_opt(gsm_mem + NK_G + k_i_next, N_S, K_A_next*sizeof(double), (K_G-K_A_next)*sizeof(double),
                                                spm_B[0], N_S, K_A_next*sizeof(double), (K_A-K_A_next)*sizeof(double), 0, 0, ch0_bl+0);
                        }
                        dma_wait_p2p(ch_bl[1]);
                        micro_kernel_asm_r6c64_only_write(spm_B[1], spm_A[cnt_a], spm_C[cnt_c_1] + (n_b+N_S)*(M_A>>4), K_A_cur, K_A, M_A);
                    }
                    // renew index B
                    cnt_a = cnt_a_1;
                    K_A_cur = K_A_next;
                // ------------------ subsequent iterations of k_i ------------------
                for(k_i=K_A; k_i<K_G_cur; k_i+=K_A){    
                    // compute A index      
                    cnt_a_1 = (cnt_a + 1) % 2;
                    // load spm_A[cnt_a_1]
                    if(k_i + K_A < K_G_cur){ // k_i没到头
                        k_i_next = k_i + K_A;
                        K_A_next = min(K_A, K_G_cur-(k_i+K_A));
                        ch_al[cnt_a_1] = dma_p2p_opt(&A[OFFSET(k_o+k_i+K_A, m_o, M)], K_A_next, M_A_cur*sizeof(double), (M-M_A_cur)*sizeof(double),
                                                spm_A[cnt_a_1], K_A_next, M_A_cur*sizeof(double), (M_A-M_A_cur)*sizeof(double), row_syn, p2pmask_cur, ch0_al+cnt_a_1);
                    }else if(m_o + m_step < M){ // k_i到头了，m_o没到头
                        k_i_next = 0;
                        K_A_next = min(K_A, K_G_cur);
                        ch_al[cnt_a_1] = dma_p2p_opt(&A[OFFSET(k_o, m_o+m_step, M)], K_A_next, M_A_next*sizeof(double), (M-M_A_next)*sizeof(double),
                                                spm_A[cnt_a_1], K_A_next, M_A_next*sizeof(double), (M_A-M_A_next)*sizeof(double), row_syn, p2pmask_next, ch0_al+cnt_a_1);
                    }else if(n_o_next != -1){ // k_i到头了，m_o到头了，n_o,k_o还没到头
                        k_i_next = 0;
                        K_A_next = min(K_A, K_G_next);
                        ch_al[cnt_a_1] = dma_p2p_opt(&A[OFFSET(k_o_next, m_start, M)], K_A_next, M_A_next*sizeof(double), (M-M_A_next)*sizeof(double),
                                                spm_A[cnt_a_1], K_A_next, M_A_next*sizeof(double), (M_A-M_A_next)*sizeof(double), row_syn, p2pmask_next, ch0_al+cnt_a_1);
                    }
                    // wait for spm_A[cnt_a]
                    dma_wait_p2p(ch_al[cnt_a]);
                    // inner loop
                    for(int n_b=0; n_b<N_G_cur; n_b+=2*N_S){
                        // preload B to sm
                        ch_bl[1] = dma_p2p_opt(gsm_mem + NK_G + (n_b+N_S)*K_G + k_i, N_S, K_A_cur*sizeof(double), (K_G-K_A_cur)*sizeof(double),
                                            spm_B[1], N_S, K_A_cur*sizeof(double), (K_A-K_A_cur)*sizeof(double), 0, 0, ch0_bl+1);
                        dma_wait_p2p(ch_bl[0]);
                        micro_kernel_asm_r6c64(spm_B[0], spm_A[cnt_a], spm_C[cnt_c_1] + n_b*(M_A>>4), K_A_cur, K_A, M_A);
                        // preload B to sm
                        if(n_b + 2*N_S < N_G_cur){ // n_b没到头
                            ch_bl[0] = dma_p2p_opt(gsm_mem + NK_G + (n_b+2*N_S)*K_G + k_i, N_S, K_A_cur*sizeof(double), (K_G-K_A_cur)*sizeof(double),
                                                spm_B[0], N_S, K_A_cur*sizeof(double), (K_A-K_A_cur)*sizeof(double), 0, 0, ch0_bl+0);
                        }else if(k_i + K_A < K_G_cur || m_o + m_step < M){ // n_b到头了，k_i或m_o没到头
                            ch_bl[0] = dma_p2p_opt(gsm_mem + NK_G + k_i_next, N_S, K_A_next*sizeof(double), (K_G-K_A_next)*sizeof(double),
                                                spm_B[0], N_S, K_A_next*sizeof(double), (K_A-K_A_next)*sizeof(double), 0, 0, ch0_bl+0);
                        }
                        dma_wait_p2p(ch_bl[1]);
                        micro_kernel_asm_r6c64(spm_B[1], spm_A[cnt_a], spm_C[cnt_c_1] + (n_b+N_S)*(M_A>>4), K_A_cur, K_A, M_A);
                    }
                    // renew index B
                    cnt_a = cnt_a_1;
                    K_A_cur = K_A_next;
                }
                // ------------------ end loop of k_i ------------------

                // [2] wait for C loaded to spm_C[cnt_c]
                dma_wait_p2p(ch_cl[cnt_c]);

                // [3] transpose spm_C[cnt_c_1] to spm_C[cnt_c]
                for(int n_a=0; n_a<N_G_cur; n_a+=16){
                    for(int m_a=0; m_a<M_A_cur; m_a+=32){
                        micro_kernel_16x16x2_add(&spm_C[cnt_c_1][OFFSET(n_a, m_a>>4, M_A>>4)], &spm_C[cnt_c][OFFSET(m_a, n_a>>4, N_A>>4)], spm_D, N_A, M_A);
                    }
                }

                // [4] store spm_C[cnt_c] to C
                ch_cs = dma_p2p_opt(spm_C[cnt_c], M_A_cur, N_G_cur*sizeof(double), (N_G-N_G_cur)*sizeof(double),
                            &C[OFFSET(m_o, n_o, N)], M_A_cur, N_G_cur*sizeof(double), (N-N_G_cur)*sizeof(double), row_syn, p2pmask_cur, ch0_cs);

                // [5] preload C to spm_C[cnt_c_1]
                if(m_o + m_step < M){ // m_o没到头
                    ch_cl[cnt_c_1] = dma_p2p_opt(&C[OFFSET(m_o_next, n_o, N)], M_A_next, N_G_cur*sizeof(double), (N-N_G_cur)*sizeof(double),
                                        spm_C[cnt_c_1], M_A_next, N_G_cur*sizeof(double), (N_A-N_G_cur)*sizeof(double), row_syn, p2pmask_next, ch0_cl+cnt_c_1);
                }else if(m_o_next != -1){ // m_o到头了，n_o,k_o没到头
                    ch_cl[cnt_c_1] = dma_p2p_opt(&C[OFFSET(m_o_next, n_o_next, N)], M_A_next, N_G_next*sizeof(double), (N-N_G_next)*sizeof(double),
                                        spm_C[cnt_c_1], M_A_next, N_G_next*sizeof(double), (N_A-N_G_next)*sizeof(double), row_syn, p2pmask_next, ch0_cl+cnt_c_1);
                }

                // renew index C
                cnt_c = cnt_c_1;
                p2pmask_cur = p2pmask_next;
                M_A_cur = M_A_next;
            }
            K_G_cur = K_G_next;
            // gsm_1 using finished
            group_barrier(b_id);
        }
        N_G_cur = N_G_next;
    }
    // [0] wait for C stored
        dma_wait_p2p(ch_cs); 
    scalar_free(spm_B[0]);
    scalar_free(spm_B[1]);
    vector_free(spm_A[0]);
    vector_free(spm_A[1]);
    vector_free(spm_C[0]);
    vector_free(spm_C[1]);
    vector_free(spm_D);
}

__global__ void irr_thinB_dgemm_TN_dbuf(unlong M, unlong N, unlong K, unlong bid_main, unlong bid_T, double* A, double* B, double* C){
    const int n_cores_for_transpose = 4;
    const int tid = get_thread_id();
    const int grp_size = get_group_size();
    const int n_cores_for_compute = grp_size - n_cores_for_transpose;
    if(tid >= n_cores_for_compute){
        transpose_gsm_16x16(N, K, bid_main, bid_T, n_cores_for_transpose, B);
    }else{
        irr_thinB_dgemm_TN_dbuf_mainbody(M, N, K, bid_main, n_cores_for_compute, A, B, C);
    }
}

// traverse order: N_G -> K_G -> M_A -> K_A -> N_S
// to be ensured: N_G == N_A
__global__ void irr_thinB_dgemm_NN_dbuf_mainbody(unlong M, unlong N, unlong K, unlong b_id, const int grp_size,
                                double* A, double* B, double* C){
    const int tid = get_thread_id();
    // --------------------------- blocking params ---------------------------
    const indexType N_G = 192;
    const indexType K_G = 2048;
    const indexType M_A = 64;
    const indexType K_A = 512;
    const indexType N_A = N_G;
    const indexType N_S = 6;
    // --------------------------- blocking params ---------------------------
    const indexType NK_G = N_G * K_G;
    const indexType m_start = tid * M_A;
    const indexType m_step = grp_size * M_A;
    bool row_syn = false;
    unint p2pmask_most = 0x00;
    unint p2pmask_last = 0x00;
    if(grp_size != 1){
        row_syn = true;
        long M_last = M % m_step;
        if(M_last==0)
            M_last = m_step;
        int ncore_last = (M_last + M_A - 1) / M_A;
        int i=0;
        for(; i<ncore_last; i++){
            p2pmask_most <<= 1;
            p2pmask_most |= 1;
        }
        p2pmask_last = p2pmask_most;
        if(M != M_last){
            for(; i<grp_size; i++){
                p2pmask_most <<= 1;
                p2pmask_most |= 1;
            }
        }
    }
    unint p2pmask_cur = p2pmask_most;
    
    double* spm_B[2];
    lvector double* spm_A[2];
    lvector double* spm_C[2];
    spm_B[0] = scalar_malloc(N_S * K_A *sizeof(double));
    spm_B[1] = scalar_malloc(N_S * K_A *sizeof(double));
    spm_A[0] = vector_malloc(K_A * M_A * sizeof(double));
    spm_A[1] = vector_malloc(K_A * M_A * sizeof(double));
    spm_C[0] = vector_malloc(N_A * M_A * sizeof(double));
    spm_C[1] = vector_malloc(N_A * M_A * sizeof(double));
    lvector double* spm_D = vector_malloc(512 * sizeof(double));

    int ch_bl[2], ch_al, ch_cl[2], ch_cs;

    const int ch0_al = 2;
    const int ch0_cl = 4; // high level: 4, 5
    const int ch0_bl = 0; // high level: 0, 1 
    const int ch0_cs = 6; // high level: 6, 7

    const unlong prir = 0b0000000011;
    set_prir(prir);

    long N_G_cur = min(N_G, N);
    long K_G_cur = min(K_G, K);
    long M_A_cur = min(M_A, M-m_start);
    long K_A_cur = min(K_A, K);
    long N_G_next, K_G_next, M_A_next, K_A_next;

    if(m_start < M){
        // 启动一次无效传输
        ch_cs = dma_p2p_opt(C, 1, 1*sizeof(double), 0,
                            spm_C[1], 1, 1*sizeof(double), 0, 0, 0, ch0_cs);
        // preload spm_A
        ch_al = dma_p2p_opt(&A[OFFSET(m_start, 0, K)], M_A_cur, K_A_cur*sizeof(double), (K-K_A_cur)*sizeof(double),
                            spm_A[0], M_A_cur, K_A_cur*sizeof(double), (K_A-K_A_cur)*sizeof(double), row_syn, p2pmask_cur, ch0_al);
        // preload spm_C
        ch_cl[0] = dma_p2p_opt(&C[OFFSET(m_start, 0, N)], M_A_cur, N_G_cur*sizeof(double), (N-N_G_cur)*sizeof(double),
                            spm_C[0], M_A_cur, N_G_cur*sizeof(double), (N_A-N_G_cur)*sizeof(double), row_syn, p2pmask_cur, ch0_cl);
    }
    int cnt_c = 0;
    for(long n_o=0; n_o<N; n_o+=N_G){
        N_G_next = N_G_cur;
        for(long k_o=0; k_o<K; k_o+=K_G){
            int n_o_next, k_o_next;
            if(k_o + K_G < K){
                n_o_next = n_o;
                k_o_next = k_o + K_G;
                K_G_next = min(K_G, K-(k_o+K_G));
            }else if(n_o + N_G < N){
                n_o_next = n_o + N_G;
                k_o_next = 0;
                K_G_next = min(K_G, K);
                N_G_next = min(N_G, N-(n_o+N_G));
            }else{
                n_o_next = -1;
            }
            group_barrier(b_id);

            // preload gsm_mem to sm_B
            if(m_start < M){
                ch_bl[0] = dma_p2p_opt(gsm_mem + NK_G, N_S, K_A_cur*sizeof(double), (K_G-K_A_cur)*sizeof(double),
                                    spm_B[0], N_S, K_A_cur*sizeof(double), (K_A-K_A_cur)*sizeof(double), 0, 0, ch0_bl);
            }
            unint cnt_b = 0;

            long m_tail = m_step;
            for(long m_o=m_start; m_o<M; m_o+=m_step){
                unint p2pmask_next;
                if(m_o+m_step < M && m_tail+m_step >= M){ // current is not the last, next is the last
                    p2pmask_next = p2pmask_last;
                }else{
                    p2pmask_next = p2pmask_most;
                }
                m_tail += m_step;
                // compute C index
                unint cnt_c_1 = (cnt_c + 1) % 2;
                // compute m_o_next
                int m_o_next;
                if(m_o + m_step < M){ // m_o没到头
                    m_o_next = m_o + m_step;
                    M_A_next = min(M_A, M-(m_o+m_step));
                }else if(n_o_next == -1){ // m_o到头了，n_o,k_o也到头了
                    m_o_next = -1;
                }else{ // m_o到头了，n_o,k_o还没到头
                    m_o_next = m_start;
                    M_A_next = min(M_A, M-m_start);
                }

                // [0] wait for C stored
                dma_wait_p2p(ch_cs); 

                // [1] compute spm_C[cnt_c_1] 
                // ------------------ first iteration of k_i ------------------
                long k_i = 0, k_i_next;
                    // wait for spm_A[0]
                    dma_wait_p2p(ch_al);
                    // transpsoe spm_A[0] to spm_A[1]
                    for(int m_a=0; m_a<M_A_cur; m_a+=16){
                        for(int k_a=0; k_a<K_A_cur; k_a+=32){
                            micro_kernel_16x16x2(spm_A[0] + m_a*(K_A>>4) + (k_a>>4), spm_A[1] + k_a*(M_A>>4) + (m_a>>4), spm_D, M_A, K_A);
                        }
                    }
                    // preload next A_tile to spm_A[0]
                    if(k_i + K_A < K_G_cur){ // k_i没到头
                        k_i_next = k_i + K_A;
                        K_A_next = min(K_A, K_G_cur-(k_i+K_A));
                        ch_al = dma_p2p_opt(&A[OFFSET(m_o, k_o+k_i+K_A, K)], M_A_cur, K_A_next*sizeof(double), (K-K_A_next)*sizeof(double),
                                        spm_A[0], M_A_cur, K_A_next*sizeof(double), (K_A-K_A_next)*sizeof(double), row_syn, p2pmask_cur, ch0_al);
                    }else if(m_o + m_step < M){ // k_i到头了，m_o没到头
                        k_i_next = 0;
                        K_A_next = min(K_A, K_G_cur);
                        ch_al = dma_p2p_opt(&A[OFFSET(m_o+m_step, k_o, K)], M_A_next, K_A_next*sizeof(double), (K-K_A_next)*sizeof(double),
                                        spm_A[0], M_A_next, K_A_next*sizeof(double), (K_A-K_A_next)*sizeof(double), row_syn, p2pmask_next, ch0_al);
                    }else if(n_o_next != -1){ // k_i到头了，m_o到头了，n_o,k_o还没到头
                        k_i_next = 0;
                        K_A_next = min(K_A, K_G_next);
                        ch_al = dma_p2p_opt(&A[OFFSET(m_start, k_o_next, K)], M_A_next, K_A_next*sizeof(double), (K-K_A_next)*sizeof(double),
                                        spm_A[0], M_A_next, K_A_next*sizeof(double), (K_A-K_A_next)*sizeof(double), row_syn, p2pmask_next, ch0_al);
                    }
                    // inner loop
                    for(int n_b=0; n_b<N_G_cur; n_b+=2*N_S){
                        // preload B to sm
                        ch_bl[1] = dma_p2p_opt(gsm_mem + NK_G + (n_b+N_S)*K_G + k_i, N_S, K_A_cur*sizeof(double), (K_G-K_A_cur)*sizeof(double),
                                            spm_B[1], N_S, K_A_cur*sizeof(double), (K_A-K_A_cur)*sizeof(double), 0, 0, ch0_bl+1);
                        dma_wait_p2p(ch_bl[0]);
                        micro_kernel_asm_r6c64_only_write(spm_B[0], spm_A[1], spm_C[cnt_c_1] + n_b*(M_A>>4), K_A_cur, K_A, M_A);
                        // preload B to sm
                        if(n_b + 2*N_S < N_G_cur){ // n_b没到头
                            ch_bl[0] = dma_p2p_opt(gsm_mem + NK_G + (n_b+2*N_S)*K_G + k_i, N_S, K_A_cur*sizeof(double), (K_G-K_A_cur)*sizeof(double),
                                                spm_B[0], N_S, K_A_cur*sizeof(double), (K_A-K_A_cur)*sizeof(double), 0, 0, ch0_bl+0);
                        }else if(k_i + K_A < K_G_cur || m_o + m_step < M){ // n_b到头了，k_i或m_o没到头
                            ch_bl[0] = dma_p2p_opt(gsm_mem + NK_G + k_i_next, N_S, K_A_next*sizeof(double), (K_G-K_A_next)*sizeof(double),
                                                spm_B[0], N_S, K_A_next*sizeof(double), (K_A-K_A_next)*sizeof(double), 0, 0, ch0_bl+0);
                        }
                        dma_wait_p2p(ch_bl[1]);
                        micro_kernel_asm_r6c64_only_write(spm_B[1], spm_A[1], spm_C[cnt_c_1] + (n_b+N_S)*(M_A>>4), K_A_cur, K_A, M_A);
                    }
                    K_A_cur = K_A_next;
                // ------------------ subsequent iterations of k_i ------------------
                for(k_i=K_A; k_i<K_G_cur; k_i+=K_A){    
                    // wait for spm_A[0]
                    dma_wait_p2p(ch_al);
                    // transpsoe spm_A[0] to spm_A[1]
                    for(int m_a=0; m_a<M_A_cur; m_a+=16){
                        for(int k_a=0; k_a<K_A_cur; k_a+=32){
                            micro_kernel_16x16x2(spm_A[0] + m_a*(K_A>>4) + (k_a>>4), spm_A[1] + k_a*(M_A>>4) + (m_a>>4), spm_D, M_A, K_A);
                        }
                    }
                    // preload next A_tile to spm_A[0]
                    if(k_i + K_A < K_G_cur){ // k_i没到头
                        k_i_next = k_i + K_A;
                        K_A_next = min(K_A, K_G_cur-(k_i+K_A));
                        ch_al = dma_p2p_opt(&A[OFFSET(m_o, k_o+k_i+K_A, K)], M_A_cur, K_A_next*sizeof(double), (K-K_A_next)*sizeof(double),
                                        spm_A[0], M_A_cur, K_A_next*sizeof(double), (K_A-K_A_next)*sizeof(double), row_syn, p2pmask_cur, ch0_al);
                    }else if(m_o + m_step < M){ // k_i到头了，m_o没到头
                        k_i_next = 0;
                        K_A_next = min(K_A, K_G_cur);
                        ch_al = dma_p2p_opt(&A[OFFSET(m_o+m_step, k_o, K)], M_A_next, K_A_next*sizeof(double), (K-K_A_next)*sizeof(double),
                                        spm_A[0], M_A_next, K_A_next*sizeof(double), (K_A-K_A_next)*sizeof(double), row_syn, p2pmask_next, ch0_al);
                    }else if(n_o_next != -1){ // k_i到头了，m_o到头了，n_o,k_o还没到头
                        k_i_next = 0;
                        K_A_next = min(K_A, K_G_next);
                        ch_al = dma_p2p_opt(&A[OFFSET(m_start, k_o_next, K)], M_A_next, K_A_next*sizeof(double), (K-K_A_next)*sizeof(double),
                                        spm_A[0], M_A_next, K_A_next*sizeof(double), (K_A-K_A_next)*sizeof(double), row_syn, p2pmask_next, ch0_al);
                    }
                    // inner loop
                    for(int n_b=0; n_b<N_G_cur; n_b+=2*N_S){
                        // preload B to sm
                        ch_bl[1] = dma_p2p_opt(gsm_mem + NK_G + (n_b+N_S)*K_G + k_i, N_S, K_A_cur*sizeof(double), (K_G-K_A_cur)*sizeof(double),
                                            spm_B[1], N_S, K_A_cur*sizeof(double), (K_A-K_A_cur)*sizeof(double), 0, 0, ch0_bl+1);
                        dma_wait_p2p(ch_bl[0]);
                        micro_kernel_asm_r6c64(spm_B[0], spm_A[1], spm_C[cnt_c_1] + n_b*(M_A>>4), K_A_cur, K_A, M_A);
                        // preload B to sm
                        if(n_b + 2*N_S < N_G_cur){ // n_b没到头
                            ch_bl[0] = dma_p2p_opt(gsm_mem + NK_G + (n_b+2*N_S)*K_G + k_i, N_S, K_A_cur*sizeof(double), (K_G-K_A_cur)*sizeof(double),
                                                spm_B[0], N_S, K_A_cur*sizeof(double), (K_A-K_A_cur)*sizeof(double), 0, 0, ch0_bl+0);
                        }else if(k_i + K_A < K_G_cur || m_o + m_step < M){ // n_b到头了，k_i或m_o没到头
                            ch_bl[0] = dma_p2p_opt(gsm_mem + NK_G + k_i_next, N_S, K_A_next*sizeof(double), (K_G-K_A_next)*sizeof(double),
                                                spm_B[0], N_S, K_A_next*sizeof(double), (K_A-K_A_next)*sizeof(double), 0, 0, ch0_bl+0);
                        }
                        dma_wait_p2p(ch_bl[1]);
                        micro_kernel_asm_r6c64(spm_B[1], spm_A[1], spm_C[cnt_c_1] + (n_b+N_S)*(M_A>>4), K_A_cur, K_A, M_A);
                    }
                    K_A_cur = K_A_next;
                }
                // ------------------ end loop of k_i ------------------

                // [2] wait for C loaded to spm_C[cnt_c]
                dma_wait_p2p(ch_cl[cnt_c]);

                // [3] transpose spm_C[cnt_c_1] to spm_C[cnt_c]
                for(int n_a=0; n_a<N_G_cur; n_a+=16){
                    for(int m_a=0; m_a<M_A_cur; m_a+=32){
                        micro_kernel_16x16x2_add(&spm_C[cnt_c_1][OFFSET(n_a, m_a>>4, M_A>>4)], &spm_C[cnt_c][OFFSET(m_a, n_a>>4, N_A>>4)], spm_D, N_A, M_A);
                    }
                }

                // [4] store spm_C[cnt_c] to C
                ch_cs = dma_p2p_opt(spm_C[cnt_c], M_A_cur, N_G_cur*sizeof(double), (N_G-N_G_cur)*sizeof(double),
                            &C[OFFSET(m_o, n_o, N)], M_A_cur, N_G_cur*sizeof(double), (N-N_G_cur)*sizeof(double), row_syn, p2pmask_cur, ch0_cs);

                // [5] preload C to spm_C[cnt_c_1]
                if(m_o + m_step < M){ // m_o没到头
                    ch_cl[cnt_c_1] = dma_p2p_opt(&C[OFFSET(m_o_next, n_o, N)], M_A_next, N_G_cur*sizeof(double), (N-N_G_cur)*sizeof(double),
                                        spm_C[cnt_c_1], M_A_next, N_G_cur*sizeof(double), (N_A-N_G_cur)*sizeof(double), row_syn, p2pmask_next, ch0_cl+cnt_c_1);
                }else if(m_o_next != -1){ // m_o到头了，n_o,k_o没到头
                    ch_cl[cnt_c_1] = dma_p2p_opt(&C[OFFSET(m_o_next, n_o_next, N)], M_A_next, N_G_next*sizeof(double), (N-N_G_next)*sizeof(double),
                                        spm_C[cnt_c_1], M_A_next, N_G_next*sizeof(double), (N_A-N_G_next)*sizeof(double), row_syn, p2pmask_next, ch0_cl+cnt_c_1);
                }

                // renew index C
                cnt_c = cnt_c_1;
                p2pmask_cur = p2pmask_next;
                M_A_cur = M_A_next;
            }
            K_G_cur = K_G_next;
            group_barrier(b_id);
        }
        N_G_cur = N_G_next;
    }
    // [0] wait for C stored
        dma_wait_p2p(ch_cs); 
    scalar_free(spm_B[0]);
    scalar_free(spm_B[1]);
    vector_free(spm_A[0]);
    vector_free(spm_A[1]);
    vector_free(spm_C[0]);
    vector_free(spm_C[1]);
    vector_free(spm_D);
}

__global__ void irr_thinB_dgemm_NN_dbuf(unlong M, unlong N, unlong K, unlong bid_main, unlong bid_T, double* A, double* B, double* C){
    const int n_cores_for_transpose = 4;
    const int tid = get_thread_id();
    const int grp_size = get_group_size();
    const int n_cores_for_compute = grp_size - n_cores_for_transpose;
    if(tid >= n_cores_for_compute){
        transpose_gsm_16x16(N, K, bid_main, bid_T, n_cores_for_transpose, B);
    }else{
        irr_thinB_dgemm_NN_dbuf_mainbody(M, N, K, bid_main, n_cores_for_compute, A, B, C);
    }
}

__global__ void irr_thinB_dgemm_null(unlong M, unlong N, unlong K, 
                        unlong b_id, float* A, float* B, float* C){
}

