PLATFORM?=xilinx_u55c_gen3x16_xdma_3_202210_1
IPS=hardware kernels test.xclbin

HW_PATH=../../../hardware
KERNELS_PATH=../../kernels

CONFIG_FILE?=hw_emu.cfg
FREQUENCY?=250

all: $(IPS) 
.PHONY: clean

hardware: 
	$(MAKE) -C $(HW_PATH)

kernels:
	$(MAKE) -C $(KERNELS_PATH)

test.xclbin: $(wildcard $(HW_PATH)/*.xo) $(wildcard $(KERNELS_PATH)/*.xo)
	v++ -l -t hw_emu --platform $(PLATFORM) --config $(CONFIG_FILE) --kernel_frequency $(FREQUENCY) $(wildcard $(HW_PATH)/*.xo) $(wildcard $(KERNELS_PATH)/*.xo) -o $@

clean:
	rm -rf $(wildcard *.log)
	rm -rf $(wildcard *.xml)
	rm -rf $(wildcard *_x)
	rm -rf $(wildcard *.Xil)
	rm -rf $(wildcard *.xclbin*)
	$(MAKE) -C $(HW_PATH) clean
	$(MAKE) -C $(KERNELS_PATH) clean