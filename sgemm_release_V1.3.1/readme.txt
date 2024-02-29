-------------------------------------------------------------
功能:

GEMM计算: C = A(^T) x B(^T) + C
矩阵形状: A(MxK), B(KxN), C(MxN)
GEMM kernel调用接口见 sgemm_kernel.h

-------------------------------------------------------------
输入数据要求:

支持大多数输入形状，除了:
1.任何维度的长度是奇数

2.输入形状与分块参数之间满足以下关系: 
for NN, NT, TN mode:
    (M <= M_A) && (K > K_G) && (N <= N_A * (2*n_cores-1))
for TT mode:
    (N <= N_A) && (K > K_G) && (M <= M_A * (2*n_cores-1))

各种转置模式的分块参数如下:
NN mode:
    M_G = 576; K_G = 512; N_A = 128; M_A = 144;
NT mode:
    M_G = 576; K_G = 512; N_A = 128; M_A = 144;
TN mode:
    M_G = 864; K_G = 512; N_A = 128; M_A = 144;
TT mode:
    N_G = 576; K_G = 512; M_A = 128; N_A = 144;
-------------------------------------------------------------
示例:

1. 在/host_code路径下编译host端文件sgemm_demo.c
2. 在/device_code路径下编译device端文件sgemm_launcher.c (其中会调用sgemm_lib)
3. 进入/bin目录, 在命令行中输入:
    yhrun -N 1 -p thcp4 --reservation=pretraining ./sgemm_demo 1 M K N P
    其中 M,K,N 是输入矩阵三个维度的大小, P是运行的dsp核数。
    得到GEMM程序的执行结果和性能数据。
    打开sgemm_demo.c中的宏定义CHECK_DATA, 可与cpu的计算结果进行对比。
-------------------------------------------------------------



