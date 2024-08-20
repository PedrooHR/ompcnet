CXX?=g++

TARGETS=ompcnet-lib vnxsh main

CURRENT_DIR=$(shell pwd)

INCLUDES_PATH=-I$(XILINX_XRT)/include -I$(CURRENT_DIR)/ompcnet/include
LIBRARIES_PATH=-L$(XILINX_XRT)/lib -L$(CURRENT_DIR)/ompcnet/lib
LIBRARIES=-lxrt_coreutil -luuid -lompcnet -lvnxsh -pthread
STANDARD=-std=c++17

# VNX
VNX=$(CURRENT_DIR)/../hw/xup_vitis_network_example
VNX_HOST=$(VNX)/xrt_host_api

all: $(TARGETS) 
.PHONY: clean

ompcnet-lib: 
	$(MAKE) -C ../../../software
	PREFIX=$(CURRENT_DIR)/ompcnet $(MAKE) -C ../../../software install

vnxsh:
	cd $(VNX_HOST) && mkdir -p build && cd build && cmake .. && make -j 8
	cp $(VNX_HOST)/build/libvnxsh.so $(CURRENT_DIR)/ompcnet/lib/
	cp -r $(VNX_HOST)/include/ $(CURRENT_DIR)/ompcnet/

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