PLATFORM?=xilinx_u55c_gen3x16_xdma_3_202210_1
IPS=hardware kernels test.xclbin

HW_PATH=../../../hardware
KERNELS_PATH=../../kernels

APP2NET_XO 		= $(HW_PATH)/app2net.xo
NET2APP_XO 		= $(HW_PATH)/net2app.xo
DATAMOVER_XO 	= $(HW_PATH)/datamover.xo
ARBITER_XO 	= $(HW_PATH)/arbiter.xo
INCREMENT_XO 	= $(KERNELS_PATH)/increment.xo
NETWORK_XO 	= $(KERNELS_PATH)/network.xo

EVERY_XO = $(APP2NET_XO) $(NET2APP_XO) $(DATAMOVER_XO) $(ARBITER_XO) $(INCREMENT_XO) $(NETWORK_XO)

CONFIG_FILE?=hw_emu.cfg
FREQUENCY?=250

all: $(IPS) 
.PHONY: clean

hardware: 
	$(MAKE) -C $(HW_PATH)

kernels:
	$(MAKE) -C $(KERNELS_PATH)

test.xclbin: $(EVERY_XO)
	v++ -l -t hw_emu --platform $(PLATFORM) --config $(CONFIG_FILE) --kernel_frequency $(FREQUENCY) $(EVERY_XO) -o $@

clean:
	rm -rf $(wildcard *.log)
	rm -rf $(wildcard *.xml)
	rm -rf $(wildcard *_x)
	rm -rf $(wildcard *.Xil)
	rm -rf $(wildcard *.xclbin*)
	$(MAKE) -C $(HW_PATH) clean
	$(MAKE) -C $(KERNELS_PATH) clean