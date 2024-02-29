ENV_ROOT=/thfs1/software/programming_env/mt3000_programming_env/hthreads

EXE=dgemm_demo
ALL:
	gcc -O2 ${EXE}.c -std=c99 -I./ -I$(ENV_ROOT)/include $(ENV_ROOT)/lib/libhthread_host.a -lpthread -fopenmp -o ${EXE}
	cp ${EXE} ../bin

clean:
	rm ${EXE}
