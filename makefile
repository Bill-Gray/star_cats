# GNU MAKE Makefile for star catalog(ue) reading functions
#
# Usage: make [CLANG=Y] [XCOMPILE=Y] [MSWIN=Y] [tgt]
#
# where tgt can be any of:
# [all|cmcrange|cmc_xvt|g32_test... |clean]
#
#   'XCOMPILE' = cross-compile for Windows,  using MinGW,  on a Linux or BSD box
#   'MSWIN' = compile for Windows,  using MinGW,  on a Windows machine
#   'CLANG' = use clang instead of GCC;  BSD/Linux only
# None of these: compile using gcc on BSD or Linux

CC=gcc
EXE=
CFLAGS=-Wextra -Wall -O3 -pedantic -Werror
RM=rm -f

ifdef CLANG
	CC=clang
endif

ifdef MSWIN
	EXE=.exe
	RM=del
endif

ifdef DEBUG
	CFLAGS += -g
endif

ifdef XCOMPILE
	CC=x86_64-w64-mingw32-gcc
	EXE=.exe
endif

all:  cmcrange$(EXE) cmc_xvt$(EXE) extr_cmc$(EXE) gaia_ast$(EXE) \
     gaia_idx$(EXE) g32test$(EXE) \
     urat1_t$(EXE) u2test$(EXE) u3test$(EXE) u4test$(EXE)

urat1_t$(EXE): urat1_t.o urat1.o
	$(CC)  -o urat1_t$(EXE) urat1_t.o urat1.o

u2test$(EXE): u2test.o ucac2.o
	$(CC) -o u2test$(EXE) u2test.o ucac2.o

u3test$(EXE): u3test.o ucac3.o
	$(CC) -o u3test$(EXE) u3test.o ucac3.o

u4test$(EXE): u4test.o ucac4.o
	$(CC) -o u4test$(EXE) u4test.o ucac4.o

g32test$(EXE): g32test.o gaia32.o
	$(CC) -o g32test$(EXE) g32test.o gaia32.o

gaia_ast$(EXE): gaia_ast.c gaia32.o
	$(CC) -o gaia_ast$(EXE) gaia_ast.c gaia32.o -I ~/include -L ~/lib -llunar -lm

gaia_idx$(EXE): gaia_idx.o
	$(CC) -o gaia_idx$(EXE) gaia_idx.o

cmc_xvt$(EXE): cmc_xvt.o cmc.o
	$(CC) -o cmc_xvt$(EXE) cmc_xvt.o cmc.o

extr_cmc$(EXE): extr_cmc.o cmc.o get_cmc.o
	$(CC) -o extr_cmc$(EXE) extr_cmc.o cmc.o get_cmc.o

cmcrange$(EXE): cmcrange.o cmc.o
	$(CC) -o cmcrange$(EXE) cmcrange.o cmc.o

.c.o:
	$(CC) $(CFLAGS) -c $<

clean:
	-$(RM) cmcrange$(EXE)
	-$(RM) cmc_xvt$(EXE)
	-$(RM) extr_cmc$(EXE)
	-$(RM) g32test$(EXE)
	-$(RM) gaia_ast$(EXE)
	-$(RM) gaia_idx$(EXE)
	-$(RM) urat1_t$(EXE)
	-$(RM) u2test$(EXE)
	-$(RM) u3test$(EXE)
	-$(RM) u4test$(EXE)
	-$(RM) *.o
