
# Fig 1
#----------------------------------------------------------------------------------
#(a)
# yhrun -N 1 -p thcp4 --reservation=pretraining ./dgemm_demo 1 1512 2048 1536 4
# yhrun -N 1 -p thcp4 --reservation=pretraining ./dgemm_demo 1 3024 2048 1536 1
# yhrun -N 1 -p thcp4 --reservation=pretraining ./dgemm_demo 1 6048 2048 1536 1
# yhrun -N 1 -p thcp4 --reservation=pretraining ./dgemm_demo 1 12096 2048 1536 1
# yhrun -N 1 -p thcp4 --reservation=pretraining ./dgemm_demo 1 24192 2048 1536 1
# yhrun -N 1 -p thcp4 --reservation=pretraining ./dgemm_demo 1 48384 2048 1536 1
#(b)
# yhrun -N 1 -p thcp4 --reservation=pretraining ./dgemm_demo 1 3024 512 1536 1
# yhrun -N 1 -p thcp4 --reservation=pretraining ./dgemm_demo 1 3024 1024 1536 1
# yhrun -N 1 -p thcp4 --reservation=pretraining ./dgemm_demo 1 3024 2048 1536 1
# yhrun -N 1 -p thcp4 --reservation=pretraining ./dgemm_demo 1 3024 4096 1536 1
# yhrun -N 1 -p thcp4 --reservation=pretraining ./dgemm_demo 1 3024 8192 1536 1
# yhrun -N 1 -p thcp4 --reservation=pretraining ./dgemm_demo 1 3024 16384 1536 1
#(c)
# yhrun -N 1 -p thcp4 --reservation=pretraining ./dgemm_demo 1 3024 20480 48 1
# yhrun -N 1 -p thcp4 --reservation=pretraining ./dgemm_demo 1 3024 20480 96 1
# yhrun -N 1 -p thcp4 --reservation=pretraining ./dgemm_demo 1 3024 20480 192 1
# yhrun -N 1 -p thcp4 --reservation=pretraining ./dgemm_demo 1 3024 2048 384 1
# yhrun -N 1 -p thcp4 --reservation=pretraining ./dgemm_demo 1 3024 2048 768 1
# yhrun -N 1 -p thcp4 --reservation=pretraining ./dgemm_demo 1 3024 2048 1536 1
# yhrun -N 1 -p thcp4 --reservation=pretraining ./dgemm_demo 1 3024 2048 3072 1
# yhrun -N 1 -p thcp4 --reservation=pretraining ./dgemm_demo 1 3024 2048 6144 1
#----------------------------------------------------------------------------------


# Fig 2 

# K=2048 N=1536
#----------------------------------------------------------------------------------
# M=1512
# yhrun -N 1 -p thcp4 --reservation=pretraining ./dgemm_demo 1 1512 2048 1536 1
# yhrun -N 1 -p thcp4 --reservation=pretraining ./dgemm_demo 1 1512 2048 3072 2
# yhrun -N 1 -p thcp4 --reservation=pretraining ./dgemm_demo 1 1512 2048 6144 4
# yhrun -N 1 -p thcp4 --reservation=pretraining ./dgemm_demo 1 1512 2048 12288 8

# M=3024
# yhrun -N 1 -p thcp4 --reservation=pretraining ./dgemm_demo 1 3024 2048 1536 1
# yhrun -N 1 -p thcp4 --reservation=pretraining ./dgemm_demo 1 3024 2048 3072 2
# yhrun -N 1 -p thcp4 --reservation=pretraining ./dgemm_demo 1 3024 2048 6144 4
# yhrun -N 1 -p thcp4 --reservation=pretraining ./dgemm_demo 1 3024 2048 12288 8

# M=6048
# yhrun -N 1 -p thcp4 --reservation=pretraining ./dgemm_demo 1 6048 2048 1536 1
# yhrun -N 1 -p thcp4 --reservation=pretraining ./dgemm_demo 1 6048 2048 3072 2
# yhrun -N 1 -p thcp4 --reservation=pretraining ./dgemm_demo 1 6048 2048 6144 4
# yhrun -N 1 -p thcp4 --reservation=pretraining ./dgemm_demo 1 6048 2048 12288 8

