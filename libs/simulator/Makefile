# Installation Directory
LIB_PREFIX	= ..
INC_PREFIX = ../../include
DATA_LIB = $(LIB_PREFIX)/libdata.a
CALC_LIB = $(LIB_PREFIX)/libcalc.a
SUPERLU_LIB = $(LIB_PREFIX)/libsuperlu.a
BLAS_LIB = $(LIB_PREFIX)/libblas.a
LAPACK_LIB = $(LIB_PREFIX)/liblapack.a
TOMS_LIB = $(LIB_PREFIX)/libtoms.a
CEPHES_LIB = $(LIB_PREFIX)/libcephes.a

# Quiet (set to @ for a quite compile)
Q	?= @
#Q	?= 

# Build Tools
CROSS	?=
CC 	:= $(CROSS)gcc
AR 	:= $(CROSS)ar
RANLIB 	:= $(CROSS)ranlib
OBJDUMP	:= $(CROSS)objdump

# Tool Flags
CFLAGS 	?= -Wall -O2 -DLIBNAME=simulator -I./include/ -I$(INC_PREFIX)
LDFLAGS ?= -Wall -O2
RE2C = re2c -b -s
MAKEHEADERS = makeheaders
ARFLAGS = -cr
ifeq ($(OS), Windows_NT)
	CFLAGS += -mno-cygwin
	LDFLAGS += -mconsole -mno-cygwin
else
	CFLAGS += -fPIC
endif

LIB = libsimulator.a
SRC_OBJS = control.o device.o history.o matrix.o node.o row.o simulator.o
DEV_OBJS = capacitor.o source_i.o source_v.o vicurve.o inductor.o  resistor.o\
		tline.o nonlinear_i.o nonlinear_v.o callback_v.o callback_i.o tline_w.o\
		nonlinear_c.o
MATH_OBJS = checkbreak.o checklinear.o integrator.o piecewise.o waveform.o \
		history_interp.o complex.o mfunc.o netlib.o
LIB_OBJ = $(addprefix core/, $(SRC_OBJS)) \
	$(addprefix devices/, $(DEV_OBJS)) \
	$(addprefix math/, $(MATH_OBJS))
INC = ./include/simulator.h

EXE_LIBS = $(LIB) $(SUPERLU_LIB) $(LAPACK_LIB) $(BLAS_LIB) $(CALC_LIB) \
		$(DATA_LIB) $(TOMS_LIB) $(CEPHES_LIB) -lgfortran -lm
EXE_OBJ = tester.o
ifeq ($(OS), Windows_NT)
	EXE = tester.exe
else
	EXE = tester
endif

.PHONY: all clean install devel dep
.SECONDARY: ./core/parse.c

all: Makefile $(EXE)

dep:
	@echo DEP $@
	$(Q)for i in *.c; do $(CC) -MM $(CFLAGS) "$${i}" ; done > makefile.dep
	$(Q)for i in core/*.c; do echo -n core/ ; $(CC) -MM $(CFLAGS) "$${i}" ; done >> makefile.dep
	$(Q)for i in devices/*.c; do echo -n devices/ ; $(CC) -MM $(CFLAGS) "$${i}" ; done >> makefile.dep
	$(Q)for i in math/*.c; do echo -n math/ ; $(CC) -MM $(CFLAGS) "$${i}" ; done >> makefile.dep

-include makefile.dep

$(EXE): $(LIB) $(EXE_OBJ)
	@echo LD $@
	$(Q)$(CC) $(LDFLAGS) -o $(EXE) $(EXE_OBJ) $(EXE_LIBS)

$(LIB):$(LIB_OBJ)
	@echo AR $@
	$(Q)$(AR) $(ARFLAGS) $@ $(LIB_OBJ)
	$(Q)$(RANLIB) $@
	
clean:
	@echo Cleaning...
	$(Q)rm -vf $(EXE_OBJ) $(EXE)
	$(Q)rm -vf $(LIB_OBJ) $(LIB) 
	$(Q)rm -vf *~ */*~

$(LIB_PREFIX)/$(LIB):$(LIB)
	@echo Installing...
	$(Q)cp $(LIB) $(LIB_PREFIX)
	$(Q)cp $(INC) $(INC_PREFIX)

install:$(LIB_PREFIX)/$(LIB)

%.o:%.c
	@echo CC $@
	$(Q)$(CC) $(CFLAGS) -c -o $@ $<	
	
%.dis:%.o
	@echo DIS $@
	$(Q)$(OBJDUMP) -D $< >> $@

%.c:%.re
	@echo RE2C $@
	$(Q)$(RE2C) -o $@ $<

