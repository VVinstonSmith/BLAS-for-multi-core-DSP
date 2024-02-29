
# yhrun -N 1 -p thcp4 --reservation=pretraining ./irr_thinB_dgemm_demo 1 128 2048 192 1

yhrun -N 1 -p thcp4 --reservation=pretraining ./irr_thinB_dgemm_demo 1 12288 4096 192 8

# yhrun -N 1 -p thcp4 --reservation=pretraining ./irr_thinB_dgemm_demo 1 512 2048 192 4


# yhrun -N 1 -p thcp4 --reservation=pretraining ./irr_thinB_dgemm_demo 1 4096 2048 192 8
