
# yhrun -N 1 -p thcp4 --reservation=pretraining ./irr_thinB_sgemm_demo 1 24576 4096 192 8
# yhrun -N 1 -p thcp4 --reservation=pretraining ./irr_thinB_sgemm_demo 1 24576 6144 200 8

# yhrun -N 1 -p thcp4 --reservation=pretraining ./irr_thinB_sgemm_demo 1 512 2048 300 2

# yhrun -N 1 -p thcp4 --reservation=pretraining ./irr_thinB_sgemm_demo 1 700 24576 200 8

yhrun -N 1 -p thcp4 --reservation=pretraining ./irr_thinB_sgemm_demo 1 700 33278 300 8





# 单核情况：
# M = 2^16
# N = K = 16, 32, 48, 64, 80, 96
# yhrun -N 1 -p thcp4 --reservation=pretraining ./irr_thinB_sgemm_demo 1 65536 16 16 1
# yhrun -N 1 -p thcp4 --reservation=pretraining ./irr_thinB_sgemm_demo 1 65536 32 32 1
# yhrun -N 1 -p thcp4 --reservation=pretraining ./irr_thinB_sgemm_demo 1 65536 48 48 1
# yhrun -N 1 -p thcp4 --reservation=pretraining ./irr_thinB_sgemm_demo 1 65536 64 64 1
# yhrun -N 1 -p thcp4 --reservation=pretraining ./irr_thinB_sgemm_demo 1 65536 80 80 1
# yhrun -N 1 -p thcp4 --reservation=pretraining ./irr_thinB_sgemm_demo 1 65536 96 96 1

# 单核情况：
# M = K = 20480
# N = 16, 32, 48, 64, 80, 96
# yhrun -N 1 -p thcp4 --reservation=pretraining ./irr_thinB_sgemm_demo 1 20480 20480 16 1
# yhrun -N 1 -p thcp4 --reservation=pretraining ./irr_thinB_sgemm_demo 1 20480 20480 32 1
# yhrun -N 1 -p thcp4 --reservation=pretraining ./irr_thinB_sgemm_demo 1 20480 20480 48 1
# yhrun -N 1 -p thcp4 --reservation=pretraining ./irr_thinB_sgemm_demo 1 20480 20480 64 1
# yhrun -N 1 -p thcp4 --reservation=pretraining ./irr_thinB_sgemm_demo 1 20480 20480 80 1
# yhrun -N 1 -p thcp4 --reservation=pretraining ./irr_thinB_sgemm_demo 1 20480 20480 96 1

# file_name="result_singlecore.txt"

# if [ -f $file_name ];then
#   rm -f $file_name
# fi

# echo -e "单核情况：\nM = 2^16\nN = K = 16, 32, 48, 64, 80, 96" >> $file_name
# echo -e "============================================================================================================================" >> $file_name
# M=65536
# K=16
# N=16
# for ((i=0; i<6; i++))
# do
#     for ((j=0; j<3; j++))
#     do
#         yhrun -N 1 -p thcp4 --reservation=pretraining ./irr_thinB_sgemm_demo 1 $M $K $N 1 >> $file_name
#     done
#     echo -e "\n" >> $file_name
#     declare -i K=$K+16
#     declare -i N=$N+16
# done
# echo -e "============================================================================================================================\n" >> $file_name

# file_name="result_singlecore.txt"
# echo -e "单核情况：\nM = K = 20480\nN = 16, 32, 48, 64, 80, 96" >> $file_name
# echo -e "============================================================================================================================" >> $file_name
# M=20480
# K=20480
# N=16
# for ((i=0; i<6; i++))
# do
#     for ((j=0; j<3; j++))
#     do
#         yhrun -N 1 -p thcp4 --reservation=pretraining ./irr_thinB_sgemm_demo 1 $M $K $N 1 >> $file_name
#     done
#     echo -e "\n" >> $file_name
#     declare -i N=$N+16
# done
# echo -e "============================================================================================================================\n" >> $file_name


