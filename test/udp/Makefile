PLATFORM?=xilinx_u55c_gen3x16_xdma_3_202210_1
TARGETS=hardware software 

CURRENT_DIR=$(shell pwd)

all: $(TARGETS) 
.PHONY: clean

hardware: 
	$(MAKE) -C ./hw

software:
	$(MAKE) -C ./sw

test:
	./sw/main $(CURRENT_DIR)/hw/test.xclbin

clean:
	rm -rf *.xo
	rm -rf *.log
	rm -rf *.xml
	rm -rf .Xil
	$(MAKE) -C hw clean
	$(MAKE) -C sw clean