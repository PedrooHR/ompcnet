CXX?=g++

TARGETS=ompcnet main

CURRENT_DIR=$(shell pwd)

INCLUDES_PATH=-I$(XILINX_XRT)/include -I$(CURRENT_DIR)/ompcnet/include
LIBRARIES_PATH=-L$(XILINX_XRT)/lib -L$(CURRENT_DIR)/ompcnet/lib
LIBRARIES=-lxrt_coreutil -luuid -lompcnet -pthread
STANDARD=-std=c++17

all: $(TARGETS) 
.PHONY: clean

ompcnet: 
	PREFIX=$(CURRENT_DIR)/ompcnet $(MAKE) -C ../../../software install

main: main.cpp 
	$(CXX) $< $(INCLUDES_PATH) $(LIBRARIES_PATH) $(STANDARD) -o $@ $(LIBRARIES)

clean:
	rm -rf ompcnet
	rm -rf .run
	rm -rf $(wildcard *.json)
	rm -rf $(wildcard *.log)
	rm -rf $(wildcard *.xml)
	$(MAKE) -C ../../../software clean
	rm -rf $(TARGETS)