# M=12096
# yhrun -N 1 -p thcp4 --reservation=pretraining ./dgemm_demo 1 12096 2048 1536 1
# yhrun -N 1 -p thcp4 --reservation=pretraining ./dgemm_demo 1 12096 2048 3072 2
# yhrun -N 1 -p thcp4 --reservation=pretraining ./dgemm_demo 1 12096 2048 6144 4
# yhrun -N 1 -p thcp4 --reservation=pretraining ./dgemm_demo 1 12096 2048 12288 8
#----------------------------------------------------------------------------------

# M=3024 N=1536
#----------------------------------------------------------------------------------
# K=512
# yhrun -N 1 -p thcp4 --reservation=pretraining ./dgemm_demo 1 3024 512 1536 1
# yhrun -N 1 -p thcp4 --reservation=pretraining ./dgemm_demo 1 3024 512 3072 2
# yhrun -N 1 -p thcp4 --reservation=pretraining ./dgemm_demo 1 3024 512 6144 4
# yhrun -N 1 -p thcp4 --reservation=pretraining ./dgemm_demo 1 3024 512 12288 8

# K=1024
# yhrun -N 1 -p thcp4 --reservation=pretraining ./dgemm_demo 1 3024 1024 1536 1
# yhrun -N 1 -p thcp4 --reservation=pretraining ./dgemm_demo 1 3024 1024 3072 2
# yhrun -N 1 -p thcp4 --reservation=pretraining ./dgemm_demo 1 3024 1024 6144 4
# yhrun -N 1 -p thcp4 --reservation=pretraining ./dgemm_demo 1 3024 1024 12288 8

# K=2048
# yhrun -N 1 -p thcp4 --reservation=pretraining ./dgemm_demo 1 3024 2048 1536 1
# yhrun -N 1 -p thcp4 --reservation=pretraining ./dgemm_demo 1 3024 2048 3072 2
# yhrun -N 1 -p thcp4 --reservation=pretraining ./dgemm_demo 1 3024 2048 6144 4
# yhrun -N 1 -p thcp4 --reservation=pretraining ./dgemm_demo 1 3024 2048 12288 8

# K=4096
# yhrun -N 1 -p thcp4 --reservation=pretraining ./dgemm_demo 1 3024 4096 1536 1
# yhrun -N 1 -p thcp4 --reservation=pretraining ./dgemm_demo 1 3024 4096 3072 2
# yhrun -N 1 -p thcp4 --reservation=pretraining ./dgemm_demo 1 3024 4096 6144 4
# yhrun -N 1 -p thcp4 --reservation=pretraining ./dgemm_demo 1 3024 4096 12288 8
#----------------------------------------------------------------------------------


# M=3024 K=2048
#----------------------------------------------------------------------------------
# N=48
# yhrun -N 1 -p thcp4 --reservation=pretraining ./dgemm_demo 1 3024 20480 48 1
# yhrun -N 1 -p thcp4 --reservation=pretraining ./dgemm_demo 1 3024 20480 96 2
# yhrun -N 1 -p thcp4 --reservation=pretraining ./dgemm_demo 1 3024 20480 192 4
# yhrun -N 1 -p thcp4 --reservation=pretraining ./dgemm_demo 1 3024 20480 384 8

# N=96
# yhrun -N 1 -p thcp4 --reservation=pretraining ./dgemm_demo 1 3024 20480 96 1
# yhrun -N 1 -p thcp4 --reservation=pretraining ./dgemm_demo 1 3024 20480 192 2
# yhrun -N 1 -p thcp4 --reservation=pretraining ./dgemm_demo 1 3024 20480 384 4
# yhrun -N 1 -p thcp4 --reservation=pretraining ./dgemm_demo 1 3024 20480 768 8

# N=192
# yhrun -N 1 -p thcp4 --reservation=pretraining ./dgemm_demo 1 3024 20480 192 1
# yhrun -N 1 -p thcp4 --reservation=pretraining ./dgemm_demo 1 3024 2048 384 2
# yhrun -N 1 -p thcp4 --reservation=pretraining ./dgemm_demo 1 3024 2048 768 4
# yhrun -N 1 -p thcp4 --reservation=pretraining ./dgemm_demo 1 3024 20480 1536 8

