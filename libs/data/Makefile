# Installation Directory
LIB_PREFIX	= ../../libs
INC_PREFIX = ../../include

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
CFLAGS 	?= -Wall -O2 -DLIBNAME=data -I. -I$(INC_PREFIX)
LDFLAGS ?= -Wall -O2
ARFLAGS = -cr
ifeq ($(OS), Windows_NT)
	CFLAGS += -mno-cygwin
	LDFLAGS += -mconsole -mno-cygwin
else
	CFLAGS += -fPIC
endif

LIB = libdata.a
LIB_OBJ = list.o dblhash.o data.o hash.o
INC = data.h

EXE_LIBS = $(LIB)
EXE_OBJ = test.o
ifeq ($(OS), Windows_NT)
	EXE = test.exe
else
	EXE = test
endif

.PHONY: all clean install dep

all: Makefile $(EXE)

dep:
	@echo DEP $@
	$(Q)for i in *.c; do $(CC) -MM $(CFLAGS) "$${i}" ; done > makefile.dep

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
	$(Q)rm -vf *~

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

