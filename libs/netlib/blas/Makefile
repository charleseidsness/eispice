# Installation Directory
LIB_PREFIX	= ../..
INC_PREFIX = ../../../include

# Quiet (set to @ for a quite compile)
Q	?= @
#Q	?= 

# Build Tools
CROSS	?=
CC 	:= $(CROSS)gcc
FC := $(CROSS)gfortran
AR 	:= $(CROSS)ar
RANLIB 	:= $(CROSS)ranlib
OBJDUMP	:= $(CROSS)objdump

# Tool Flags
CFLAGS 	?= -Wall -O2 -I.
FFLAGS	?= -Wall -Wno-uninitialized -funroll-all-loops -O3
LDFLAGS ?= -Wall -O2
ARFLAGS = -cr
ifeq ($(OS), Windows_NT)
	CFLAGS += -mno-cygwin
	FFLAGS += -mno-cygwin
	LDFLAGS += -mconsole -mno-cygwin
else
	CFLAGS 	+= -fPIC
	FFLAGS	+= -fPIC
endif

LIB = libblas.a
SRC_BLAS = idamax.o dasum.o daxpy.o dcopy.o ddot.o dnrm2.o \
	 drot.o dscal.o dgemv.o dsymv.o dtrsv.o dger.o dsyr2.o \
	 zsymm.o zgemm.o xerbla.o lsame.o zcopy.o ztrmm.o \
	 zaxpy.o zswap.o izamax.o zdscal.o ztrsm.o zdotu.o zdotc.o \
	 zgemv.o zscal.o ztrmv.o dcabs1.o dznrm2.o dznrm2.o zgeru.o \
	 zgerc.o dtrsm.o dgemm.o dswap.o dtrmm.o dtrmv.o


LIB_OBJ = $(addprefix SRC/, $(SRC_BLAS))

.PHONY: all clean install

all: Makefile $(LIB)

$(LIB):$(LIB_OBJ)
	@echo AR $@
	$(Q)$(AR) $(ARFLAGS) $@ $(LIB_OBJ)
	$(Q)$(RANLIB) $@
	
clean:
	@echo Cleaning...
	$(Q)rm -vf $(LIB_OBJ) $(LIB) 
	$(Q)rm -vf *~ */*~

$(LIB_PREFIX)/$(LIB):$(LIB)
	@echo Installing...
	$(Q)cp $(LIB) $(LIB_PREFIX)	

install:$(LIB_PREFIX)/$(LIB)

%.o:%.c
	@echo CC $@
	$(Q)$(CC) $(CFLAGS) -c -o $@ $<

%.o:%.f
	@echo FORTRAN $@
	$(Q)$(FC) $(FFLAGS) -c -o $@ $<

%.dis:%.o
	@echo DIS $@
	$(Q)$(OBJDUMP) -D $< >> $@

