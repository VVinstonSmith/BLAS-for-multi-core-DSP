

# yhrun -N 1 -p thcp4 --reservation=pretraining ./matrix_stencil 55 111 1 1

# yhrun -N 1 -p thcp4 --reservation=pretraining ./matrix_stencil 2048 2048 1 1
# yhrun -N 1 -p thcp4 --reservation=pretraining ./matrix_stencil 4096 4096 1 1
# yhrun -N 1 -p thcp4 --reservation=pretraining ./matrix_stencil 6144 6144 1 1
# yhrun -N 1 -p thcp4 --reservation=pretraining ./matrix_stencil 8192 8192 1 1




# yhalloc -p thmt1 -w cn6355
# yhalloc -p thcp4 -w cn15627

# yhrun -N 1 -p thmt1 --reservation=pretraining ./matrix_stencil 4096 4096 1 1

yhrun -N 1 -p thcp4 --reservation=pretraining ./matrix_stencil 4096 4096 4 1