# N=384
# yhrun -N 1 -p thcp4 --reservation=pretraining ./dgemm_demo 1 3024 20480 384 1
# yhrun -N 1 -p thcp4 --reservation=pretraining ./dgemm_demo 1 3024 2048 768 2
# yhrun -N 1 -p thcp4 --reservation=pretraining ./dgemm_demo 1 3024 2048 1536 4
# yhrun -N 1 -p thcp4 --reservation=pretraining ./dgemm_demo 1 3024 2048 3072 8

# N=768
# yhrun -N 1 -p thcp4 --reservation=pretraining ./dgemm_demo 1 3024 2048 768 1
# yhrun -N 1 -p thcp4 --reservation=pretraining ./dgemm_demo 1 3024 2048 1536 2
# yhrun -N 1 -p thcp4 --reservation=pretraining ./dgemm_demo 1 3024 2048 3072 4
# yhrun -N 1 -p thcp4 --reservation=pretraining ./dgemm_demo 1 3024 2048 6144 8

# N=1536
# yhrun -N 1 -p thcp4 --reservation=pretraining ./dgemm_demo 1 3024 2048 1536 1
# yhrun -N 1 -p thcp4 --reservation=pretraining ./dgemm_demo 1 3024 2048 3072 2
# yhrun -N 1 -p thcp4 --reservation=pretraining ./dgemm_demo 1 3024 2048 6144 4
# yhrun -N 1 -p thcp4 --reservation=pretraining ./dgemm_demo 1 3024 2048 12288 8
#----------------------------------------------------------------------------------


# Fig 1
#----------------------------------------------------------------------------------
#(a)
# yhrun -N 1 -p thcp4 --reservation=pretraining ./dgemm_demo 1 1512 2048 1536 1
# yhrun -N 1 -p thcp4 --reservation=pretraining ./dgemm_demo 1 3024 2048 1536 1
# yhrun -N 1 -p thcp4 --reservation=pretraining ./dgemm_demo 1 6048 2048 1536 1
# yhrun -N 1 -p thcp4 --reservation=pretraining ./dgemm_demo 1 12096 2048 1536 1
# yhrun -N 1 -p thcp4 --reservation=pretraining ./dgemm_demo 1 24192 2048 1536 1
# yhrun -N 1 -p thcp4 --reservation=pretraining ./dgemm_demo 1 48384 2048 1536 1
# yhrun -N 1 -p thcp4 --reservation=pretraining ./dgemm_demo 1 96768 2048 1536 1
#(b)
# yhrun -N 1 -p thcp4 --reservation=pretraining ./dgemm_demo 1 6048 128 1536 1
# yhrun -N 1 -p thcp4 --reservation=pretraining ./dgemm_demo 1 6048 256 1536 1
# yhrun -N 1 -p thcp4 --reservation=pretraining ./dgemm_demo 1 6048 512 1536 1
# yhrun -N 1 -p thcp4 --reservation=pretraining ./dgemm_demo 1 6048 1024 1536 1
# yhrun -N 1 -p thcp4 --reservation=pretraining ./dgemm_demo 1 6048 2048 1536 1
# yhrun -N 1 -p thcp4 --reservation=pretraining ./dgemm_demo 1 6048 4096 1536 1
# yhrun -N 1 -p thcp4 --reservation=pretraining ./dgemm_demo 1 6048 8192 1536 1
# yhrun -N 1 -p thcp4 --reservation=pretraining ./dgemm_demo 1 6048 16384 1536 1
#(c)
# yhrun -N 1 -p thcp4 --reservation=pretraining ./dgemm_demo 1 6048 20480 48 1
# yhrun -N 1 -p thcp4 --reservation=pretraining ./dgemm_demo 1 6048 20480 96 1
# yhrun -N 1 -p thcp4 --reservation=pretraining ./dgemm_demo 1 6048 20480 192 1
# yhrun -N 1 -p thcp4 --reservation=pretraining ./dgemm_demo 1 6048 2048 384 1
# yhrun -N 1 -p thcp4 --reservation=pretraining ./dgemm_demo 1 6048 2048 768 1
# yhrun -N 1 -p thcp4 --reservation=pretraining ./dgemm_demo 1 6048 2048 1536 1
# yhrun -N 1 -p thcp4 --reservation=pretraining ./dgemm_demo 1 6048 2048 3072 1
# yhrun -N 1 -p thcp4 --reservation=pretraining ./dgemm_demo 1 6048 2048 12288 1
# yhrun -N 1 -p thcp4 --reservation=pretraining ./dgemm_demo 1 6048 2048 24576 1
#----------------------------------------------------------------------------------

