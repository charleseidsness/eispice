# Installation Directory
LIB_PREFIX	= ../../libs
INC_PREFIX = ../../include
DATA_LIB = $(LIB_PREFIX)/libdata.a

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
CFLAGS 	?= -Wall -O2 -DLIBNAME=calculon -I. -I$(INC_PREFIX)
LDFLAGS ?= -Wall -O2
RE2C = re2c -b -s
LEMON = lemon
MAKEHEADERS = makeheaders
ARFLAGS = -cr
ifeq ($(OS), Windows_NT)
	CFLAGS += -mno-cygwin
	LDFLAGS += -mconsole -mno-cygwin
else
	CFLAGS += -fPIC
endif

LIB = libcalc.a
LIB_OBJ = parser.o tokenizer.o calc.o
INC = calc.h

EXE_LIBS = $(LIB) $(DATA_LIB) -lm
EXE_OBJ = main.o
ifeq ($(OS), Windows_NT)
	EXE = calculon.exe
else
	EXE = calculon
endif

.PHONY: all clean install dep
.SECONDARY: tokenizer.c parser.c parser.h

all: Makefile $(EXE)

dep:
	@echo DEP $@
	$(Q)for i in *.c; do $(CC) -MM $(CFLAGS) "$${i}" ; done > makefile.dep

-include makefile.dep

$(EXE): $(LIB) $(EXE_OBJ)
	@echo LD $@
	$(Q)$(CC) $(LDFLAGS) -o $(EXE) $(EXE_OBJ) $(EXE_LIBS)

$(LIB):$(LIB_OBJ) parser.h
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

%.c:%.lem
	@echo LEMON $@
	$(Q)$(LEMON) -m $<
	
parser.h:
	$(Q)rm -vf $@
	$(Q)$(MAKEHEADERS) parser.c

