# 面向多核DSP的矩阵乘算子优化与实现



## 双精度通用矩阵乘 (single-precision General Matrix Multiplication)

dgemm_release_V1.0: 普通整合版本，支持大部分输入形状，支持四种转置模式，实现 C = A x B + C
dgemm_release_V1.1: 相比V1.0，增加了lda, ldb, ldc参数



## 单精度通用矩阵乘 (double-precision Multiplication)

sgemm_release_V1.3.1: 普通整合版本，支持大部分输入形状，支持四种转置模式，实现 C = A x B + C
sgemm_release_V1.3.2: 相比V1.3.1，实现了 C = alpha*A x B + beta



## 双精度不规则矩阵乘 (single-precision irregular-shaped Matrix Multiplication)

irr_shortA_dgemm_release_V0.0：shortA型不规则矩阵乘，支持大部分输入形状，支持四种转置模式，实现 C = A x B + C
irr_thinB_dgemm_release_V0.0：thinB型不规则矩阵乘，支持大部分输入形状，支持四种转置模式，实现 C = A x B + C



## 单精度不规则矩阵乘 (double-precision irregular-shaped Matrix Multiplication)

irr_shortA_sgemm_release_V0.0：shortA型不规则矩阵乘，支持大部分输入形状，支持四种转置模式，实现 C = A x B + C
irr_thinB_sgemm_release_V0.0：thinB型不规则矩阵乘，支持大部分输入形状，支持四种转置模式，实现 C = A x B + C
irr_thinB_sgemm_release_V1.2：thinB型不规则矩阵乘，支持大部分输入形状，支持四种转置模式，实现 C =alpha*AxB + beta



**(更详细的理论分析和性能测试报告放在** <u>/相关说明</u> **路径下)**



## 矩阵转置算子 (Matrix Transpose)

transpose: 支持 单精度/双精度 任意形状的矩阵转置操作。



## 五点stencil算子 (Jacobi Iteration)

stencil_2d_V1.2: 通过 指令级并行+数据预取 优化 Jacobi 迭代。






