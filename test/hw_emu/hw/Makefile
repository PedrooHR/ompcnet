PLATFORM?=xilinx_u55c_gen3x16_xdma_3_202210_1
IPS=hardware kernels test.xclbin

HW_PATH=../../../hardware
KERNELS_PATH=../../kernels

CONFIG_FILE?=hw_emu.cfg
FREQUENCY?=250

all: $(IPS) 
.PHONY: $(IPS)

hardware: 
	$(MAKE) -C $(HW_PATH)

kernels:
	$(MAKE) -C $(KERNELS_PATH)

test.xclbin: #hardware kernels
	v++ -l -t hw_emu --platform $(PLATFORM) --config $(CONFIG_FILE) --kernel_frequency $(FREQUENCY) $(HW_PATH)/*.xo $(KERNELS_PATH)/*.xo -o $@

clean:
	rm -rf *.log
	rm -rf *.xml
	rm -rf *_x
	rm -rf *.Xil
	rm -rf *.xclbin*
	$(MAKE) -C $(HW_PATH) clean
	$(MAKE) -C $(KERNELS_PATH) clean