# Fig2
# M=6048, K=2048, N=1536 
# for K_G = 512, 256, 128
#----------------------------------------------------------------------------------
# yhrun -N 1 -p thcp4 --reservation=pretraining ./dgemm_demo 1 2304 2048 1536 1

# yhrun -N 1 -p thcp4 --reservation=pretraining ./dgemm_demo 1 579 1022 63 3
# yhrun -N 1 -p thcp4 --reservation=pretraining ./dgemm_demo 1 579 1022 64 3
# yhrun -N 1 -p thcp4 --reservation=pretraining ./dgemm_demo 1 579 1022 65 3

# yhrun -N 1 -p thcp4 --reservation=pretraining ./dgemm_demo 1 579 1022 128 3
# yhrun -N 1 -p thcp4 --reservation=pretraining ./dgemm_demo 1 579 1022 129 3

# yhrun -N 1 -p thcp4 --reservation=pretraining ./dgemm_demo 1 579 1022 191 3
# yhrun -N 1 -p thcp4 --reservation=pretraining ./dgemm_demo 1 579 1022 192 3
# yhrun -N 1 -p thcp4 --reservation=pretraining ./dgemm_demo 1 579 1022 193 3

# yhrun -N 1 -p thcp4 --reservation=pretraining ./dgemm_demo 1 579 1022 255 3
# yhrun -N 1 -p thcp4 --reservation=pretraining ./dgemm_demo 1 579 1022 256 3
# yhrun -N 1 -p thcp4 --reservation=pretraining ./dgemm_demo 1 579 1022 257 3

# yhrun -N 1 -p thcp4 --reservation=pretraining ./dgemm_demo 1 579 1022 319 3
# yhrun -N 1 -p thcp4 --reservation=pretraining ./dgemm_demo 1 579 1022 320 3
# yhrun -N 1 -p thcp4 --reservation=pretraining ./dgemm_demo 1 579 1022 321 3

# yhrun -N 1 -p thcp4 --reservation=pretraining ./dgemm_demo 1 579 1022 383 3
# yhrun -N 1 -p thcp4 --reservation=pretraining ./dgemm_demo 1 579 1022 384 3
# yhrun -N 1 -p thcp4 --reservation=pretraining ./dgemm_demo 1 579 1022 385 3

# yhrun -N 1 -p thcp4 --reservation=pretraining ./dgemm_demo 1 579 1022 447 3
# yhrun -N 1 -p thcp4 --reservation=pretraining ./dgemm_demo 1 579 1022 448 3
# yhrun -N 1 -p thcp4 --reservation=pretraining ./dgemm_demo 1 579 1022 449 3

# yhrun -N 1 -p thcp4 --reservation=pretraining ./dgemm_demo 1 579 1022 512 3
# yhrun -N 1 -p thcp4 --reservation=pretraining ./dgemm_demo 1 579 1022 513 3

# yhrun -N 1 -p thcp4 --reservation=pretraining ./dgemm_demo 1 579 1022 576 3
# yhrun -N 1 -p thcp4 --reservation=pretraining ./dgemm_demo 1 579 1022 577 3


# M = 4, 8, 12, 16,  576, 580, 588, 592, 596
# K = 4, 8, 12, 512, 516, 1022, 1024, 1028
# N = 65, 113, 128, 131, 190, 192, 197

# yhrun -N 1 -p thcp4 --reservation=pretraining ./dgemm_demo 1 1311 1112 128 8
# yhrun -N 1 -p thcp4 --reservation=pretraining ./dgemm_demo 1 2304 2048 3072 2
# yhrun -N 1 -p thcp4 --reservation=pretraining ./dgemm_demo 1 2304 2048 6144 4
# yhrun -N 1 -p thmt1 --reservation=pretraining ./dgemm_demo 1 2304 2048 12288 8
# yhrun -N 1 -p thcp4 --reservation=pretraining ./dgemm_demo 1 2304 2048 18432 12
#----------------------------------------------------------------------------------
# yhalloc -p thcp4 -w cn15627

yhrun -N 1 -p thcp4 --reservation=pretraining ./dgemm_demo 1 1152 2048 1536 1

# yhrun -N 1 -p thcp4 --reservation=pretraining ./dgemm_demo 1 2304 2048 3072 1

