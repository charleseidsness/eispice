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

LIB = libtoms.a
LIB_OBJ = rpoly.o

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

