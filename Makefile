
CC=gcc
CFLAGS+=-O3 -Wall
XXD=xxd
RM=rm

all: prepack6502

prepack6502: prepack6502.o compiler.o depacker.o asm/asmtab.o asm/chnkpool.o asm/expr.o asm/lexyy.o asm/log.o asm/membuf.o asm/membufio.o asm/namedbuf.o asm/parse.o asm/pc.o asm/vec.o

prepack6502.o: depacker.c

depacker.c: depacker.asm
	$(XXD) -i $< $@

clean:
	$(RM) *.o
	$(RM) asm/*.o
	$(RM) depacker.c
	$(RM) prepack6502
