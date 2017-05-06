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
FFLAGS	?= -Wall -Wno-uninitialized -Wno-unused -funroll-all-loops -O3
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

LIB = liblapack.a
SRC_LAPACK = zgesv.o zgees.o ilaenv.o zhseqr.o dlabad.o zlange.o zlascl.o \
	zgebal.o zgehrd.o zhseqr.o ztrsen.o zlacpy.o zunghr.o zgebak.o \
	zgetrf.o zgetrs.o ztrexc.o zlacn2.o ztrsyl.o zungqr.o zgehd2.o zlahr2.o \
	zlarfb.o zlassq.o zlaqr0.o zlaset.o iparmq.o ieeeck.o zlahqr.o zlaswp.o \
	zgetf2.o dzsum1.o izmax1.o zladiv.o zrot.o zlarf.o dlapy2.o \
	zlartg.o zung2r.o zlarft.o zung2r.o zlarfg.o zlacgv.o dladiv.o dlapy3.o \
	zlahqr.o zlaqr3.o zlaqr5.o zlaqr4.o zlahqr.o zlaqr1.o zlaqr2.o dlamch.o \
	dgesv.o dgetrf.o dgetrs.o dgetf2.o dlaswp.o dgetri.o dtrtri.o dtrti2.o 

LIB_OBJ = $(addprefix SRC/, $(SRC_LAPACK))

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

##################################
# Do not optimize these routines #
##################################
SRC/dlamch.o: SRC/dlamch.f
	@echo FORTRAN_NO_OPT $@
	$(Q)$(FC) -c $(NOOPT) -c -o $@ $<
##################################

%.o:%.c
	@echo CC $@
	$(Q)$(CC) $(CFLAGS) -c -o $@ $<

%.o:%.f
	@echo FORTRAN $@
	$(Q)$(FC) $(FFLAGS) -c -o $@ $<

%.dis:%.o
	@echo DIS $@
	$(Q)$(OBJDUMP) -D $< >> $@

