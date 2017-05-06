# Installation Directory
INC_PREFIX = ../../include

# Quiet (set to @ for a quite compile)
Q	?= 
#Q	?= 

INC = log.h

.PHONY: all clean install

all: Makefile
	@echo Nothing to Build...

$(INC_PREFIX)/$(INC):$(INC)
	@echo Installing...
	$(Q)cp $(INC) $(INC_PREFIX)

install: $(INC_PREFIX)/$(INC)
	
