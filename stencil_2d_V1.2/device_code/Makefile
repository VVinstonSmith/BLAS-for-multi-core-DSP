GCCROOT=/thfs1/software/programming_env/mt3000_programming_env/dsp_compiler
ENV_ROOT=/thfs1/software/programming_env/mt3000_programming_env/hthreads

CC=MT-3000-gcc
AR=MT-3000-ar
LD=MT-3000-ld

CFLAGS=-c -O2 -fenable-m3000 -ffunction-sections -flax-vector-conversions -I./ -I$(ENV_ROOT)/include -I$(GCCROOT)/include/
LDFLAGS= -L$(ENV_ROOT)/lib --gc-sections -Tdsp.lds
SRC=kernel_stencil.c
OBJ=kernel_stencil.o
ASM=kernel_stencil.s
EXE=kernel_stencil.out
DAT=kernel_stencil.dat

ALL: $(EXE)
	$(GCCROOT)/bin/MT-3000-makedat -J $(EXE)
	cp ${DAT} ../bin

$(OBJ): $(SRC)
	$(GCCROOT)/bin/$(CC) $(CFLAGS) $(SRC) -o $(OBJ) 
	$(GCCROOT)/bin/$(CC) -c -O2 -fenable-m3000 -ffunction-sections -flax-vector-conversions -S -I./ -I$(ENV_ROOT)/include -I$(GCCROOT)/include/ $(SRC) -o $(ASM)

$(EXE): $(OBJ)
	$(GCCROOT)/bin/$(LD) $(LDFLAGS) $(OBJ) $(ENV_ROOT)/lib/libhthread_device.a $(GCCROOT)/lib/vlib3000.a $(GCCROOT)/lib/slib3000.a -o $(EXE) 

clean:
	rm $(EXE) $(OBJ) $(DAT)