# 多核情况：
# M = 2^16
# N = K = 16, 32, 48, 64, 80, 96
# yhrun -N 1 -p thcp4 --reservation=pretraining ./irr_thinB_sgemm_demo 1 65536 16 16 8
# yhrun -N 1 -p thcp4 --reservation=pretraining ./irr_thinB_sgemm_demo 1 65536 32 32 8
# yhrun -N 1 -p thcp4 --reservation=pretraining ./irr_thinB_sgemm_demo 1 65536 48 48 8
# yhrun -N 1 -p thcp4 --reservation=pretraining ./irr_thinB_sgemm_demo 1 65536 64 64 8
# yhrun -N 1 -p thcp4 --reservation=pretraining ./irr_thinB_sgemm_demo 1 65536 80 80 8
# yhrun -N 1 -p thcp4 --reservation=pretraining ./irr_thinB_sgemm_demo 1 65536 96 96 8

# 多核情况：
# M = K = 20480
# N = 16, 32, 48, 64, 80, 96
# yhrun -N 1 -p thcp4 --reservation=pretraining ./irr_thinB_sgemm_demo 1 20480 20480 16 8
# yhrun -N 1 -p thcp4 --reservation=pretraining ./irr_thinB_sgemm_demo 1 20480 20480 32 8
# yhrun -N 1 -p thcp4 --reservation=pretraining ./irr_thinB_sgemm_demo 1 20480 20480 48 8
# yhrun -N 1 -p thcp4 --reservation=pretraining ./irr_thinB_sgemm_demo 1 20480 20480 64 8
# yhrun -N 1 -p thcp4 --reservation=pretraining ./irr_thinB_sgemm_demo 1 20480 20480 80 8
# yhrun -N 1 -p thcp4 --reservation=pretraining ./irr_thinB_sgemm_demo 1 20480 20480 96 8

# 多核情况：
# N = K = 32
# M = 65536, 131072, 262144, 524288, 1048576, 2097152, 4194304  （2^16 ~ 2^22）
# yhrun -N 1 -p thcp4 --reservation=pretraining ./irr_thinB_sgemm_demo 1 65536 32 32 8
# yhrun -N 1 -p thcp4 --reservation=pretraining ./irr_thinB_sgemm_demo 1 131072 32 32 8
# yhrun -N 1 -p thcp4 --reservation=pretraining ./irr_thinB_sgemm_demo 1 262144 32 32 8
# yhrun -N 1 -p thcp4 --reservation=pretraining ./irr_thinB_sgemm_demo 1 524288 32 32 8
# yhrun -N 1 -p thcp4 --reservation=pretraining ./irr_thinB_sgemm_demo 1 1048576 32 32 8
# yhrun -N 1 -p thcp4 --reservation=pretraining ./irr_thinB_sgemm_demo 1 2097152 32 32 8
# yhrun -N 1 -p thcp4 --reservation=pretraining ./irr_thinB_sgemm_demo 1 4194304 32 32 8

# 多核情况：
# N = 32
# M = K = 8192, 10240, 12288, 14336, 16384, 18432, 20480
# yhrun -N 1 -p thcp4 --reservation=pretraining ./irr_thinB_sgemm_demo 1 8192 8192 32 8
# yhrun -N 1 -p thcp4 --reservation=pretraining ./irr_thinB_sgemm_demo 1 10240 10240 32 8
# yhrun -N 1 -p thcp4 --reservation=pretraining ./irr_thinB_sgemm_demo 1 12288 12288 32 8
# yhrun -N 1 -p thcp4 --reservation=pretraining ./irr_thinB_sgemm_demo 1 14336 14336 32 8
# yhrun -N 1 -p thcp4 --reservation=pretraining ./irr_thinB_sgemm_demo 1 16384 16384 32 8
# yhrun -N 1 -p thcp4 --reservation=pretraining ./irr_thinB_sgemm_demo 1 18432 18432 32 8
# yhrun -N 1 -p thcp4 --reservation=pretraining ./irr_thinB_sgemm_demo 1 20480 20480 32 8


# file_name="result_multicore.txt"

# if [ -f $file_name ];then
#   rm -f $file_name
# fi

