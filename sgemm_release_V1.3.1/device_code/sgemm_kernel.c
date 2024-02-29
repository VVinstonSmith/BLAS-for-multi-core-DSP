#include <compiler/m3000.h>
#include <compiler/vsip.h>
#include "hthread_device.h"
#define unlong unsigned long
#define unint unsigned int
#define indexType long

#define OFFSET(row, col, ld) ((row) * (ld) + (col))

#define enable_alpha
#define enable_beta
#define unroll_inner_loop
// #define CNACEL_DMA
// #define use_r6c96
#define use_r6c128

__gsm__ float gsm_mem[1024*256*6];

unlong min(unlong x, unlong y){
    if(x<y) return x;
    return y;
}

#ifdef enable_beta
void C_plus_beta(lvector float* src_C, unlong len_C, float beta){
    __asm__ __volatile__(
    // [-2]
    "       SMVAGA.M1       %0, AR0                         \t\n"
    "   |   SBALE2          %2, %2, %2                      \t\n"
    "   |   VMOVI           0x3F8000003F800000, VR63        \t\n"
    // [-1]
    "       SVBCAST.M1      %2, VR60    ;; VR60 = beta      \t\n"
    "   |   SMOVI24         1024, R11   ;; 32(num of vecs) x 32(len of vec)\t\n"
    // [0]
    "       SMOVI24         4096, R50   ;; R50 = 32*(32*4)  \t\n"
    "   |   VLDDW           *AR0, VR1:VR0                   \t\n"
    "   |   VLDDW           *+AR0[16], VR3:VR2              \t\n"
    // [1]
    "       SMOVI24         1024, R51                       \t\n"
    "   |   VLDDW           *+AR0[32], VR5:VR4              \t\n"
    "   |   VLDDW           *+AR0[48], VR7:VR6              \t\n"
    // [2]
    "       VLDDW           *+AR0[64], VR9:VR8              \t\n"
    "   |   VLDDW           *+AR0[80], VR11:VR10            \t\n"
    // [3]
    "       VLDDW           *+AR0[96], VR13:VR12            \t\n"
    "   |   VLDDW           *+AR0[112], VR15:VR14           \t\n"
    // [4]
    "       VLDDW           *+AR0[128], VR17:VR16           \t\n"
    "   |   VLDDW           *+AR0[144], VR19:VR18           \t\n"
    // [5]
    "       VLDDW           *+AR0[160], VR21:VR20           \t\n"
    "   |   VLDDW           *+AR0[176], VR23:VR22           \t\n"
    "   |   SADDA.M2        R50, AR0, AR1                   \t\n"
    // [6]
    "       VLDDW           *+AR0[192], VR25:VR24           \t\n"
    "   |   VLDDW           *+AR0[208], VR27:VR26           \t\n"
    // [7]
    "       VLDDW           *+AR0[224], VR29:VR28           \t\n"
    "   |   VLDDW           *+AR0[240], VR31:VR30           \t\n"
    // [8]
    "       SNOP            1       \t\n"
    // [9]
    "       VFMULAS32.M1    VR63, VR0, VR60, VR0            \t\n"
    "   |   VFMULAS32.M2    VR63, VR1, VR60, VR1            \t\n"

    "loop_C_plus_beta:   \t\n"
// ------------------------------ main loop start ------------------------------
    // [0]
    "       VFMULAS32.M1    VR63, VR2, VR60, VR2            \t\n"
    "   |   VFMULAS32.M2    VR63, VR3, VR60, VR3            \t\n"
    "   |   VFMULAS32.M3    VR63, VR4, VR60, VR4            \t\n"
    "   |   VLDDW           *AR1, VR1:VR0                   \t\n"
    "   |   VLDDW           *+AR1[16], VR3:VR2              \t\n"
    // [1]
    "       SLT             R11, %1, R0                     \t\n"
    "   |   VFMULAS32.M1    VR63, VR5, VR60, VR5            \t\n"
    "   |   VFMULAS32.M2    VR63, VR6, VR60, VR6            \t\n"
    "   |   VFMULAS32.M3    VR63, VR7, VR60, VR7            \t\n"
    "   |   VLDDW           *+AR1[32], VR5:VR4              \t\n"
    "   |   VLDDW           *+AR1[48], VR7:VR6              \t\n"
    // [2]
    "       SADD.M1         R51, R11, R11                   \t\n"
    "   |   VFMULAS32.M1    VR63, VR8, VR60, VR8            \t\n"
    "   |   VFMULAS32.M2    VR63, VR9, VR60, VR9            \t\n"
    "   |   VFMULAS32.M3    VR63, VR10, VR60, VR10          \t\n"
    "   |   VLDDW           *+AR1[64], VR9:VR8              \t\n"
    "   |   VLDDW           *+AR1[80], VR11:VR10            \t\n"
    // [3]
    "       VFMULAS32.M1    VR63, VR11, VR60, VR11          \t\n"
    "   |   VFMULAS32.M2    VR63, VR12, VR60, VR12          \t\n"
    "   |   VFMULAS32.M3    VR63, VR13, VR60, VR13          \t\n"
    "   |   VLDDW           *+AR1[96], VR13:VR12            \t\n"
    "   |   VLDDW           *+AR1[112], VR15:VR14           \t\n"
    // [4]
    "       VFMULAS32.M1    VR63, VR14, VR60, VR14          \t\n"
    "   |   VFMULAS32.M2    VR63, VR15, VR60, VR15          \t\n"
    "   |   VFMULAS32.M3    VR63, VR16, VR60, VR16          \t\n"
    "   |   VLDDW           *+AR1[128], VR17:VR16           \t\n"
    "   |   VLDDW           *+AR1[144], VR19:VR18           \t\n"
    // [5]
    "       VFMULAS32.M1    VR63, VR17, VR60, VR17          \t\n"
    "   |   VFMULAS32.M2    VR63, VR18, VR60, VR18          \t\n"
    "   |   VFMULAS32.M3    VR63, VR19, VR60, VR19          \t\n"
    "   |   VLDDW           *+AR1[160], VR21:VR20           \t\n"
    "   |   VLDDW           *+AR1[176], VR23:VR22           \t\n"
    "   |   SADDA.M1        R50, AR1, AR1                   \t\n"
    // [6]
    "       VFMULAS32.M1    VR63, VR20, VR60, VR20          \t\n"
    "   |   VFMULAS32.M2    VR63, VR21, VR60, VR21          \t\n"
    "   |   VFMULAS32.M3    VR63, VR22, VR60, VR22          \t\n"
    "   |   VLDDW           *+AR1[192], VR25:VR24           \t\n"
    "   |   VLDDW           *+AR1[208], VR27:VR26           \t\n"
    // [7]
    "       VFMULAS32.M1    VR63, VR23, VR60, VR23          \t\n"
    "   |   VFMULAS32.M2    VR63, VR24, VR60, VR24          \t\n"
    "   |   VFMULAS32.M3    VR63, VR25, VR60, VR25          \t\n"
    "   |   VLDDW           *+AR1[224], VR29:VR28           \t\n"
    "   |   VLDDW           *+AR1[240], VR31:VR30           \t\n" 
    // [8]
    "       VFMULAS32.M1    VR63, VR26, VR60, VR26          \t\n"
    "   |   VFMULAS32.M2    VR63, VR27, VR60, VR27          \t\n"
    "   |   VFMULAS32.M3    VR63, VR28, VR60, VR28          \t\n"
    "   |   VSTDW           VR1:VR0, *AR0                   \t\n"
    "   |   VSTDW           VR3:VR2, *+AR0[16]              \t\n"
    // [9]
    "       VFMULAS32.M1    VR63, VR29, VR60, VR29          \t\n"
    "   |   VFMULAS32.M2    VR63, VR30, VR60, VR30          \t\n"
    "   |   VFMULAS32.M3    VR63, VR31, VR60, VR31          \t\n"
    "   |   VSTDW           VR5:VR4, *+AR0[32]              \t\n"
    "   |   VSTDW           VR7:VR6, *+AR0[48]              \t\n"
    "   |   [R0]    SBR     loop_C_plus_beta        \t\n"
    // [10]
    "       VSTDW           VR9:VR8, *+AR0[64]              \t\n"
    "   |   VSTDW           VR11:VR10, *+AR0[80]            \t\n"
    "   |   [!R0]   SBR     R63                     \t\n"
    // [11]
    "       VSTDW           VR13:VR12, *+AR0[96]            \t\n"
    "   |   VSTDW           VR15:VR14, *+AR0[112]           \t\n"
    // [12]
    "       VSTDW           VR17:VR16, *+AR0[128]           \t\n"
    "   |   VSTDW           VR19:VR18, *+AR0[144]           \t\n"
    // [13]
    "       VSTDW           VR21:VR20, *+AR0[160]           \t\n"
    "   |   VSTDW           VR23:VR22, *+AR0[176]           \t\n"
    "   |   SADDA.M1        R50, AR0, AR0                   \t\n"
    // [14]
    "       VSTDW           VR25:VR24, *+AR0[192]           \t\n"
    "   |   VSTDW           VR27:VR26, *+AR0[208]           \t\n"
    // [15]
    "       VSTDW           VR29:VR28, *+AR0[224]           \t\n"
    "   |   VSTDW           VR31:VR30, *+AR0[240]           \t\n"
    "   |   VFMULAS32.M1    VR63, VR0, VR60, VR0            \t\n"
    "   |   VFMULAS32.M2    VR63, VR1, VR60, VR1            \t\n"
// ------------------------------ main loop end ------------------------------
    "       SNOP            1       \t\n"
    :
    :"r"(src_C), "r"(len_C), "r"(beta)
    );
}
#endif

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
        "   |   VSTW            VR26, *+AR2[OR5]            \t\n"
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

