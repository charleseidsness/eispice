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
NOOPTS	?= -Wall -DAdd_
CFLAGS 	?= $(NOOPTS) -O2
LDFLAGS ?= -Wall -O2
ARFLAGS = -cr
ifeq ($(OS), Windows_NT)
	CFLAGS += -mno-cygwin
	LDFLAGS += -mconsole -mno-cygwin
else
	CFLAGS += -fPIC
endif

LIB = libsuperlu.a
SRC_DBL_OBJS = dgssv.o dgssvx.o \
		dsp_blas2.o dsp_blas3.o dgscon.o \
		dlangs.o dgsequ.o dlaqgs.o dpivotgrowth.o  \
		dgsrfs.o dgstrf.o dgstrs.o dcopy_to_ucol.o \
		dsnode_dfs.o dsnode_bmod.o \
		dpanel_dfs.o dpanel_bmod.o dreadhb.o \
		dcolumn_dfs.o dcolumn_bmod.o dpivotL.o dpruneL.o \
		dmemory.o dutil.o dmyblas2.o
SRC_AUX 	= superlu_timer.o util.o memory.o get_perm_c.o mmd.o \
	  sp_coletree.o sp_preorder.o sp_ienv.o relax_snode.o \
	  heap_relax_snode.o colamd.o dlacon.o
LIB_OBJ = $(addprefix SRC/, $(SRC_AUX)) $(addprefix SRC/, $(SRC_DBL_OBJS))
INC = superlu.h

.PHONY: all clean install dep

all: Makefile $(LIB)

dep:
	@echo DEP $@
	$(Q)for i in SRC/*.c; do echo -n SRC/ ; $(CC) -MM $(CFLAGS) "$${i}" ; done > makefile.dep

-include makefile.dep

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
	$(Q)cp $(INC) $(INC_PREFIX)

install:$(LIB_PREFIX)/$(LIB)

##################################
# Do not optimize these routines #
##################################
SRC/dlamch.o: SRC/dlamch.c
	@echo CC_NO_OPT $@
	$(Q)$(CC) -c $(NOOPTS) -c -o $@ $<
SRC/superlu_timer.o:  SRC/superlu_timer.c
	@echo CC_NO_OPT $@
	$(Q)$(CC) -c $(NOOPTS) -c -o $@ $<
##################################

%.o:%.c
	@echo CC $@
	$(Q)$(CC) $(CFLAGS) -c -o $@ $<

%.dis:%.o
	@echo DIS $@
	$(Q)$(OBJDUMP) -D $< >> $@

