# Source Code Diectories
SIM_DIR = libs/simulator
SPARSE_DIR = libs/superlu
BLAS_DIR = libs/netlib/blas
LAPACK_DIR = libs/netlib/lapack
TOMS_DIR = libs/netlib/toms
CEPHES_DIR = libs/netlib/cephes
CALC_DIR = libs/calculon
LOG_DIR = libs/log
DATA_DIR = libs/data

# Quiet (set to @ for a quite compile)
Q	?= @
#Q	?= 

# Build Tools
CROSS	?=
CC 	:= $(CROSS)gcc
AR 	:= $(CROSS)ar
RANLIB 	:= $(CROSS)ranlib
OBJDUMP	:= $(CROSS)objdump

ifeq ($(OS), Windows_NT)
	EISPICE = _eispice.pyd
	PYFLAG = -c mingw32
else
	EISPICE = eispice.so
endif

.PHONY: all clean distclean install windist uninstall libs

all: Makefile $(EISPICE)

$(EISPICE):
	@echo Building Module
	$(Q)python setup.py build $(PYFLAG)
	$(Q)cp build/*/simulator_.so module/
	$(Q)rm -fdr build

libs:
	@echo Building Libraries...
	$(Q)(cd $(LOG_DIR); $(MAKE) install)
	$(Q)(cd $(DATA_DIR); $(MAKE) install)
	$(Q)(cd $(BLAS_DIR); $(MAKE) install)
	$(Q)(cd $(TOMS_DIR); $(MAKE) install)
	$(Q)(cd $(CEPHES_DIR); $(MAKE) install)
	$(Q)(cd $(LAPACK_DIR); $(MAKE) install)
	$(Q)(cd $(SPARSE_DIR); $(MAKE) install)
	$(Q)(cd $(CALC_DIR); $(MAKE) install)
	$(Q)(cd $(SIM_DIR); $(MAKE) install)	

distclean:
	@echo Cleaning...
	$(Q)(cd $(DATA_DIR); $(MAKE) clean)
	$(Q)(cd $(BLAS_DIR); $(MAKE) clean)
	$(Q)(cd $(TOMS_DIR); $(MAKE) clean)
	$(Q)(cd $(CEPHES_DIR); $(MAKE) clean)
	$(Q)(cd $(LAPACK_DIR); $(MAKE) clean)
	$(Q)(cd $(SPARSE_DIR); $(MAKE) clean)
	$(Q)(cd $(CALC_DIR); $(MAKE) clean)	
	$(Q)(cd $(SIM_DIR); $(MAKE) clean)
	$(Q)rm -vf libs/*.a
	$(Q)rm -vf include/*
	$(Q)rm -vf *~ */*~ */*/*~
	$(Q)rm -vf *.stackdump */*.stackdump	
	$(Q)rm -vf ./module/*.o
	$(Q)rm -vf ./module/*.pyc
	$(Q)rm -vf ./eispice.dll
	$(Q)rm -fdr ./build

clean:
	@echo Cleaning...
	$(Q)rm -vf *~ */*~ */*/*~
	$(Q)rm -vf ./module/*.o
	$(Q)rm -vf ./module/simulator_.dll
	$(Q)rm -vf ./module/simulator_.so

uninstall:
	@echo Un-Installing
	$(Q)python uninstall.py
	

install: uninstall
	@echo Installing
	$(Q)python setup.py build $(PYFLAG) install
	$(Q)rm -fdr ./build

windist: $(LIBS)
	@echo Installing
	$(Q)python setup.py build $(PYFLAG) bdist_wininst
	$(Q)cp ./dist/* .
	$(Q)rm -fdr ./build
	$(Q)rm -fdr ./dist