# echo -e "多核情况：\nM = 2^16\nN = K = 16, 32, 48, 64, 80, 96" >> $file_name
# echo -e "============================================================================================================================" >> $file_name
# M=65536
# K=16
# N=16
# for ((i=0; i<6; i++))
# do
#     for ((j=0; j<3; j++))
#     do
#         yhrun -N 1 -p thcp4 --reservation=pretraining ./irr_thinB_sgemm_demo 1 $M $K $N 8 >> $file_name
#     done
#     echo -e "\n" >> $file_name
#     declare -i K=$K+16
#     declare -i N=$N+16
# done
# echo -e "============================================================================================================================\n" >> $file_name

# file_name="result_multicore.txt"
# echo -e "多核情况：\nM = K = 20480\nN = 16, 32, 48, 64, 80, 96" >> $file_name
# echo -e "============================================================================================================================" >> $file_name
# M=20480
# K=20480
# N=16
# for ((i=0; i<6; i++))
# do
#     for ((j=0; j<3; j++))
#     do
#         yhrun -N 1 -p thcp4 --reservation=pretraining ./irr_thinB_sgemm_demo 1 $M $K $N 8 >> $file_name
#     done
#     echo -e "\n" >> $file_name
#     declare -i N=$N+16
# done
# echo -e "============================================================================================================================\n" >> $file_name

# file_name="result_multicore.txt"
# echo -e "多核情况：\nN = K = 32\nM = 65536, 131072, 262144, 524288, 1048576, 2097152, 4194304 (2^16 ~ 2^22)" >> $file_name
# echo -e "============================================================================================================================" >> $file_name
# M=65536
# K=32
# N=32
# for ((i=0; i<7; i++))
# do
#     for ((j=0; j<3; j++))
#     do
#         yhrun -N 1 -p thcp4 --reservation=pretraining ./irr_thinB_sgemm_demo 1 $M $K $N 8 >> $file_name
#     done
#     echo -e "\n" >> $file_name
#     declare -i M=$M*2
# done
# echo -e "============================================================================================================================\n" >> $file_name

# file_name="result_multicore.txt"
# echo -e "多核情况：\nN = 32\nM = K = 8192, 10240, 12288, 14336, 16384, 18432, 20480" >> $file_name
# echo -e "============================================================================================================================" >> $file_name
# M=8192
# K=8192
# N=32
# for ((i=0; i<7; i++))
# do
#     for ((j=0; j<3; j++))
#     do
#         yhrun -N 1 -p thcp4 --reservation=pretraining ./irr_thinB_sgemm_demo 1 $M $K $N 8 >> $file_name
#     done
#     echo -e "\n" >> $file_name
#     declare -i M=$M+2048
#     declare -i K=$K+2048
# done
# echo -e "============================================================================================================================\n" >> $file_name


# file_name="result_speedup.txt"

# if [ -f $file_name ];then
#   rm -f $file_name
# fi

# echo -e "多核情况：\nM = 1048576\nN = K = 32" >> $file_name
# echo -e "============================================================================================================================" >> $file_name
# M=1048576
# K=32
# N=32
# n_core=1
# for ((i=0; i<4; i++))
# do
#     for ((j=0; j<3; j++))
#     do
#         yhrun -N 1 -p thcp4 --reservation=pretraining ./irr_thinB_sgemm_demo 1 $M $K $N $n_core >> $file_name
#     done
#     echo -e "\n" >> $file_name
#     declare -i n_core=$n_core*2
# done
# echo -e "============================================================================================================================\n" >> $file_name

# echo -e "多核情况：\nM = K = 20480\nN = 32" >> $file_name
# echo -e "============================================================================================================================" >> $file_name
# M=20480
# K=20480
# N=32
# n_core=1
# for ((i=0; i<4; i++))
# do
#     for ((j=0; j<3; j++))
#     do
#         yhrun -N 1 -p thcp4 --reservation=pretraining ./irr_thinB_sgemm_demo 1 $M $K $N $n_core >> $file_name
#     done
#     echo -e "\n" >> $file_name
#     declare -i n_core=$n_core*2
# done
# echo -e "============================================================================================================================\n" >> $file_name
