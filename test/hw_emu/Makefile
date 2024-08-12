PLATFORM?=xilinx_u55c_gen3x16_xdma_3_202210_1
TARGETS=hardware software gen_hw_emu

all: $(TARGETS) 
.PHONY: hardware software

hardware: 
	$(MAKE) -C ./hw

software:
	$(MAKE) -C ./sw

gen_hw_emu:
	emconfigutil --nd 1 --platform $(PLATFORM) --od ./sw

test:
  XCL_EMULATION_MODE=hw_emu ./sw/main $(PWD)/hw/test.xclbin

clean:
	rm -rf *.xo
	rm -rf *.log
	rm -rf *.xml
	rm -rf .Xil
	$(MAKE) -C hw clean
	$(MAKE) -C sw clean