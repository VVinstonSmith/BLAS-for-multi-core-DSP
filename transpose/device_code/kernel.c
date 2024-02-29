#include <compiler/m3000.h>
#include <compiler/vsip.h>
#include "hthread_device.h"
#define unlong unsigned long
#define unint unsigned int
#define indexType long

// #define CNACEL_DMA
// #define GET_CLK

#define OFFSET(row, col, ld) ((row) * (ld) + (col))

// __gsm__ double gsm_mem[1024*128*6];

unlong min(unlong x, unlong y){
    if(x<y) return x;
    return y;
}

unlong max(unlong x, unlong y){
    if(x>y) return x;
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



__global__ void  transpose_scalar(unlong M0, unlong N0, unlong b_id, double* A, double* B){
    const int tid = get_thread_id();
    const int grp_size = get_group_size();
    const unlong h_a = 8;
    const unlong w_a = 8;
    const unlong M = (M0 + h_a - 1) / h_a * h_a;
    const unlong N = (N0 + w_a - 1) / w_a * w_a;

    // --------- blocking parameters ---------
    const unlong M_A = 16;
    const unlong N_A = 16;
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

    double* spm_A[2];
    double* spm_B;
    spm_A[0] = scalar_malloc(M_A * N_A * sizeof(double));
    spm_A[1] = scalar_malloc(M_A * N_A * sizeof(double));
    spm_B = scalar_malloc(M_A * N_A * sizeof(double));

    int ch_al[2], ch_bs;

    const int ch0_al = 0;
    const int ch0_bs = 2;

    int cnt_a = 0;
    // int cnt_b = 0;
    unlong M_A_cur = min(M_A, M0);
    unlong N_A_cur = min(N_A, N0-c_start);
    ch_al[0] = dma_p2p_opt(&A[OFFSET(0, c_start, N0)], M_A_cur, N_A_cur*sizeof(double), (N0-N_A_cur)*sizeof(double),
                            spm_A[0], M_A_cur, N_A_cur*sizeof(double), (N_A-N_A_cur)*sizeof(double), row_syn, p2pmask, ch0_al);

    for(unlong r=0; r<M; r+=M_A){
        unlong M_A_next;

        for(unlong c=c_start; c<N; c+=c_step){
            // -------------------------- next round indexes --------------------------
            unlong N_A_next;
            
            int cnt_a_1 = (cnt_a + 1) % 2;
            // int cnt_b_1 = (cnt_b + 1) % 2;
            // ------------------------------------------------------------------------

            if(c + c_step < N){
                N_A_next = min(N_A, N0-(c+c_step));
                ch_al[cnt_a_1] = dma_p2p_opt(&A[OFFSET(r, c+c_step, N0)], M_A_cur, N_A_next*sizeof(double), (N0-N_A_next)*sizeof(double),
                            spm_A[cnt_a_1], M_A_cur, N_A_next*sizeof(double), (N_A-N_A_next)*sizeof(double), row_syn, p2pmask, ch0_al+cnt_a_1);
            }else if(r + M_A < M){
                N_A_next = min(N_A, N0-c_start);
                M_A_next = min(M_A, M0-(r+M_A));
                ch_al[cnt_a_1] = dma_p2p_opt(&A[OFFSET(r+M_A, c_start, N0)], M_A_next, N_A_next*sizeof(double), (N0-N_A_next)*sizeof(double),
                            spm_A[cnt_a_1], M_A_next, N_A_next*sizeof(double), (N_A-N_A_next)*sizeof(double), row_syn, p2pmask, ch0_al+cnt_a_1);
            }
            
            dma_wait_p2p(ch_al[cnt_a]);

            for(unlong m=0; m<M_A_cur; m++){
                for(unlong n=0; n<N_A_cur; n++){
                    spm_B[OFFSET(n, m, M_A)] = spm_A[cnt_a][OFFSET(m, n, N_A)];
                }
            }

            ch_bs = dma_p2p_opt(spm_B, N_A_cur, M_A_cur*sizeof(double), (M_A-M_A_cur)*sizeof(double),
                            &B[OFFSET(c, r, M0)], N_A_cur, M_A_cur*sizeof(double), (M0-M_A_cur)*sizeof(double), row_syn, p2pmask, ch0_bs);

            dma_wait_p2p(ch_bs);

            // -------------------------- renew indexes --------------------------
            cnt_a = cnt_a_1;
            N_A_cur = N_A_next;
        }
        M_A_cur = M_A_next;
    }

   scalar_free(spm_A[0]);
   scalar_free(spm_A[1]);
   scalar_free(spm_B);
}

__global__ void  transpose_scalar_dbuf(unlong M0, unlong N0, unlong b_id, double* A, double* B){
    const int tid = get_thread_id();
    const int grp_size = get_group_size();
    const unlong h_a = 8;
    const unlong w_a = 8;
    const unlong M = (M0 + h_a - 1) / h_a * h_a;
    const unlong N = (N0 + w_a - 1) / w_a * w_a;

    // --------- blocking parameters ---------
    const unlong M_A = 16;
    const unlong N_A = 16;
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

    double* spm_A[2];
    double* spm_B[2];
    spm_A[0] = scalar_malloc(M_A * N_A * sizeof(double));
    spm_A[1] = scalar_malloc(M_A * N_A * sizeof(double));
    spm_B[0] = scalar_malloc(M_A * N_A * sizeof(double));
    spm_B[1] = scalar_malloc(M_A * N_A * sizeof(double));

    int ch_al[2], ch_bs[2];

    const int ch0_al = 0;
    const int ch0_bs = 2;

    unint cnt_a = 0;
    unint cnt_d = 0;
    unint cnt_b = 0;
    unlong M_A_cur = min(M_A, M0);
    unlong N_A_cur = min(N_A, N0-c_start);
    ch_al[0] = dma_p2p_opt(&A[OFFSET(0, c_start, N0)], M_A_cur, N_A_cur*sizeof(double), (N0-N_A_cur)*sizeof(double),
                            spm_A[0], M_A_cur, N_A_cur*sizeof(double), (N_A-N_A_cur)*sizeof(double), row_syn, p2pmask, ch0_al);
    ch_bs[1] = dma_p2p_opt(B, 1, 1*sizeof(double), 0, // 发起一次无效传输
                            spm_B[1], 1, 1*sizeof(double), 0, 0, 0, ch0_bs+1);
    for(unlong r=0; r<M; r+=M_A){
        unlong M_A_next;

        for(unlong c=c_start; c<N; c+=c_step){
            // -------------------------- next round indexes --------------------------
            unlong N_A_next;
            unint cnt_a_1 = (cnt_a + 1) % 2;
            unint cnt_b_1 = (cnt_b + 1) % 2;
            // ------------------------------------------------------------------------
            if(c + c_step < N){
                N_A_next = min(N_A, N0-(c+c_step));
                ch_al[cnt_a_1] = dma_p2p_opt(&A[OFFSET(r, c+c_step, N0)], M_A_cur, N_A_next*sizeof(double), (N0-N_A_next)*sizeof(double),
                            spm_A[cnt_a_1], M_A_cur, N_A_next*sizeof(double), (N_A-N_A_next)*sizeof(double), row_syn, p2pmask, ch0_al+cnt_a_1);
            }else if(r + M_A < M){
                N_A_next = min(N_A, N0-c_start);
                M_A_next = min(M_A, M0-(r+M_A));
                ch_al[cnt_a_1] = dma_p2p_opt(&A[OFFSET(r+M_A, c_start, N0)], M_A_next, N_A_next*sizeof(double), (N0-N_A_next)*sizeof(double),
                            spm_A[cnt_a_1], M_A_next, N_A_next*sizeof(double), (N_A-N_A_next)*sizeof(double), row_syn, p2pmask, ch0_al+cnt_a_1);
            }
            
            dma_wait_p2p(ch_al[cnt_a]);

            for(unlong m=0; m<M_A_cur; m++){
                for(unlong n=0; n<N_A_cur; n++){
                    spm_B[cnt_b][OFFSET(n, m, M_A)] = spm_A[cnt_a][OFFSET(m, n, N_A)];
                }
            }

            ch_bs[cnt_b] = dma_p2p_opt(spm_B[cnt_b], N_A_cur, M_A_cur*sizeof(double), (M_A-M_A_cur)*sizeof(double),
                            &B[OFFSET(c, r, M0)], N_A_cur, M_A_cur*sizeof(double), (M0-M_A_cur)*sizeof(double), row_syn, p2pmask, ch0_bs+cnt_b);
            
            dma_wait_p2p(ch_bs[cnt_b_1]);
            // -------------------------- renew indexes --------------------------
            cnt_a = cnt_a_1;
            cnt_b = cnt_b_1;
            N_A_cur = N_A_next;
        }
        M_A_cur = M_A_next;
    }
    dma_wait_p2p(ch_bs[(cnt_b+1)%2]);
    scalar_free(spm_A[0]);
    scalar_free(spm_A[1]);
    scalar_free(spm_B[0]);
    scalar_free(spm_B[1]);
}


__global__ void  transpose_16x16_dbuf(unlong M, unlong N, unlong b_id, double* A, double* B){
    const int tid = get_thread_id();
    const int grp_size = get_group_size();
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

    lvector double* spm_A[2];
    lvector double* spm_B[2];
    spm_A[0] = vector_malloc(M_A * N_A * sizeof(double));
    spm_A[1] = vector_malloc(M_A * N_A * sizeof(double));
    spm_B[0] = vector_malloc(M_A * N_A * sizeof(double));
    spm_B[1] = vector_malloc(M_A * N_A * sizeof(double));
    lvector double* spm_D = vector_malloc(512 * sizeof(double));

    //-----------------------------------------------
    for(int i=0; i<(M_A*N_A>>4); i++){
        spm_B[0][i] = 1.0;
        spm_B[1][i] = 1.0;
    }
    //-----------------------------------------------

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
                    micro_kernel_16x16x2(&spm_A[cnt_a][OFFSET(m, n>>4, N_A>>4)], &spm_B[cnt_b][OFFSET(n, m>>4, M_A>>4)], spm_D, M_A, N_A);
                    n += w_2_a;
                }
                if(n < N_A_cur){
                    micro_kernel_16x16(&spm_A[cnt_a][OFFSET(m, n>>4, N_A>>4)], &spm_B[cnt_b][OFFSET(n, m>>4, M_A>>4)], spm_D, M_A, N_A);
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
    vector_free(spm_A[0]);
    vector_free(spm_A[1]);
    vector_free(spm_B[0]);
    vector_free(spm_B[1]);
    vector_free(spm_D);
}


__global__ void  transpose_null(unlong M, unlong N, unlong b_id, double* A, double* B){

}

void micro_kernel_float_32x32(lvector float* src_a, lvector float* dst_b, lvector double* spm_D,
                        const unlong m, const unlong n){
    __asm__ __volatile__(// m:%3     n:%4
        // R51 = n/2, R53 = 3/2*n
        // R54 = 4*(4*n), R56 = 8*(4*n)
        // OR1 = n/2, OR2 = n, OR3 = 3/2*n
        // --------------------------- step 1 ---------------------------
        // [-3]
        "       SSHFLR          1, %4, R51      ;; R51 = n/2                    \t\n"
        "   |   SMVAGA.M1       %0, AR0         ;; AR0 = src_A                  \t\n"
        "   |   SMVAGA.M2       %4, OR2         ;; OR2 = n                      \t\n"
        // [-2]
        "       SSHFLL          5, %4, R55      ;; R55 = 8*(4*n)                \t\n"
        "   |   SADD.M1         %4, R51, R53    ;; R53 = 3/2*n                  \t\n" 
        "   |   SMVAGA.M2       R51, OR1        ;; OR1 = n/2                    \t\n"
        // [-1]
        "       SSHFLL          4, %4, R54      ;; R54 = 4*(4*n)                \t\n"
        "   |   SADD.M1         R55, %0, R56    ;; R56 = src_A + 8*(4*n)        \t\n"
        "   |   SMVAGA.M2       R53, OR3        ;; OR3 = 3/2*n                  \t\n"
        // [0]
        "       SSHFLL          6, %4, R59      ;; R59 = 16*(4*n)               \t\n"
        "   |   SMVAGA.M1       R56, AR1        ;; AR1 = src_A + 8*(4*n)        \t\n"
        "   |   VLDW            *AR0, VR32                  \t\n"
        "   |   VLDW            *+AR0[OR1], VR48            \t\n"
        // [1]
        "       SADD.M1         R54, %0, R57    ;; R57 = src_A + 4*(4*n)        \t\n"
        "   |   VLDW            *+AR0[OR2], VR36            \t\n"
        "   |   VLDW            *+AR0[OR3], VR52            \t\n"
        "   |   SADDA.M2        R59, AR0, AR0                                   \t\n"
        // [2]
        "       SADD.M1         R54, R56, R58   ;; R58 = src_A + 12*(4*n)       \t\n"
        "   |   VLDW            *AR1, VR33                  \t\n"
        "   |   VLDW            *+AR1[OR1], VR49            \t\n"
        "   |   SADDA.M2        R59, AR1, AR1                                   \t\n"
        // [3]
        "       SMVAGA.M1       %2, AR4         ;; AR4 = spm_D                  \t\n"
        "   |   VLDW            *+AR1[OR2], VR37            \t\n"
        "   |   VLDW            *+AR1[OR3], VR53            \t\n"
        // [4]
        "       VLDW            *AR0, VR34                  \t\n"
        "   |   VLDW            *+AR0[OR1], VR50            \t\n"
        // [5]
        "       VLDW            *AR1, VR35                  \t\n"
        // [6]
        "       VLDW            *+AR1[OR1], VR51            \t\n"
        // [7]
        "       VLDW            *+AR0[OR2], VR38            \t\n"
        // [8]
        "       VLDW            *+AR0[OR3], VR54            \t\n"
        "   |   SMVAGA.M2       R57, AR0        ;; AR0 = src_A + 8*(4*n)        \t\n"
        // [9]
        "       VLDW            *+AR1[OR2], VR39            \t\n"
        "   |   VBALE2          VR32, VR48, VR0             \t\n"
        // [10]
        "       VLDW            *+AR1[OR3], VR55            \t\n"
        "   |   SMVAGA.M2       R58, AR1        ;; AR1 = src_A + 12*(4*n)       \t\n"  
        "   |   VBALE2H         VR32, VR48, VR16            \t\n"
        // [11]
        "       VLDW            *AR0, VR40                  \t\n"
        "   |   VBALE2          VR33, VR49, VR1             \t\n"
        // [12]
        "       VLDW            *+AR0[OR1], VR56            \t\n"
        "   |   VBALE2H         VR33, VR49, VR17            \t\n"
        // [13]
        "       VLDW            *AR1, VR41                  \t\n"
        "   |   VBALE2          VR34, VR50, VR2             \t\n"
        // [14]
        "       VLDW            *+AR1[OR1], VR57            \t\n"
        "   |   VBALE2H         VR34, VR50, VR18            \t\n"
        // [15]
        "       VLDW            *+AR0[OR2], VR44            \t\n"
        "   |   VBALE2          VR35, VR51, VR3             \t\n"
        // [16]
        "       VLDW            *+AR0[OR3], VR60            \t\n"
        "   |   SADDA.M2        R59, AR0, AR0                                   \t\n"
        "   |   VBALE2H         VR35, VR51, VR19            \t\n"
        // [17]
        "       VLDW            *+AR1[OR2], VR45            \t\n"
        "   |   VBALE2          VR36, VR52, VR4             \t\n"
        // [18]
        "       VLDW            *+AR1[OR3], VR61            \t\n"
        "   |   SADDA.M2        R59, AR1, AR1                                   \t\n"
        "   |   VBALE2          VR37, VR53, VR5             \t\n"
        // [19]
        "       VLDW            *AR0, VR42                  \t\n"
        "   |   VBALE2          VR38, VR54, VR6             \t\n"
        // [20]
        "       VLDW            *+AR0[OR1], VR58            \t\n"
        "   |   VBALE2          VR39, VR55, VR7             \t\n"
        // [21]
        "       VLDW            *AR1, VR43                  \t\n"
        "   |   VBALE2H         VR36, VR52, VR20            \t\n"
        // [22]
        "       VLDW            *+AR1[OR1], VR59            \t\n"
        "   |   VBALE2H         VR37, VR53, VR21            \t\n"
        // [23]
        "       VLDW            *+AR0[OR2], VR46            \t\n"
        "   |   VBALE2H         VR38, VR54, VR22            \t\n"
        // [24]
        "       VLDW            *+AR0[OR3], VR62            \t\n"
        "   |   VBALE2H         VR39, VR55, VR23            \t\n"
        // [25]
        "       VLDW            *+AR1[OR2], VR47            \t\n"
        "   |   VBALE2          VR40, VR56, VR8             \t\n"
        // [26]
        "       VLDW            *+AR1[OR3], VR63            \t\n"
        "   |   VBALE2H         VR40, VR56, VR24            \t\n"

        // --------------------------- step 2,3 ---------------------------
        // [0]
        "       VBALE2          VR44, VR60, VR12            \t\n"
        "   |   VSTDW0M16       VR1:VR0, *AR4               \t\n"
        "   |   VSTDW1M16       VR3:VR2, *+AR4[16]          \t\n"
        // [1]
        "       VBALE2H         VR44, VR60, VR28            \t\n"
        "   |   VLDW            *AR4, VR0                   \t\n"
        "   |   VLDW            *+AR4[16], VR4              \t\n"
        // [2]
        "       VBALE2          VR41, VR57, VR9             \t\n"
        "   |   VSTDW0M16       VR17:VR16, *+AR4[128]       \t\n"
        "   |   VSTDW1M16       VR19:VR18, *+AR4[144]       \t\n"
        // [3]
        "       VBALE2H         VR41, VR57, VR25            \t\n"
        "   |   VLDW            *+AR4[256], VR16            \t\n"
        "   |   VLDW            *+AR4[272], VR20            \t\n"
        // [4]
        "       VBALE2          VR45, VR61, VR13            \t\n"
        "   |   VSTDW0M16       VR5:VR4, *+AR4[32]          \t\n"
        "   |   VSTDW1M16       VR7:VR6, *+AR4[48]          \t\n"
        // [5]
        "       VBALE2H         VR45, VR61, VR29            \t\n"
        "   |   VLDW            *+AR4[64], VR1              \t\n"
        "   |   VLDW            *+AR4[80], VR5              \t\n"
        // [6]
        "       VBALE2          VR42, VR58, VR10            \t\n"
        "   |   VSTDW0M16       VR21:VR20, *+AR4[160]       \t\n"
        "   |   VSTDW1M16       VR23:VR22, *+AR4[176]       \t\n"
        // [7]
        "       VBALE2          VR43, VR59, VR11            \t\n"
        "   |   VLDW            *+AR4[320], VR17            \t\n"
        "   |   VLDW            *+AR4[336], VR21            \t\n"
        // [8]
        "       VBALE2H         VR42, VR58, VR26            \t\n" 
        "   |   VSTDW0M16       VR9:VR8, *+AR4[64]          \t\n"
        "   |   VSTDW1M16       VR11:VR10, *+AR4[80]        \t\n"
        // [9]
        "       VBALE2H         VR43, VR59, VR27            \t\n"
        "   |   VLDW            *+AR4[128], VR2             \t\n"
        "   |   VLDW            *+AR4[144], VR6             \t\n"
        // [10]
        "       VBALE2          VR46, VR62, VR14            \t\n"
        "   |   VSTDW0M16       VR25:VR24, *+AR4[192]       \t\n"
        "   |   VSTDW1M16       VR27:VR26, *+AR4[208]       \t\n"
        // [11]
        "       VBALE2          VR47, VR63, VR15            \t\n"
        "   |   VLDW            *+AR4[384], VR18            \t\n"
        "   |   VLDW            *+AR4[400], VR22            \t\n" 
        // [12]
        "       VBALE2H         VR46, VR62, VR30            \t\n"
        "   |   VSTDW0M16       VR13:VR12, *+AR4[96]        \t\n"
        "   |   VSTDW1M16       VR15:VR14, *+AR4[112]       \t\n"
        // [13]
        "       VBALE2H         VR47, VR63, VR31            \t\n"
        "   |   VLDW            *+AR4[192], VR3             \t\n"
        "   |   VLDW            *+AR4[208], VR7             \t\n"
        // [14]
        "       VSTDW0M16       VR29:VR28, *+AR4[224]       \t\n"
        "   |   VSTDW1M16       VR31:VR30, *+AR4[240]       \t\n"
        // [15]
        "       VLDW            *+AR4[448], VR19            \t\n"
        "   |   VLDW            *+AR4[464], VR23            \t\n"
        // [16]
        "       VLDW            *+AR4[32], VR8              \t\n"
        "   |   VLDW            *+AR4[48], VR12             \t\n"
        // [17]
        "       VLDW            *+AR4[288], VR24            \t\n"
        "   |   VLDW            *+AR4[304], VR28            \t\n"
        // [18]
        "       VLDW            *+AR4[96], VR9              \t\n"
        "   |   VLDW            *+AR4[112], VR13            \t\n"
        // [19]
        "       VLDW            *+AR4[352], VR25            \t\n" 
        "   |   VLDW            *+AR4[368], VR29            \t\n"
        // [20]
        "       VLDW            *+AR4[160], VR10            \t\n"
        "   |   VLDW            *+AR4[176], VR14            \t\n"
        // [21]
        "       VLDW            *+AR4[416], VR26            \t\n"
        "   |   VLDW            *+AR4[432], VR30            \t\n"
        // [22]
        "       VLDW            *+AR4[224], VR11            \t\n"
        "   |   VLDW            *+AR4[240], VR15            \t\n"
        // [23]
        "       VLDW            *+AR4[480], VR27            \t\n"
        "   |   VLDW            *+AR4[496], VR31            \t\n"

        // --------------------------- step 4, 5 ---------------------------
        // [0]
        "       VSTDW0M16       VR1:VR0, *AR4               \t\n"
        "   |   VSTDW1M16       VR3:VR2, *+AR4[16]          \t\n"
        "   |   SSHFLR          1, %3, R43      ;; R43 = m/2                    \t\n"
        "   |   SMVAGA.M1       %1, AR2         ;; AR2 = src_B                  \t\n"
        "   |   SMVAGA.M2       %3, OR6         ;; OR6 = m                      \t\n"
        // [1]
        "       VSTDW0M16       VR5:VR4, *+AR4[32]          \t\n"
        "   |   VSTDW1M16       VR7:VR6, *+AR4[48]          \t\n"
        "   |   SSHFLL          5, %3, R47      ;; R47 = 8*(4*m)                \t\n"
        "   |   SADD.M1         %3, R43, R45    ;; R45 = 3/2*m                  \t\n"
        "   |   SMVAGA.M2       R43, OR5        ;; OR5 = m/2                    \t\n"
        // [2]
        "       VLDDW           *AR4, VR1:VR0               \t\n"
        "   |   VLDDW           *+AR4[16], VR3:VR2          \t\n"
        "   |   SSHFLL          4, %3, R46      ;; R46 = 4*(4*m)                \t\n"
        "   |   SADD.M1         R47, %1, R48    ;; R48 = src_B + 8*(4*m)        \t\n"
        "   |   SMVAGA.M2       R45, OR7        ;; OR7 = 3/2*m                  \t\n"
        // [3]
        "       VLDDW           *+AR4[32], VR5:VR4          \t\n"
        "   |   VLDDW           *+AR4[48], VR7:VR6          \t\n"
        "   |   SMVAGA.M1       R48, AR3        ;; AR3 = src_B + 8*(4*m)        \t\n" 
        "   |   SADD.M2         R47, R48, R49   ;; R49 = src_B + 16*(4*m)       \t\n"
        // [4]
        "       VSTDW0M16       VR17:VR16, *+AR4[128]       \t\n"
        "   |   VSTDW1M16       VR19:VR18, *+AR4[144]       \t\n"
        "   |   SADD.M1         R47, R49, R50   ;; R50 = src_B + 24*(4*m)       \t\n"
        // [5]
        "       VSTDW0M16       VR21:VR20, *+AR4[160]       \t\n"
        "   |   VSTDW1M16       VR23:VR22, *+AR4[176]       \t\n"
        // [6]
        "       VLDDW           *+AR4[128], VR17:VR16       \t\n"
        "   |   VLDDW           *+AR4[144], VR19:VR18       \t\n"
        // [7]
        "       VLDDW           *+AR4[160], VR21:VR20       \t\n"
        "   |   VLDDW           *+AR4[176], VR23:VR22       \t\n"
        // [8]
        "       VSTDW0M16       VR9:VR8, *+AR4[64]          \t\n"
        "   |   VSTDW1M16       VR11:VR10, *+AR4[80]        \t\n"
        // [9]
        "       VSTDW0M16       VR13:VR12, *+AR4[96]        \t\n"
        "   |   VSTDW1M16       VR15:VR14, *+AR4[112]       \t\n"
        // [10]
        "       VLDDW           *+AR4[64], VR9:VR8          \t\n"
        "   |   VLDDW           *+AR4[80], VR11:VR10        \t\n"
        // [11]
        "       VLDDW           *+AR4[96], VR13:VR12        \t\n"
        "   |   VLDDW           *+AR4[112], VR15:VR14       \t\n"
        // [12]
        "       VSTDW0M16       VR25:VR24, *+AR4[192]       \t\n"
        "   |   VSTDW1M16       VR27:VR26, *+AR4[208]       \t\n"
        // [13]
        "       VSTDW0M16       VR29:VR28, *+AR4[224]       \t\n"
        "   |   VSTDW1M16       VR31:VR30, *+AR4[240]       \t\n"
        // [14]
        "       VLDDW           *+AR4[192], VR25:VR24       \t\n"
        "   |   VLDDW           *+AR4[208], VR27:VR26       \t\n"
        // [15]
        "       VLDDW           *+AR4[224], VR29:VR28       \t\n"
        "   |   VLDDW           *+AR4[240], VR31:VR30       \t\n"

        // --------------------------- step 6 ---------------------------
        // [0]
        "       VSTW            VR0, *AR2                   \t\n"
        "   |   VSTW            VR16, *+AR2[OR5]            \t\n"
        // [1]
        "       VSTW            VR4, *AR3                   \t\n"
        "   |   VSTW            VR20, *+AR3[OR5]            \t\n"
        "   |   SADDA.M1        R46, AR2, AR2                                   \t\n"
        // [2]
        "       VSTW            VR1, *+AR2[OR6]             \t\n"
        "   |   VSTW            VR17, *+AR2[OR7]            \t\n"
        "   |   SADDA.M1        R46, AR3, AR3                                   \t\n"
        // [3]
        "       VSTW            VR5, *+AR3[OR6]             \t\n"
        "   |   VSTW            VR21, *+AR3[OR7]            \t\n"
        // [4]
        "       VSTW            VR2, *AR2                   \t\n"
        "   |   VSTW            VR18, *+AR2[OR5]            \t\n"
        // [5]
        "       VSTW            VR6, *AR3                   \t\n"
        "   |   VSTW            VR22, *+AR3[OR5]            \t\n"
        "   |   SMVAGA.M1       R49, AR2        ;; AR2 = src_B + 16*(4*m)       \t\n"
        // [6]
        "       VSTW            VR3, *+AR2[OR6]            \t\n"
        "   |   VSTW            VR19, *+AR2[OR7]            \t\n"
        "   |   SMVAGA.M1       R50, AR3        ;; AR3 = src_B + 24*(4*m)       \t\n"  
        // [7]
        "       VSTW            VR7, *+AR3[OR6]            \t\n"
        "   |   VSTW            VR23, *+AR3[OR7]            \t\n"
        // [8]
        "       VSTW            VR8, *AR2                   \t\n"
        "   |   VSTW            VR24, *+AR2[OR5]            \t\n"
        // [9]
        "       VSTW            VR12, *AR3                   \t\n"
        "   |   VSTW            VR28, *+AR3[OR5]            \t\n"
        "   |   SADDA.M1        R46, AR2, AR2                                   \t\n"
        "   |   SBR             R63         \t\n"
        // [10]
        "       VSTW            VR9, *+AR2[OR6]             \t\n"
        "   |   VSTW            VR25, *+AR2[OR7]            \t\n"
        "   |   SADDA.M1        R46, AR3, AR3                                   \t\n"
        // [11]
        "       VSTW            VR13, *+AR3[OR6]             \t\n"
        "   |   VSTW            VR29, *+AR3[OR7]            \t\n"
        // [12]
        "       VSTW            VR10, *AR2                  \t\n"
        "   |   VSTW            VR26, *+AR2[OR5]            \t\n"
        // [13]
        "       VSTW            VR14, *AR3                  \t\n"
        "   |   VSTW            VR30, *+AR3[OR5]            \t\n"
        // [14]
        "       VSTW            VR11, *+AR2[OR6]            \t\n"
        "   |   VSTW            VR27, *+AR2[OR7]            \t\n"
        // [15]
        "       VSTW            VR15, *+AR3[OR6]            \t\n"
        "   |   VSTW            VR31, *+AR3[OR7]            \t\n"
    :
    :"r"(src_a), "r"(dst_b), "r"(spm_D), "r"(m), "r"(n)
    );
}

void micro_kernel_float_32x32_add(lvector float* src_a, lvector float* dst_b, lvector double* spm_D,
                        const unlong m, const unlong n){
    __asm__ __volatile__(// m:%3     n:%4
        // R51 = n/2, R53 = 3/2*n
        // R54 = 4*(4*n), R56 = 8*(4*n)
        // OR1 = n/2, OR2 = n, OR3 = 3/2*n
        // --------------------------- step 1 (load A) ---------------------------
        // [-3]
        "       SSHFLR          1, %4, R51      ;; R51 = n/2                    \t\n" // for A
        "   |   SMVAGA.M1       %0, AR0         ;; AR0 = src_A                  \t\n"
        "   |   SMVAGA.M2       %4, OR2         ;; OR2 = n                      \t\n"
        // [-2]
        "       SSHFLL          5, %4, R55      ;; R55 = 8*(4*n)                \t\n" // for A
        "   |   SADD.M1         %4, R51, R53    ;; R53 = 3/2*n                  \t\n" 
        "   |   SMVAGA.M2       R51, OR1        ;; OR1 = n/2                    \t\n"
        // [-1]
        "       SSHFLL          4, %4, R54      ;; R54 = 4*(4*n)                \t\n" // for A
        "   |   SADD.M1         R55, %0, R56    ;; R56 = src_A + 8*(4*n)        \t\n"
        "   |   SMVAGA.M2       R53, OR3        ;; OR3 = 3/2*n                  \t\n"
        // [0]
        "       SSHFLL          6, %4, R59      ;; R59 = 16*(4*n)               \t\n" // for A
        "   |   SMVAGA.M1       R56, AR1        ;; AR1 = src_A + 8*(4*n)        \t\n"
        "   |   SMVAGA.M2       %2, AR4         ;; AR4 = spm_D                  \t\n"
        "   |   VLDW            *AR0, VR32                  \t\n"
        "   |   VLDW            *+AR0[OR1], VR48            \t\n"
        // [1]
        "       SADD.M1         R54, %0, R57    ;; R57 = src_A + 4*(4*n)        \t\n" // for A
        "   |   VLDW            *+AR0[OR2], VR36            \t\n"
        "   |   VLDW            *+AR0[OR3], VR52            \t\n"
        "   |   SADDA.M2        R59, AR0, AR0                                   \t\n"
        // [2]
        "       SADD.M1         R54, R56, R58   ;; R58 = src_A + 12*(4*n)       \t\n" // for A
        "   |   VLDW            *AR1, VR33                  \t\n"
        "   |   VLDW            *+AR1[OR1], VR49            \t\n"
        "   |   SADDA.M2        R59, AR1, AR1                                   \t\n"
        // [3]
        "       SSHFLR          1, %3, R43      ;; R43 = m/2                    \t\n" // for B
        "   |   SMVAGA.M1       %1, AR2         ;; AR2 = src_B                  \t\n"
        "   |   VLDW            *+AR1[OR2], VR37            \t\n"
        "   |   VLDW            *+AR1[OR3], VR53            \t\n"
        // [4]
        "       SSHFLL          5, %3, R47      ;; R47 = 8*(4*m)                \t\n" // for B
        "   |   SADD.M1         %3, R43, R45    ;; R45 = 3/2*m                  \t\n"
        "   |   SMVAGA.M2       R43, OR5        ;; OR5 = m/2                    \t\n"
        "   |   VLDW            *AR0, VR34                  \t\n"
        "   |   VLDW            *+AR0[OR1], VR50            \t\n"
        // [5]
        "       SMVAGA.M1       %3, OR6         ;; OR6 = m                      \t\n" // for B
        "   |   VLDW            *AR1, VR35                  \t\n"
        "   |   VLDW            *+AR1[OR1], VR51            \t\n"
        "   |   SMVAGA.M2       R57, AR0        ;; AR0 = src_A + 8*(4*n)        \t\n"
        // [6]
        "       VLDW            *+AR0[OR2], VR38            \t\n"
        "   |   VLDW            *+AR0[OR3], VR54            \t\n"
        "   |   SMVAGA.M2       R58, AR1        ;; AR1 = src_A + 12*(4*n)       \t\n"  
        // [7]
        "       SSHFLL          4, %3, R46      ;; R46 = 4*(4*m)                \t\n" // for B
        "   |   SADD.M1         R47, %1, R48    ;; R48 = src_B + 8*(4*m)        \t\n"
        "   |   SMVAGA.M2       R45, OR7        ;; OR7 = 3/2*m                  \t\n"
        "   |   VLDW            *+AR1[OR2], VR39            \t\n"
        "   |   VLDW            *+AR1[OR3], VR55            \t\n"
        // [8]
        "       SMVAGA.M1       R48, AR3        ;; AR3 = src_B + 8*(4*m)        \t\n" // for B
        "   |   SADD.M2         R47, R48, R49   ;; R49 = src_B + 16*(4*m)       \t\n"
        "   |   VLDW            *AR0, VR40                  \t\n"
        "   |   VLDW            *+AR0[OR1], VR56            \t\n"
        // [9]
        "       SADD.M1         R47, R49, R50   ;; R50 = src_B + 24*(4*m)       \t\n" // for B
        "   |   VLDW            *AR1, VR41                  \t\n"
        "   |   VLDW            *+AR1[OR1], VR57            \t\n"
        "   |   SADDA.M2        R59, AR0, AR0                                   \t\n"
        "   |   VBALE2          VR32, VR48, VR0             \t\n"
        // [10]
        "       VLDW            *+AR0[OR2], VR44            \t\n"
        "   |   VLDW            *+AR0[OR3], VR60            \t\n"
        "   |   SADDA.M2        R59, AR1, AR1                                   \t\n"
        "   |   VBALE2H         VR32, VR48, VR16            \t\n"
        // [11]
        "       VLDW            *+AR1[OR2], VR45            \t\n"
        "   |   VLDW            *+AR1[OR3], VR61            \t\n"
        "   |   VBALE2          VR33, VR49, VR1             \t\n"
        // [12]
        "       VLDW            *AR0, VR42                  \t\n"
        "   |   VLDW            *+AR0[OR1], VR58            \t\n"
        "   |   VBALE2H         VR33, VR49, VR17            \t\n"
        // [13]
        "       VLDW            *AR1, VR43                  \t\n"
        "   |   VLDW            *+AR1[OR1], VR59            \t\n"
        "   |   VBALE2          VR34, VR50, VR2             \t\n"
        // [14]
        "       VLDW            *+AR0[OR2], VR46            \t\n"
        "   |   VLDW            *+AR0[OR3], VR62            \t\n"
        "   |   VBALE2          VR35, VR51, VR3             \t\n"
        // [15]
        "       VLDW            *+AR1[OR2], VR47            \t\n"
        "   |   VLDW            *+AR1[OR3], VR63            \t\n"
        "   |   VBALE2H         VR34, VR50, VR18            \t\n"
        
        // --------------------------- step 1.5 (load B) ---------------------------
        // [0]
        "       VBALE2H         VR35, VR51, VR19            \t\n"
        "   |   VLDW            *AR2      , VR32            \t\n"
        "   |   VLDW            *+AR2[OR5], VR48            \t\n"
        // [1]
        "       VBALE2          VR36, VR52, VR4             \t\n"
        "   |   VLDW            *AR3      , VR36            \t\n"
        "   |   VLDW            *+AR3[OR5], VR52            \t\n"
        "   |   SADDA.M1        R46, AR2, AR2                                   \t\n"
        // [2]
        "       VBALE2          VR37, VR53, VR5             \t\n"
        "   |   VLDW            *+AR2[OR6], VR33            \t\n"
        "   |   VLDW            *+AR2[OR7], VR49            \t\n"
        "   |   SADDA.M1        R46, AR3, AR3                                   \t\n"
        // [3]
        "       VBALE2          VR38, VR54, VR6             \t\n"
        "   |   VLDW            *+AR3[OR6], VR37            \t\n"
        "   |   VLDW            *+AR3[OR7], VR53            \t\n"
        // [4]
        "       VBALE2          VR39, VR55, VR7             \t\n"
        "   |   VLDW            *AR2      , VR34            \t\n"
        "   |   VLDW            *+AR2[OR5], VR50            \t\n"
        // [5]
        "       VBALE2H         VR36, VR52, VR20            \t\n"
        "   |   VLDW            *AR3      , VR38            \t\n"
        "   |   VLDW            *+AR3[OR5], VR54            \t\n"
        "   |   SMVAGA.M1       R49, AR2        ;; AR2 = src_B + 16*(4*m)       \t\n"
        // [6]
        "       VBALE2H         VR37, VR53, VR21            \t\n"
        "   |   VLDW            *+AR2[OR6], VR35            \t\n"
        "   |   VLDW            *+AR2[OR7], VR51            \t\n"
        "   |   SMVAGA.M1       R50, AR3        ;; AR3 = src_B + 24*(4*m)       \t\n"  
        // [7]
        "       VBALE2H         VR38, VR54, VR22            \t\n"
        "   |   VLDW            *+AR3[OR6], VR39            \t\n"
        "   |   VLDW            *+AR3[OR7], VR55            \t\n"
        // [8]
        "       VBALE2H         VR39, VR55, VR23            \t\n"
        "   |   VLDW            *AR2      , VR40            \t\n"
        "   |   VLDW            *+AR2[OR5], VR56            \t\n"
        // [9]
        "       VBALE2          VR40, VR56, VR8             \t\n"
        "   |   VLDW            *AR3      , VR44            \t\n"
        "   |   VLDW            *+AR3[OR5], VR60            \t\n"
        "   |   SADDA.M1        R46, AR2, AR2                                   \t\n"
        // [10]
        "       VBALE2H         VR40, VR56, VR24            \t\n"
        "   |   VLDW            *+AR2[OR6], VR41            \t\n"
        "   |   VLDW            *+AR2[OR7], VR57            \t\n"
        // [11]
        "       VBALE2          VR44, VR60, VR12            \t\n"
        "   |   VLDW            *+AR3[OR6], VR45            \t\n"
        "   |   VLDW            *+AR3[OR7], VR61            \t\n"
        "   |   SADDA.M1        R46, AR3, AR3                                   \t\n"
        // [12]
        "       VBALE2H         VR44, VR60, VR28            \t\n"
        "   |   VLDW            *AR2      , VR42            \t\n"
        "   |   VLDW            *+AR2[OR5], VR58            \t\n"
        // [13]
        "       VBALE2          VR41, VR57, VR9             \t\n"
        "   |   VLDW            *+AR2[OR6], VR43            \t\n"
        "   |   VLDW            *+AR2[OR7], VR59            \t\n"
        // --------------------------- step 2,3 ---------------------------
        // [0]
        "       VBALE2H         VR41, VR57, VR25            \t\n"
        "   |   VSTDW0M16       VR1:VR0, *AR4               \t\n"
        "   |   VSTDW1M16       VR3:VR2, *+AR4[16]          \t\n"
        // [1]
        "       VBALE2          VR45, VR61, VR13            \t\n"
        "   |   VLDW            *AR4, VR0                   \t\n"
        "   |   VLDW            *+AR4[16], VR4              \t\n"
        // [2]
        "       VBALE2H         VR45, VR61, VR29            \t\n"
        "   |   VSTDW0M16       VR17:VR16, *+AR4[128]       \t\n"
        "   |   VSTDW1M16       VR19:VR18, *+AR4[144]       \t\n"
        // [3]
        "       VBALE2          VR42, VR58, VR10            \t\n"
        "   |   VLDW            *+AR4[256], VR16            \t\n"
        "   |   VLDW            *+AR4[272], VR20            \t\n"
        // [4]
        "       VBALE2          VR43, VR59, VR11            \t\n"
        "   |   VSTDW0M16       VR5:VR4, *+AR4[32]          \t\n"
        "   |   VSTDW1M16       VR7:VR6, *+AR4[48]          \t\n"
        // [5]
        "       VBALE2H         VR42, VR58, VR26            \t\n" 
        "   |   VLDW            *+AR4[64], VR1              \t\n"
        "   |   VLDW            *+AR4[80], VR5              \t\n"
        // [6]
        "       VBALE2H         VR43, VR59, VR27            \t\n"
        "   |   VSTDW0M16       VR21:VR20, *+AR4[160]       \t\n"
        "   |   VSTDW1M16       VR23:VR22, *+AR4[176]       \t\n"
        // [7]
        "       VBALE2          VR46, VR62, VR14            \t\n"
        "   |   VLDW            *+AR4[320], VR17            \t\n"
        "   |   VLDW            *+AR4[336], VR21            \t\n"
        // [8]
        "       VBALE2          VR47, VR63, VR15            \t\n"
        "   |   VSTDW0M16       VR9:VR8, *+AR4[64]          \t\n"
        "   |   VSTDW1M16       VR11:VR10, *+AR4[80]        \t\n"
        // [9]
        "       VBALE2H         VR46, VR62, VR30            \t\n"
        "   |   VLDW            *+AR4[128], VR2             \t\n"
        "   |   VLDW            *+AR4[144], VR6             \t\n"
        // [10]
        "       VBALE2H         VR47, VR63, VR31            \t\n"
        "   |   VSTDW0M16       VR25:VR24, *+AR4[192]       \t\n"
        "   |   VSTDW1M16       VR27:VR26, *+AR4[208]       \t\n"
        // [11]
        "       VLDW            *+AR4[384], VR18            \t\n"
        "   |   VLDW            *+AR4[400], VR22            \t\n" 
        // [12]
        "       VSTDW0M16       VR13:VR12, *+AR4[96]        \t\n"
        "   |   VSTDW1M16       VR15:VR14, *+AR4[112]       \t\n"
        // [13]
        "       VLDW            *+AR4[192], VR3             \t\n"
        "   |   VLDW            *+AR4[208], VR7             \t\n"
        // [14]
        "       VSTDW0M16       VR29:VR28, *+AR4[224]       \t\n"
        "   |   VSTDW1M16       VR31:VR30, *+AR4[240]       \t\n"
        // [15]
        "       VLDW            *+AR4[448], VR19            \t\n"
        "   |   VLDW            *+AR4[464], VR23            \t\n"
        // [16]
        "       VLDW            *+AR4[32], VR8              \t\n"
        "   |   VLDW            *+AR4[48], VR12             \t\n"
        // [17]
        "       VLDW            *+AR4[288], VR24            \t\n"
        "   |   VLDW            *+AR4[304], VR28            \t\n"
        // [18]
        "       VLDW            *+AR4[96], VR9              \t\n"
        "   |   VLDW            *+AR4[112], VR13            \t\n"
        // [19]
        "       VLDW            *+AR4[352], VR25            \t\n" 
        "   |   VLDW            *+AR4[368], VR29            \t\n"
        // [20]
        "       VLDW            *+AR4[160], VR10            \t\n"
        "   |   VLDW            *+AR4[176], VR14            \t\n"
        // [21]
        "       VLDW            *+AR4[416], VR26            \t\n"
        "   |   VLDW            *+AR4[432], VR30            \t\n"
        // [22]
        "       VLDW            *+AR4[224], VR11            \t\n"
        "   |   VLDW            *+AR4[240], VR15            \t\n"
        // [23]
        "       VLDW            *+AR4[480], VR27            \t\n"
        "   |   VLDW            *+AR4[496], VR31            \t\n"
        "   |   VMOVI    	    0x3F8000003F800000, VR63	;; VR63 = 1.0 \t\n" 
        // --------------------------- step 4, 5 ---------------------------
        // [0]
        "       VSTDW0M16       VR1:VR0, *AR4               \t\n"
        "   |   VSTDW1M16       VR3:VR2, *+AR4[16]          \t\n"
        // [1]
        "       VLDDW           *AR4, VR1:VR0               \t\n"
        "   |   VLDDW           *+AR4[16], VR3:VR2          \t\n"
        // [2]
        "       VSTDW0M16       VR17:VR16, *+AR4[128]       \t\n"
        "   |   VSTDW1M16       VR19:VR18, *+AR4[144]       \t\n"
        // [3]
        "       VLDDW           *+AR4[128], VR17:VR16       \t\n"
        "   |   VLDDW           *+AR4[144], VR19:VR18       \t\n"
        // [4]
        "       VSTDW0M16       VR5:VR4, *+AR4[32]          \t\n"
        "   |   VSTDW1M16       VR7:VR6, *+AR4[48]          \t\n"
        // [5]
        "       VLDDW           *+AR4[32], VR5:VR4          \t\n"
        "   |   VLDDW           *+AR4[48], VR7:VR6          \t\n"
        // [6]
        "       VSTDW0M16       VR21:VR20, *+AR4[160]       \t\n"
        "   |   VSTDW1M16       VR23:VR22, *+AR4[176]       \t\n"
        // [7]
        "       VLDDW           *+AR4[160], VR21:VR20       \t\n"
        "   |   VLDDW           *+AR4[176], VR23:VR22       \t\n"
        // [8]
        "       VSTDW0M16       VR9:VR8, *+AR4[64]          \t\n"
        "   |   VSTDW1M16       VR11:VR10, *+AR4[80]        \t\n"
        // [9]
        "       VLDDW           *+AR4[64], VR9:VR8          \t\n"
        "   |   VLDDW           *+AR4[80], VR11:VR10        \t\n"
        // [10]
        "       VSTDW0M16       VR25:VR24, *+AR4[192]       \t\n"
        "   |   VSTDW1M16       VR27:VR26, *+AR4[208]       \t\n"
        "   |   VFMULAS32.M1	VR32, VR63, VR0, VR0        \t\n"
        "   |   VFMULAS32.M2    VR33, VR63, VR1, VR1        \t\n"
        "   |   VMOVI    	    0x3F8000003F800000, VR32	;; VR33 = 1.0 \t\n"
        // [11]
        "       VLDDW           *+AR4[192], VR25:VR24       \t\n"
        "   |   VLDDW           *+AR4[208], VR27:VR26       \t\n"
        "   |   VFMULAS32.M1    VR34, VR32, VR2, VR2        \t\n"
        "   |   VFMULAS32.M2    VR35, VR32, VR3, VR3        \t\n"
        // [12]
        "       VSTDW0M16       VR13:VR12, *+AR4[96]        \t\n"
        "   |   VSTDW1M16       VR15:VR14, *+AR4[112]       \t\n"
        "   |   VFMULAS32.M1    VR48, VR32, VR16, VR16      \t\n"
        "   |   VFMULAS32.M2    VR49, VR32, VR17, VR17      \t\n"
        // [13]
        "       VLDDW           *+AR4[96], VR13:VR12        \t\n"
        "   |   VLDDW           *+AR4[112], VR15:VR14       \t\n"
        "   |   VFMULAS32.M1    VR50, VR32, VR18, VR18      \t\n"
        "   |   VFMULAS32.M2    VR51, VR32, VR19, VR19      \t\n"
        // [14]
        "       VSTDW0M16       VR29:VR28, *+AR4[224]       \t\n"
        "   |   VSTDW1M16       VR31:VR30, *+AR4[240]       \t\n"
        // [15]
        "       VLDDW           *+AR4[224], VR29:VR28       \t\n"
        "   |   VLDDW           *+AR4[240], VR31:VR30       \t\n"
        "   |   SMVAGA.M1       %1, AR2         ;; AR2 = src_B                  \t\n" // for B
        // --------------------------- step 5.9 (load B) ---------------------------
        // [0]
        "       VFMULAS32.M1    VR36, VR32, VR4, VR4        \t\n"
        "   |   VFMULAS32.M2    VR52, VR32, VR20, VR20      \t\n"
        "   |   VLDW            *AR3      , VR46            \t\n"
        "   |   VLDW            *+AR3[OR5], VR62            \t\n"
        // [1]
        "       VFMULAS32.M1    VR37, VR32, VR5, VR5        \t\n"
        "   |   VFMULAS32.M2    VR53, VR32, VR21, VR21      \t\n"
        "   |   VLDW            *+AR3[OR6], VR47            \t\n"
        "   |   VLDW            *+AR3[OR7], VR63            \t\n"
        "   |   SMVAGA.M1       R48, AR3        ;; AR3 = src_B + 8*(4*m)        \t\n" // for B
        "   |   SADDA.M2        R46, AR2, AR2                                   \t\n"
        // --------------------------- step 6 ---------------------------
        // [0]
        "       VFMULAS32.M1    VR38, VR32, VR6, VR6        \t\n"
        "   |   VFMULAS32.M2    VR54, VR32, VR22, VR22      \t\n"
        "   |   VSTW            VR0, *AR2                   \t\n"
        "   |   VSTW            VR16, *+AR2[OR5]            \t\n"
        // [1]
        "       VFMULAS32.M1    VR39, VR32, VR7, VR7        \t\n"
        "   |   VFMULAS32.M2    VR55, VR32, VR23, VR23      \t\n"
        "   |   VSTW            VR1, *+AR2[OR6]             \t\n"
        "   |   VSTW            VR17, *+AR2[OR7]            \t\n"
        // [2]
        "       VFMULAS32.M1    VR40, VR32, VR8, VR8        \t\n"
        "   |   VFMULAS32.M2    VR56, VR32, VR24, VR24      \t\n"
        "   |   VSTW            VR2, *AR2                   \t\n"
        "   |   VSTW            VR18, *+AR2[OR5]            \t\n"
        // [3]
        "       VFMULAS32.M1    VR41, VR32, VR9, VR9        \t\n"
        "   |   VFMULAS32.M2    VR57, VR32, VR25, VR25      \t\n"
        "   |   VSTW            VR3, *+AR2[OR6]             \t\n"
        "   |   VSTW            VR19, *+AR2[OR7]            \t\n"
        "   |   SADDA.M1        R46, AR3, AR3                                   \t\n"
        // [4]
        "       VFMULAS32.M1    VR42, VR32, VR10, VR10      \t\n"
        "   |   VFMULAS32.M2    VR58, VR32, VR26, VR26      \t\n"
        "   |   VSTW            VR4, *AR3                   \t\n"
        "   |   VSTW            VR20, *+AR3[OR5]            \t\n"
        // [5]
        "       VFMULAS32.M1    VR43, VR32, VR11, VR11      \t\n"
        "   |   VFMULAS32.M2    VR59, VR32, VR27, VR27      \t\n"
        "   |   VSTW            VR5, *+AR3[OR6]             \t\n"
        "   |   VSTW            VR21, *+AR3[OR7]            \t\n"
        "   |   SMVAGA.M1       R49, AR2        ;; AR2 = src_B + 16*(4*m)       \t\n"
        // [6]
        "       VFMULAS32.M1    VR44, VR32, VR12, VR12      \t\n"
        "   |   VFMULAS32.M2    VR60, VR32, VR28, VR28      \t\n"
        "   |   VSTW            VR6, *AR3                   \t\n"
        "   |   VSTW            VR22, *+AR3[OR5]            \t\n"
        // [7]
        "       VFMULAS32.M1    VR45, VR32, VR13, VR13      \t\n"
        "   |   VFMULAS32.M2    VR61, VR32, VR29, VR29      \t\n"
        "   |   VSTW            VR7, *+AR3[OR6]             \t\n"
        "   |   VSTW            VR23, *+AR3[OR7]            \t\n"
        "   |   SADDA.M1        R46, AR2, AR2                                   \t\n"
        // [8]
        "       VFMULAS32.M1    VR46, VR32, VR14, VR14      \t\n"
        "   |   VFMULAS32.M2    VR62, VR32, VR30, VR30      \t\n"
        "   |   VSTW            VR8, *AR2                   \t\n"
        "   |   VSTW            VR24, *+AR2[OR5]            \t\n"
        // [9]
        "       VFMULAS32.M1    VR47, VR32, VR15, VR15      \t\n"
        "   |   VFMULAS32.M2    VR63, VR32, VR31, VR31      \t\n"
        "   |   VSTW            VR9, *+AR2[OR6]             \t\n"
        "   |   VSTW            VR25, *+AR2[OR7]            \t\n"
        "   |   SMVAGA.M1       R50, AR3        ;; AR3 = src_B + 24*(4*m)       \t\n"
        "   |   SBR             R63         \t\n"  
        // [10]
        "       VSTW            VR10, *AR2                  \t\n"
        "   |   VSTW            VR26, *+AR2[OR5]            \t\n"/////////
        // [11]
        "       VSTW            VR11, *+AR2[OR6]            \t\n"
        "   |   VSTW            VR27, *+AR2[OR7]            \t\n"
        "   |   SADDA.M1        R46, AR3, AR3                                   \t\n"
        // [12]
        "       VSTW            VR12, *AR3                  \t\n"
        "   |   VSTW            VR28, *+AR3[OR5]            \t\n"
        // [13]
        "       VSTW            VR13, *+AR3[OR6]            \t\n"
        "   |   VSTW            VR29, *+AR3[OR7]            \t\n"
        // [14]
        "       VSTW            VR14, *AR3                  \t\n"
        "   |   VSTW            VR30, *+AR3[OR5]            \t\n"
        // [15]
        "       VSTW            VR15, *+AR3[OR6]            \t\n"
        "   |   VSTW            VR31, *+AR3[OR7]            \t\n"   
    :
    :"r"(src_a), "r"(dst_b), "r"(spm_D), "r"(m), "r"(n)
    );
}


__global__ void  transpose_float_32x32_dbuf(unlong M, unlong N, unlong b_id, float* A, float* B){
    const int tid = get_thread_id();
    const int grp_size = get_group_size();
    const unlong h_a = 32;
    const unlong w_a = 32;
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

    lvector float* spm_A[2];
    lvector float* spm_B[2];
    spm_A[0] = vector_malloc(M_A * N_A * sizeof(float));
    spm_A[1] = vector_malloc(M_A * N_A * sizeof(float));
    spm_B[0] = vector_malloc(M_A * N_A * sizeof(float));
    spm_B[1] = vector_malloc(M_A * N_A * sizeof(float));
    lvector double* spm_D = vector_malloc(2048 * sizeof(float));

    //-----------------------------------------------
    // int ch_tmp = dma_p2p_opt(B, M_A, N_A*sizeof(float), (N-N_A)*sizeof(float),
    //                         spm_B[0], M_A, N_A*sizeof(float), 0, row_syn, p2pmask, 1);

    // dma_wait_p2p(ch_tmp);
    //-----------------------------------------------

    int ch_al[2], ch_bs[2];

    const int ch0_al = 0;
    const int ch0_bs = 2;

    unint cnt_a = 0;
    unint cnt_d = 0;
    unint cnt_b = 0;
    unlong M_A_cur = min(M_A, M);
    unlong N_A_cur = min(N_A, N-c_start);
#ifndef CNACEL_DMA
    ch_al[0] = dma_p2p_opt(&A[OFFSET(0, c_start, N)], M_A_cur, N_A_cur*sizeof(float), (N-N_A_cur)*sizeof(float),
                            spm_A[0], M_A_cur, N_A_cur*sizeof(float), (N_A-N_A_cur)*sizeof(float), row_syn, p2pmask, ch0_al);
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
                ch_al[cnt_a_1] = dma_p2p_opt(&A[OFFSET(r, c+c_step, N)], M_A_cur, N_A_next*sizeof(float), (N-N_A_next)*sizeof(float),
                            spm_A[cnt_a_1], M_A_cur, N_A_next*sizeof(float), (N_A-N_A_next)*sizeof(float), row_syn, p2pmask, ch0_al+cnt_a_1);
        #endif
            }else if(r + M_A < M){
                N_A_next = min(N_A, N-c_start);
                M_A_next = min(M_A, M-(r+M_A));
        #ifndef CNACEL_DMA
                ch_al[cnt_a_1] = dma_p2p_opt(&A[OFFSET(r+M_A, c_start, N)], M_A_next, N_A_next*sizeof(float), (N-N_A_next)*sizeof(float),
                            spm_A[cnt_a_1], M_A_next, N_A_next*sizeof(float), (N_A-N_A_next)*sizeof(float), row_syn, p2pmask, ch0_al+cnt_a_1);
        #endif
            }
        #ifndef CNACEL_DMA
            dma_wait_p2p(ch_al[cnt_a]);
        #endif
            for(unlong m=0; m<M_A_cur; m+=h_a){
                for(unlong n=0; n<N_A_cur; n+=w_a){
                    micro_kernel_float_32x32(&spm_A[cnt_a][OFFSET(m, n>>5, N_A>>5)], &spm_B[cnt_b][OFFSET(n, m>>5, M_A>>5)], spm_D, M_A, N_A);
                }
            }
        #ifndef CNACEL_DMA
            ch_bs[cnt_b] = dma_p2p_opt(spm_B[cnt_b], N_A_cur, M_A_cur*sizeof(float), (M_A-M_A_cur)*sizeof(float),
                            &B[OFFSET(c, r, M)], N_A_cur, M_A_cur*sizeof(float), (M-M_A_cur)*sizeof(float), row_syn, p2pmask, ch0_bs+cnt_b);

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
    vector_free(spm_A[0]);
    vector_free(spm_A[1]);
    vector_free(spm_B[0]);
    vector_free(spm_B[1]);
    vector_free(spm_D);
}


__global__ void  transpose_float_null(unlong M, unlong N, unlong b_id, float* A, float* B){

}