__global__ void  transpose_A2B_float32(unlong M, unlong N, unlong b_id, float* A, float* B){
    const int max_core = 4;
    const int grp_size = min(get_group_size(), max_core);
    const int tid = get_thread_id();
    if(tid >= grp_size)
        return;    
    const unlong h_a = 32;
    const unlong w_a = 32;
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


#ifdef use_r6c96
void micro_kernel_asm_r6c96(float* src_a, lvector float* src_b, lvector float* dst_c, 
                        const indexType K_data, const indexType K_buffer, const indexType N_buffer){
    __asm__ __volatile__(
            // [0]
        "       SSHFLR          1, %4, R7                   ;; R7 = K_buffer/2  // A矩阵一行的字数               \t\n"
	    "   |   SMVAGA.M1	    %0, AR10   			        ;; AR10 = src_A                                     \t\n"
	    "   |   SMVAGA.M2	    %2, AR5				        ;; AR5 = dst_C                                      \t\n"
        "   |   VMOVI.M1 	    0x00, VR0				    ;; 初始化C寄存器                                     \t\n"
            // [1]
        "       SSHFLR          2, %5, R42                  ;; B/C_buffer一行多少个双字                          \t\n"
        "   |   SADD.M1         R7, %4, R9                  ;; A矩阵3行的字数: 1.5*K_buffer                      \r\n"
        "   |   SMVAGA.M2	    %1, AR4    			        ;; AR7 = src_B                                      \t\n"
        "   |   VMOV.M1		    VR0, VR1				    ;; 初始化C寄存器                                     \t\n"
	    "   |	VMOV.M2		    VR0, VR2				    ;; 初始化C寄存器                                     \t\n"
	    "   |	VMOV.M3		    VR0, VR3				    ;; 初始化C寄存器                                     \t\n"
            // [2]
        "       SSHFLL          3, R9, R9                   ;; A矩阵3行的字节数, R9=8*(1.5*K_buffer)             \r\n"
	    "   |   SMVAGA.M1       R42, OR2                    ;; B/C_buffer一行多少个双字                          \r\n"
        "   |   SMVAGA.M2       R7, OR8                     ;; OR8 = A矩阵一行的字数: K_buffer/2                 \r\n"
        "   |   VMOV.M1		    VR0, VR4				    ;; 初始化C寄存器                                     \t\n"
	    "   |	VMOV.M2		    VR0, VR5				    ;; 初始化C寄存器                                     \t\n"
	    "   |	VMOV.M3		    VR0, VR6				    ;; 初始化C寄存器                                     \t\n"
            // [3]
        "       SSHFLR          1, %5, R43                  ;; B/C_buffer一行多少个单字                          \t\n"
        "   |   SADD.M1         R9, %0, R9                  ;; R9 = src_A + 8*(1.5*K_buffer)                    \t\n"
        "   |   SMVAGA.M2       %4, OR9                     ;; OR9 = A矩阵两行的字数: K_buffer                   \r\n"
        "   |   SLDW		    *AR10, R51		            ;; load nextA[0][0,1]                               \t\n"
        "   |   VMOV.M1		    VR0, VR7				    ;; 初始化C寄存器                                     \t\n"
	    "   |	VMOV.M2		    VR0, VR8				    ;; 初始化C寄存器                                     \t\n"
	    "   |	VMOV.M3		    VR0, VR9				    ;; 初始化C寄存器                                     \t\n"
            // [4]
        "       SSHFLL		    2, %3, R21				    ;; R21 = K_data * 4 // A矩阵一行的字节数             \t\n"
        "   |   SMVAGA.M1       R43, OR1                    ;; B/C_buffer一行多少个单字                          \r\n"
        "   |   SMVAGA.M2	    R9, AR13				    ;; AR13 = src_A + 8*(1.5*K_buffer)                  \t\n"
        "   |   VLDDW 		    *AR4++[OR2], VR19:VR18		;; load next_B[0][0,1]                              \t\n"
        "   |   VLDW            *+AR4[32], VR20             ;; load next_B[0][2]                                \t\n"
        "   |   SLDW		    *+AR10[OR8], R53		    ;; load nextA[1][0,1]                               \t\n"
        "   |   VMOV.M1		    VR0, VR10				    ;; 初始化C寄存器                                     \t\n"
	    "   |	VMOV.M2		    VR0, VR11				    ;; 初始化C寄存器                                     \t\n"
	    "   |	VMOV.M3		    VR0, VR12				    ;; 初始化C寄存器                                     \t\n"
            // [5]
        "       SADD.M1		    R21, %0, R21			    ;; R21 = src_A + K_data * 4 // A矩阵的首行末地址     \t\n"
        "   |   SADDA.M2        8, AR10, AR10               ;; AR10 += 8 Bytes                                  \r\n"
        "   |   SLDW		    *+AR10[OR9], R55		    ;; load nextA[2][0,1]                               \t\n"
        "   |   VMOV.M1		    VR0, VR13				    ;; 初始化C寄存器                                     \t\n"
	    "   |	VMOV.M2		    VR0, VR14				    ;; 初始化C寄存器                                     \t\n"
	    "   |	VMOV.M3		    VR0, VR15				    ;; 初始化C寄存器                                     \t\n"
            // [6]
        "       VLDW            *AR4++[OR1], VR21           ;; load next_B[1][0]                                \t\n"
        "   |   VLDDW 		    *+AR4[8], VR23:VR22	        ;; load next_B[1][1,2]                              \t\n" 
        "   |   SLDW		    *AR13, R57		            ;; load nextA[3][0,1]                               \t\n"
        "   |   VMOV.M1		    VR0, VR16				    ;; 初始化C寄存器                                     \t\n"
	    "   |	VMOV.M2		    VR0, VR17				    ;; 初始化C寄存器                                     \t\n"
        "   |   VMOVI.M3 	    0x3F8000003F800000,VR63	    ;; VR63 = 1.0                                       \t\n" 
            // [7]
        "       SLDW		    *+AR13[OR8], R59             ;; load nextA[4][0,1]                              \t\n"
            // [8]
        "       SADDA.M1        8, AR13, AR13               ;; AR13 += 8 Bytes                                  \r\n"
        "   |   SLDW		    *+AR13[OR9], R61             ;; load nextA[5][0,1]                              \t\n"
            // [9]
        "       VLDDW 		    *AR5++[OR2], VR37:VR36		;; load mat_C[0][0,1]                               \t\n"
        "   |   VLDW            *+AR5[32], VR38             ;; load mat_C[0][2]                                 \t\n"
            // [10]
        "       SVBCAST.M1 	    R51, VR25                   ;; broadcast next_A[0][0,1]                         \t\n"
        "   |   VLDW            *AR5++[OR1], VR39           ;; load mat_C[1][0]                                 \t\n"
        "   |   VLDDW 		    *+AR5[8], VR41:VR40		    ;; load mat_C[1][1,2]                               \t\n"    
            // [11]    
        "       SVBCAST.M1      R53, VR27                   ;; broadcast next_A[1][0,1]                         \t\n"
        "   |   VLDDW 		    *AR5++[OR2], VR43:VR42		;; load mat_C[2][0,1]                               \t\n"
        "   |   VLDW            *+AR5[32], VR44             ;; load mat_C[2][2]                                 \t\n"
            // [12]
        "       SVBCAST.M1      R55, VR29                   ;; broadcast next_A[2][0,1]                         \t\n"
        "   |   VLDW            *AR5++[OR1], VR45           ;; load mat_C[3][0]                                 \t\n"
        "   |   VLDDW 		    *+AR5[8], VR47:VR46		    ;; load mat_C[3][1,2]                               \t\n"
            // [13]
        "       SVBCAST.M1      R57, VR31                   ;; broadcast next_A[3][0,1]                         \t\n"
        "   |   VLDDW 		    *AR5++[OR2], VR49:VR48		;; load mat_C[4][0,1]                               \t\n"
        "   |   VLDW            *+AR5[32], VR50             ;; load mat_C[4][2]                                 \t\n"
            // [14]
        "       SVBCAST.M1      R59, VR33                   ;; broadcast next_A[4][0,1]                         \t\n"
        "   |   VBALE2          VR25, VR25, VR24                                                                \t\n"
        "   |   VLDW            *AR5++[OR1], VR51           ;; load mat_C[5][0]                                 \t\n"
        "   |   VLDDW 		    *+AR5[8], VR53:VR52		    ;; load mat_C[5][1,2]                               \t\n"

        "loop_k_r6c96:       \t\n"
        // -------------------------------------- MAIN LOOP --------------------------------------
            // [0]
	    "       VFMULAS32.M1	VR18, VR24, VR0, VR0		;; VR0 += src_B[0][0] * src_A[0][0]                 \t\n"
	    "   |	VFMULAS32.M2	VR19, VR24, VR1, VR1		;; VR1 += src_B[0][1] * src_A[0][0]                 \t\n"
	    "   |	VFMULAS32.M3	VR20, VR24, VR2, VR2		;; VR2 += src_B[0][2] * src_A[0][0]                 \t\n"
        "   |   SLDW            *AR10, R51  		        ;; load nextA[0][0,1]                               \t\n"
        "   |   SVBCAST.M2      R61, VR35                   ;; broadcast next_A[5][0,1]                         \t\n"
        "   |   VBALE2          VR27, VR27, VR26                                                                \t\n"
            // [1]
        "       SLT			    %0, R21, R0                 ;; if R10 < src_A + K_data * 4                      \t\n"
        "   |   VFMULAS32.M1	VR18, VR26, VR3, VR3		;; VR3 += src_B[0][0] * src_A[1][0]                 \t\n"
	    "   |	VFMULAS32.M2	VR19, VR26, VR4, VR4		;; VR4 += src_B[0][1] * src_A[1][0]                 \t\n"
	    "   |	VFMULAS32.M3	VR20, VR26, VR5, VR5		;; VR5 += src_B[0][2] * src_A[1][0]                 \t\n"
        "   |   SLDW            *+AR10[OR8], R53		    ;; load nextA[1][0,1]                               \t\n"
        "   |   VBALE2          VR29, VR29, VR28                                                                \t\n"
            // [2]
        "       SADDA.M1        8, AR10, AR10               ;; AR10 += 8 Bytes                                  \r\n"
        "   |   VFMULAS32.M1	VR18, VR28, VR6, VR6		;; VR6 += src_B[0][0] * src_A[1][0]                 \t\n"
	    "   |	VFMULAS32.M2	VR19, VR28, VR7, VR7		;; VR7 += src_B[0][1] * src_A[1][0]                 \t\n"
	    "   |	VFMULAS32.M3	VR20, VR28, VR8, VR8		;; VR8 += src_B[0][2] * src_A[1][0]                 \t\n"
        "   |   SLDW            *+AR10[OR9], R55		    ;; load nextA[2][0,1]                               \t\n"
        "   |   VBALE2          VR31, VR31, VR30                                                                \t\n"
            // [3]
        "       VFMULAS32.M1	VR18, VR30, VR9, VR9		;; VR9 += src_B[0][0] * src_A[1][0]                 \t\n"
	    "   |	VFMULAS32.M2	VR19, VR30, VR10, VR10		;; VR10 += src_B[0][1] * src_A[1][0]                \t\n"
	    "   |	VFMULAS32.M3	VR20, VR30, VR11, VR11		;; VR11 += src_B[0][2] * src_A[1][0]                \t\n"
        "   |   VLDDW 		    *AR4++[OR2], VR19:VR18		;; load next_B[0][0,1]                              \t\n"
        "   |   VLDW            *+AR4[32], VR20             ;; load next_B[0][2]                                \t\n"
        "   |   SLDW            *AR13, R57                  ;; load nextA[3][0,1]                               \t\n"
        "   |   VBALE2          VR33, VR33, VR32                                                                \t\n"
            // [4]
        "   	SMVAAG.M1	    AR10, %0		            ;; R10 = src_A + 1                                  \t\n"
        "   |   VFMULAS32.M1	VR18, VR32, VR12, VR12		;; VR12 += src_B[0][0] * src_A[1][0]                \t\n"
	    "   |	VFMULAS32.M2	VR19, VR32, VR13, VR13		;; VR13 += src_B[0][1] * src_A[1][0]                \t\n"
	    "   |	VFMULAS32.M3	VR20, VR32, VR14, VR14		;; VR14 += src_B[0][2] * src_A[1][0]                \t\n"
        "   |   SLDW            *+AR13[OR8], R59            ;; load nextA[4][0,1]                               \t\n"
        "   |   VBALE2          VR35, VR35, VR34                                                                \t\n"
            // [5]
        "       SADDA.M1        8, AR13, AR13               ;; AR13 += 8 Bytes                                  \r\n"
        "   |   VFMULAS32.M1	VR18, VR34, VR15, VR15		;; VR15 += src_B[0][0] * src_A[1][0]                \t\n"
	    "   |	VFMULAS32.M2	VR19, VR34, VR16, VR16		;; VR16 += src_B[0][1] * src_A[1][0]                \t\n"
	    "   |	VFMULAS32.M3	VR20, VR34, VR17, VR17		;; VR17 += src_B[0][2] * src_A[1][0]                \t\n"
        "   |   SLDW            *+AR13[OR9], R61            ;; load nextA[5][0,1]                               \t\n"
        "   |   VBALE2H         VR25, VR25, VR25                                                                \t\n"
        "   |   [R0]	    SBR		loop_k_r6c96            ;; condjump to .L26 occurs                          \t\n"
            // [6]
        "       VFMULAS32.M1	VR21, VR25, VR0, VR0		;; VR0 += src_B[1][0] * src_A[0][1]                 \t\n"
	    "   |	VFMULAS32.M2	VR22, VR25, VR1, VR1		;; VR1 += src_B[1][1] * src_A[0][1]                 \t\n"
	    "   |	VFMULAS32.M3	VR23, VR25, VR2, VR2		;; VR2 += src_B[1][2] * src_A[0][1]                 \t\n"
        "   |   VBALE2H         VR27, VR27, VR27                                                                \t\n"
            // [7]
        "       VFMULAS32.M1	VR21, VR27, VR3, VR3		;; VR3 += src_B[1][0] * src_A[1][1]                 \t\n"
        "   |	VFMULAS32.M2	VR22, VR27, VR4, VR4		;; VR4 += src_B[1][1] * src_A[1][1]                 \t\n"
        "   |	VFMULAS32.M3	VR23, VR27, VR5, VR5		;; VR5 += src_B[1][2] * src_A[1][1]                 \t\n"
        "   |   SVBCAST.M1 	    R51, VR25                   ;; broadcast next_A[0][0,1]                         \t\n"
        "   |   VBALE2H         VR29, VR29, VR29                                                                \t\n"
            // [8]
        "       VFMULAS32.M1	VR21, VR29, VR6, VR6		;; VR2 += src_B[1][0] * src_A[1][1]                 \t\n"
        "   |   VFMULAS32.M2	VR22, VR29, VR7, VR7		;; VR7 += src_B[1][1] * src_A[1][1]                 \t\n"
        "   |   VFMULAS32.M3	VR23, VR29, VR8, VR8		;; VR8 += src_B[1][2] * src_A[1][1]                 \t\n"
        "   |   SVBCAST.M2      R53, VR27                   ;; broadcast next_A[1][0,1]                         \t\n"
        "   |   VBALE2H         VR31, VR31, VR31                                                                \t\n"
            // [9]
        "       VFMULAS32.M1	VR21, VR31, VR9, VR9		;; VR9 += src_B[1][0] * src_A[1][1]                 \t\n"
        "   |	VFMULAS32.M2	VR22, VR31, VR10, VR10		;; VR10 += src_B[1][1] * src_A[1][1]                \t\n"
        "   |	VFMULAS32.M3	VR23, VR31, VR11, VR11		;; VR11 += src_B[1][2] * src_A[1][1]                \t\n"
        "   |   VLDW            *AR4++[OR1], VR21           ;; load next_B[1][0]                                \t\n"
        "   |   VLDDW           *+AR4[8], VR23:VR22	        ;; load next_B[1][1,2]                              \t\n"
        "   |   SVBCAST.M2      R55, VR29                   ;; broadcast next_A[2][0,1]                         \t\n"
        "   |   VBALE2H         VR33, VR33, VR33                                                                \t\n"
            // [10]
        "       VFMULAS32.M1	VR21, VR33, VR12, VR12		;; VR12 += src_B[1][0] * src_A[1][1]                \t\n"
	    "   |	VFMULAS32.M2	VR22, VR33, VR13, VR13		;; VR13 += src_B[1][1] * src_A[1][1]                \t\n"
	    "   |	VFMULAS32.M3	VR23, VR33, VR14, VR14		;; VR14 += src_B[1][2] * src_A[1][1]                \t\n"
        "   |   SVBCAST.M2      R57, VR31                   ;; broadcast next_A[3][0,1]                         \t\n"
        "   |   VBALE2H         VR35, VR35, VR35                                                                \t\n"
            // [11]
        "       VFMULAS32.M1	VR21, VR35, VR15, VR15		;; VR15 += src_B[1][0] * src_A[1][1]                \t\n"
	    "   |	VFMULAS32.M2	VR22, VR35, VR16, VR16		;; VR16 += src_B[1][1] * src_A[1][1]                \t\n"
	    "   |	VFMULAS32.M3	VR23, VR35, VR17, VR17		;; VR17 += src_B[1][2] * src_A[1][1]                \t\n"
        "   |   SVBCAST.M2      R59, VR33                   ;; broadcast next_A[4][0,1]                         \t\n"
        "   |   VBALE2          VR25, VR25, VR24                                                                \t\n"
        // -------------------------------------- write back --------------------------------------
	        // [0]
        "       SMVAGA.M2	    %2, AR6				        ;; AR6 = dst_C      \t\n"
	    "   |   VFMULAS32.M1	VR0, VR63, VR36, VR36                           \t\n"
	    "   |	VFMULAS32.M2	VR1, VR63, VR37, VR37                           \t\n"
	    "   |	VFMULAS32.M3	VR2, VR63, VR38, VR38                           \t\n"
            // [1]
	    "   	VFMULAS32.M1	VR3, VR63, VR39, VR39                           \t\n"
	    "   |	VFMULAS32.M2	VR4, VR63, VR40, VR40                           \t\n"
	    "   |	VFMULAS32.M3	VR5, VR63, VR41, VR41                           \t\n"
	        // [2]
	    "   	VFMULAS32.M1	VR6, VR63, VR42, VR42                           \t\n"
	    "   |	VFMULAS32.M2	VR7, VR63, VR43, VR43                           \t\n"
	    "   |	VFMULAS32.M3	VR8, VR63, VR44, VR44                           \t\n"
	        // [3]
	    "   	VFMULAS32.M1	VR9, VR63, VR45, VR45                           \t\n"
	    "   |	VFMULAS32.M2	VR10, VR63, VR46, VR46                          \t\n"
	    "   |	VFMULAS32.M3	VR11, VR63, VR47, VR47                          \t\n"
	        // [4]
	    "   	VFMULAS32.M1	VR12, VR63, VR48, VR48                          \t\n"
	    "   |	VFMULAS32.M2	VR13, VR63, VR49, VR49                          \t\n"
	    "   |	VFMULAS32.M3	VR14, VR63, VR50, VR50                          \t\n"
	        // [5]
	    "   	VFMULAS32.M1	VR15, VR63, VR51, VR51                          \t\n"
	    "   |	VFMULAS32.M2	VR16, VR63, VR52, VR52                          \t\n"
	    "   |	VFMULAS32.M3	VR17, VR63, VR53, VR53                          \t\n"
	    "   |	SBR		        R63                                             \t\n"
            // [6]
        "       VSTDW           VR37:VR36, *AR6++[OR2]                          \r\n"
        "   |   VSTW            VR38, *+AR6[32]                                 \r\n"
            // [7]                                             
        "       VSTW            VR39, *AR6++[OR1]                               \r\n"
        "   |   VSTDW           VR41:VR40, *+AR6[8]                              \r\n"
            // [8]                                         
        "       VSTDW           VR43:VR42, *AR6++[OR2]                          \r\n"
        "   |   VSTW            VR44, *+AR6[32]                                 \r\n"
            // [9]  
        "       VSTW            VR45, *AR6++[OR1]                               \r\n"
        "   |   VSTDW           VR47:VR46, *+AR6[8]                             \r\n"
            // [10] 
        "       VSTDW           VR49:VR48, *AR6++[OR2]                          \r\n"
        "   |   VSTW            VR50, *+AR6[32]                                 \r\n"
            // [11] 
        "       VSTW            VR51, *AR6++[OR1]                               \r\n"
        "   |   VSTDW           VR53:VR52, *+AR6[8]                             \r\n"
    :
    :"r"(src_a), "r"(src_b), "r"(dst_c), "r"(K_data), "r"(K_buffer), "r"(N_buffer)
    );
}

void micro_kernel_asm_r6c96_only_write(float* src_a, lvector float* src_b, lvector float* dst_c, 
                        const indexType K_data, const indexType K_buffer, const indexType N_buffer){
    __asm__ __volatile__(
            // [0]
        "       SSHFLR          1, %4, R7                   ;; R7 = K_buffer/2  // A矩阵一行的字数               \t\n"
	    "   |   SMVAGA.M1	    %0, AR10   			        ;; AR10 = src_A                                     \t\n"
	    "   |   SMVAGA.M2	    %2, AR6				        ;; AR6 = dst_C                                      \t\n"
        "   |   VMOVI.M1 	    0x00, VR0				    ;; 初始化C寄存器                                     \t\n"
            // [1]
        "       SSHFLR          2, %5, R42                  ;; B/C_buffer一行多少个双字                          \t\n"
        "   |   SADD.M1         R7, %4, R9                  ;; A矩阵3行的字数: 1.5*K_buffer                      \r\n"
        "   |   SMVAGA.M2	    %1, AR4    			        ;; AR7 = src_B                                      \t\n"
        "   |   VMOV.M1		    VR0, VR1				    ;; 初始化C寄存器                                     \t\n"
	    "   |	VMOV.M2		    VR0, VR2				    ;; 初始化C寄存器                                     \t\n"
	    "   |	VMOV.M3		    VR0, VR3				    ;; 初始化C寄存器                                     \t\n"
            // [2]
        "       SSHFLL          3, R9, R9                   ;; A矩阵3行的字节数, R9=8*(1.5*K_buffer)             \r\n"
	    "   |   SMVAGA.M1       R42, OR2                    ;; B/C_buffer一行多少个双字                          \r\n"
        "   |   SMVAGA.M2       R7, OR8                     ;; OR8 = A矩阵一行的字数: K_buffer/2                 \r\n"
        "   |   VMOV.M1		    VR0, VR4				    ;; 初始化C寄存器                                     \t\n"
	    "   |	VMOV.M2		    VR0, VR5				    ;; 初始化C寄存器                                     \t\n"
	    "   |	VMOV.M3		    VR0, VR6				    ;; 初始化C寄存器                                     \t\n"
            // [3]
        "       SSHFLR          1, %5, R43                  ;; B/C_buffer一行多少个单字                          \t\n"
        "   |   SADD.M1         R9, %0, R9                  ;; R9 = src_A + 8*(1.5*K_buffer)                    \t\n"
        "   |   SMVAGA.M2       %4, OR9                     ;; OR9 = A矩阵两行的字数: K_buffer                   \r\n"
        "   |   SLDW		    *AR10, R51		            ;; load nextA[0][0,1]                               \t\n"
        "   |   VMOV.M1		    VR0, VR7				    ;; 初始化C寄存器                                     \t\n"
	    "   |	VMOV.M2		    VR0, VR8				    ;; 初始化C寄存器                                     \t\n"
	    "   |	VMOV.M3		    VR0, VR9				    ;; 初始化C寄存器                                     \t\n"
            // [4]
        "       SSHFLL		    2, %3, R21				    ;; R21 = K_data * 4 // A矩阵一行的字节数             \t\n"
        "   |   SMVAGA.M1       R43, OR1                    ;; B/C_buffer一行多少个单字                          \r\n"
        "   |   SMVAGA.M2	    R9, AR13				    ;; AR13 = src_A + 8*(1.5*K_buffer)                  \t\n"
        "   |   VLDDW 		    *AR4++[OR2], VR19:VR18		;; load next_B[0][0,1]                              \t\n"
        "   |   VLDW            *+AR4[32], VR20             ;; load next_B[0][2]                                \t\n"
        "   |   SLDW		    *+AR10[OR8], R53		    ;; load nextA[1][0,1]                               \t\n"
        "   |   VMOV.M1		    VR0, VR10				    ;; 初始化C寄存器                                     \t\n"
	    "   |	VMOV.M2		    VR0, VR11				    ;; 初始化C寄存器                                     \t\n"
	    "   |	VMOV.M3		    VR0, VR12				    ;; 初始化C寄存器                                     \t\n"
            // [5]
        "       SADD.M1		    R21, %0, R21			    ;; R21 = src_A + K_data * 4 // A矩阵的首行末地址     \t\n"
        "   |   SADDA.M2        8, AR10, AR10               ;; AR10 += 8 Bytes                                  \r\n"
        "   |   SLDW		    *+AR10[OR9], R55		    ;; load nextA[2][0,1]                               \t\n"
        "   |   VMOV.M1		    VR0, VR13				    ;; 初始化C寄存器                                     \t\n"
	    "   |	VMOV.M2		    VR0, VR14				    ;; 初始化C寄存器                                     \t\n"
	    "   |	VMOV.M3		    VR0, VR15				    ;; 初始化C寄存器                                     \t\n"
            // [6]
        "       VLDW            *AR4++[OR1], VR21           ;; load next_B[1][0]                                \t\n"
        "   |   VLDDW 		    *+AR4[8], VR23:VR22	        ;; load next_B[1][1,2]                              \t\n" 
        "   |   SLDW		    *AR13, R57		            ;; load nextA[3][0,1]                               \t\n"
        "   |   VMOV.M1		    VR0, VR16				    ;; 初始化C寄存器                                     \t\n"
	    "   |	VMOV.M2		    VR0, VR17				    ;; 初始化C寄存器                                     \t\n"
        "   |   VMOVI.M3 	    0x3F8000003F800000,VR63	    ;; VR63 = 1.0                                       \t\n" 
            // [7]
        "       SLDW		    *+AR13[OR8], R59             ;; load nextA[4][0,1]                              \t\n"
            // [8]
        "       SADDA.M1        8, AR13, AR13               ;; AR13 += 8 Bytes                                  \r\n"
        "   |   SLDW		    *+AR13[OR9], R61             ;; load nextA[5][0,1]                              \t\n"
            // [9]
        "       SNOP            1       \t\n"
            // [10]
        "       SVBCAST.M1 	    R51, VR25                   ;; broadcast next_A[0][0,1]                         \t\n"   
            // [11]    
        "       SVBCAST.M1      R53, VR27                   ;; broadcast next_A[1][0,1]                         \t\n"
            // [12]
        "       SVBCAST.M1      R55, VR29                   ;; broadcast next_A[2][0,1]                         \t\n"
            // [13]
        "       SVBCAST.M1      R57, VR31                   ;; broadcast next_A[3][0,1]                         \t\n"
            // [14]
        "       SVBCAST.M1      R59, VR33                   ;; broadcast next_A[4][0,1]                         \t\n"
        "   |   VBALE2          VR25, VR25, VR24                                                                \t\n"

        "loop_k_r6c96_only_write:       \t\n"
        // -------------------------------------- MAIN LOOP --------------------------------------
            // [0]
	    "       VFMULAS32.M1	VR18, VR24, VR0, VR0		;; VR0 += src_B[0][0] * src_A[0][0]                 \t\n"
	    "   |	VFMULAS32.M2	VR19, VR24, VR1, VR1		;; VR1 += src_B[0][1] * src_A[0][0]                 \t\n"
	    "   |	VFMULAS32.M3	VR20, VR24, VR2, VR2		;; VR2 += src_B[0][2] * src_A[0][0]                 \t\n"
        "   |   SLDW            *AR10, R51  		        ;; load nextA[0][0,1]                               \t\n"
        "   |   SVBCAST.M2      R61, VR35                   ;; broadcast next_A[5][0,1]                         \t\n"
        "   |   VBALE2          VR27, VR27, VR26                                                                \t\n"
            // [1]
        "       SLT			    %0, R21, R0                 ;; if R10 < src_A + K_data * 4                      \t\n"
        "   |   VFMULAS32.M1	VR18, VR26, VR3, VR3		;; VR3 += src_B[0][0] * src_A[1][0]                 \t\n"
	    "   |	VFMULAS32.M2	VR19, VR26, VR4, VR4		;; VR4 += src_B[0][1] * src_A[1][0]                 \t\n"
	    "   |	VFMULAS32.M3	VR20, VR26, VR5, VR5		;; VR5 += src_B[0][2] * src_A[1][0]                 \t\n"
        "   |   SLDW            *+AR10[OR8], R53		    ;; load nextA[1][0,1]                               \t\n"
        "   |   VBALE2          VR29, VR29, VR28                                                                \t\n"
            // [2]
        "       SADDA.M1        8, AR10, AR10               ;; AR10 += 8 Bytes                                  \r\n"
        "   |   VFMULAS32.M1	VR18, VR28, VR6, VR6		;; VR6 += src_B[0][0] * src_A[1][0]                 \t\n"
	    "   |	VFMULAS32.M2	VR19, VR28, VR7, VR7		;; VR7 += src_B[0][1] * src_A[1][0]                 \t\n"
	    "   |	VFMULAS32.M3	VR20, VR28, VR8, VR8		;; VR8 += src_B[0][2] * src_A[1][0]                 \t\n"
        "   |   SLDW            *+AR10[OR9], R55		    ;; load nextA[2][0,1]                               \t\n"
        "   |   VBALE2          VR31, VR31, VR30                                                                \t\n"
            // [3]
        "       VFMULAS32.M1	VR18, VR30, VR9, VR9		;; VR9 += src_B[0][0] * src_A[1][0]                 \t\n"
	    "   |	VFMULAS32.M2	VR19, VR30, VR10, VR10		;; VR10 += src_B[0][1] * src_A[1][0]                \t\n"
	    "   |	VFMULAS32.M3	VR20, VR30, VR11, VR11		;; VR11 += src_B[0][2] * src_A[1][0]                \t\n"
        "   |   VLDDW 		    *AR4++[OR2], VR19:VR18		;; load next_B[0][0,1]                              \t\n"
        "   |   VLDW            *+AR4[32], VR20             ;; load next_B[0][2]                                \t\n"
        "   |   SLDW            *AR13, R57                  ;; load nextA[3][0,1]                               \t\n"
        "   |   VBALE2          VR33, VR33, VR32                                                                \t\n"
            // [4]
        "   	SMVAAG.M1	    AR10, %0		            ;; R10 = src_A + 1                                  \t\n"
        "   |   VFMULAS32.M1	VR18, VR32, VR12, VR12		;; VR12 += src_B[0][0] * src_A[1][0]                \t\n"
	    "   |	VFMULAS32.M2	VR19, VR32, VR13, VR13		;; VR13 += src_B[0][1] * src_A[1][0]                \t\n"
	    "   |	VFMULAS32.M3	VR20, VR32, VR14, VR14		;; VR14 += src_B[0][2] * src_A[1][0]                \t\n"
        "   |   SLDW            *+AR13[OR8], R59            ;; load nextA[4][0,1]                               \t\n"
        "   |   VBALE2          VR35, VR35, VR34                                                                \t\n"
            // [5]
        "       SADDA.M1        8, AR13, AR13               ;; AR13 += 8 Bytes                                  \r\n"
        "   |   VFMULAS32.M1	VR18, VR34, VR15, VR15		;; VR15 += src_B[0][0] * src_A[1][0]                \t\n"
	    "   |	VFMULAS32.M2	VR19, VR34, VR16, VR16		;; VR16 += src_B[0][1] * src_A[1][0]                \t\n"
	    "   |	VFMULAS32.M3	VR20, VR34, VR17, VR17		;; VR17 += src_B[0][2] * src_A[1][0]                \t\n"
        "   |   SLDW            *+AR13[OR9], R61            ;; load nextA[5][0,1]                               \t\n"
        "   |   VBALE2H         VR25, VR25, VR25                                                                \t\n"
        "   |   [R0]	    SBR		loop_k_r6c96_only_write            \t\n"
            // [6]
        "       VFMULAS32.M1	VR21, VR25, VR0, VR0		;; VR0 += src_B[1][0] * src_A[0][1]                 \t\n"
	    "   |	VFMULAS32.M2	VR22, VR25, VR1, VR1		;; VR1 += src_B[1][1] * src_A[0][1]                 \t\n"
	    "   |	VFMULAS32.M3	VR23, VR25, VR2, VR2		;; VR2 += src_B[1][2] * src_A[0][1]                 \t\n"
        "   |   VBALE2H         VR27, VR27, VR27                                                                \t\n"
            // [7]
        "       VFMULAS32.M1	VR21, VR27, VR3, VR3		;; VR3 += src_B[1][0] * src_A[1][1]                 \t\n"
        "   |	VFMULAS32.M2	VR22, VR27, VR4, VR4		;; VR4 += src_B[1][1] * src_A[1][1]                 \t\n"
        "   |	VFMULAS32.M3	VR23, VR27, VR5, VR5		;; VR5 += src_B[1][2] * src_A[1][1]                 \t\n"
        "   |   SVBCAST.M1 	    R51, VR25                   ;; broadcast next_A[0][0,1]                         \t\n"
        "   |   VBALE2H         VR29, VR29, VR29                                                                \t\n"
            // [8]
        "       VFMULAS32.M1	VR21, VR29, VR6, VR6		;; VR2 += src_B[1][0] * src_A[1][1]                 \t\n"
        "   |   VFMULAS32.M2	VR22, VR29, VR7, VR7		;; VR7 += src_B[1][1] * src_A[1][1]                 \t\n"
        "   |   VFMULAS32.M3	VR23, VR29, VR8, VR8		;; VR8 += src_B[1][2] * src_A[1][1]                 \t\n"
        "   |   SVBCAST.M2      R53, VR27                   ;; broadcast next_A[1][0,1]                         \t\n"
        "   |   VBALE2H         VR31, VR31, VR31                                                                \t\n"
            // [9]
        "       VFMULAS32.M1	VR21, VR31, VR9, VR9		;; VR9 += src_B[1][0] * src_A[1][1]                 \t\n"
        "   |	VFMULAS32.M2	VR22, VR31, VR10, VR10		;; VR10 += src_B[1][1] * src_A[1][1]                \t\n"
        "   |	VFMULAS32.M3	VR23, VR31, VR11, VR11		;; VR11 += src_B[1][2] * src_A[1][1]                \t\n"
        "   |   VLDW            *AR4++[OR1], VR21           ;; load next_B[1][0]                                \t\n"
        "   |   VLDDW           *+AR4[8], VR23:VR22	        ;; load next_B[1][1,2]                              \t\n"
        "   |   SVBCAST.M2      R55, VR29                   ;; broadcast next_A[2][0,1]                         \t\n"
        "   |   VBALE2H         VR33, VR33, VR33                                                                \t\n"
            // [10]
        "       VFMULAS32.M1	VR21, VR33, VR12, VR12		;; VR12 += src_B[1][0] * src_A[1][1]                \t\n"
	    "   |	VFMULAS32.M2	VR22, VR33, VR13, VR13		;; VR13 += src_B[1][1] * src_A[1][1]                \t\n"
	    "   |	VFMULAS32.M3	VR23, VR33, VR14, VR14		;; VR14 += src_B[1][2] * src_A[1][1]                \t\n"
        "   |   SVBCAST.M2      R57, VR31                   ;; broadcast next_A[3][0,1]                         \t\n"
        "   |   VBALE2H         VR35, VR35, VR35                                                                \t\n"
            // [11]
        "       VFMULAS32.M1	VR21, VR35, VR15, VR15		;; VR15 += src_B[1][0] * src_A[1][1]                \t\n"
	    "   |	VFMULAS32.M2	VR22, VR35, VR16, VR16		;; VR16 += src_B[1][1] * src_A[1][1]                \t\n"
	    "   |	VFMULAS32.M3	VR23, VR35, VR17, VR17		;; VR17 += src_B[1][2] * src_A[1][1]                \t\n"
        "   |   SVBCAST.M2      R59, VR33                   ;; broadcast next_A[4][0,1]                         \t\n"
        "   |   VBALE2          VR25, VR25, VR24                                                                \t\n"
        "   |	[!R0]    SBR     R63                         \t\n"
        // -------------------------------------- write back --------------------------------------
            // [0]
        "       VSTDW           VR1:VR0, *AR6++[OR2]        \r\n"
        "   |   VSTW            VR2, *+AR6[32]              \r\n"
            // [1]                                             
        "       VSTW            VR3, *AR6++[OR1]            \r\n"
        "   |   VSTDW           VR5:VR4, *+AR6[8]           \r\n"
            // [2]                                         
        "       VSTDW           VR7:VR6, *AR6++[OR2]        \r\n"
        "   |   VSTW            VR8, *+AR6[32]              \r\n"
            // [3]  
        "       VSTW            VR9, *AR6++[OR1]            \r\n"
        "   |   VSTDW           VR11:VR10, *+AR6[8]         \r\n"
            // [4] 
        "       VSTDW           VR13:VR12, *AR6++[OR2]      \r\n"
        "   |   VSTW            VR14, *+AR6[32]             \r\n"
            // [5] 
        "       VSTW            VR15, *AR6++[OR1]           \r\n"
        "   |   VSTDW           VR17:VR16, *+AR6[8]         \r\n"
    :
    :"r"(src_a), "r"(src_b), "r"(dst_c), "r"(K_data), "r"(K_buffer), "r"(N_buffer)
    );
}
#endif

#ifdef use_r6c128
void micro_kernel_asm_r6c128(float* src_a, lvector float* src_b, lvector float* dst_c, 
                        const indexType K_data, const indexType K_buffer, const indexType N_buffer){
    __asm__ __volatile__(
            // [-2]
        "       SSHFLR          1, %4, R7                   ;; R7 = K_buffer/2  // A矩阵一行的字数   \t\n"
	    "   |   SMVAGA.M1	    %0, AR10   			        ;; AR10 = src_A                         \t\n"
	    "   |   SMVAGA.M2       %4, OR9                     ;; OR9 = A矩阵两行的字数: K_buffer       \t\n"
            // [-1]
        "       SSHFLR          2, %5, R42                  ;; B/C_buffer一行多少个双字              \t\n"
        "   |   SADD.M1         R7, %4, R9                  ;; A矩阵3行的字数: 1.5*K_buffer          \t\n"
        "   |   SMVAGA.M2       R7, OR8                     ;; OR8 = A矩阵一行的字数: K_buffer/2     \t\n"
            // [4]
        "       SSHFLL          3, R9, R9                   ;; A矩阵3行的字节数, R9=8*(1.5*K_buffer) \t\n"
	    "   |   SMVAGA.M1       R42, OR2                    ;; B/C_buffer一行多少个双字              \t\n"
        "   |   SMVAGA.M2	    %1, AR4    			        ;; AR7 = src_B                          \t\n"
        "   |   SLDW            *AR10, R51  	            ;; load nextA[0]                        \t\n"
            // [5]
        "       SSHFLL		    2, %3, R21				    ;; R20 = K_data * 4 // A矩阵一行的字节数 \t\n"
        "   |   SADD.M1         R9, %0, R9                  ;; R9 = src_A + 8*(1.5*K_buffer)        \t\n"
        "   |   SADDA.M2        8, AR10, AR10                                                       \t\n"
        "   |   SLDW            *+AR10[OR8], R53            ;; load nextA[1]                        \t\n"
            // [6]
        "       SMVAGA.M1	    R9, AR13				    ;; AR13 = src_A + 8*(1.5*K_buffer)      \t\n"
        "   |   SADD.M2		    R21, %0, R21			    ;; R21 = src_A + K_data * 4             \t\n"
        "   |   SLDW            *+AR10[OR9], R55            ;; load nextA[2]                        \t\n"
            // [7]
        "       SSUB.M1		    8, R21, R21			        ;; R21 = src_A+(K_data-2)*4             \t\n"
        "   |   SMVAAG.M2	    AR10, %0		                                                    \t\n"
        "   |   VLDDW 		    *AR4++[OR2], VR25:VR24		                                        \t\n"
        "   |   VLDDW 		    *+AR4[16], VR27:VR26		                                        \t\n"
        "   |   VMOVI.M1 	    0x00, VR0				    ;; 初始化C寄存器                         \t\n"
        "   |   VMOVI.M2 	    0x3F8000003F800000,VR63	    ;; VR63 = 1.0                           \t\n"
            // [8]
        "       SLDW            *AR13, R57                  ;; load nextA[3]                        \t\n"
        "   |   SMVAGA.M1	    %2, AR5				        ;; AR5 = dst_C                          \t\n"
        "   |   SMVAGA.M2	    %2, AR6				        ;; AR6 = dst_C                          \t\n"
        "   |   VMOV.M2		    VR0, VR1    \t\n"
        "   |   VMOV.M3		    VR0, VR2    \t\n"
            // [9]
        "       SADDA.M1        8, AR13, AR13                                                       \t\n"
        "   |   SLDW            *+AR13[OR8], R59    ;; load nextA[4]                                \t\n"
        "   |   VMOV.M1		    VR0, VR3    \t\n"
        "   |   VMOV.M2		    VR0, VR4    \t\n"
        "   |   VMOV.M3		    VR0, VR5    \t\n"
            // [10]
        "       SLDW            *+AR13[OR9], R61    ;; load nextA[5]                                \t\n"
        "   |   VMOV.M1		    VR0, VR6    \t\n"
        "   |   VMOV.M2		    VR0, VR7    \t\n"
        "   |   VMOV.M3		    VR0, VR8    \t\n"
            // [11]
        "       SVBCAST.M2      R51, VR33   ;; bcast nextA[0]                                       \t\n"
        "   |   VLDDW 		    *AR5++[OR2], VR45:VR44		;; load mat_C[0][0,1]   \t\n"
        "   |   VLDDW           *+AR5[16], VR47:VR46        ;; load mat_C[0][2,3]   \t\n"
        "   |   VMOV.M1		    VR0, VR9    \t\n"
        "   |   VMOV.M2		    VR0, VR10   \t\n"
        "   |   VMOV.M3		    VR0, VR11   \t\n"
            // [12]
        "       SVBCAST.M2      R53, VR35   ;; bcast nextA[1]                                       \t\n"
        "   |   VLDDW 		    *AR5++[OR2], VR49:VR48		;; load mat_C[1][0,1]   \t\n"
        "   |   VLDDW           *+AR5[16], VR51:VR50        ;; load mat_C[1][2,3]   \t\n"
        "   |   VMOV.M1		    VR0, VR12   \t\n"
        "   |   VMOV.M2		    VR0, VR13   \t\n"
        "   |   VMOV.M3		    VR0, VR14   \t\n"
            // [13]
        "       SVBCAST.M2      R55, VR37   ;; bcast nextA[2]                                       \t\n"
        "   |   VLDDW 		    *AR5++[OR2], VR53:VR52		;; load mat_C[2][0,1]   \t\n"
        "   |   VLDDW           *+AR5[16], VR55:VR54        ;; load mat_C[2][2,3]   \t\n"
        "   |   VMOV.M1		    VR0, VR15   \t\n"
        "   |   VMOV.M2		    VR0, VR16   \t\n"
        "   |   VMOV.M3		    VR0, VR17   \t\n"
            // [14]
        "       VLDDW 		    *AR5++[OR2], VR57:VR56		;; load mat_C[3][0,1]   \t\n"
        "   |   VLDDW           *+AR5[16], VR59:VR58        ;; load mat_C[3][2,3]   \t\n"
        "   |   VMOV.M1		    VR0, VR18   \t\n"
        "   |   VMOV.M2		    VR0, VR19   \t\n"
        "   |   VMOV.M3		    VR0, VR20   \t\n"
            // [15]
        "       SVBCAST.M2      R57, VR39   ;; bcast nextA[3]                                       \t\n"
        "   |   VBALE2          VR33, VR33, VR32                                                    \t\n"
        "   |   VLDDW 		    *AR4++[OR2], VR29:VR28		                                        \t\n"
        "   |   VLDDW 		    *+AR4[16], VR31:VR30		                                        \t\n"
        "   |   VMOV.M1		    VR0, VR21   \t\n"
        "   |   VMOV.M2		    VR0, VR22   \t\n"
        "   |   VMOV.M3		    VR0, VR23   \t\n"

        "loop_k_r6c128:       \t\n"
//------------------------------------------ main loop start ------------------------------------------
            // [0]
        "       VFMULAS32.M1	VR24, VR32, VR0, VR0		\t\n"
        "   |   VFMULAS32.M2	VR25, VR32, VR1, VR1		\t\n"
        "   |   VFMULAS32.M3	VR26, VR32, VR2, VR2		\t\n"
        "   |   SVBCAST.M2      R59, VR41   ;; bcast A[4]   \t\n"
        "   |   VBALE2          VR35, VR35, VR34            \t\n"     
            // [1]
        "   	SLT		        %0, R21, R0                 \t\n"
        "   |   VFMULAS32.M1	VR27, VR32, VR3, VR3		\t\n"
        "   |   VFMULAS32.M2	VR24, VR34, VR4, VR4		\t\n"
        "   |   VFMULAS32.M3	VR25, VR34, VR5, VR5		\t\n"
        "   |   SVBCAST.M2      R61, VR43   ;; bcast A[5]   \t\n"
        "   |   VBALE2          VR37, VR37, VR36            \t\n"
            // [2]
        "       VFMULAS32.M1	VR26, VR34, VR6, VR6		\t\n"
        "   |   VFMULAS32.M2	VR27, VR34, VR7, VR7		\t\n"
        "   |   VFMULAS32.M3	VR24, VR36, VR8, VR8		\t\n"
            // [3]
        "       VFMULAS32.M1	VR25, VR36, VR9, VR9		\t\n"
        "   |   VFMULAS32.M2	VR26, VR36, VR10, VR10		\t\n"
        "   |   VFMULAS32.M3	VR27, VR36, VR11, VR11		\t\n"
        "   |   VBALE2          VR39, VR39, VR38            \t\n"
            // [4]
        "       VFMULAS32.M1	VR24, VR38, VR12, VR12		\t\n"
        "   |   VFMULAS32.M2	VR25, VR38, VR13, VR13		\t\n"
        "   |   VFMULAS32.M3	VR26, VR38, VR14, VR14		\t\n"
        "   |   SLDW            *AR10, R51  	    ;; load nextA[0]    \t\n"
        "   |   VBALE2          VR41, VR41, VR40            \t\n"
            // [5]
        "       SADDA.M1        8, AR10, AR10               \t\n"
        "   |   VFMULAS32.M1	VR27, VR38, VR15, VR15		\t\n"
        "   |   VFMULAS32.M2	VR24, VR40, VR16, VR16		\t\n"
        "   |   VFMULAS32.M3	VR25, VR40, VR17, VR17		\t\n"
        "   |   SLDW            *+AR10[OR8], R53    ;; load nextA[1]    \t\n"
        "   |   VBALE2          VR43, VR43, VR42            \t\n"
            // [6]
        "       VFMULAS32.M1	VR26, VR40, VR18, VR18		\t\n"
        "   |   VFMULAS32.M2	VR27, VR40, VR19, VR19		\t\n"
        "   |   VFMULAS32.M3	VR24, VR42, VR20, VR20		\t\n"
        "   |   SLDW            *+AR10[OR9], R55    ;; load nextA[2]    \t\n"
            // [7]
        "       SMVAAG.M1	    AR10, %0		            \t\n"
        "   |   VFMULAS32.M1	VR25, VR42, VR21, VR21		\t\n"
        "   |   VFMULAS32.M2	VR26, VR42, VR22, VR22		\t\n"
        "   |   VFMULAS32.M3	VR27, VR42, VR23, VR23		\t\n"
        "   |   VBALE2H         VR33, VR33, VR33            \t\n"
        "   |   VLDDW 		    *AR4++[OR2], VR25:VR24		\t\n"
        "   |   VLDDW 		    *+AR4[16], VR27:VR26		\t\n"
            // [8]
        "       VFMULAS32.M1	VR28, VR33, VR0, VR0		\t\n"
        "   |   VFMULAS32.M2	VR29, VR33, VR1, VR1		\t\n"
        "   |   VFMULAS32.M3	VR30, VR33, VR2, VR2		\t\n"
        "   |   SLDW            *AR13, R57          ;; load nextA[3]    \t\n"
        "   |   VBALE2H         VR35, VR35, VR35            \t\n"
            // [9]
        "       SADDA.M1        8, AR13, AR13               \t\n"
        "   |   VFMULAS32.M1	VR31, VR33, VR3, VR3		\t\n"
        "   |   VFMULAS32.M2	VR28, VR35, VR4, VR4		\t\n"
        "   |   VFMULAS32.M3	VR29, VR35, VR5, VR5		\t\n"
        "   |   SLDW            *+AR13[OR8], R59    ;; load nextA[4]    \t\n"
        "   |   VBALE2H         VR37, VR37, VR37            \t\n"
        "   |   [R0]	SBR		loop_k_r6c128               \t\n"
            // [10]
        "       VFMULAS32.M1	VR30, VR35, VR6, VR6		\t\n"
        "   |   VFMULAS32.M2	VR31, VR35, VR7, VR7		\t\n"
        "   |   VFMULAS32.M3	VR28, VR37, VR8, VR8		\t\n"
        "   |   SLDW            *+AR13[OR9], R61    ;; load nextA[5]    \t\n"
            // [11]
        "       VFMULAS32.M1	VR29, VR37, VR9, VR9		\t\n"
        "   |   VFMULAS32.M2	VR30, VR37, VR10, VR10		\t\n"
        "   |   VFMULAS32.M3	VR31, VR37, VR11, VR11		\t\n"
        "   |   SVBCAST.M2      R51, VR33   ;; bcast nextA[0]   \t\n"
        "   |   VBALE2H         VR39, VR39, VR39            \t\n"
            // [12]
        "       VFMULAS32.M1	VR28, VR39, VR12, VR12		\t\n"
        "   |   VFMULAS32.M2	VR29, VR39, VR13, VR13		\t\n"
        "   |   VFMULAS32.M3	VR30, VR39, VR14, VR14		\t\n"
        "   |   SVBCAST.M2      R53, VR35   ;; bcast nextA[1]   \t\n"
        "   |   VBALE2H         VR41, VR41, VR41            \t\n"
            // [13]
        "       VFMULAS32.M1	VR31, VR39, VR15, VR15		\t\n"
        "   |   VFMULAS32.M2	VR28, VR41, VR16, VR16		\t\n"
        "   |   VFMULAS32.M3	VR29, VR41, VR17, VR17		\t\n"
        "   |   SVBCAST.M2      R55, VR37   ;; bcast nextA[2]   \t\n"
        "   |   VBALE2H         VR43, VR43, VR43            \t\n"
            // [14]
        "       VFMULAS32.M1	VR30, VR41, VR18, VR18		\t\n"
        "   |   VFMULAS32.M2	VR31, VR41, VR19, VR19		\t\n"
        "   |   VFMULAS32.M3	VR28, VR43, VR20, VR20		\t\n"
            // [15]
        "       VFMULAS32.M1	VR29, VR43, VR21, VR21		\t\n"
        "   |   VFMULAS32.M2	VR30, VR43, VR22, VR22		\t\n"
        "   |   VFMULAS32.M3	VR31, VR43, VR23, VR23		\t\n"
        "   |   SVBCAST.M2      R57, VR39   ;; bcast nextA[3]   \t\n"
        "   |   VBALE2          VR33, VR33, VR32            \t\n"
        "   |   VLDDW 		    *AR4++[OR2], VR29:VR28		\t\n"
        "   |   VLDDW 		    *+AR4[16], VR31:VR30		\t\n"
//------------------------------------------ last iteration ------------------------------------------
            // [0]
        "       VFMULAS32.M1	VR24, VR32, VR0, VR0		\t\n"
        "   |   VFMULAS32.M2	VR25, VR32, VR1, VR1		\t\n"
        "   |   VFMULAS32.M3	VR26, VR32, VR2, VR2		\t\n"
        "   |   SVBCAST.M2      R59, VR41   ;; bcast A[4]   \t\n"
        "   |   VBALE2          VR35, VR35, VR34            \t\n"     
            // [1]
        "       VFMULAS32.M1	VR27, VR32, VR3, VR3		\t\n"
        "   |   VFMULAS32.M2	VR24, VR34, VR4, VR4		\t\n"
        "   |   VFMULAS32.M3	VR25, VR34, VR5, VR5		\t\n"
        "   |   SVBCAST.M2      R61, VR43   ;; bcast A[5]   \t\n"
        "   |   VBALE2          VR37, VR37, VR36            \t\n"
            // [2]
        "       VFMULAS32.M1	VR26, VR34, VR6, VR6		\t\n"
        "   |   VFMULAS32.M2	VR27, VR34, VR7, VR7		\t\n"
        "   |   VFMULAS32.M3	VR24, VR36, VR8, VR8		\t\n"
            // [3]
        "       VFMULAS32.M1	VR25, VR36, VR9, VR9		\t\n"
        "   |   VFMULAS32.M2	VR26, VR36, VR10, VR10		\t\n"
        "   |   VFMULAS32.M3	VR27, VR36, VR11, VR11		\t\n"
        "   |   VBALE2          VR39, VR39, VR38            \t\n"
            // [4]
        "       VFMULAS32.M1	VR24, VR38, VR12, VR12		\t\n"
        "   |   VFMULAS32.M2	VR25, VR38, VR13, VR13		\t\n"
        "   |   VFMULAS32.M3	VR26, VR38, VR14, VR14		\t\n"
        "   |   VBALE2          VR41, VR41, VR40            \t\n"
            // [5]
        "       VFMULAS32.M1	VR27, VR38, VR15, VR15		\t\n"
        "   |   VFMULAS32.M2	VR24, VR40, VR16, VR16		\t\n"
        "   |   VFMULAS32.M3	VR25, VR40, VR17, VR17		\t\n"
        "   |   VBALE2          VR43, VR43, VR42            \t\n"
            // [6]
        "       VFMULAS32.M1	VR26, VR40, VR18, VR18		\t\n"
        "   |   VFMULAS32.M2	VR27, VR40, VR19, VR19		\t\n"
        "   |   VFMULAS32.M3	VR24, VR42, VR20, VR20		\t\n"
            // [7]
        "       VFMULAS32.M1	VR25, VR42, VR21, VR21		\t\n"
        "   |   VFMULAS32.M2	VR26, VR42, VR22, VR22		\t\n"
        "   |   VFMULAS32.M3	VR27, VR42, VR23, VR23		\t\n"
        "   |   VBALE2H         VR33, VR33, VR33            \t\n"
            // [8]
        "       VFMULAS32.M1	VR28, VR33, VR0, VR0		\t\n"
        "   |   VFMULAS32.M2	VR29, VR33, VR1, VR1		\t\n"
        "   |   VFMULAS32.M3	VR30, VR33, VR2, VR2		\t\n"
        "   |   VBALE2H         VR35, VR35, VR35            \t\n"
            // [9]
        "       VFMULAS32.M1	VR31, VR33, VR3, VR3		\t\n"
        "   |   VFMULAS32.M2	VR28, VR35, VR4, VR4		\t\n"
        "   |   VFMULAS32.M3	VR29, VR35, VR5, VR5		\t\n"
        "   |   VBALE2H         VR37, VR37, VR37            \t\n"
            // [10]
        "       VFMULAS32.M1	VR30, VR35, VR6, VR6		\t\n"
        "   |   VFMULAS32.M2	VR31, VR35, VR7, VR7		\t\n"
        "   |   VFMULAS32.M3	VR28, VR37, VR8, VR8		\t\n"
            // [11]
        "       VFMULAS32.M1	VR29, VR37, VR9, VR9		\t\n"
        "   |   VFMULAS32.M2	VR30, VR37, VR10, VR10		\t\n"
        "   |   VFMULAS32.M3	VR31, VR37, VR11, VR11		\t\n"
        "   |   VBALE2H         VR39, VR39, VR39            \t\n"
            // [12]
        "       VFMULAS32.M1	VR28, VR39, VR12, VR12		\t\n"
        "   |   VFMULAS32.M2	VR29, VR39, VR13, VR13		\t\n"
        "   |   VFMULAS32.M3	VR30, VR39, VR14, VR14		\t\n"
        "   |   VBALE2H         VR41, VR41, VR41            \t\n"
        "   |   VLDDW 		    *AR5++[OR2], VR25:VR24		;; load mat_C[4][0,1]   \t\n"
        "   |   VLDDW           *+AR5[16], VR27:VR26        ;; load mat_C[4][2,3]   \t\n"
            // [13]
        "       VFMULAS32.M1	VR31, VR39, VR15, VR15		\t\n"
        "   |   VFMULAS32.M2	VR28, VR41, VR16, VR16		\t\n"
        "   |   VFMULAS32.M3	VR29, VR41, VR17, VR17		\t\n"
        "   |   VBALE2H         VR43, VR43, VR43            \t\n"
        "   |   VLDDW 		    *AR5++[OR2], VR33:VR32		;; load mat_C[5][0,1]   \t\n"
        "   |   VLDDW           *+AR5[16], VR35:VR34        ;; load mat_C[5][2,3]   \t\n"
            // [14]
        "       VFMULAS32.M1	VR30, VR41, VR18, VR18		\t\n"
        "   |   VFMULAS32.M2	VR31, VR41, VR19, VR19		\t\n"
        "   |   VFMULAS32.M3	VR28, VR43, VR20, VR20		\t\n"
            // [15]
        "       VFMULAS32.M1	VR29, VR43, VR21, VR21		\t\n"
        "   |   VFMULAS32.M2	VR30, VR43, VR22, VR22		\t\n"
        "   |   VFMULAS32.M3	VR31, VR43, VR23, VR23		\t\n"
//------------------------------------------ main loop end ------------------------------------------
        // write-back:
            // [0]
	    "   	VFMULAS32.M1	VR0, VR63, VR44, VR0        \t\n"
	    "   |	VFMULAS32.M2	VR1, VR63, VR45, VR1        \t\n"
	    "   |	VFMULAS32.M3	VR2, VR63, VR46, VR2        \t\n"
            // [1]
	    "   	VFMULAS32.M1	VR3, VR63, VR47, VR3        \t\n"
	    "   |	VFMULAS32.M2	VR4, VR63, VR48, VR4        \t\n"
	    "   |	VFMULAS32.M3	VR5, VR63, VR49, VR5        \t\n"
	        // [2]
	    "   	VFMULAS32.M1	VR6, VR63, VR50, VR6        \t\n"
	    "   |	VFMULAS32.M2	VR7, VR63, VR51, VR7        \t\n"
	    "   |	VFMULAS32.M3	VR8, VR63, VR52, VR8        \t\n"
	        // [3]
	    "   	VFMULAS32.M1	VR9, VR63, VR53, VR9        \t\n"
	    "   |	VFMULAS32.M2	VR10, VR63, VR54, VR10      \t\n"
	    "   |	VFMULAS32.M3	VR11, VR63, VR55, VR11      \t\n"
	        // [4]
	    "   	VFMULAS32.M1	VR12, VR63, VR56, VR12      \t\n"
	    "   |	VFMULAS32.M2	VR13, VR63, VR57, VR13      \t\n"
	    "   |	VFMULAS32.M3	VR14, VR63, VR58, VR14      \t\n"
	        // [5]
	    "   	VFMULAS32.M1	VR15, VR63, VR59, VR15      \t\n"
	    "   |	VFMULAS32.M2	VR16, VR63, VR24, VR16      \t\n"
	    "   |	VFMULAS32.M3	VR17, VR63, VR25, VR17      \t\n"
            // [6]
	    "   	VFMULAS32.M1	VR18, VR63, VR26, VR18      \t\n"
	    "   |	VFMULAS32.M2	VR19, VR63, VR27, VR19      \t\n"
	    "   |	VFMULAS32.M3	VR20, VR63, VR32, VR20      \t\n"
            // [7]
        "   	VFMULAS32.M1	VR21, VR63, VR33, VR21      \t\n"
	    "   |	VFMULAS32.M2	VR22, VR63, VR34, VR22      \t\n"
	    "   |	VFMULAS32.M3	VR23, VR63, VR35, VR23      \t\n"
        "   |   VSTDW           VR1:VR0, *AR6++[OR2]        \r\n"
        "   |   VSTDW           VR3:VR2, *+AR6[16]          \r\n"
        "   |   SBR		        R63                         \t\n"
            // [8]
        "       VSTDW           VR5:VR4, *AR6++[OR2]        \r\n"
        "   |   VSTDW           VR7:VR6, *+AR6[16]          \r\n"
            // [9]
        "       VSTDW           VR9:VR8, *AR6++[OR2]        \r\n"
        "   |   VSTDW           VR11:VR10, *+AR6[16]        \r\n"
            // [10]
        "       SNOP            1       \t\n"
            // [11]
        "       VSTDW           VR13:VR12, *AR6++[OR2]      \r\n"
        "   |   VSTDW           VR15:VR14, *+AR6[16]        \r\n"
            // [12] 
        "       VSTDW           VR17:VR16, *AR6++[OR2]      \r\n"
        "   |   VSTDW           VR19:VR18, *+AR6[16]        \r\n"
            // [13]     
        "       VSTDW           VR21:VR20, *AR6++[OR2]      \r\n"
        "   |   VSTDW           VR23:VR22, *+AR6[16]        \r\n"
    :
    :"r"(src_a), "r"(src_b), "r"(dst_c), "r"(K_data), "r"(K_buffer), "r"(N_buffer)
    );
}

void micro_kernel_asm_r6c128_only_write(float* src_a, lvector float* src_b, lvector float* dst_c, 
                        const indexType K_data, const indexType K_buffer, const indexType N_buffer){
    __asm__ __volatile__(
            // [-2]
        "       SSHFLR          1, %4, R7                   ;; R7 = K_buffer/2  // A矩阵一行的字数   \t\n"
	    "   |   SMVAGA.M1	    %0, AR10   			        ;; AR10 = src_A                         \t\n"
	    "   |   SMVAGA.M2       %4, OR9                     ;; OR9 = A矩阵两行的字数: K_buffer       \t\n"
            // [-1]
        "       SSHFLR          2, %5, R42                  ;; B/C_buffer一行多少个双字              \t\n"
        "   |   SADD.M1         R7, %4, R9                  ;; A矩阵3行的字数: 1.5*K_buffer          \t\n"
        "   |   SMVAGA.M2       R7, OR8                     ;; OR8 = A矩阵一行的字数: K_buffer/2     \t\n"
            // [4]
        "       SSHFLL          3, R9, R9                   ;; A矩阵3行的字节数, R9=8*(1.5*K_buffer) \t\n"
	    "   |   SMVAGA.M1       R42, OR2                    ;; B/C_buffer一行多少个双字              \t\n"
        "   |   SMVAGA.M2	    %1, AR4    			        ;; AR7 = src_B                          \t\n"
        "   |   SLDW            *AR10, R51  	            ;; load nextA[0]                        \t\n"
            // [5]
        "       SSHFLL		    2, %3, R21				    ;; R20 = K_data * 4 // A矩阵一行的字节数 \t\n"
        "   |   SADD.M1         R9, %0, R9                  ;; R9 = src_A + 8*(1.5*K_buffer)        \t\n"
        "   |   SADDA.M2        8, AR10, AR10                                                       \t\n"
        "   |   SLDW            *+AR10[OR8], R53            ;; load nextA[1]                        \t\n"
            // [6]
        "       SMVAGA.M1	    R9, AR13				    ;; AR13 = src_A + 8*(1.5*K_buffer)      \t\n"
        "   |   SADD.M2		    R21, %0, R21			    ;; R21 = src_A + K_data * 4             \t\n"
        "   |   SLDW            *+AR10[OR9], R55            ;; load nextA[2]                        \t\n"
            // [7]
        "       SMVAAG.M2	    AR10, %0		                                                    \t\n"
        "   |   VLDDW 		    *AR4++[OR2], VR25:VR24		                                        \t\n"
        "   |   VLDDW 		    *+AR4[16], VR27:VR26		                                        \t\n"
        "   |   VMOVI.M1 	    0x00, VR0				    ;; 初始化C寄存器                         \t\n"
        "   |   VMOVI.M2 	    0x3F8000003F800000,VR63	    ;; VR63 = 1.0                           \t\n"
            // [8]
        "       SLDW            *AR13, R57                  ;; load nextA[3]                        \t\n"
        "   |   VMOV.M2		    VR0, VR1    \t\n"
        "   |   VMOV.M3		    VR0, VR2    \t\n"
            // [9]
        "       SADDA.M1        8, AR13, AR13                                                       \t\n"
        "   |   SLDW            *+AR13[OR8], R59            ;; load nextA[4]                        \t\n"
        "   |   VMOV.M1		    VR0, VR3    \t\n"
        "   |   VMOV.M2		    VR0, VR4    \t\n"
        "   |   VMOV.M3		    VR0, VR5    \t\n"
            // [10]
        "       SLDW            *+AR13[OR9], R61            ;; load nextA[5]                        \t\n"
        "   |   VMOV.M1		    VR0, VR6    \t\n"
        "   |   VMOV.M2		    VR0, VR7    \t\n"
        "   |   VMOV.M3		    VR0, VR8    \t\n"
            // [11]
        "       SVBCAST.M2      R51, VR33   ;; bcast nextA[0]                                       \t\n"
        "   |   VMOV.M1		    VR0, VR9    \t\n"
        "   |   VMOV.M2		    VR0, VR10   \t\n"
        "   |   VMOV.M3		    VR0, VR11   \t\n"
            // [12]
        "       SVBCAST.M2      R53, VR35   ;; bcast nextA[1]                                       \t\n"
        "   |   VMOV.M1		    VR0, VR12   \t\n"
        "   |   VMOV.M2		    VR0, VR13   \t\n"
        "   |   VMOV.M3		    VR0, VR14   \t\n"
            // [13]
        "       SVBCAST.M2      R55, VR37   ;; bcast nextA[2]                                       \t\n"
        "   |   VMOV.M1		    VR0, VR15   \t\n"
        "   |   VMOV.M2		    VR0, VR16   \t\n"
        "   |   VMOV.M3		    VR0, VR17   \t\n"
            // [14]
        "       VMOV.M1		    VR0, VR18   \t\n"
        "   |   VMOV.M2		    VR0, VR19   \t\n"
        "   |   VMOV.M3		    VR0, VR20   \t\n"
            // [15]
        "       SVBCAST.M2      R57, VR39   ;; bcast nextA[3]                                       \t\n"
        "   |   VBALE2          VR33, VR33, VR32                                                    \t\n"
        "   |   VLDDW 		    *AR4++[OR2], VR29:VR28		                                        \t\n"
        "   |   VLDDW 		    *+AR4[16], VR31:VR30		                                        \t\n"
        "   |   VMOV.M1		    VR0, VR21   \t\n"
        "   |   VMOV.M2		    VR0, VR22   \t\n"
        "   |   VMOV.M3		    VR0, VR23   \t\n"

        "loop_k_r6c128_only_write:       \t\n"
//------------------------------------------ main loop start ------------------------------------------
            // [0]
        "       VFMULAS32.M1	VR24, VR32, VR0, VR0		\t\n"
        "   |   VFMULAS32.M2	VR25, VR32, VR1, VR1		\t\n"
        "   |   VFMULAS32.M3	VR26, VR32, VR2, VR2		\t\n"
        "   |   SVBCAST.M2      R59, VR41   ;; bcast A[4]   \t\n"
        "   |   VBALE2          VR35, VR35, VR34            \t\n"     
            // [1]
        "   	SLT		        %0, R21, R0                 \t\n"
        "   |   VFMULAS32.M1	VR27, VR32, VR3, VR3		\t\n"
        "   |   VFMULAS32.M2	VR24, VR34, VR4, VR4		\t\n"
        "   |   VFMULAS32.M3	VR25, VR34, VR5, VR5		\t\n"
        "   |   SVBCAST.M2      R61, VR43   ;; bcast A[5]   \t\n"
        "   |   VBALE2          VR37, VR37, VR36            \t\n"
            // [2]
        "       VFMULAS32.M1	VR26, VR34, VR6, VR6		\t\n"
        "   |   VFMULAS32.M2	VR27, VR34, VR7, VR7		\t\n"
        "   |   VFMULAS32.M3	VR24, VR36, VR8, VR8		\t\n"
            // [3]
        "       VFMULAS32.M1	VR25, VR36, VR9, VR9		\t\n"
        "   |   VFMULAS32.M2	VR26, VR36, VR10, VR10		\t\n"
        "   |   VFMULAS32.M3	VR27, VR36, VR11, VR11		\t\n"
        "   |   VBALE2          VR39, VR39, VR38            \t\n"
            // [4]
        "       VFMULAS32.M1	VR24, VR38, VR12, VR12		\t\n"
        "   |   VFMULAS32.M2	VR25, VR38, VR13, VR13		\t\n"
        "   |   VFMULAS32.M3	VR26, VR38, VR14, VR14		\t\n"
        "   |   SLDW            *AR10, R51  	    ;; load nextA[0]    \t\n"
        "   |   VBALE2          VR41, VR41, VR40            \t\n"
            // [5]
        "       SADDA.M1        8, AR10, AR10               \t\n"
        "   |   VFMULAS32.M1	VR27, VR38, VR15, VR15		\t\n"
        "   |   VFMULAS32.M2	VR24, VR40, VR16, VR16		\t\n"
        "   |   VFMULAS32.M3	VR25, VR40, VR17, VR17		\t\n"
        "   |   SLDW            *+AR10[OR8], R53    ;; load nextA[1]    \t\n"
        "   |   VBALE2          VR43, VR43, VR42            \t\n"
            // [6]
        "       VFMULAS32.M1	VR26, VR40, VR18, VR18		\t\n"
        "   |   VFMULAS32.M2	VR27, VR40, VR19, VR19		\t\n"
        "   |   VFMULAS32.M3	VR24, VR42, VR20, VR20		\t\n"
        "   |   SLDW            *+AR10[OR9], R55    ;; load nextA[2]    \t\n"
            // [7]
        "       SMVAAG.M1	    AR10, %0		            \t\n"
        "   |   VFMULAS32.M1	VR25, VR42, VR21, VR21		\t\n"
        "   |   VFMULAS32.M2	VR26, VR42, VR22, VR22		\t\n"
        "   |   VFMULAS32.M3	VR27, VR42, VR23, VR23		\t\n"
        "   |   VBALE2H         VR33, VR33, VR33            \t\n"
        "   |   VLDDW 		    *AR4++[OR2], VR25:VR24		\t\n"
        "   |   VLDDW 		    *+AR4[16], VR27:VR26		\t\n"
            // [8]
        "       VFMULAS32.M1	VR28, VR33, VR0, VR0		\t\n"
        "   |   VFMULAS32.M2	VR29, VR33, VR1, VR1		\t\n"
        "   |   VFMULAS32.M3	VR30, VR33, VR2, VR2		\t\n"
        "   |   SLDW            *AR13, R57          ;; load nextA[3]    \t\n"
        "   |   VBALE2H         VR35, VR35, VR35            \t\n"
            // [9]
        "       SADDA.M1        8, AR13, AR13               \t\n"
        "   |   VFMULAS32.M1	VR31, VR33, VR3, VR3		\t\n"
        "   |   VFMULAS32.M2	VR28, VR35, VR4, VR4		\t\n"
        "   |   VFMULAS32.M3	VR29, VR35, VR5, VR5		\t\n"
        "   |   SLDW            *+AR13[OR8], R59    ;; load nextA[4]    \t\n"
        "   |   VBALE2H         VR37, VR37, VR37            \t\n"
        "   |   [R0]	SBR		loop_k_r6c128_only_write    \t\n"
            // [10]
        "       VFMULAS32.M1	VR30, VR35, VR6, VR6		\t\n"
        "   |   VFMULAS32.M2	VR31, VR35, VR7, VR7		\t\n"
        "   |   VFMULAS32.M3	VR28, VR37, VR8, VR8		\t\n"
        "   |   SLDW            *+AR13[OR9], R61    ;; load nextA[5]    \t\n"
            // [11]
        "       VFMULAS32.M1	VR29, VR37, VR9, VR9		\t\n"
        "   |   VFMULAS32.M2	VR30, VR37, VR10, VR10		\t\n"
        "   |   VFMULAS32.M3	VR31, VR37, VR11, VR11		\t\n"
        "   |   SVBCAST.M2      R51, VR33   ;; bcast nextA[0]   \t\n"
        "   |   VBALE2H         VR39, VR39, VR39            \t\n"
            // [12]
        "       VFMULAS32.M1	VR28, VR39, VR12, VR12		\t\n"
        "   |   VFMULAS32.M2	VR29, VR39, VR13, VR13		\t\n"
        "   |   VFMULAS32.M3	VR30, VR39, VR14, VR14		\t\n"
        "   |   SVBCAST.M2      R53, VR35   ;; bcast nextA[1]   \t\n"
        "   |   VBALE2H         VR41, VR41, VR41            \t\n"
            // [13]
        "       VFMULAS32.M1	VR31, VR39, VR15, VR15		\t\n"
        "   |   VFMULAS32.M2	VR28, VR41, VR16, VR16		\t\n"
        "   |   VFMULAS32.M3	VR29, VR41, VR17, VR17		\t\n"
        "   |   SVBCAST.M2      R55, VR37   ;; bcast nextA[2]   \t\n"
        "   |   VBALE2H         VR43, VR43, VR43            \t\n"
            // [14]
        "       VFMULAS32.M1	VR30, VR41, VR18, VR18		\t\n"
        "   |   VFMULAS32.M2	VR31, VR41, VR19, VR19		\t\n"
        "   |   VFMULAS32.M3	VR28, VR43, VR20, VR20		\t\n"
        "   |   [!R0]   SMVAGA.M1	    %2, AR6     ;; AR6 = dst_C  \t\n"
            // [15]
        "       VFMULAS32.M1	VR29, VR43, VR21, VR21		\t\n"
        "   |   VFMULAS32.M2	VR30, VR43, VR22, VR22		\t\n"
        "   |   VFMULAS32.M3	VR31, VR43, VR23, VR23		\t\n"
        "   |   SVBCAST.M2      R57, VR39   ;; bcast nextA[3]   \t\n"
        "   |   VBALE2          VR33, VR33, VR32            \t\n"
        "   |   VLDDW 		    *AR4++[OR2], VR29:VR28		\t\n"
        "   |   VLDDW 		    *+AR4[16], VR31:VR30		\t\n"
        "   |   [!R0]   SBR     R63                         \t\n"
//------------------------------------------ main loop end ------------------------------------------
        // write-back:
            // [0]
        "       VSTDW           VR1:VR0, *AR6++[OR2]        \r\n"
        "   |   VSTDW           VR3:VR2, *+AR6[16]          \r\n"
            // [1]
        "       VSTDW           VR5:VR4, *AR6++[OR2]        \r\n"
        "   |   VSTDW           VR7:VR6, *+AR6[16]          \r\n"
            // [2]
        "       VSTDW           VR9:VR8, *AR6++[OR2]        \r\n"
        "   |   VSTDW           VR11:VR10, *+AR6[16]        \r\n"
            // [3]
        "       VSTDW           VR13:VR12, *AR6++[OR2]      \r\n"
        "   |   VSTDW           VR15:VR14, *+AR6[16]        \r\n"
            // [4] 
        "       VSTDW           VR17:VR16, *AR6++[OR2]      \r\n"
        "   |   VSTDW           VR19:VR18, *+AR6[16]        \r\n"
            // [5]     
        "       VSTDW           VR21:VR20, *AR6++[OR2]      \r\n"
        "   |   VSTDW           VR23:VR22, *+AR6[16]        \r\n"
    :
    :"r"(src_a), "r"(src_b), "r"(dst_c), "r"(K_data), "r"(K_buffer), "r"(N_buffer)
    );
}

#ifdef enable_alpha
void micro_kernel_asm_r6c128_multiAlpha(float* src_a, lvector float* src_b, lvector float* dst_c, 
                    const indexType K_data, const indexType K_buffer, const indexType N_buffer, const float alpha){
    __asm__ __volatile__(
            // [-2]
        "       SSHFLR          1, %4, R7                   ;; R7 = K_buffer/2  // A矩阵一行的字数   \t\n"
	    "   |   SMVAGA.M1	    %0, AR10   			        ;; AR10 = src_A                         \t\n"
	    "   |   SMVAGA.M2       %4, OR9                     ;; OR9 = A矩阵两行的字数: K_buffer       \t\n"
            // [-1]
        "       SSHFLR          2, %5, R42                  ;; B/C_buffer一行多少个双字              \t\n"
        "   |   SADD.M1         R7, %4, R9                  ;; A矩阵3行的字数: 1.5*K_buffer          \t\n"
        "   |   SMVAGA.M2       R7, OR8                     ;; OR8 = A矩阵一行的字数: K_buffer/2     \t\n"
            // [0]
        "       SSHFLL          3, R9, R9                   ;; A矩阵3行的字节数, R9=8*(1.5*K_buffer) \t\n"
	    "   |   SMVAGA.M1       R42, OR2                    ;; B/C_buffer一行多少个双字              \t\n"
        "   |   SMVAGA.M2	    %1, AR4    			        ;; AR7 = src_B                          \t\n"
        "   |   SLDW            *AR10, R51  	            ;; load nextA[0]                        \t\n"
            // [1]
        "       SSHFLL		    2, %3, R21				    ;; R20 = K_data * 4 // A矩阵一行的字节数 \t\n"
        "   |   SADD.M1         R9, %0, R9                  ;; R9 = src_A + 8*(1.5*K_buffer)        \t\n"
        "   |   SADDA.M2        8, AR10, AR10                                                       \t\n"
        "   |   SLDW            *+AR10[OR8], R53            ;; load nextA[1]                        \t\n"
            // [2]
        "       SMVAGA.M1	    R9, AR13				    ;; AR13 = src_A + 8*(1.5*K_buffer)      \t\n"
        "   |   SADD.M2		    R21, %0, R21			    ;; R21 = src_A + K_data * 4             \t\n"
        "   |   SLDW            *+AR10[OR9], R55            ;; load nextA[2]                        \t\n"
            // [3]
        "       SSUB.M1		    8, R21, R21			        ;; R21 = src_A+(K_data-2)*4             \t\n"
        "   |   SMVAAG.M2	    AR10, %0		                                                    \t\n"
        "   |   VMOVI.M1 	    0x00, VR0				    ;; 初始化C寄存器                         \t\n"
        "   |   VMOVI.M2 	    0x3F8000003F800000,VR63	    ;; VR63 = 1.0                           \t\n"
            // [4]
        "       SLDW            *AR13, R57                  ;; load nextA[3]                        \t\n"
        "   |   SMVAGA.M1	    %2, AR5				        ;; AR5 = dst_C                          \t\n"
        "   |   SMVAGA.M2	    %2, AR6				        ;; AR6 = dst_C                          \t\n"
        "   |   VMOV.M2		    VR0, VR1    \t\n"
        "   |   VMOV.M3		    VR0, VR2    \t\n"
            // [5]
        "       SADDA.M1        8, AR13, AR13                                                       \t\n"
        "   |   SLDW            *+AR13[OR8], R59    ;; load nextA[4]                                \t\n"
        "   |   VMOV.M1		    VR0, VR3    \t\n"
        "   |   VMOV.M2		    VR0, VR4    \t\n"
        "   |   VMOV.M3		    VR0, VR5    \t\n"
            // [6]
        "       SLDW            *+AR13[OR9], R61    ;; load nextA[5]                                \t\n"
        "   |   SBALE2          %6, %6, %6              \t\n"
        "   |   VMOV.M1		    VR0, VR6    \t\n"
        "   |   VMOV.M2		    VR0, VR7    \t\n"
        "   |   VMOV.M3		    VR0, VR8    \t\n"
            // [7]
        "       SFMULS32.M1     %6, R51, R51            \t\n"
        "   |   VLDDW 		    *AR4++[OR2], VR25:VR24		                                        \t\n"
        "   |   VLDDW 		    *+AR4[16], VR27:VR26		                                        \t\n"
            // [8]
        "       SFMULS32.M1     %6, R53, R53            \t\n"
            // [9]
        "       SFMULS32.M1     %6, R55, R55            \t\n"
            // [10]
        "       SNOP            1                       \t\n"
            // [11]
        "       SFMULS32.M1     %6, R57, R57            \t\n"
        "   |   SVBCAST.M2      R51, VR33   ;; bcast nextA[0]                                       \t\n"
        "   |   VLDDW 		    *AR5++[OR2], VR45:VR44		;; load mat_C[0][0,1]   \t\n"
        "   |   VLDDW           *+AR5[16], VR47:VR46        ;; load mat_C[0][2,3]   \t\n"
        "   |   VMOV.M1		    VR0, VR9    \t\n"
        "   |   VMOV.M2		    VR0, VR10   \t\n"
        "   |   VMOV.M3		    VR0, VR11   \t\n"
            // [12]
        "       SFMULS32.M1     %6, R59, R59             \t\n"
        "   |   SVBCAST.M2      R53, VR35   ;; bcast nextA[1]                                       \t\n"
        "   |   VLDDW 		    *AR5++[OR2], VR49:VR48		;; load mat_C[1][0,1]   \t\n"
        "   |   VLDDW           *+AR5[16], VR51:VR50        ;; load mat_C[1][2,3]   \t\n"
        "   |   VMOV.M1		    VR0, VR12   \t\n"
        "   |   VMOV.M2		    VR0, VR13   \t\n"
        "   |   VMOV.M3		    VR0, VR14   \t\n"
            // [13]
        "       SFMULS32.M1     %6, R61, R61             \t\n"
        "   |   SVBCAST.M2      R55, VR37   ;; bcast nextA[2]                                       \t\n"
        "   |   VLDDW 		    *AR5++[OR2], VR53:VR52		;; load mat_C[2][0,1]   \t\n"
        "   |   VLDDW           *+AR5[16], VR55:VR54        ;; load mat_C[2][2,3]   \t\n"
        "   |   VMOV.M1		    VR0, VR15   \t\n"
        "   |   VMOV.M2		    VR0, VR16   \t\n"
        "   |   VMOV.M3		    VR0, VR17   \t\n"
            // [14]
        "       VLDDW 		    *AR5++[OR2], VR57:VR56		;; load mat_C[3][0,1]   \t\n"
        "   |   VLDDW           *+AR5[16], VR59:VR58        ;; load mat_C[3][2,3]   \t\n"
        "   |   VMOV.M1		    VR0, VR18   \t\n"
        "   |   VMOV.M2		    VR0, VR19   \t\n"
        "   |   VMOV.M3		    VR0, VR20   \t\n"
        "   |   VMOVI 	        0x3F8000003F800000,VR62	    ;; 无效指令 \t\n"
            // [15]
        "       SVBCAST.M2      R57, VR39   ;; bcast nextA[3]                                       \t\n"
        "   |   VBALE2          VR33, VR33, VR32                                                    \t\n"
        "   |   VLDDW 		    *AR4++[OR2], VR29:VR28		                                        \t\n"
        "   |   VLDDW 		    *+AR4[16], VR31:VR30		                                        \t\n"
        "   |   VMOV.M1		    VR0, VR21   \t\n"
        "   |   VMOV.M2		    VR0, VR22   \t\n"
        "   |   VMOV.M3		    VR0, VR23   \t\n"

        "loop_k_r6c128_multiAlpha:       \t\n"
//------------------------------------------ main loop start ------------------------------------------
            // [0]
        "       VFMULAS32.M1	VR24, VR32, VR0, VR0		\t\n"
        "   |   VFMULAS32.M2	VR25, VR32, VR1, VR1		\t\n"
        "   |   VFMULAS32.M3	VR26, VR32, VR2, VR2		\t\n"
        "   |   SLDW            *AR10, R51  	    ;; load nextA[0]    \t\n"
        "   |   SVBCAST.M2      R59, VR41   ;; bcast A[4]   \t\n"
        "   |   VBALE2          VR35, VR35, VR34            \t\n"     
            // [1]
        "   	SLT		        %0, R21, R0                 \t\n"
        "   |   VFMULAS32.M1	VR27, VR32, VR3, VR3		\t\n"
        "   |   VFMULAS32.M2	VR24, VR34, VR4, VR4		\t\n"
        "   |   VFMULAS32.M3	VR25, VR34, VR5, VR5		\t\n"
        "   |   SLDW            *+AR10[OR8], R53    ;; load nextA[1]    \t\n"
        "   |   SVBCAST.M2      R61, VR43   ;; bcast A[5]   \t\n"
        "   |   VBALE2          VR37, VR37, VR36            \t\n"
            // [2]
        "       VFMULAS32.M1	VR26, VR34, VR6, VR6		\t\n"
        "   |   VFMULAS32.M2	VR27, VR34, VR7, VR7		\t\n"
        "   |   VFMULAS32.M3	VR24, VR36, VR8, VR8		\t\n"
        "   |   SLDW            *+AR10[OR9], R55    ;; load nextA[2]    \t\n"
            // [3]
        "       VFMULAS32.M1	VR25, VR36, VR9, VR9		\t\n"
        "   |   VFMULAS32.M2	VR26, VR36, VR10, VR10		\t\n"
        "   |   VFMULAS32.M3	VR27, VR36, VR11, VR11		\t\n"
        "   |   VBALE2          VR39, VR39, VR38            \t\n"
            // [4]
        "       VFMULAS32.M1	VR24, VR38, VR12, VR12		\t\n"
        "   |   VFMULAS32.M2	VR25, VR38, VR13, VR13		\t\n"
        "   |   VFMULAS32.M3	VR26, VR38, VR14, VR14		\t\n"
        "   |   SLDW            *AR13, R57          ;; load nextA[3]    \t\n"
        "   |   VBALE2          VR41, VR41, VR40            \t\n"
            // [5]
        "       SADDA.M1        8, AR10, AR10               \t\n"
        "   |   VFMULAS32.M1	VR27, VR38, VR15, VR15		\t\n"
        "   |   VFMULAS32.M2	VR24, VR40, VR16, VR16		\t\n"
        "   |   VFMULAS32.M3	VR25, VR40, VR17, VR17		\t\n"
        "   |   SLDW            *+AR13[OR8], R59    ;; load nextA[4]    \t\n"
        "   |   VBALE2          VR43, VR43, VR42            \t\n"
            // [6]
        "       VFMULAS32.M1	VR26, VR40, VR18, VR18		\t\n"
        "   |   VFMULAS32.M2	VR27, VR40, VR19, VR19		\t\n"
        "   |   VFMULAS32.M3	VR24, VR42, VR20, VR20		\t\n"
        "   |   SLDW            *+AR13[OR9], R61    ;; load nextA[5]    \t\n"
            // [7]
        "       SMVAAG.M1	    AR10, %0		            \t\n"
        "   |   VFMULAS32.M1	VR25, VR42, VR21, VR21		\t\n"
        "   |   VFMULAS32.M2	VR26, VR42, VR22, VR22		\t\n"
        "   |   VFMULAS32.M3	VR27, VR42, VR23, VR23		\t\n"
        "   |   SFMULS32.M2     %6, R51, R51                \t\n"
        "   |   VBALE2H         VR33, VR33, VR33            \t\n"
        "   |   VLDDW 		    *AR4++[OR2], VR25:VR24		\t\n"
        "   |   VLDDW 		    *+AR4[16], VR27:VR26		\t\n"
            // [8]
        "       VFMULAS32.M1	VR28, VR33, VR0, VR0		\t\n"
        "   |   VFMULAS32.M2	VR29, VR33, VR1, VR1		\t\n"
        "   |   VFMULAS32.M3	VR30, VR33, VR2, VR2		\t\n"
        "   |   SFMULS32.M2     %6, R53, R53                \t\n"
        "   |   VBALE2H         VR35, VR35, VR35            \t\n"
            // [9]
        "       SADDA.M1        8, AR13, AR13               \t\n"
        "   |   VFMULAS32.M1	VR31, VR33, VR3, VR3		\t\n"
        "   |   VFMULAS32.M2	VR28, VR35, VR4, VR4		\t\n"
        "   |   VFMULAS32.M3	VR29, VR35, VR5, VR5		\t\n"
        "   |   SFMULS32.M2     %6, R55, R55                \t\n"
        "   |   VBALE2H         VR37, VR37, VR37            \t\n"
        "   |   [R0]	SBR		loop_k_r6c128_multiAlpha               \t\n"
            // [10]
        "       VFMULAS32.M1	VR30, VR35, VR6, VR6		\t\n"
        "   |   VFMULAS32.M2	VR31, VR35, VR7, VR7		\t\n"
        "   |   VFMULAS32.M3	VR28, VR37, VR8, VR8		\t\n"
            // [11]
        "       VFMULAS32.M1	VR29, VR37, VR9, VR9		\t\n"
        "   |   VFMULAS32.M2	VR30, VR37, VR10, VR10		\t\n"
        "   |   VFMULAS32.M3	VR31, VR37, VR11, VR11		\t\n"
        "   |   SFMULS32.M1     %6, R57, R57                \t\n"
        "   |   SVBCAST.M2      R51, VR33   ;; bcast nextA[0]   \t\n"
        "   |   VBALE2H         VR39, VR39, VR39            \t\n"
            // [12]
        "       VFMULAS32.M1	VR28, VR39, VR12, VR12		\t\n"
        "   |   VFMULAS32.M2	VR29, VR39, VR13, VR13		\t\n"
        "   |   VFMULAS32.M3	VR30, VR39, VR14, VR14		\t\n"
        "   |   SFMULS32.M1     %6, R59, R59                \t\n"
        "   |   SVBCAST.M2      R53, VR35   ;; bcast nextA[1]   \t\n"
        "   |   VBALE2H         VR41, VR41, VR41            \t\n"
            // [13]
        "       VFMULAS32.M1	VR31, VR39, VR15, VR15		\t\n"
        "   |   VFMULAS32.M2	VR28, VR41, VR16, VR16		\t\n"
        "   |   VFMULAS32.M3	VR29, VR41, VR17, VR17		\t\n"
        "   |   SFMULS32.M1     %6, R61, R61                \t\n"
        "   |   SVBCAST.M2      R55, VR37   ;; bcast nextA[2]   \t\n"
        "   |   VBALE2H         VR43, VR43, VR43            \t\n"
            // [14]
        "       VFMULAS32.M1	VR30, VR41, VR18, VR18		\t\n"
        "   |   VFMULAS32.M2	VR31, VR41, VR19, VR19		\t\n"
        "   |   VFMULAS32.M3	VR28, VR43, VR20, VR20		\t\n"
            // [15]
        "       VFMULAS32.M1	VR29, VR43, VR21, VR21		\t\n"
        "   |   VFMULAS32.M2	VR30, VR43, VR22, VR22		\t\n"
        "   |   VFMULAS32.M3	VR31, VR43, VR23, VR23		\t\n"
        "   |   SVBCAST.M2      R57, VR39   ;; bcast nextA[3]   \t\n"
        "   |   VBALE2          VR33, VR33, VR32            \t\n"
        "   |   VLDDW 		    *AR4++[OR2], VR29:VR28		\t\n"
        "   |   VLDDW 		    *+AR4[16], VR31:VR30		\t\n"
//------------------------------------------ last iteration ------------------------------------------
            // [0]
        "       VFMULAS32.M1	VR24, VR32, VR0, VR0		\t\n"
        "   |   VFMULAS32.M2	VR25, VR32, VR1, VR1		\t\n"
        "   |   VFMULAS32.M3	VR26, VR32, VR2, VR2		\t\n"
        "   |   SVBCAST.M2      R59, VR41   ;; bcast A[4]   \t\n"
        "   |   VBALE2          VR35, VR35, VR34            \t\n"     
            // [1]
        "       VFMULAS32.M1	VR27, VR32, VR3, VR3		\t\n"
        "   |   VFMULAS32.M2	VR24, VR34, VR4, VR4		\t\n"
        "   |   VFMULAS32.M3	VR25, VR34, VR5, VR5		\t\n"
        "   |   SVBCAST.M2      R61, VR43   ;; bcast A[5]   \t\n"
        "   |   VBALE2          VR37, VR37, VR36            \t\n"
            // [2]
        "       VFMULAS32.M1	VR26, VR34, VR6, VR6		\t\n"
        "   |   VFMULAS32.M2	VR27, VR34, VR7, VR7		\t\n"
        "   |   VFMULAS32.M3	VR24, VR36, VR8, VR8		\t\n"
            // [3]
        "       VFMULAS32.M1	VR25, VR36, VR9, VR9		\t\n"
        "   |   VFMULAS32.M2	VR26, VR36, VR10, VR10		\t\n"
        "   |   VFMULAS32.M3	VR27, VR36, VR11, VR11		\t\n"
        "   |   VBALE2          VR39, VR39, VR38            \t\n"
            // [4]
        "       VFMULAS32.M1	VR24, VR38, VR12, VR12		\t\n"
        "   |   VFMULAS32.M2	VR25, VR38, VR13, VR13		\t\n"
        "   |   VFMULAS32.M3	VR26, VR38, VR14, VR14		\t\n"
        "   |   VBALE2          VR41, VR41, VR40            \t\n"
            // [5]
        "       VFMULAS32.M1	VR27, VR38, VR15, VR15		\t\n"
        "   |   VFMULAS32.M2	VR24, VR40, VR16, VR16		\t\n"
        "   |   VFMULAS32.M3	VR25, VR40, VR17, VR17		\t\n"
        "   |   VBALE2          VR43, VR43, VR42            \t\n"
            // [6]
        "       VFMULAS32.M1	VR26, VR40, VR18, VR18		\t\n"
        "   |   VFMULAS32.M2	VR27, VR40, VR19, VR19		\t\n"
        "   |   VFMULAS32.M3	VR24, VR42, VR20, VR20		\t\n"
            // [7]
        "       VFMULAS32.M1	VR25, VR42, VR21, VR21		\t\n"
        "   |   VFMULAS32.M2	VR26, VR42, VR22, VR22		\t\n"
        "   |   VFMULAS32.M3	VR27, VR42, VR23, VR23		\t\n"
        "   |   VBALE2H         VR33, VR33, VR33            \t\n"
            // [8]
        "       VFMULAS32.M1	VR28, VR33, VR0, VR0		\t\n"
        "   |   VFMULAS32.M2	VR29, VR33, VR1, VR1		\t\n"
        "   |   VFMULAS32.M3	VR30, VR33, VR2, VR2		\t\n"
        "   |   VBALE2H         VR35, VR35, VR35            \t\n"
            // [9]
        "       VFMULAS32.M1	VR31, VR33, VR3, VR3		\t\n"
        "   |   VFMULAS32.M2	VR28, VR35, VR4, VR4		\t\n"
        "   |   VFMULAS32.M3	VR29, VR35, VR5, VR5		\t\n"
        "   |   VBALE2H         VR37, VR37, VR37            \t\n"
            // [10]
        "       VFMULAS32.M1	VR30, VR35, VR6, VR6		\t\n"
        "   |   VFMULAS32.M2	VR31, VR35, VR7, VR7		\t\n"
        "   |   VFMULAS32.M3	VR28, VR37, VR8, VR8		\t\n"
            // [11]
        "       VFMULAS32.M1	VR29, VR37, VR9, VR9		\t\n"
        "   |   VFMULAS32.M2	VR30, VR37, VR10, VR10		\t\n"
        "   |   VFMULAS32.M3	VR31, VR37, VR11, VR11		\t\n"
        "   |   VBALE2H         VR39, VR39, VR39            \t\n"
            // [12]
        "       VFMULAS32.M1	VR28, VR39, VR12, VR12		\t\n"
        "   |   VFMULAS32.M2	VR29, VR39, VR13, VR13		\t\n"
        "   |   VFMULAS32.M3	VR30, VR39, VR14, VR14		\t\n"
        "   |   VBALE2H         VR41, VR41, VR41            \t\n"
        "   |   VLDDW 		    *AR5++[OR2], VR25:VR24		;; load mat_C[4][0,1]   \t\n"
        "   |   VLDDW           *+AR5[16], VR27:VR26        ;; load mat_C[4][2,3]   \t\n"
            // [13]
        "       VFMULAS32.M1	VR31, VR39, VR15, VR15		\t\n"
        "   |   VFMULAS32.M2	VR28, VR41, VR16, VR16		\t\n"
        "   |   VFMULAS32.M3	VR29, VR41, VR17, VR17		\t\n"
        "   |   VBALE2H         VR43, VR43, VR43            \t\n"
        "   |   VLDDW 		    *AR5++[OR2], VR33:VR32		;; load mat_C[5][0,1]   \t\n"
        "   |   VLDDW           *+AR5[16], VR35:VR34        ;; load mat_C[5][2,3]   \t\n"
            // [14]
        "       VFMULAS32.M1	VR30, VR41, VR18, VR18		\t\n"
        "   |   VFMULAS32.M2	VR31, VR41, VR19, VR19		\t\n"
        "   |   VFMULAS32.M3	VR28, VR43, VR20, VR20		\t\n"
            // [15]
        "       VFMULAS32.M1	VR29, VR43, VR21, VR21		\t\n"
        "   |   VFMULAS32.M2	VR30, VR43, VR22, VR22		\t\n"
        "   |   VFMULAS32.M3	VR31, VR43, VR23, VR23		\t\n"
//------------------------------------------ main loop end ------------------------------------------
        // write-back:
            // [0]
	    "   	VFMULAS32.M1	VR0, VR63, VR44, VR0        \t\n"
	    "   |	VFMULAS32.M2	VR1, VR63, VR45, VR1        \t\n"
	    "   |	VFMULAS32.M3	VR2, VR63, VR46, VR2        \t\n"
            // [1]
	    "   	VFMULAS32.M1	VR3, VR63, VR47, VR3        \t\n"
	    "   |	VFMULAS32.M2	VR4, VR63, VR48, VR4        \t\n"
	    "   |	VFMULAS32.M3	VR5, VR63, VR49, VR5        \t\n"
	        // [2]
	    "   	VFMULAS32.M1	VR6, VR63, VR50, VR6        \t\n"
	    "   |	VFMULAS32.M2	VR7, VR63, VR51, VR7        \t\n"
	    "   |	VFMULAS32.M3	VR8, VR63, VR52, VR8        \t\n"
	        // [3]
	    "   	VFMULAS32.M1	VR9, VR63, VR53, VR9        \t\n"
	    "   |	VFMULAS32.M2	VR10, VR63, VR54, VR10      \t\n"
	    "   |	VFMULAS32.M3	VR11, VR63, VR55, VR11      \t\n"
	        // [4]
	    "   	VFMULAS32.M1	VR12, VR63, VR56, VR12      \t\n"
	    "   |	VFMULAS32.M2	VR13, VR63, VR57, VR13      \t\n"
	    "   |	VFMULAS32.M3	VR14, VR63, VR58, VR14      \t\n"
	        // [5]
	    "   	VFMULAS32.M1	VR15, VR63, VR59, VR15      \t\n"
	    "   |	VFMULAS32.M2	VR16, VR63, VR24, VR16      \t\n"
	    "   |	VFMULAS32.M3	VR17, VR63, VR25, VR17      \t\n"
            // [6]
	    "   	VFMULAS32.M1	VR18, VR63, VR26, VR18      \t\n"
	    "   |	VFMULAS32.M2	VR19, VR63, VR27, VR19      \t\n"
	    "   |	VFMULAS32.M3	VR20, VR63, VR32, VR20      \t\n"
            // [7]
        "   	VFMULAS32.M1	VR21, VR63, VR33, VR21      \t\n"
	    "   |	VFMULAS32.M2	VR22, VR63, VR34, VR22      \t\n"
	    "   |	VFMULAS32.M3	VR23, VR63, VR35, VR23      \t\n"
        "   |   VSTDW           VR1:VR0, *AR6++[OR2]        \r\n"
        "   |   VSTDW           VR3:VR2, *+AR6[16]          \r\n"
        "   |   SBR		        R63                         \t\n"
            // [8]
        "       VSTDW           VR5:VR4, *AR6++[OR2]        \r\n"
        "   |   VSTDW           VR7:VR6, *+AR6[16]          \r\n"
            // [9]
        "       VSTDW           VR9:VR8, *AR6++[OR2]        \r\n"
        "   |   VSTDW           VR11:VR10, *+AR6[16]        \r\n"
            // [10]
        "       SNOP            1       \t\n"
            // [11]
        "       VSTDW           VR13:VR12, *AR6++[OR2]      \r\n"
        "   |   VSTDW           VR15:VR14, *+AR6[16]        \r\n"
            // [12] 
        "       VSTDW           VR17:VR16, *AR6++[OR2]      \r\n"
        "   |   VSTDW           VR19:VR18, *+AR6[16]        \r\n"
            // [13]     
        "       VSTDW           VR21:VR20, *AR6++[OR2]      \r\n"
        "   |   VSTDW           VR23:VR22, *+AR6[16]        \r\n"
    :
    :"r"(src_a), "r"(src_b), "r"(dst_c), "r"(K_data), "r"(K_buffer), "r"(N_buffer), "r"(alpha)
    );
}

void micro_kernel_asm_r6c128_multiAlpha_only_write(float* src_a, lvector float* src_b, lvector float* dst_c, 
                    const indexType K_data, const indexType K_buffer, const indexType N_buffer, const float alpha){
    __asm__ __volatile__(
            // [-2]
        "       SSHFLR          1, %4, R7                   ;; R7 = K_buffer/2  // A矩阵一行的字数   \t\n"
	    "   |   SMVAGA.M1	    %0, AR10   			        ;; AR10 = src_A                         \t\n"
	    "   |   SMVAGA.M2       %4, OR9                     ;; OR9 = A矩阵两行的字数: K_buffer       \t\n"
            // [-1]
        "       SSHFLR          2, %5, R42                  ;; B/C_buffer一行多少个双字              \t\n"
        "   |   SADD.M1         R7, %4, R9                  ;; A矩阵3行的字数: 1.5*K_buffer          \t\n"
        "   |   SMVAGA.M2       R7, OR8                     ;; OR8 = A矩阵一行的字数: K_buffer/2     \t\n"
            // [0]
        "       SSHFLL          3, R9, R9                   ;; A矩阵3行的字节数, R9=8*(1.5*K_buffer) \t\n"
	    "   |   SMVAGA.M1       R42, OR2                    ;; B/C_buffer一行多少个双字              \t\n"
        "   |   SMVAGA.M2	    %1, AR4    			        ;; AR7 = src_B                          \t\n"
        "   |   SLDW            *AR10, R51  	            ;; load nextA[0]                        \t\n"
            // [1]
        "       SSHFLL		    2, %3, R21				    ;; R20 = K_data * 4 // A矩阵一行的字节数 \t\n"
        "   |   SADD.M1         R9, %0, R9                  ;; R9 = src_A + 8*(1.5*K_buffer)        \t\n"
        "   |   SADDA.M2        8, AR10, AR10                                                       \t\n"
        "   |   SLDW            *+AR10[OR8], R53            ;; load nextA[1]                        \t\n"
            // [2]
        "       SMVAGA.M1	    R9, AR13				    ;; AR13 = src_A + 8*(1.5*K_buffer)      \t\n"
        "   |   SADD.M2		    R21, %0, R21			    ;; R21 = src_A + K_data * 4             \t\n"
        "   |   SLDW            *+AR10[OR9], R55            ;; load nextA[2]                        \t\n"
            // [3]
        "       SMVAAG.M2	    AR10, %0		                                                    \t\n"
        "   |   VMOVI.M1 	    0x00, VR0				    ;; 初始化C寄存器                         \t\n"
        "   |   VMOVI.M2 	    0x3F8000003F800000,VR63	    ;; VR63 = 1.0                           \t\n"
            // [4]
        "       SLDW            *AR13, R57                  ;; load nextA[3]                        \t\n"
        "   |   SMVAGA.M2	    %2, AR6				        ;; AR6 = dst_C                          \t\n"
        "   |   VMOV.M2		    VR0, VR1    \t\n"
        "   |   VMOV.M3		    VR0, VR2    \t\n"
            // [5]
        "       SADDA.M1        8, AR13, AR13                                                       \t\n"
        "   |   SLDW            *+AR13[OR8], R59    ;; load nextA[4]                                \t\n"
        "   |   VMOV.M1		    VR0, VR3    \t\n"
        "   |   VMOV.M2		    VR0, VR4    \t\n"
        "   |   VMOV.M3		    VR0, VR5    \t\n"
            // [6]
        "       SLDW            *+AR13[OR9], R61    ;; load nextA[5]                                \t\n"
        "   |   SBALE2          %6, %6, %6              \t\n"
        "   |   VMOV.M1		    VR0, VR6    \t\n"
        "   |   VMOV.M2		    VR0, VR7    \t\n"
        "   |   VMOV.M3		    VR0, VR8    \t\n"
            // [7]
        "       SFMULS32.M1     %6, R51, R51            \t\n"
        "   |   VLDDW 		    *AR4++[OR2], VR25:VR24		                                        \t\n"
        "   |   VLDDW 		    *+AR4[16], VR27:VR26		                                        \t\n"
            // [8]
        "       SFMULS32.M1     %6, R53, R53            \t\n"
            // [9]
        "       SFMULS32.M1     %6, R55, R55            \t\n"
            // [10]
        "       SNOP            1                       \t\n"
            // [11]
        "       SFMULS32.M1     %6, R57, R57            \t\n"
        "   |   SVBCAST.M2      R51, VR33   ;; bcast nextA[0]                                       \t\n"
        "   |   VMOV.M1		    VR0, VR9    \t\n"
        "   |   VMOV.M2		    VR0, VR10   \t\n"
        "   |   VMOV.M3		    VR0, VR11   \t\n"
            // [12]
        "       SFMULS32.M1     %6, R59, R59             \t\n"
        "   |   SVBCAST.M2      R53, VR35   ;; bcast nextA[1]                                       \t\n"
        "   |   VMOV.M1		    VR0, VR12   \t\n"
        "   |   VMOV.M2		    VR0, VR13   \t\n"
        "   |   VMOV.M3		    VR0, VR14   \t\n"
            // [13]
        "       SFMULS32.M1     %6, R61, R61             \t\n"
        "   |   SVBCAST.M2      R55, VR37   ;; bcast nextA[2]                                       \t\n"
        "   |   VMOV.M1		    VR0, VR15   \t\n"
        "   |   VMOV.M2		    VR0, VR16   \t\n"
        "   |   VMOV.M3		    VR0, VR17   \t\n"
            // [14]
        "       VMOV.M1		    VR0, VR18   \t\n"
        "   |   VMOV.M2		    VR0, VR19   \t\n"
        "   |   VMOV.M3		    VR0, VR20   \t\n"
            // [15]
        "       SVBCAST.M2      R57, VR39   ;; bcast nextA[3]                                       \t\n"
        "   |   VBALE2          VR33, VR33, VR32                                                    \t\n"
        "   |   VLDDW 		    *AR4++[OR2], VR29:VR28		                                        \t\n"
        "   |   VLDDW 		    *+AR4[16], VR31:VR30		                                        \t\n"
        "   |   VMOV.M1		    VR0, VR21   \t\n"
        "   |   VMOV.M2		    VR0, VR22   \t\n"
        "   |   VMOV.M3		    VR0, VR23   \t\n"

        "loop_k_r6c128_multiAlpha_only_write:       \t\n"
//------------------------------------------ main loop start ------------------------------------------
            // [0]
        "       VFMULAS32.M1	VR24, VR32, VR0, VR0		\t\n"
        "   |   VFMULAS32.M2	VR25, VR32, VR1, VR1		\t\n"
        "   |   VFMULAS32.M3	VR26, VR32, VR2, VR2		\t\n"
        "   |   SLDW            *AR10, R51  	    ;; load nextA[0]    \t\n"
        "   |   SVBCAST.M2      R59, VR41   ;; bcast A[4]   \t\n"
        "   |   VBALE2          VR35, VR35, VR34            \t\n"     
            // [1]
        "   	SLT		        %0, R21, R0                 \t\n"
        "   |   VFMULAS32.M1	VR27, VR32, VR3, VR3		\t\n"
        "   |   VFMULAS32.M2	VR24, VR34, VR4, VR4		\t\n"
        "   |   VFMULAS32.M3	VR25, VR34, VR5, VR5		\t\n"
        "   |   SLDW            *+AR10[OR8], R53    ;; load nextA[1]    \t\n"
        "   |   SVBCAST.M2      R61, VR43   ;; bcast A[5]   \t\n"
        "   |   VBALE2          VR37, VR37, VR36            \t\n"
            // [2]
        "       VFMULAS32.M1	VR26, VR34, VR6, VR6		\t\n"
        "   |   VFMULAS32.M2	VR27, VR34, VR7, VR7		\t\n"
        "   |   VFMULAS32.M3	VR24, VR36, VR8, VR8		\t\n"
        "   |   SLDW            *+AR10[OR9], R55    ;; load nextA[2]    \t\n"
            // [3]
        "       VFMULAS32.M1	VR25, VR36, VR9, VR9		\t\n"
        "   |   VFMULAS32.M2	VR26, VR36, VR10, VR10		\t\n"
        "   |   VFMULAS32.M3	VR27, VR36, VR11, VR11		\t\n"
        "   |   VBALE2          VR39, VR39, VR38            \t\n"
            // [4]
        "       VFMULAS32.M1	VR24, VR38, VR12, VR12		\t\n"
        "   |   VFMULAS32.M2	VR25, VR38, VR13, VR13		\t\n"
        "   |   VFMULAS32.M3	VR26, VR38, VR14, VR14		\t\n"
        "   |   SLDW            *AR13, R57          ;; load nextA[3]    \t\n"
        "   |   VBALE2          VR41, VR41, VR40            \t\n"
            // [5]
        "       SADDA.M1        8, AR10, AR10               \t\n"
        "   |   VFMULAS32.M1	VR27, VR38, VR15, VR15		\t\n"
        "   |   VFMULAS32.M2	VR24, VR40, VR16, VR16		\t\n"
        "   |   VFMULAS32.M3	VR25, VR40, VR17, VR17		\t\n"
        "   |   SLDW            *+AR13[OR8], R59    ;; load nextA[4]    \t\n"
        "   |   VBALE2          VR43, VR43, VR42            \t\n"
            // [6]
        "       VFMULAS32.M1	VR26, VR40, VR18, VR18		\t\n"
        "   |   VFMULAS32.M2	VR27, VR40, VR19, VR19		\t\n"
        "   |   VFMULAS32.M3	VR24, VR42, VR20, VR20		\t\n"
        "   |   SLDW            *+AR13[OR9], R61    ;; load nextA[5]    \t\n"
            // [7]
        "       SMVAAG.M1	    AR10, %0		            \t\n"
        "   |   VFMULAS32.M1	VR25, VR42, VR21, VR21		\t\n"
        "   |   VFMULAS32.M2	VR26, VR42, VR22, VR22		\t\n"
        "   |   VFMULAS32.M3	VR27, VR42, VR23, VR23		\t\n"
        "   |   SFMULS32.M2     %6, R51, R51                \t\n"
        "   |   VBALE2H         VR33, VR33, VR33            \t\n"
        "   |   VLDDW 		    *AR4++[OR2], VR25:VR24		\t\n"
        "   |   VLDDW 		    *+AR4[16], VR27:VR26		\t\n"
            // [8]
        "       VFMULAS32.M1	VR28, VR33, VR0, VR0		\t\n"
        "   |   VFMULAS32.M2	VR29, VR33, VR1, VR1		\t\n"
        "   |   VFMULAS32.M3	VR30, VR33, VR2, VR2		\t\n"
        "   |   SFMULS32.M2     %6, R53, R53                \t\n"
        "   |   VBALE2H         VR35, VR35, VR35            \t\n"
            // [9]
        "       SADDA.M1        8, AR13, AR13               \t\n"
        "   |   VFMULAS32.M1	VR31, VR33, VR3, VR3		\t\n"
        "   |   VFMULAS32.M2	VR28, VR35, VR4, VR4		\t\n"
        "   |   VFMULAS32.M3	VR29, VR35, VR5, VR5		\t\n"
        "   |   SFMULS32.M2     %6, R55, R55                \t\n"
        "   |   VBALE2H         VR37, VR37, VR37            \t\n"
        "   |   [R0]	SBR		loop_k_r6c128_multiAlpha_only_write               \t\n"
            // [10]
        "       VFMULAS32.M1	VR30, VR35, VR6, VR6		\t\n"
        "   |   VFMULAS32.M2	VR31, VR35, VR7, VR7		\t\n"
        "   |   VFMULAS32.M3	VR28, VR37, VR8, VR8		\t\n"
            // [11]
        "       VFMULAS32.M1	VR29, VR37, VR9, VR9		\t\n"
        "   |   VFMULAS32.M2	VR30, VR37, VR10, VR10		\t\n"
        "   |   VFMULAS32.M3	VR31, VR37, VR11, VR11		\t\n"
        "   |   SFMULS32.M1     %6, R57, R57                \t\n"
        "   |   SVBCAST.M2      R51, VR33   ;; bcast nextA[0]   \t\n"
        "   |   VBALE2H         VR39, VR39, VR39            \t\n"
            // [12]
        "       VFMULAS32.M1	VR28, VR39, VR12, VR12		\t\n"
        "   |   VFMULAS32.M2	VR29, VR39, VR13, VR13		\t\n"
        "   |   VFMULAS32.M3	VR30, VR39, VR14, VR14		\t\n"
        "   |   SFMULS32.M1     %6, R59, R59                \t\n"
        "   |   SVBCAST.M2      R53, VR35   ;; bcast nextA[1]   \t\n"
        "   |   VBALE2H         VR41, VR41, VR41            \t\n"
            // [13]
        "       VFMULAS32.M1	VR31, VR39, VR15, VR15		\t\n"
        "   |   VFMULAS32.M2	VR28, VR41, VR16, VR16		\t\n"
        "   |   VFMULAS32.M3	VR29, VR41, VR17, VR17		\t\n"
        "   |   SFMULS32.M1     %6, R61, R61                \t\n"
        "   |   SVBCAST.M2      R55, VR37   ;; bcast nextA[2]   \t\n"
        "   |   VBALE2H         VR43, VR43, VR43            \t\n"
            // [14]
        "       VFMULAS32.M1	VR30, VR41, VR18, VR18		\t\n"
        "   |   VFMULAS32.M2	VR31, VR41, VR19, VR19		\t\n"
        "   |   VFMULAS32.M3	VR28, VR43, VR20, VR20		\t\n"
            // [15]
        "       VFMULAS32.M1	VR29, VR43, VR21, VR21		\t\n"
        "   |   VFMULAS32.M2	VR30, VR43, VR22, VR22		\t\n"
        "   |   VFMULAS32.M3	VR31, VR43, VR23, VR23		\t\n"
        "   |   SVBCAST.M2      R57, VR39   ;; bcast nextA[3]   \t\n"
        "   |   VBALE2          VR33, VR33, VR32            \t\n"
        "   |   VLDDW 		    *AR4++[OR2], VR29:VR28		\t\n"
        "   |   VLDDW 		    *+AR4[16], VR31:VR30		\t\n"
        "   |   [!R0]   SBR     R63                         \t\n"
//------------------------------------------ last iteration ------------------------------------------
        // write-back:
            // [0]
        "       VSTDW           VR1:VR0, *AR6++[OR2]        \r\n"
        "   |   VSTDW           VR3:VR2, *+AR6[16]          \r\n"
            // [1]
        "       VSTDW           VR5:VR4, *AR6++[OR2]        \r\n"
        "   |   VSTDW           VR7:VR6, *+AR6[16]          \r\n"
            // [2]
        "       VSTDW           VR9:VR8, *AR6++[OR2]        \r\n"
        "   |   VSTDW           VR11:VR10, *+AR6[16]        \r\n"
            // [3]
        "       VSTDW           VR13:VR12, *AR6++[OR2]      \r\n"
        "   |   VSTDW           VR15:VR14, *+AR6[16]        \r\n"
            // [4] 
        "       VSTDW           VR17:VR16, *AR6++[OR2]      \r\n"
        "   |   VSTDW           VR19:VR18, *+AR6[16]        \r\n"
            // [5]     
        "       VSTDW           VR21:VR20, *AR6++[OR2]      \r\n"
        "   |   VSTDW           VR23:VR22, *+AR6[16]        \r\n"
    :
    :"r"(src_a), "r"(src_b), "r"(dst_c), "r"(K_data), "r"(K_buffer), "r"(N_buffer), "r"(alpha)
    );
}
#endif

#endif


__global__ void sgemm_NN_qxx(unlong M, unlong N, unlong K, unlong b_id, float* A, float* B, float* C, float alpha, float beta){
    const int tid = get_thread_id();
    const int grp_size = get_group_size();
    // ------------------ blocking params ------------------
#ifdef use_r6c96
    const indexType M_G = 960;
    const indexType K_G = 512;
    const indexType N_A = 96;
    const indexType M_A = 240;
    const indexType M_S = 12;
#endif
#ifdef use_r6c128
    const indexType M_G = 576;
    const indexType K_G = 512;
    const indexType N_A = 128;
    const indexType M_A = 144;
    const indexType M_S = 12;
#endif
    const indexType MK_G = M_G * K_G;
    const indexType M_S_half = M_S>>1;
    // ------------------ blocking params ------------------
    const indexType c_start = tid * N_A;
    const indexType c_step = grp_size * N_A;
    bool row_syn = false;
    unint p2pmask_most = 0x00;
    unint p2pmask_last = 0x00;
    if(grp_size != 1){
        row_syn = true;
        long N_last = N % c_step;
        if(N_last==0)
            N_last = c_step;
        int ncore_last = (N_last + N_A - 1) / N_A;
        int i=0;
        for(; i<ncore_last; i++){
            p2pmask_most <<= 1;
            p2pmask_most |= 1;
        }
        p2pmask_last = p2pmask_most;
        if(N != N_last){
            for(; i<grp_size; i++){
                p2pmask_most <<= 1;
                p2pmask_most |= 1;
            }
        }
    }
    unint p2pmask_cur = p2pmask_most;

    float* spm_A[2];
    lvector float* spm_B[2];
    lvector float* spm_C[3];
    spm_A[0] = scalar_malloc(M_S * K_G * sizeof(float));
    spm_A[1] = scalar_malloc(M_S * K_G * sizeof(float));
    spm_B[0] = vector_malloc(K_G * N_A * sizeof(float));
    spm_B[1] = vector_malloc(K_G * N_A * sizeof(float));
    spm_C[0] = vector_malloc(M_A * N_A * sizeof(float));
    spm_C[1] = vector_malloc(M_A * N_A * sizeof(float));
    spm_C[2] = vector_malloc(M_A * N_A * sizeof(float));

    int ch_gsm[2], ch_al[2], ch_bl[2], ch_cl[3], ch_cs[3];

    const int ch0_gsm = 8;
    const int ch0_bl = 2;
    const int ch0_cl = 4; // high level: 4, 5
    const int ch0_al = 0; // high level: 0, 1 
    const int ch0_cs = 6; // high level: 6, 7

    unlong prir = 0b0000000011;
    set_prir(prir);

    long M_G_cur = min(M_G, M);
    long K_G_cur = min(K_G, K);
    long N_A_cur = min(N_A, N-c_start);
    long M_A_cur = min(M_A, M);
    long M_G_next, K_G_next, N_A_next, M_A_next;
    
    // preload A to gsm_mem
    if(tid==0){
        ch_gsm[0] = dma_p2p_opt(&A[OFFSET(0, 0, K)], M_G_cur, K_G_cur*sizeof(float), (K-K_G_cur)*sizeof(float),
                            gsm_mem, M_G_cur, K_G_cur*sizeof(float), (K_G-K_G_cur)*sizeof(float), 0, 0, ch0_gsm);
    }
    if(c_start < N){
        // preload spm_B
        ch_bl[0] = dma_p2p_opt(&B[OFFSET(0, c_start, N)], K_G_cur, N_A_cur*sizeof(float), (N-N_A_cur)*sizeof(float),
                            spm_B[0], K_G_cur, N_A_cur*sizeof(float), (N_A-N_A_cur)*sizeof(float), row_syn, p2pmask_cur, ch0_bl);
        // preload spm_C
        ch_cl[0] = dma_p2p_opt(&C[OFFSET(0, c_start, N)], M_A_cur, N_A_cur*sizeof(float), (N-N_A_cur)*sizeof(float),
                            spm_C[0], M_A_cur, N_A_cur*sizeof(float), (N_A-N_A_cur)*sizeof(float), row_syn, p2pmask_cur, ch0_cl);
        // prestore spm_C (启动第一次无效传输，为了配合第一次wait)
        ch_cs[2] = dma_p2p_opt(&C[0], 1, 1*sizeof(double), 0,
                            spm_C[2], 1, 1*sizeof(double), 0, 0, 0, 7);
    }

    int cnt_gsm = 0;
    int cnt_b = 0;
    int cnt_c = 0, cnt_d = 0;
    for(long k=0; k<K; k+=K_G){
        K_G_next = K_G_cur;
            for(long r=0; r<M; r+=M_G){
            int cnt_gsm_1 = (cnt_gsm + 1) % 2;

            int k_next, r_next;
            if(r + M_G < M){
                k_next = k;
                r_next = r + M_G;
            }else if(k + K_G < K){
                k_next = k + K_G;
                r_next = 0;
            }else{
                r_next = -1;
            }

            if(tid==0){
                // preload A_next to gsm_mem
                if(r + M_G < M){ // not column end
                    M_G_next = min(M_G, M-(r+M_G));
                    ch_gsm[cnt_gsm_1] = dma_p2p_opt(&A[OFFSET(r_next, k, K)], M_G_next, K_G_next*sizeof(float), (K-K_G_next)*sizeof(float),
                                            gsm_mem + MK_G*cnt_gsm_1, M_G_next, K_G_next*sizeof(float), (K_G-K_G_next)*sizeof(float), 0, 0, ch0_gsm+cnt_gsm_1);
                }else if(k + K_G < K){ // r + M_G >= M
                    M_G_next = min(M_G, M);
                    K_G_next = min(K_G, K-(k+K_G));
                    ch_gsm[cnt_gsm_1] = dma_p2p_opt(&A[OFFSET(0, k_next, K)], M_G_next, K_G_next*sizeof(float), (K-K_G_next)*sizeof(float),
                                            gsm_mem + MK_G*cnt_gsm_1, M_G_next, K_G_next*sizeof(float), (K_G-K_G_next)*sizeof(float), 0, 0, ch0_gsm+cnt_gsm_1);
                }
                // wait for A_cur loaded to gsm_mem
                dma_wait_p2p(ch_gsm[cnt_gsm]);
            } else{
                if(r + M_G < M){
                    M_G_next = min(M_G, M-(r+M_G));
                }else if(k + K_G < K){
                    M_G_next = min(M_G, M);
                    K_G_next = min(K_G, K-(k+K_G));
                }
            }
            group_barrier(b_id);

            // preload gsm_mem to sm_A
            if(c_start<N){
                ch_al[0] = dma_p2p_opt(gsm_mem + MK_G*cnt_gsm, M_S, K_G_cur*sizeof(float), (K_G-K_G_cur)*sizeof(float),
                                    spm_A[0], M_S, K_G_cur*sizeof(float), (K_G-K_G_cur)*sizeof(float), 0, 0, ch0_al);
            }
            unint cnt_a = 0;

            long c_tail = c_step;
            for(long c=c_start; c<N; c+=c_step){ 
                unint p2pmask_next;
                if(c+c_step < N && c_tail+c_step >= N){ // current is not the last, next is the last
                    p2pmask_next = p2pmask_last;
                }else{
                    p2pmask_next = p2pmask_most;
                }
                c_tail += c_step;

                int cnt_b_1 = (cnt_b + 1) % 2;

                int c_next;
                if(c + c_step < N){ // c没到头
                    c_next = c + c_step;
                }else if(r_next == -1){ // c到头了，r,k也到头了
                    c_next = -1;
                }else{ // c到头了，r,k还没到头
                    c_next = c_start;
                }

                // load spm_B[cnt_b_1]
                if(c + c_step < N){ // c没到头
                    N_A_next = min(N_A, N-(c+c_step));
                    ch_bl[cnt_b_1] = dma_p2p_opt(&B[OFFSET(k, c+c_step, N)], K_G_cur, N_A_next*sizeof(float), (N-N_A_next)*sizeof(float),
                                                    spm_B[cnt_b_1], K_G_cur, N_A_next*sizeof(float), (N_A-N_A_next)*sizeof(float), row_syn, p2pmask_next, ch0_bl+cnt_b_1);
                }else if(r_next != -1){ // c到头了，r,k还没到头
                    N_A_next = min(N_A, N-c_start);
                    ch_bl[cnt_b_1] = dma_p2p_opt(&B[OFFSET(k_next, c_start, N)], K_G_next, N_A_next*sizeof(float), (N-N_A_next)*sizeof(float),
                                                    spm_B[cnt_b_1], K_G_next, N_A_next*sizeof(float), (N_A-N_A_next)*sizeof(float), row_syn, p2pmask_next, ch0_bl+cnt_b_1);
                }

                // wait for spm_B[cnt_b]
                dma_wait_p2p(ch_bl[cnt_b]);  

                for(int rc=0; rc<M_G_cur; rc+=M_A){
                    // compute C index
                    unint cnt_d_1 = (cnt_d + 1) % 2;
                    unint cnt_c_1 = cnt_c + 1;
                    unint cnt_c_2 = cnt_c + 2;
                    if(cnt_c_1 > 2) cnt_c_1 -= 3;
                    if(cnt_c_2 > 2) cnt_c_2 -= 3;

                    // preload C to am_C, gsm_mem to sm_A
                    if(rc + M_A < M_G_cur){ // rc没到头
                        M_A_next = min(M_A, M_G_cur-(rc + M_A));
                        ch_cl[cnt_c_1] = dma_p2p_opt(&C[OFFSET(r+(rc+M_A), c, N)], M_A_next, N_A_cur*sizeof(float), (N-N_A_cur)*sizeof(float),
                                            spm_C[cnt_c_1], M_A_next, N_A_cur*sizeof(float), (N_A-N_A_cur)*sizeof(float), row_syn, p2pmask_cur, ch0_cl+cnt_d_1);
                    }else if(c_next == c_start){ // rc到头了，c也到头了，r,k还没到头
                        M_A_next = min(M_A, M_G_next);
                        ch_cl[cnt_c_1] = dma_p2p_opt(&C[OFFSET(r_next, c_next, N)], M_A_next, N_A_next*sizeof(float), (N-N_A_next)*sizeof(float),
                                            spm_C[cnt_c_1], M_A_next, N_A_next*sizeof(float), (N_A-N_A_next)*sizeof(float), row_syn, p2pmask_next, ch0_cl+cnt_d_1);
                    }else if(c_next != -1){ // rc到头了，c没到头
                        M_A_next = min(M_A, M_G_cur);
                        ch_cl[cnt_c_1] = dma_p2p_opt(&C[OFFSET(r, c_next, N)], M_A_next, N_A_next*sizeof(float), (N-N_A_next)*sizeof(float),
                                            spm_C[cnt_c_1], M_A_next, N_A_next*sizeof(float), (N_A-N_A_next)*sizeof(float), row_syn, p2pmask_next, ch0_cl+cnt_d_1);
                    }

                    // wait for am_C_cur
                    dma_wait_p2p(ch_cl[cnt_c]);

#ifdef unroll_inner_loop             
                    for(int ra=0; ra<M_A_cur; ra+=2*M_S){
                        ch_al[1] = dma_p2p_opt(gsm_mem + MK_G*cnt_gsm + (rc+ra+M_S)*K_G, M_S, K_G_cur*sizeof(float), (K_G-K_G_cur)*sizeof(float),
                                            spm_A[1], M_S, K_G_cur*sizeof(float), (K_G-K_G_cur)*sizeof(float), 0, 0, ch0_al+1);
                        dma_wait_p2p(ch_al[0]);
                #ifdef use_r6c96
                        micro_kernel_asm_r6c96(spm_A[0], spm_B[cnt_b], spm_C[cnt_c] + ra*(N_A>>5), K_G_cur, K_G, N_A);
                        micro_kernel_asm_r6c96(spm_A[0] + M_S_half*K_G, spm_B[cnt_b], spm_C[cnt_c] + (ra + M_S_half)*(N_A>>5), K_G_cur, K_G, N_A);
                #endif
                #ifdef use_r6c128
                    #ifndef enable_alpha
                        micro_kernel_asm_r6c128(spm_A[0], spm_B[cnt_b], spm_C[cnt_c] + ra*(N_A>>5), K_G_cur, K_G, N_A);
                        micro_kernel_asm_r6c128(spm_A[0] + M_S_half*K_G, spm_B[cnt_b], spm_C[cnt_c] + (ra + M_S_half)*(N_A>>5), K_G_cur, K_G, N_A);
                    #else
                        micro_kernel_asm_r6c128_multiAlpha(spm_A[0], spm_B[cnt_b], spm_C[cnt_c] + ra*(N_A>>5), K_G_cur, K_G, N_A, alpha);
                        micro_kernel_asm_r6c128_multiAlpha(spm_A[0] + M_S_half*K_G, spm_B[cnt_b], spm_C[cnt_c] + (ra + M_S_half)*(N_A>>5), K_G_cur, K_G, N_A, alpha);
                    #endif
                #endif

                        if(ra + 2*M_S < M_A_cur){ // ra没到头
                            ch_al[0] = dma_p2p_opt(gsm_mem + MK_G*cnt_gsm + (rc+ra+2*M_S)*K_G, M_S, K_G_cur*sizeof(float), (K_G-K_G_cur)*sizeof(float),
                                                    spm_A[0], M_S, K_G_cur*sizeof(float), (K_G-K_G_cur)*sizeof(float), 0, 0, ch0_al+0);
                        }else if(rc + M_A < M_G_cur){ // ra到头了，rc没到头
                            ch_al[0] = dma_p2p_opt(gsm_mem + MK_G*cnt_gsm + (rc+M_A)*K_G, M_S, K_G_cur*sizeof(float), (K_G-K_G_cur)*sizeof(float),
                                                    spm_A[0], M_S, K_G_cur*sizeof(float), (K_G-K_G_cur)*sizeof(float), 0, 0, ch0_al+0);
                        }else if(c_next > c_start){ // ra到头了, rc到头了，c没到头
                            ch_al[0] = dma_p2p_opt(gsm_mem + MK_G*cnt_gsm, M_S, K_G_cur*sizeof(float), (K_G-K_G_cur)*sizeof(float),
                                                    spm_A[0], M_S, K_G_cur*sizeof(float), (K_G-K_G_cur)*sizeof(float), 0, 0, ch0_al+0);
                        }
                        dma_wait_p2p(ch_al[1]);
                #ifdef use_r6c96
                        micro_kernel_asm_r6c96(spm_A[1], spm_B[cnt_b], spm_C[cnt_c] + (ra + M_S)*(N_A>>5), K_G_cur, K_G, N_A);
                        micro_kernel_asm_r6c96(spm_A[1] + M_S_half*K_G, spm_B[cnt_b], spm_C[cnt_c] + (ra + M_S + M_S_half)*(N_A>>5), K_G_cur, K_G, N_A);
                #endif
                #ifdef use_r6c128
                    #ifndef enable_alpha
                        micro_kernel_asm_r6c128(spm_A[1], spm_B[cnt_b], spm_C[cnt_c] + (ra + M_S)*(N_A>>5), K_G_cur, K_G, N_A);
                        micro_kernel_asm_r6c128(spm_A[1] + M_S_half*K_G, spm_B[cnt_b], spm_C[cnt_c] + (ra + M_S + M_S_half)*(N_A>>5), K_G_cur, K_G, N_A);
                    #else
                        micro_kernel_asm_r6c128_multiAlpha(spm_A[1], spm_B[cnt_b], spm_C[cnt_c] + (ra + M_S)*(N_A>>5), K_G_cur, K_G, N_A, alpha);
                        micro_kernel_asm_r6c128_multiAlpha(spm_A[1] + M_S_half*K_G, spm_B[cnt_b], spm_C[cnt_c] + (ra + M_S + M_S_half)*(N_A>>5), K_G_cur, K_G, N_A, alpha);
                    #endif
                #endif
                    }
#else
                    for(int ra=0; ra<M_A_cur; ra+=M_S){
                        int cnt_a_1 = (cnt_a + 1) % 2;
                        
                        // preload A to sm
                        if(ra + M_S < M_A_cur){ // ra没到头
                            ch_al[cnt_a_1] = dma_p2p_opt(gsm_mem + MK_G*cnt_gsm + (rc+ra+M_S)*K_G, M_S, K_G_cur*sizeof(float), (K_G-K_G_cur)*sizeof(float),
                                                    spm_A[cnt_a_1], M_S, K_G_cur*sizeof(float), (K_G-K_G_cur)*sizeof(float), 0, 0, ch0_al+cnt_a_1);
                        }else if(rc + M_A < M_G_cur){ // ra到头了，rc没到头
                            ch_al[cnt_a_1] = dma_p2p_opt(gsm_mem + MK_G*cnt_gsm + (rc+M_A)*K_G, M_S, K_G_cur*sizeof(float), (K_G-K_G_cur)*sizeof(float),
                                                    spm_A[cnt_a_1], M_S, K_G_cur*sizeof(float), (K_G-K_G_cur)*sizeof(float), 0, 0, ch0_al+cnt_a_1);
                        }else if(c_next > c_start){ // ra到头了, rc到头了，c没到头
                            ch_al[cnt_a_1] = dma_p2p_opt(gsm_mem + MK_G*cnt_gsm, M_S, K_G_cur*sizeof(float), (K_G-K_G_cur)*sizeof(float),
                                                    spm_A[cnt_a_1], M_S, K_G_cur*sizeof(float), (K_G-K_G_cur)*sizeof(float), 0, 0, ch0_al+cnt_a_1);
                        }
                        dma_wait_p2p(ch_al[cnt_a]);

                #ifdef use_r6c96
                        micro_kernel_asm_r6c96(spm_A[cnt_a], spm_B[cnt_b], spm_C[cnt_c] + ra*(N_A>>5), K_G_cur, K_G, N_A);
                        micro_kernel_asm_r6c96(spm_A[cnt_a] + M_S_half*K_G, spm_B[cnt_b], spm_C[cnt_c] + (ra + M_S_half)*(N_A>>5), K_G_cur, K_G, N_A);
                #endif
                #ifdef use_r6c128
                    #ifndef enable_alpha
                        micro_kernel_asm_r6c128(spm_A[cnt_a], spm_B[cnt_b], spm_C[cnt_c] + ra*(N_A>>5), K_G_cur, K_G, N_A);
                        micro_kernel_asm_r6c128(spm_A[cnt_a] + M_S_half*K_G, spm_B[cnt_b], spm_C[cnt_c] + (ra + M_S_half)*(N_A>>5), K_G_cur, K_G, N_A);
                    #else
                        micro_kernel_asm_r6c128_multiAlpha(spm_A[cnt_a], spm_B[cnt_b], spm_C[cnt_c] + ra*(N_A>>5), K_G_cur, K_G, N_A, alpha);
                        micro_kernel_asm_r6c128_multiAlpha(spm_A[cnt_a] + M_S_half*K_G, spm_B[cnt_b], spm_C[cnt_c] + (ra + M_S_half)*(N_A>>5), K_G_cur, K_G, N_A, alpha);
                    #endif
                #endif
                        cnt_a = cnt_a_1;
                    }
#endif
                    // last_C plus beta
                #ifdef enable_beta
                    if(k + K_G >= K){
                        C_plus_beta(spm_C[cnt_c], M_A*N_A, beta);
                    }
                #endif
                    // store spm_C to C
                    ch_cs[cnt_c] = dma_p2p_opt(spm_C[cnt_c], M_A_cur, N_A_cur*sizeof(float), (N_A-N_A_cur)*sizeof(float),
                                &C[OFFSET(r+rc, c, N)], M_A_cur, N_A_cur*sizeof(float), (N-N_A_cur)*sizeof(float), row_syn, p2pmask_cur, ch0_cs+cnt_d);

                    // wait for C stored
                    dma_wait_p2p(ch_cs[cnt_c_2]); 

                    // renew idnex C
                    cnt_d = cnt_d_1;
                    cnt_c = cnt_c_1;
                    M_A_cur = M_A_next;
                }
                cnt_b = cnt_b_1;
                p2pmask_cur = p2pmask_next;
                N_A_cur = N_A_next;
            }
            cnt_gsm = cnt_gsm_1;
            M_G_cur = M_G_next;
            group_barrier(b_id);
        }
        K_G_cur = K_G_next;
    }
    if(c_start < N){
        unint cnt_c_2 = cnt_c + 2;
        if(cnt_c_2 > 2) cnt_c_2 -= 3;
        dma_wait_p2p(ch_cs[cnt_c_2]);
    }
    scalar_free(spm_A[0]);
    scalar_free(spm_A[1]);
    vector_free(spm_B[0]);
    vector_free(spm_B[1]);
    vector_free(spm_C[0]);
    vector_free(spm_C[1]);
    vector_free(spm_C[2]);
}


__global__ void sgemm_NT_qxx(unlong M, unlong N, unlong K, unlong b_id, float* A, float* B, float* C, float alpha, float beta){
    const int tid = get_thread_id();
    const int grp_size = get_group_size();
    // -------------------------- blocking params --------------------------
#ifdef use_r6c96
    const indexType M_G = 960;
    const indexType K_G = 512;
    const indexType N_A = 96;
    const indexType M_A = 240;
    const indexType M_S = 12;
#endif
#ifdef use_r6c128
    const indexType M_G = 576;
    const indexType K_G = 512;
    const indexType N_A = 128;
    const indexType M_A = 144;
    const indexType M_S = 12;
#endif
    const indexType MK_G = M_G * K_G;
    const indexType M_S_half = M_S>>1;
    // -------------------------- blocking params --------------------------
    const indexType c_start = tid * N_A;
    const indexType c_step = grp_size * N_A;
    bool row_syn = false;
    unint p2pmask_most = 0x00;
    unint p2pmask_last = 0x00;
    if(grp_size != 1){
        row_syn = true;
        long N_last = N % c_step;
        if(N_last==0)
            N_last = c_step;
        int ncore_last = (N_last + N_A - 1) / N_A;
        int i=0;
        for(; i<ncore_last; i++){
            p2pmask_most <<= 1;
            p2pmask_most |= 1;
        }
        p2pmask_last = p2pmask_most;
        if(N != N_last){
            for(; i<grp_size; i++){
                p2pmask_most <<= 1;
                p2pmask_most |= 1;
            }
        }
    }
    unint p2pmask_cur = p2pmask_most;
    
    float* spm_A[2];
    lvector float* spm_B[2];
    lvector float* spm_C[3];
    spm_A[0] = scalar_malloc(M_S * K_G *sizeof(float));
    spm_A[1] = scalar_malloc(M_S * K_G *sizeof(float));
    spm_B[0] = vector_malloc(K_G * N_A * sizeof(float));
    spm_B[1] = vector_malloc(K_G * N_A * sizeof(float));
    spm_C[0] = vector_malloc(M_A * N_A * sizeof(float));
    spm_C[1] = vector_malloc(M_A * N_A * sizeof(float));
    spm_C[2] = vector_malloc(M_A * N_A * sizeof(float));
    lvector double* spm_D = vector_malloc(2048 * sizeof(float));

    int ch_gsm[2], ch_al[2], ch_bl, ch_cl[3], ch_cs[3];

    const int ch0_gsm = 8;
    const int ch0_bl = 2;
    const int ch0_cl = 4; // high level: 4, 5
    const int ch0_al = 0; // high level: 0, 1 
    const int ch0_cs = 6; // high level: 6, 7

    const unlong prir = 0b0000000011;
    set_prir(prir);

    long M_G_cur = min(M_G, M);
    long K_G_cur = min(K_G, K);
    long N_A_cur = min(N_A, N-c_start);
    long M_A_cur = min(M_A, M);
    long M_G_next, K_G_next, N_A_next, M_A_next;

    // preload A to gsm_mem
    if(tid==0){
        ch_gsm[0] = dma_p2p_opt(&A[OFFSET(0, 0, K)], M_G_cur, K_G_cur*sizeof(float), (K-K_G_cur)*sizeof(float),
                            gsm_mem, M_G_cur, K_G_cur*sizeof(float), (K_G-K_G_cur)*sizeof(float), 0, 0, ch0_gsm);
    }
    if(c_start < N){
        // preload spm_B
        ch_bl = dma_p2p_opt(&B[OFFSET(c_start, 0, K)], N_A_cur, K_G_cur*sizeof(float), (K-K_G_cur)*sizeof(float),
                            spm_B[0], N_A_cur, K_G_cur*sizeof(float), (K_G-K_G_cur)*sizeof(float), row_syn, p2pmask_cur, ch0_bl);
        // preload spm_C
        ch_cl[0] = dma_p2p_opt(&C[OFFSET(0, c_start, N)], M_A_cur, N_A_cur*sizeof(float), (N-N_A_cur)*sizeof(float),
                            spm_C[0], M_A_cur, N_A_cur*sizeof(float), (N_A-N_A_cur)*sizeof(float), row_syn, p2pmask_cur, ch0_cl);
        // prestore spm_C (启动第一次无效传输，为了配合第一次wait)
        ch_cs[2] = dma_p2p_opt(&C[0], 1, 1*sizeof(double), 0,
                            spm_C[2], 1, 1*sizeof(double), 0, 0, 0, 7);
    }
    int cnt_gsm = 0;
    int cnt_c = 0, cnt_d = 0;
    for(long k=0; k<K; k+=K_G){
        K_G_next = K_G_cur;
            for(long r=0; r<M; r+=M_G){
            int cnt_gsm_1 = (cnt_gsm + 1) % 2;

            int k_next, r_next;
            if(r + M_G < M){
                k_next = k;
                r_next = r + M_G;
            }else if(k + K_G < K){
                k_next = k + K_G;
                r_next = 0;
            }else{
                r_next = -1;
            }

            if(tid==0){
                // preload A_next to gsm_mem
                if(r + M_G < M){ // not column end
                    M_G_next = min(M_G, M-(r+M_G));
                    ch_gsm[cnt_gsm_1] = dma_p2p_opt(&A[OFFSET(r_next, k, K)], M_G_next, K_G_next*sizeof(float), (K-K_G_next)*sizeof(float),
                                            gsm_mem + MK_G*cnt_gsm_1, M_G_next, K_G_next*sizeof(float), (K_G-K_G_next)*sizeof(float), 0, 0, ch0_gsm+cnt_gsm_1);
                }else if(k + K_G < K){ // r + M_G >= M
                    M_G_next = min(M_G, M);
                    K_G_next = min(K_G, K-(k+K_G));
                    ch_gsm[cnt_gsm_1] = dma_p2p_opt(&A[OFFSET(0, k_next, K)], M_G_next, K_G_next*sizeof(float), (K-K_G_next)*sizeof(float),
                                            gsm_mem + MK_G*cnt_gsm_1, M_G_next, K_G_next*sizeof(float), (K_G-K_G_next)*sizeof(float), 0, 0, ch0_gsm+cnt_gsm_1);
                }
                // wait for A_cur loaded to gsm_mem
                dma_wait_p2p(ch_gsm[cnt_gsm]);
            } else{
                if(r + M_G < M){
                    M_G_next = min(M_G, M-(r+M_G));
                }else if(k + K_G < K){
                    M_G_next = min(M_G, M);
                    K_G_next = min(K_G, K-(k+K_G));
                }
            }
            group_barrier(b_id);

            // preload gsm_mem to sm_A
            if(c_start<N){
                ch_al[0] = dma_p2p_opt(gsm_mem + MK_G*cnt_gsm, M_S, K_G_cur*sizeof(float), (K_G-K_G_cur)*sizeof(float),
                                    spm_A[0], M_S, K_G_cur*sizeof(float), (K_G-K_G_cur)*sizeof(float), 0, 0, ch0_al);
            }
            unint cnt_a = 0;
            
            long c_tail = c_step;
            for(long c=c_start; c<N; c+=c_step){
                unint p2pmask_next;
                if(c+c_step < N && c_tail+c_step >= N){ // current is not the last, next is the last
                    p2pmask_next = p2pmask_last;
                }else{
                    p2pmask_next = p2pmask_most;
                }
                c_tail += c_step;

                int c_next;
                if(c + c_step < N){ // c没到头
                    c_next = c + c_step;
                }else if(r_next == -1){ // c到头了，rk也到头了
                    c_next = -1;
                }else{ // c到头了，rk还没到头
                    c_next = c_start;
                }

                // wait for spm_B[0]
                dma_wait_p2p(ch_bl);

                // transpsoe spm_B[0] to spm_B[1]
                for(int n_a=0; n_a<N_A_cur; n_a+=32){
                    for(int k_a=0; k_a<K_G_cur; k_a+=32){
                        micro_kernel_float_32x32(spm_B[0] + n_a*(K_G>>5) + (k_a>>5), spm_B[1] + k_a*(N_A>>5) + (n_a>>5), spm_D, N_A, K_G);
                    }
                }

                // load spm_B[0]
                if(c + c_step < N){ // c没到头
                    N_A_next = min(N_A, N-(c+c_step));
                    ch_bl = dma_p2p_opt(&B[OFFSET(c+c_step, k, K)], N_A_next, K_G_cur*sizeof(float), (K-K_G_cur)*sizeof(float),
                                        spm_B[0], N_A_next, K_G_cur*sizeof(float), (K_G-K_G_cur)*sizeof(float), row_syn, p2pmask_next, ch0_bl);
                }else if(r_next != -1){ // c到头了，rk还没到头
                    N_A_next = min(N_A, N-c_start);
                    ch_bl = dma_p2p_opt(&B[OFFSET(c_start, k_next, K)], N_A_next, K_G_next*sizeof(float), (K-K_G_next)*sizeof(float),
                                        spm_B[0], N_A_next, K_G_next*sizeof(float), (K_G-K_G_next)*sizeof(float), row_syn, p2pmask_next, ch0_bl);
                }

                for(int rc=0; rc<M_G_cur; rc+=M_A){
                    // compute C index
                    unint cnt_d_1 = (cnt_d + 1) % 2;
                    unint cnt_c_1 = cnt_c + 1;
                    unint cnt_c_2 = cnt_c + 2;
                    if(cnt_c_1 > 2) cnt_c_1 -= 3;
                    if(cnt_c_2 > 2) cnt_c_2 -= 3;

                    // preload C to am_C, gsm_mem to sm_A
                    if(rc + M_A < M_G_cur){ // rc没到头
                        M_A_next = min(M_A, M_G_cur-(rc + M_A));
                        ch_cl[cnt_c_1] = dma_p2p_opt(&C[OFFSET(r+(rc+M_A), c, N)], M_A_next, N_A_cur*sizeof(float), (N-N_A_cur)*sizeof(float),
                                            spm_C[cnt_c_1], M_A_next, N_A_cur*sizeof(float), (N_A-N_A_cur)*sizeof(float), row_syn, p2pmask_cur, ch0_cl+cnt_d_1);
                    }else if(c_next == c_start){ // rc到头了，c也到头了，rk+1还没到头
                        M_A_next = min(M_A, M_G_next);
                        ch_cl[cnt_c_1] = dma_p2p_opt(&C[OFFSET(r_next, c_next, N)], M_A_next, N_A_next*sizeof(float), (N-N_A_next)*sizeof(float),
                                            spm_C[cnt_c_1], M_A_next, N_A_next*sizeof(float), (N_A-N_A_next)*sizeof(float), row_syn, p2pmask_next, ch0_cl+cnt_d_1);
                    }else if(c_next != -1){ // rc到头了，c没到头
                        M_A_next = min(M_A, M_G_cur);
                        ch_cl[cnt_c_1] = dma_p2p_opt(&C[OFFSET(r, c_next, N)], M_A_next, N_A_next*sizeof(float), (N-N_A_next)*sizeof(float),
                                            spm_C[cnt_c_1], M_A_next, N_A_next*sizeof(float), (N_A-N_A_next)*sizeof(float), row_syn, p2pmask_next, ch0_cl+cnt_d_1);
                    }

                    // wait for am_C_cur
                    dma_wait_p2p(ch_cl[cnt_c]);

                    for(int ra=0; ra<M_A_cur; ra+=2*M_S){
                        // preload A to sm
                        ch_al[1] = dma_p2p_opt(gsm_mem + MK_G*cnt_gsm + (rc+ra+M_S)*K_G, M_S, K_G_cur*sizeof(float), (K_G-K_G_cur)*sizeof(float),
                                            spm_A[1], M_S, K_G_cur*sizeof(float), (K_G-K_G_cur)*sizeof(float), 0, 0, ch0_al+1);
                        dma_wait_p2p(ch_al[0]);
                #ifdef use_r6c96
                        micro_kernel_asm_r6c96(spm_A[0], spm_B[1], spm_C[cnt_c] + ra*(N_A>>5), K_G_cur, K_G, N_A);
                        micro_kernel_asm_r6c96(spm_A[0] + M_S_half*K_G, spm_B[1], spm_C[cnt_c] + (ra + M_S_half)*(N_A>>5), K_G_cur, K_G, N_A);
                #endif
                #ifdef use_r6c128
                    #ifndef enable_alpha
                        micro_kernel_asm_r6c128(spm_A[0], spm_B[1], spm_C[cnt_c] + ra*(N_A>>5), K_G_cur, K_G, N_A);
                        micro_kernel_asm_r6c128(spm_A[0] + M_S_half*K_G, spm_B[1], spm_C[cnt_c] + (ra + M_S_half)*(N_A>>5), K_G_cur, K_G, N_A);
                    #else
                        micro_kernel_asm_r6c128_multiAlpha(spm_A[0], spm_B[1], spm_C[cnt_c] + ra*(N_A>>5), K_G_cur, K_G, N_A, alpha);
                        micro_kernel_asm_r6c128_multiAlpha(spm_A[0] + M_S_half*K_G, spm_B[1], spm_C[cnt_c] + (ra + M_S_half)*(N_A>>5), K_G_cur, K_G, N_A, alpha);
                    #endif
                #endif
                        // preload A to sm
                        if(ra + 2*M_S < M_A_cur){ // ra没到头
                            ch_al[0] = dma_p2p_opt(gsm_mem + MK_G*cnt_gsm + (rc+ra+2*M_S)*K_G, M_S, K_G_cur*sizeof(float), (K_G-K_G_cur)*sizeof(float),
                                                    spm_A[0], M_S, K_G_cur*sizeof(float), (K_G-K_G_cur)*sizeof(float), 0, 0, ch0_al+0);
                        }else if(rc + M_A < M_G_cur){ // ra到头了，rc没到头
                            ch_al[0] = dma_p2p_opt(gsm_mem + MK_G*cnt_gsm + (rc+M_A)*K_G, M_S, K_G_cur*sizeof(float), (K_G-K_G_cur)*sizeof(float),
                                                    spm_A[0], M_S, K_G_cur*sizeof(float), (K_G-K_G_cur)*sizeof(float), 0, 0, ch0_al+0);
                        }else if(c_next > c_start){ // ra到头了, rc到头了，c没到头
                            ch_al[0] = dma_p2p_opt(gsm_mem + MK_G*cnt_gsm, M_S, K_G_cur*sizeof(float), (K_G-K_G_cur)*sizeof(float),
                                                    spm_A[0], M_S, K_G_cur*sizeof(float), (K_G-K_G_cur)*sizeof(float), 0, 0, ch0_al+0);
                        }
                        dma_wait_p2p(ch_al[1]);
                #ifdef use_r6c96
                        micro_kernel_asm_r6c96(spm_A[1], spm_B[1], spm_C[cnt_c] + (ra + M_S)*(N_A>>5), K_G_cur, K_G, N_A);
                        micro_kernel_asm_r6c96(spm_A[1] + M_S_half*K_G, spm_B[1], spm_C[cnt_c] + (ra + M_S + M_S_half)*(N_A>>5), K_G_cur, K_G, N_A);
                #endif
                #ifdef use_r6c128
                    #ifndef enable_alpha
                        micro_kernel_asm_r6c128(spm_A[1], spm_B[1], spm_C[cnt_c] + (ra + M_S)*(N_A>>5), K_G_cur, K_G, N_A);
                        micro_kernel_asm_r6c128(spm_A[1] + M_S_half*K_G, spm_B[1], spm_C[cnt_c] + (ra + M_S + M_S_half)*(N_A>>5), K_G_cur, K_G, N_A);
                    #else
                        micro_kernel_asm_r6c128_multiAlpha(spm_A[1], spm_B[1], spm_C[cnt_c] + (ra + M_S)*(N_A>>5), K_G_cur, K_G, N_A, alpha);
                        micro_kernel_asm_r6c128_multiAlpha(spm_A[1] + M_S_half*K_G, spm_B[1], spm_C[cnt_c] + (ra + M_S + M_S_half)*(N_A>>5), K_G_cur, K_G, N_A, alpha);
                    #endif
                #endif
                    }
                    // last_C plus beta
                #ifdef enable_beta
                    if(k + K_G >= K){
                        C_plus_beta(spm_C[cnt_c], M_A*N_A, beta);
                    }
                #endif
                    // store spm_C to C
                    ch_cs[cnt_c] = dma_p2p_opt(spm_C[cnt_c], M_A_cur, N_A_cur*sizeof(float), (N_A-N_A_cur)*sizeof(float),
                                &C[OFFSET(r+rc, c, N)], M_A_cur, N_A_cur*sizeof(float), (N-N_A_cur)*sizeof(float), row_syn, p2pmask_cur, ch0_cs+cnt_d);

                    // wait for C stored
                    dma_wait_p2p(ch_cs[cnt_c_2]); 

                    // renew idnex C
                    cnt_d = cnt_d_1;
                    cnt_c = cnt_c_1;
                    M_A_cur = M_A_next;
                }
                p2pmask_cur = p2pmask_next;
                N_A_cur = N_A_next;
            }
            cnt_gsm = cnt_gsm_1;
            M_G_cur = M_G_next;
            group_barrier(b_id);
        }
        K_G_cur = K_G_next;
    }
    if(c_start < N){
        unint cnt_c_2 = cnt_c + 2;
        if(cnt_c_2 > 2) cnt_c_2 -= 3;
        dma_wait_p2p(ch_cs[cnt_c_2]);
    }
    scalar_free(spm_A[0]);
    scalar_free(spm_A[1]);
    vector_free(spm_B[0]);
    vector_free(spm_B[1]);
    vector_free(spm_C[0]);
    vector_free(spm_C[1]);
    vector_free(spm_C[2]);
    vector_free(spm_D);
}


void transpose_gsm_float_32x32_dbuf(unlong M, unlong K, unlong bid_main, unlong bid_T, int n_threads, float* A){ // n_threads:多少个核参与转置操作
    const int tid = get_thread_id();
    const int tid_0 = get_group_size() - n_threads;
    // hthread_printf("tid_0 %d for transpose\n", tid);   
    // --------- blocking parameters ---------
#ifdef use_r6c96
    const unlong K_G = 512; // GEMM
    const unlong M_A = 256; // transpose
    //---------------------
    const unlong M_G = 960; // GEMM
    const unlong N_A = 96; // transpose
#endif
#ifdef use_r6c128
    const unlong K_G = 512; // GEMM
    const unlong M_A = 256; // transpose
    //---------------------
    const unlong M_G = 864; // GEMM
    const unlong N_A = 96; // transpose
#endif
    const unlong h_a = 32;
    const unlong w_a = 32;
    // ---------------------------------------
    const unlong c_start = (tid-tid_0)*N_A;
    const unlong c_step = n_threads*N_A;
    bool row_syn = false;
    unint p2pmask = 0x00;
    if(n_threads != 1){
        row_syn = true;
        for(int i=0; i<n_threads; i++){
            p2pmask <<= 1;
            p2pmask |= 1;
        }
        p2pmask <<= tid_0;
    }    // hthread_printf("p2pmask = %d\n", p2pmask);
    // ---------------------------------------
    float * gsm_addr[3];
    gsm_addr[0] = gsm_mem;
    gsm_addr[1] = gsm_mem + K_G*M_G;
    gsm_addr[2] = gsm_mem + 2*K_G*M_G;

    lvector float* spm_A[2];
    lvector float* spm_B[2];
    spm_A[0] = vector_malloc(M_A * N_A * sizeof(float));
    spm_A[1] = vector_malloc(M_A * N_A * sizeof(float));
    spm_B[0] = vector_malloc(M_A * N_A * sizeof(float));
    spm_B[1] = vector_malloc(M_A * N_A * sizeof(float));
    lvector double* spm_D = vector_malloc(2048 * sizeof(float));

    int ch_gsm[2], ch_al[2], ch_bs[2];

    const int ch0_gsm = 0;
    const int ch0_al = 2;
    const int ch0_bs = 4;

    unint cnt_dgsm = 0;
    unint cnt_tgsm = 0;
    for(long k_o=0; k_o<K; k_o+=K_G){
        long K_G_cur = min(K_G, K-k_o);
        for(long m_o=0; m_o<M; m_o+=M_G){
            long M_G_cur = min(M_G, M-m_o);
            // compute gsm indexes
            unint cnt_dgsm_1 = (cnt_dgsm + 1) % 2;
            unint cnt_tgsm_1 = cnt_tgsm + 1;
            unint cnt_tgsm_2 = cnt_tgsm + 2;
            if(cnt_tgsm_1 > 2) cnt_tgsm_1 -= 3;
            if(cnt_tgsm_2 > 2) cnt_tgsm_2 -= 3;

            if(tid == tid_0){
                ch_gsm[cnt_dgsm] = dma_p2p_opt(&A[OFFSET(k_o, m_o, M)], K_G_cur, M_G_cur*sizeof(float), (M-M_G_cur)*sizeof(float),
                                                gsm_addr[cnt_tgsm], K_G_cur, M_G_cur*sizeof(float), (M_G-M_G_cur)*sizeof(float), 0, 0, ch0_gsm+cnt_dgsm);
                dma_wait_p2p(ch_gsm[cnt_dgsm]);
            }
            core_barrier(bid_T, n_threads);
            // ----------------------------------- main loop -----------------------------------
            unint cnt_a = 0;
            unint cnt_d = 0;
            unint cnt_b = 0;

            unlong M_A_cur = min(M_A, K_G_cur);
            unlong N_A_cur = min(N_A, M_G_cur-c_start);
            if(c_start < M_G_cur){
                ch_al[0] = dma_p2p_opt(&gsm_addr[cnt_tgsm][OFFSET(0, c_start, M_G)], M_A_cur, N_A_cur*sizeof(float), (M_G-N_A_cur)*sizeof(float),
                                        spm_A[0], M_A_cur, N_A_cur*sizeof(float), (N_A-N_A_cur)*sizeof(float), row_syn, p2pmask, ch0_al);
                ch_bs[1] = dma_p2p_opt(gsm_addr[cnt_tgsm_1], 1, 1*sizeof(double), 0, // 发起一次无效传输
                                        spm_B[1], 1, 1*sizeof(double), 0, 0, 0, ch0_bs+1);
            }
            for(unlong r=0; r<K_G_cur; r+=M_A){
                unlong M_A_next;
                for(unlong c=c_start; c<M_G_cur; c+=c_step){
                    // -------------------------- next round indexes --------------------------
                    unlong N_A_next;
                    unint cnt_a_1 = (cnt_a + 1) % 2;
                    unint cnt_b_1 = (cnt_b + 1) % 2;
                    // ------------------------------------------------------------------------
                    if(c + c_step < M_G_cur){
                        N_A_next = min(N_A, M_G_cur-(c+c_step));
                        ch_al[cnt_a_1] = dma_p2p_opt(&gsm_addr[cnt_tgsm][OFFSET(r, c+c_step, M_G)], M_A_cur, N_A_next*sizeof(float), (M_G-N_A_next)*sizeof(float),
                                    spm_A[cnt_a_1], M_A_cur, N_A_next*sizeof(float), (N_A-N_A_next)*sizeof(float), row_syn, p2pmask, ch0_al+cnt_a_1);
                    }else if(r + M_A < K_G_cur){
                        N_A_next = min(N_A, M_G_cur-c_start);
                        M_A_next = min(M_A, K_G_cur-(r+M_A));
                        ch_al[cnt_a_1] = dma_p2p_opt(&gsm_addr[cnt_tgsm][OFFSET(r+M_A, c_start, M_G)], M_A_next, N_A_next*sizeof(float), (M_G-N_A_next)*sizeof(float),
                                    spm_A[cnt_a_1], M_A_next, N_A_next*sizeof(float), (N_A-N_A_next)*sizeof(float), row_syn, p2pmask, ch0_al+cnt_a_1);
                    }

                    dma_wait_p2p(ch_al[cnt_a]);

                    for(unlong m=0; m<M_A_cur; m+=h_a){
                        for(unlong n=0; n<N_A_cur; n+=w_a){
                            micro_kernel_float_32x32(&spm_A[cnt_a][OFFSET(m, n>>5, N_A>>5)], &spm_B[cnt_b][OFFSET(n, m>>5, M_A>>5)], spm_D, M_A, N_A);
                        }
                    }

                    ch_bs[cnt_b] = dma_p2p_opt(spm_B[cnt_b], N_A_cur, M_A_cur*sizeof(float), (M_A-M_A_cur)*sizeof(float),
                                    &gsm_addr[cnt_tgsm_1][OFFSET(c, r, K_G)], N_A_cur, M_A_cur*sizeof(float), (K_G-M_A_cur)*sizeof(float), row_syn, p2pmask, ch0_bs+cnt_b);

                    dma_wait_p2p(ch_bs[cnt_b_1]);
                    // -------------------------- renew indexes --------------------------
                    cnt_a = cnt_a_1;
                    cnt_b = cnt_b_1;
                    N_A_cur = N_A_next;
                }
                M_A_cur = M_A_next;
            }
            if(c_start < M_G_cur){
                dma_wait_p2p(ch_bs[(cnt_b+1)%2]);
            }
            // ----------------------------------- main loop end -----------------------------------
            group_barrier(bid_main);
            cnt_dgsm = cnt_dgsm_1;
            cnt_tgsm = cnt_tgsm_2;
        }    
    }
    vector_free(spm_A[0]);
    vector_free(spm_A[1]);
    vector_free(spm_B[0]);
    vector_free(spm_B[1]);
    vector_free(spm_D);
}

void sgemm_TN_gsmA_tribuf(unlong M, unlong N, unlong K, unlong b_id, const int grp_size, float* A, float* B, float* C, float alpha, float beta){
    const int tid = get_thread_id();
    // hthread_printf("tid %d for GEMM\n", tid);
    //------------------------------ tiling params ------------------------------
    // (2*K_G + 3*M_A) * N_A < 98304
#ifdef use_r6c96
    const indexType M_G = 960;
    const indexType K_G = 512;
    const indexType N_A = 96;
    const indexType M_A = 240;
    const indexType M_S = 12;
#endif
#ifdef use_r6c128
    const indexType M_G = 864;
    const indexType K_G = 512;
    const indexType N_A = 128;
    const indexType M_A = 144;
    const indexType M_S = 12;
#endif
    const indexType MK_G = M_G * K_G;
    const indexType M_S_half = M_S>>1;
    //------------------------------ tiling params ------------------------------
    const indexType c_start = tid * N_A;
    const indexType c_step = grp_size * N_A;
    bool row_syn = false;
    unint p2pmask_most = 0x00;
    unint p2pmask_last = 0x00;
    if(grp_size != 1){
        row_syn = true;
        long N_last = N % c_step;
        if(N_last==0)
            N_last = c_step;
        int ncore_last = (N_last + N_A - 1) / N_A;
        int i=0;
        for(; i<ncore_last; i++){
            p2pmask_most <<= 1;
            p2pmask_most |= 1;
        }
        p2pmask_last = p2pmask_most;
        if(N != N_last){
            for(; i<grp_size; i++){
                p2pmask_most <<= 1;
                p2pmask_most |= 1;
            }
        }
    }
    unint p2pmask_cur = p2pmask_most;
    
    float * gsm_addr[3];
    gsm_addr[0] = gsm_mem;
    gsm_addr[1] = gsm_mem + K_G*M_G;
    gsm_addr[2] = gsm_mem + 2*K_G*M_G;
    float* spm_A[2];
    lvector float* spm_B[2];
    lvector float* spm_C[3];
    spm_A[0] = scalar_malloc(M_S * K_G *sizeof(float));
    spm_A[1] = scalar_malloc(M_S * K_G *sizeof(float));
    spm_B[0] = vector_malloc(K_G * N_A * sizeof(float));
    spm_B[1] = vector_malloc(K_G * N_A * sizeof(float));
    spm_C[0] = vector_malloc(M_A * N_A * sizeof(float));
    spm_C[1] = vector_malloc(M_A * N_A * sizeof(float));
    spm_C[2] = vector_malloc(M_A * N_A * sizeof(float));

    int ch_gsm[2], ch_al[2], ch_bl[2], ch_cl[3], ch_cs[3];

    const int ch0_gsm = 8;
    const int ch0_bl = 2;
    const int ch0_cl = 4; // high level: 4, 5
    const int ch0_al = 0; // high level: 0, 1 
    const int ch0_cs = 6; // high level: 6, 7

    const unlong prir = 0b0000000011;
    set_prir(prir);

    long M_G_cur = min(M_G, M);
    long K_G_cur = min(K_G, K);
    long N_A_cur = min(N_A, N-c_start);
    long M_A_cur = min(M_A, M);
    long M_G_next, K_G_next, N_A_next, M_A_next;

    if(c_start<N){
        // preload spm_B
        ch_bl[0] = dma_p2p_opt(&B[OFFSET(0, c_start, N)], K_G_cur, N_A_cur*sizeof(float), (N-N_A_cur)*sizeof(float),
                            spm_B[0], K_G_cur, N_A_cur*sizeof(float), (N_A-N_A_cur)*sizeof(float), row_syn, p2pmask_cur, ch0_bl);
        // preload spm_C
        ch_cl[0] = dma_p2p_opt(&C[OFFSET(0, c_start, N)], M_A_cur, N_A_cur*sizeof(float), (N-N_A_cur)*sizeof(float),
                            spm_C[0], M_A_cur, N_A_cur*sizeof(float), (N_A-N_A_cur)*sizeof(float), row_syn, p2pmask_cur, ch0_cl);
        // prestore spm_C (启动第一次无效传输，为了配合第一次wait)
        ch_cs[2] = dma_p2p_opt(&C[0], 1, 1*sizeof(double), 0,
                            spm_C[2], 1, 1*sizeof(double), 0, 0, 0, 7);
    }
    int cnt_tgsm = 0;
    int cnt_b = 0;
    int cnt_c = 0, cnt_d = 0;
    for(long k=0; k<K; k+=K_G){
        K_G_next = K_G_cur;
        for(long r=0; r<M; r+=M_G){
            int cnt_tgsm_1 = cnt_tgsm + 1;
            int cnt_tgsm_2 = cnt_tgsm + 2;
            if(cnt_tgsm_1 > 2) cnt_tgsm_1 -= 3;
            if(cnt_tgsm_2 > 2) cnt_tgsm_2 -= 3;

            int k_next, r_next;
            if(r + M_G < M){
                k_next = k;
                r_next = r + M_G;
            }else if(k + K_G < K){
                k_next = k + K_G;
                r_next = 0;
            }else{
                r_next = -1;
            }
            if(r + M_G < M){
                M_G_next = min(M_G, M-(r+M_G));
            }else if(k + K_G < K){
                M_G_next = min(M_G, M);
                K_G_next = min(K_G, K-(k+K_G));
            }
            // wait for ddr->gsm and transpose
            group_barrier(b_id);
            
            // preload gsm_mem to sm_A
            if(c_start<N){
                ch_al[0] = dma_p2p_opt(gsm_addr[cnt_tgsm_1], M_S, K_G_cur*sizeof(float), (K_G-K_G_cur)*sizeof(float),
                                    spm_A[0], M_S, K_G_cur*sizeof(float), (K_G-K_G_cur)*sizeof(float), 0, 0, ch0_al);
            }
            unint cnt_a = 0;

            long c_tail = c_step;
            for(long c=c_start; c<N; c+=c_step){
                unint p2pmask_next;
                if(c+c_step < N && c_tail+c_step >= N){ // current is not the last, next is the last
                    p2pmask_next = p2pmask_last;
                }else{
                    p2pmask_next = p2pmask_most;
                }
                c_tail += c_step;

                int cnt_b_1 = (cnt_b + 1) % 2;

                int c_next;
                if(c + c_step < N){ // c没到头
                    c_next = c + c_step;
                }else if(r_next == -1){ // c到头了，rk也到头了
                    c_next = -1;
                }else{ // c到头了，rk还没到头
                    c_next = c_start;
                }

                // load spm_B[cnt_b_1]
                if(c + c_step < N){ // c没到头
                    N_A_next = min(N_A, N-(c+c_step));
                    ch_bl[cnt_b_1] = dma_p2p_opt(&B[OFFSET(k, c+c_step, N)], K_G_cur, N_A_next*sizeof(float), (N-N_A_next)*sizeof(float),
                                                    spm_B[cnt_b_1], K_G_cur, N_A_next*sizeof(float), (N_A-N_A_next)*sizeof(float), row_syn, p2pmask_next, ch0_bl+cnt_b_1);
                }else if(r_next != -1){ // c到头了，rk还没到头
                    N_A_next = min(N_A, N-c_start);
                    ch_bl[cnt_b_1] = dma_p2p_opt(&B[OFFSET(k_next, c_start, N)], K_G_next, N_A_next*sizeof(float), (N-N_A_next)*sizeof(float),
                                                    spm_B[cnt_b_1], K_G_next, N_A_next*sizeof(float), (N_A-N_A_next)*sizeof(float), row_syn, p2pmask_next, ch0_bl+cnt_b_1);
                }

                // wait for spm_B[cnt_b]
                dma_wait_p2p(ch_bl[cnt_b]);

                for(int rc=0; rc<M_G_cur; rc+=M_A){
                    // compute C index
                    unint cnt_d_1 = (cnt_d + 1) % 2;
                    unint cnt_c_1 = cnt_c + 1;
                    unint cnt_c_2 = cnt_c + 2;
                    if(cnt_c_1 > 2) cnt_c_1 -= 3;
                    if(cnt_c_2 > 2) cnt_c_2 -= 3;

                    // preload C to am_C, gsm_mem to sm_A
                    if(rc + M_A < M_G_cur){ // rc没到头
                        M_A_next = min(M_A, M_G_cur-(rc + M_A));
                        ch_cl[cnt_c_1] = dma_p2p_opt(&C[OFFSET(r+(rc+M_A), c, N)], M_A_next, N_A_cur*sizeof(float), (N-N_A_cur)*sizeof(float),
                                            spm_C[cnt_c_1], M_A_next, N_A_cur*sizeof(float), (N_A-N_A_cur)*sizeof(float), row_syn, p2pmask_cur, ch0_cl+cnt_d_1);
                    }else if(c_next == c_start){ // rc到头了，c也到头了，rk+1还没到头
                        M_A_next = min(M_A, M_G_next);
                        ch_cl[cnt_c_1] = dma_p2p_opt(&C[OFFSET(r_next, c_next, N)], M_A_next, N_A_next*sizeof(float), (N-N_A_next)*sizeof(float),
                                            spm_C[cnt_c_1], M_A_next, N_A_next*sizeof(float), (N_A-N_A_next)*sizeof(float), row_syn, p2pmask_next, ch0_cl+cnt_d_1);
                    }else if(c_next != -1){ // rc到头了，c没到头
                        M_A_next = min(M_A, M_G_cur);
                        ch_cl[cnt_c_1] = dma_p2p_opt(&C[OFFSET(r, c_next, N)], M_A_next, N_A_next*sizeof(float), (N-N_A_next)*sizeof(float),
                                            spm_C[cnt_c_1], M_A_next, N_A_next*sizeof(float), (N_A-N_A_next)*sizeof(float), row_syn, p2pmask_next, ch0_cl+cnt_d_1);
                    }
               
                    // wait for am_C_cur
                    dma_wait_p2p(ch_cl[cnt_c]);

                    for(int ra=0; ra<M_A_cur; ra+=2*M_S){
                        // preload A to sm
                        ch_al[1] = dma_p2p_opt(gsm_addr[cnt_tgsm_1] + (rc+ra+M_S)*K_G, M_S, K_G_cur*sizeof(float), (K_G-K_G_cur)*sizeof(float),
                                            spm_A[1], M_S, K_G_cur*sizeof(float), (K_G-K_G_cur)*sizeof(float), 0, 0, ch0_al+1);
                        dma_wait_p2p(ch_al[0]);
                #ifdef use_r6c96
                        micro_kernel_asm_r6c96(spm_A[0], spm_B[cnt_b], spm_C[cnt_c] + ra*(N_A>>5), K_G_cur, K_G, N_A);
                        micro_kernel_asm_r6c96(spm_A[0] + M_S_half*K_G, spm_B[cnt_b], spm_C[cnt_c] + (ra + M_S_half)*(N_A>>5), K_G_cur, K_G, N_A);
                #endif
                #ifdef use_r6c128
                    #ifndef enable_alpha
                        micro_kernel_asm_r6c128(spm_A[0], spm_B[cnt_b], spm_C[cnt_c] + ra*(N_A>>5), K_G_cur, K_G, N_A);
                        micro_kernel_asm_r6c128(spm_A[0] + M_S_half*K_G, spm_B[cnt_b], spm_C[cnt_c] + (ra + M_S_half)*(N_A>>5), K_G_cur, K_G, N_A);
                    #else
                        micro_kernel_asm_r6c128_multiAlpha(spm_A[0], spm_B[cnt_b], spm_C[cnt_c] + ra*(N_A>>5), K_G_cur, K_G, N_A, alpha);
                        micro_kernel_asm_r6c128_multiAlpha(spm_A[0] + M_S_half*K_G, spm_B[cnt_b], spm_C[cnt_c] + (ra + M_S_half)*(N_A>>5), K_G_cur, K_G, N_A, alpha);
                    #endif
                #endif
                        // preload A to sm
                        if(ra + 2*M_S < M_A_cur){ // ra没到头
                            ch_al[0] = dma_p2p_opt(gsm_addr[cnt_tgsm_1] + (rc+ra+2*M_S)*K_G, M_S, K_G_cur*sizeof(float), (K_G-K_G_cur)*sizeof(float),
                                                    spm_A[0], M_S, K_G_cur*sizeof(float), (K_G-K_G_cur)*sizeof(float), 0, 0, ch0_al+0);
                        }else if(rc + M_A < M_G_cur){ // ra到头了，rc没到头
                            ch_al[0] = dma_p2p_opt(gsm_addr[cnt_tgsm_1] + (rc+M_A)*K_G, M_S, K_G_cur*sizeof(float), (K_G-K_G_cur)*sizeof(float),
                                                    spm_A[0], M_S, K_G_cur*sizeof(float), (K_G-K_G_cur)*sizeof(float), 0, 0, ch0_al+0);
                        }else if(c_next > c_start){ // ra到头了, rc到头了，c没到头
                            ch_al[0] = dma_p2p_opt(gsm_addr[cnt_tgsm_1], M_S, K_G_cur*sizeof(float), (K_G-K_G_cur)*sizeof(float),
                                                    spm_A[0], M_S, K_G_cur*sizeof(float), (K_G-K_G_cur)*sizeof(float), 0, 0, ch0_al+0);
                        }
                        dma_wait_p2p(ch_al[1]);
                #ifdef use_r6c96
                        micro_kernel_asm_r6c96(spm_A[1], spm_B[cnt_b], spm_C[cnt_c] + (ra + M_S)*(N_A>>5), K_G_cur, K_G, N_A);
                        micro_kernel_asm_r6c96(spm_A[1] + M_S_half*K_G, spm_B[cnt_b], spm_C[cnt_c] + (ra + M_S + M_S_half)*(N_A>>5), K_G_cur, K_G, N_A);
                #endif
                #ifdef use_r6c128
                    #ifndef enable_alpha
                        micro_kernel_asm_r6c128(spm_A[1], spm_B[cnt_b], spm_C[cnt_c] + (ra + M_S)*(N_A>>5), K_G_cur, K_G, N_A);
                        micro_kernel_asm_r6c128(spm_A[1] + M_S_half*K_G, spm_B[cnt_b], spm_C[cnt_c] + (ra + M_S + M_S_half)*(N_A>>5), K_G_cur, K_G, N_A);
                    #else
                        micro_kernel_asm_r6c128_multiAlpha(spm_A[1], spm_B[cnt_b], spm_C[cnt_c] + (ra + M_S)*(N_A>>5), K_G_cur, K_G, N_A, alpha);
                        micro_kernel_asm_r6c128_multiAlpha(spm_A[1] + M_S_half*K_G, spm_B[cnt_b], spm_C[cnt_c] + (ra + M_S + M_S_half)*(N_A>>5), K_G_cur, K_G, N_A, alpha);
                    #endif
                #endif
                    }
                    // last_C plus beta
                #ifdef enable_beta
                    if(k + K_G >= K){
                        C_plus_beta(spm_C[cnt_c], M_A*N_A, beta);
                    }
                #endif
                    // store spm_C to C
                    ch_cs[cnt_c] = dma_p2p_opt(spm_C[cnt_c], M_A_cur, N_A_cur*sizeof(float), (N_A-N_A_cur)*sizeof(float),
                                &C[OFFSET(r+rc, c, N)], M_A_cur, N_A_cur*sizeof(float), (N-N_A_cur)*sizeof(float), row_syn, p2pmask_cur, ch0_cs+cnt_d);

                    // wait for C stored
                    dma_wait_p2p(ch_cs[cnt_c_2]); 

                    // renew idnex C
                    cnt_d = cnt_d_1;
                    cnt_c = cnt_c_1;
                    M_A_cur = M_A_next;
                }
                cnt_b = cnt_b_1;
                p2pmask_cur = p2pmask_next;
                N_A_cur = N_A_next;
            }
            cnt_tgsm = cnt_tgsm_2;
            M_G_cur = M_G_next;
        }
        K_G_cur = K_G_next;
    }
    if(c_start<N){
        unint cnt_c_2 = cnt_c + 2;
        if(cnt_c_2 > 2) cnt_c_2 -= 3;
        dma_wait_p2p(ch_cs[cnt_c_2]);
    }
    scalar_free(spm_A[0]);
    scalar_free(spm_A[1]);
    vector_free(spm_B[0]);
    vector_free(spm_B[1]);
    vector_free(spm_C[0]);
    vector_free(spm_C[1]);
    vector_free(spm_C[2]);
}

__global__ void sgemm_TN_qxx(unlong M, unlong N, unlong K, unlong bid_main, unlong bid_T, float* A, float* B, float* C, float alpha, float beta){
    const int n_cores_for_transpose = 4;
    const int tid = get_thread_id();
    const int grp_size = get_group_size();
    const int n_cores_for_compute = grp_size - n_cores_for_transpose;

    if(tid >= n_cores_for_compute){
        transpose_gsm_float_32x32_dbuf(M, K, bid_main, bid_T, n_cores_for_transpose, A);
    }else{
        sgemm_TN_gsmA_tribuf(M, N, K, bid_main, n_cores_for_compute, A, B, C, alpha, beta);
    }
}


__global__ void sgemm_TT_qxx(unlong M, unlong N, unlong K, unlong b_id, float* A, float* B, float* C, float alpha, float beta){
    const int tid = get_thread_id();
    const int grp_size = get_group_size();
    // -------------------------- blocking params --------------------------
#ifdef use_r6c96
    const indexType N_G = 960;
    const indexType K_G = 512;
    const indexType M_A = 96;
    const indexType N_A = 240;
    const indexType N_S = 12;
#endif
#ifdef use_r6c128
    const indexType N_G = 576;
    const indexType K_G = 512;
    const indexType M_A = 128;
    const indexType N_A = 144;
    const indexType N_S = 12;
#endif
    const indexType N_A_buffer = (((N_A-2)>>5) + 1) << 5;
    const indexType NK_G = N_G * K_G;
    const indexType N_S_half = N_S>>1;
    // -------------------------- blocking params --------------------------
    const indexType r_start = tid * M_A;
    const indexType r_step = grp_size * M_A;
    bool row_syn = false;
    unint p2pmask_most = 0x00;
    unint p2pmask_last = 0x00;
    if(grp_size != 1){
        row_syn = true;
        long M_last = M % r_step;
        if(M_last==0)
            M_last = r_step;
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
    
    float* spm_B[2];
    lvector float* spm_A[2];
    lvector float* spm_C[3];
    spm_B[0] = scalar_malloc(N_S * K_G *sizeof(float));
    spm_B[1] = scalar_malloc(N_S * K_G *sizeof(float));
    spm_A[0] = vector_malloc(K_G * M_A * sizeof(float));
    spm_A[1] = vector_malloc(K_G * M_A * sizeof(float));
    spm_C[0] = vector_malloc(N_A_buffer * M_A * sizeof(float));
    spm_C[1] = vector_malloc(N_A_buffer * M_A * sizeof(float));
    spm_C[2] = vector_malloc(N_A * M_A * sizeof(float));
    lvector double* spm_D = vector_malloc(2048 * sizeof(float));

    int ch_gsm[2], ch_al[2], ch_bl[2], ch_cl[2], ch_cs[2];

    const int ch0_gsm = 8;
    const int ch0_al = 2;
    const int ch0_cl = 4; // high level: 4, 5
    const int ch0_bl = 0; // high level: 0, 1 
    const int ch0_cs = 6; // high level: 6, 7

    const unlong prir = 0b0000000011;
    set_prir(prir);

    long N_G_cur = min(N_G, N);
    long K_G_cur = min(K_G, K);
    long M_A_cur = min(M_A, M-r_start);
    long N_A_cur = min(N_A, N);
    long N_G_next, K_G_next, M_A_next, N_A_next;
    
    // preload A to gsm_mem
    if(tid==0){
        ch_gsm[0] = dma_p2p_opt(&B[OFFSET(0, 0, K)], N_G_cur, K_G_cur*sizeof(float), (K-K_G_cur)*sizeof(float),
                            gsm_mem, N_G_cur, K_G_cur*sizeof(float), (K_G-K_G_cur)*sizeof(float), 0, 0, ch0_gsm);
    }
    if(r_start < M){
        // preload spm_A
        ch_al[0] = dma_p2p_opt(&A[OFFSET(0, r_start, M)], K_G_cur, M_A_cur*sizeof(float), (M-M_A_cur)*sizeof(float),
                            spm_A[0], K_G_cur, M_A_cur*sizeof(float), (M_A-M_A_cur)*sizeof(float), row_syn, p2pmask_cur, ch0_al);
        // preload spm_C
        ch_cl[0] = dma_p2p_opt(&C[OFFSET(r_start, 0, N)], M_A_cur, N_A_cur*sizeof(float), (N-N_A_cur)*sizeof(float),
                            spm_C[0], M_A_cur, N_A_cur*sizeof(float), (N_A_buffer-N_A_cur)*sizeof(float), row_syn, p2pmask_cur, ch0_cl);
        // prestore spm_C (启动第一次无效传输，为了配合第一次wait)
        ch_cs[1] = dma_p2p_opt(&C[0], 1, 1*sizeof(double), 0,
                            spm_C[1], 1, 1*sizeof(double), 0, 0, 0, 7);
    }
    int cnt_gsm = 0;
    int cnt_a = 0;
    int cnt_c = 0;
    for(long k=0; k<K; k+=K_G){
        K_G_next = K_G_cur;
        for(long c=0; c<N; c+=N_G){
            int cnt_gsm_1 = (cnt_gsm + 1) % 2;

            int k_next, c_next;
            if(c + N_G < N){
                k_next = k;
                c_next = c + N_G;
            }else if(k + K_G < K){
                k_next = k + K_G;
                c_next = 0;
            }else{
                c_next = -1;
            }

            if(tid==0){
                // preload B_next to gsm_mem
                if(c + N_G < N){ // not column end
                    N_G_next = min(N_G, N-(c+N_G));
                    ch_gsm[cnt_gsm_1] = dma_p2p_opt(&B[OFFSET(c_next, k, K)], N_G_next, K_G_next*sizeof(float), (K-K_G_next)*sizeof(float),
                                            gsm_mem + NK_G*cnt_gsm_1, N_G_next, K_G_next*sizeof(float), (K_G-K_G_next)*sizeof(float), 0, 0, ch0_gsm+cnt_gsm_1);
                }else if(k + K_G < K){ // c + N_G >= N
                    N_G_next = min(N_G, N);
                    K_G_next = min(K_G, K-(k+K_G));
                    ch_gsm[cnt_gsm_1] = dma_p2p_opt(&B[OFFSET(0, k_next, K)], N_G_next, K_G_next*sizeof(float), (K-K_G_next)*sizeof(float),
                                            gsm_mem + NK_G*cnt_gsm_1, N_G_next, K_G_next*sizeof(float), (K_G-K_G_next)*sizeof(float), 0, 0, ch0_gsm+cnt_gsm_1);
                }
                // wait for B_cur loaded to gsm_mem
                dma_wait_p2p(ch_gsm[cnt_gsm]);
            } else{
                if(c + N_G < N){ // not column end
                    N_G_next = min(N_G, N-(c+N_G));
                }else if(k + K_G < K){ // c + N_G >= N
                    N_G_next = min(N_G, N);
                    K_G_next = min(K_G, K-(k+K_G));
                }
            }
            group_barrier(b_id);

            // preload gsm_mem to sm_B
            if(r_start < M){
                ch_bl[0] = dma_p2p_opt(gsm_mem + NK_G*cnt_gsm, N_S, K_G_cur*sizeof(float), (K_G-K_G_cur)*sizeof(float),
                                    spm_B[0], N_S, K_G_cur*sizeof(float), (K_G-K_G_cur)*sizeof(float), 0, 0, ch0_bl);
            }
            unint cnt_b = 0;

            long r_tail = r_step;
            for(long r=r_start; r<M; r+=r_step){
                unint p2pmask_next;
                if(r+r_step < M && r_tail+r_step >= M){ // current is not the last, next is the last
                    p2pmask_next = p2pmask_last;
                }else{
                    p2pmask_next = p2pmask_most;
                }
                r_tail += r_step;

                int cnt_a_1 = (cnt_a + 1) % 2;

                int r_next;
                if(r + r_step < M){ // r没到头
                    r_next = r + r_step;
                }else if(c_next == -1){ // r到头了，c也到头了
                    r_next = -1;
                }else{ // r到头了，c还没到头
                    r_next = r_start;
                }

                // load spm_A[cnt_a_1]
                if(r + r_step < M){ // r没到头
                    M_A_next = min(M_A, M-(r+r_step));
                    ch_al[cnt_a_1] = dma_p2p_opt(&A[OFFSET(k, r+r_step, M)], K_G_cur, M_A_next*sizeof(float), (M-M_A_next)*sizeof(float),
                                                    spm_A[cnt_a_1], K_G_cur, M_A_next*sizeof(float), (M_A-M_A_next)*sizeof(float), row_syn, p2pmask_next, ch0_al+cnt_a_1);
                }else if(c_next != -1){ // c到头了，rk还没到头
                    M_A_next = min(M_A, M-r_start);
                    ch_al[cnt_a_1] = dma_p2p_opt(&A[OFFSET(k_next, r_start, M)], K_G_next, M_A_next*sizeof(float), (M-M_A_next)*sizeof(float),
                                                    spm_A[cnt_a_1], K_G_next, M_A_next*sizeof(float), (M_A-M_A_next)*sizeof(float), row_syn, p2pmask_next, ch0_al+cnt_a_1);
                }

                // wait for spm_A[cnt_a]
                dma_wait_p2p(ch_al[cnt_a]);

                for(int cc=0; cc<N_G_cur; cc+=N_A){
                    // compute C index
                    unint cnt_c_1 = (cnt_c + 1) % 2;
                    
                    // [0] inner loop
                    for(int cb=0; cb<N_A_cur; cb+=2*N_S){
                        // preload B to sm
                        ch_bl[1] = dma_p2p_opt(gsm_mem + NK_G*cnt_gsm + (cc+cb+N_S)*K_G, N_S, K_G_cur*sizeof(float), (K_G-K_G_cur)*sizeof(float),
                                                spm_B[1], N_S, K_G_cur*sizeof(float), (K_G-K_G_cur)*sizeof(float), 0, 0, ch0_bl+1);
                        dma_wait_p2p(ch_bl[0]);
                #ifdef use_r6c96
                        micro_kernel_asm_r6c96_only_write(spm_B[0], spm_A[cnt_a], spm_C[2] + cb*(M_A>>5), K_G_cur, K_G, M_A);
                        micro_kernel_asm_r6c96_only_write(spm_B[0] + N_S_half*K_G, spm_A[cnt_a], spm_C[2] + (cb + N_S_half)*(M_A>>5), K_G_cur, K_G, M_A);
                #endif
                #ifdef use_r6c128
                    #ifndef enable_alpha
                        micro_kernel_asm_r6c128_only_write(spm_B[0], spm_A[cnt_a], spm_C[2] + cb*(M_A>>5), K_G_cur, K_G, M_A);
                        micro_kernel_asm_r6c128_only_write(spm_B[0] + N_S_half*K_G, spm_A[cnt_a], spm_C[2] + (cb + N_S_half)*(M_A>>5), K_G_cur, K_G, M_A);
                    #else
                        micro_kernel_asm_r6c128_multiAlpha_only_write(spm_B[0], spm_A[cnt_a], spm_C[2] + cb*(M_A>>5), K_G_cur, K_G, M_A, alpha);
                        micro_kernel_asm_r6c128_multiAlpha_only_write(spm_B[0] + N_S_half*K_G, spm_A[cnt_a], spm_C[2] + (cb + N_S_half)*(M_A>>5), K_G_cur, K_G, M_A, alpha);
                    #endif
                #endif
                        // preload B to sm
                        if(cb + 2*N_S < N_A_cur){ // cb没到头
                            ch_bl[0] = dma_p2p_opt(gsm_mem + NK_G*cnt_gsm + (cc+cb+2*N_S)*K_G, N_S, K_G_cur*sizeof(float), (K_G-K_G_cur)*sizeof(float),
                                                    spm_B[0], N_S, K_G_cur*sizeof(float), (K_G-K_G_cur)*sizeof(float), 0, 0, ch0_bl+0);
                        }else if(cc + N_A < N_G_cur){ // cb到头了，cc没到头
                            ch_bl[0] = dma_p2p_opt(gsm_mem + NK_G*cnt_gsm + (cc+N_A)*K_G, N_S, K_G_cur*sizeof(float), (K_G-K_G_cur)*sizeof(float),
                                                    spm_B[0], N_S, K_G_cur*sizeof(float), (K_G-K_G_cur)*sizeof(float), 0, 0, ch0_bl+0);
                        }else if(r_next > r_start){ // cb到头了, cc到头了，r没到头
                            ch_bl[0] = dma_p2p_opt(gsm_mem + NK_G*cnt_gsm, N_S, K_G_cur*sizeof(float), (K_G-K_G_cur)*sizeof(float),
                                                    spm_B[0], N_S, K_G_cur*sizeof(float), (K_G-K_G_cur)*sizeof(float), 0, 0, ch0_bl+0);
                        }
                        dma_wait_p2p(ch_bl[1]);
                #ifdef use_r6c96
                        micro_kernel_asm_r6c96_only_write(spm_B[1], spm_A[cnt_a], spm_C[2] + (cb + N_S)*(M_A>>5), K_G_cur, K_G, M_A);
                        micro_kernel_asm_r6c96_only_write(spm_B[1] + N_S_half*K_G, spm_A[cnt_a], spm_C[2] + (cb + N_S + N_S_half)*(M_A>>5), K_G_cur, K_G, M_A);
                #endif
                #ifdef use_r6c128
                    #ifndef enable_alpha
                        micro_kernel_asm_r6c128_only_write(spm_B[1], spm_A[cnt_a], spm_C[2] + (cb + N_S)*(M_A>>5), K_G_cur, K_G, M_A);
                        micro_kernel_asm_r6c128_only_write(spm_B[1] + N_S_half*K_G, spm_A[cnt_a], spm_C[2] + (cb + N_S + N_S_half)*(M_A>>5), K_G_cur, K_G, M_A);
                    #else
                        micro_kernel_asm_r6c128_multiAlpha_only_write(spm_B[1], spm_A[cnt_a], spm_C[2] + (cb + N_S)*(M_A>>5), K_G_cur, K_G, M_A, alpha);
                        micro_kernel_asm_r6c128_multiAlpha_only_write(spm_B[1] + N_S_half*K_G, spm_A[cnt_a], spm_C[2] + (cb + N_S + N_S_half)*(M_A>>5), K_G_cur, K_G, M_A, alpha);
                    #endif
                #endif
                    }

                    // [1] wait for am_C_cur
                    dma_wait_p2p(ch_cl[cnt_c]);

                    // [2] transpose src_C
                    for(int n_a=0; n_a<N_A_cur; n_a+=32){
                        for(int m_a=0; m_a<M_A_cur; m_a+=32){
                            micro_kernel_float_32x32_add(&spm_C[2][OFFSET(n_a, m_a>>5, M_A>>5)], &spm_C[cnt_c][OFFSET(m_a, n_a>>5, N_A_buffer>>5)], spm_D, N_A_buffer, M_A);
                        }
                    }
                    // last_C plus beta
                #ifdef enable_beta
                    if(k + K_G >= K){
                        C_plus_beta(spm_C[cnt_c], M_A*N_A_buffer, beta);
                    }
                #endif              
                    // [3] wait for C stored
                    dma_wait_p2p(ch_cs[cnt_c_1]);

                    // [4] preload C to am_C
                    if(cc + N_A < N_G_cur){ // cc没到头
                        N_A_next = min(N_A, N_G_cur-(cc + N_A));
                        ch_cl[cnt_c_1] = dma_p2p_opt(&C[OFFSET(r, c+(cc+N_A), N)], M_A_cur, N_A_next*sizeof(float), (N-N_A_next)*sizeof(float),
                                            spm_C[cnt_c_1], M_A_cur, N_A_next*sizeof(float), (N_A_buffer-N_A_next)*sizeof(float), row_syn, p2pmask_cur, ch0_cl+cnt_c_1);
                    }else if(r_next == r_start){ // cc到头了，r也到头了，ck+1还没到头
                        N_A_next = min(N_A, N_G_next);
                        ch_cl[cnt_c_1] = dma_p2p_opt(&C[OFFSET(r_next, c_next, N)], M_A_next, N_A_next*sizeof(float), (N-N_A_next)*sizeof(float),
                                            spm_C[cnt_c_1], M_A_next, N_A_next*sizeof(float), (N_A_buffer-N_A_next)*sizeof(float), row_syn, p2pmask_next, ch0_cl+cnt_c_1);
                    }else if(r_next != -1){ // cc到头了，r没到头
                        N_A_next = min(N_A, N_G_cur);
                        ch_cl[cnt_c_1] = dma_p2p_opt(&C[OFFSET(r_next, c, N)], M_A_next, N_A_next*sizeof(float), (N-N_A_next)*sizeof(float),
                                            spm_C[cnt_c_1], M_A_next, N_A_next*sizeof(float), (N_A_buffer-N_A_next)*sizeof(float), row_syn, p2pmask_next, ch0_cl+cnt_c_1);
                    }

                    // [5] store spm_C to C
                    ch_cs[cnt_c] = dma_p2p_opt(spm_C[cnt_c], M_A_cur, N_A_cur*sizeof(float), (N_A_buffer-N_A_cur)*sizeof(float),
                                &C[OFFSET(r, c+cc, N)], M_A_cur, N_A_cur*sizeof(float), (N-N_A_cur)*sizeof(float), row_syn, p2pmask_cur, ch0_cs+cnt_c);

                    // renew idnex C
                    cnt_c = cnt_c_1;
                    N_A_cur = N_A_next;
                }
                cnt_a = cnt_a_1;
                p2pmask_cur = p2pmask_next;
                M_A_cur = M_A_next;
            }
            cnt_gsm = cnt_gsm_1;
            N_G_cur = N_G_next;
            group_barrier(b_id);
        }
        K_G_cur = K_G_next;
    }
    if(r_start < M){
        dma_wait_p2p(ch_cs[(cnt_c+1)%2]);
    }
    scalar_free(spm_B[0]);
    scalar_free(spm_B[1]);
    vector_free(spm_A[0]);
    vector_free(spm_A[1]);
    vector_free(spm_C[0]);
    vector_free(spm_C[1]);
    vector_free(spm_C[2]);
    vector_free(spm_D);
}


__global__ void  sgemm_null(unlong M, unlong N, unlong K, 
                        unlong b_id, float* A, float* B, float* C, float alpha, float beta){
}

