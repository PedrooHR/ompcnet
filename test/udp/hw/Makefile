PLATFORM?=xilinx_u55c_gen3x16_xdma_3_202210_1
IPS=hardware kernels test.xclbin

HW_PATH=../../../hardware
KERNELS_PATH=../../kernels

CONFIG_FILE?=hw_emu.cfg
FREQUENCY?=250

XCLBIN=test.xclbin

## VNX Configs
VNX=xup_vitis_network_example
NETLAYERDIR = $(VNX)/NetLayers
CMACDIR     = $(VNX)/Ethernet
NETLAYERHLS = $(NETLAYERDIR)/100G-fpga-network-stack-core
ETH_IF=0
CMAC_UDP_XO=$(VNX)/Ethernet/_x.$(PLATFORM)/cmac_$(ETH_IF).xo
UDP_XO=$(VNX)/NetLayers/_x.$(PLATFORM)/networklayer.xo

HLS_IP_FOLDER ?= $(shell readlink -f ./$(NETLAYERHLS)/synthesis_results_noHBM)
ifeq (udp,$(HAS_HBM))
	HLS_IP_FOLDER = $(shell readlink -f ./$(NETLAYERHLS)/synthesis_results_HBM)
endif

CONFIG := --config $(CONFIGFILE)
CONFIG += --advanced.param compiler.userPostSysLinkOverlayTcl=$(shell pwd)/$(VNX)/Ethernet/post_sys_link.tcl
NET_XO = $(CMAC_UDP_XO) $(UDP_XO)
CONFIG += --user_ip_repo_paths $(HLS_IP_FOLDER)


## TARGETS
all: $(XCLBIN)
.PHONY: clean vnx
vnx: $(CMAC_UDP_XO) $(UDP_XO)

hardware: 
	$(MAKE) -C $(HW_PATH)

kernels:
	$(MAKE) -C $(KERNELS_PATH) increment

$(CMAC_UDP_XO) &:
	git submodule update --init --recursive xup_vitis_network_example
	$(MAKE) -C xup_vitis_network_example/Ethernet DEVICE=$(PLATFORM) INTERFACE=$(ETH_IF) all

$(UDP_XO):
	git submodule update --init --recursive xup_vitis_network_example
	$(MAKE) -C xup_vitis_network_example/NetLayers DEVICE=$(PLATFORM) all

$(XCLBIN): $(wildcard $(HW_PATH)/*.xo) $(wildcard $(KERNELS_PATH)/*.xo) $(NET_XO) $(CONFIG_FILE)
	v++ --link --platform $(PLATFORM) --kernel_frequency $(FREQUENCY) $(CONFIG) -o $@ $(wildcard $(HW_PATH)/*.xo) $(wildcard $(KERNELS_PATH)/*.xo) $(NET_XO) 

clean:
	rm -rf $(wildcard *.log)
	rm -rf $(wildcard *.xml)
	rm -rf $(wildcard *_x)
	rm -rf $(wildcard *.Xil)
	rm -rf $(wildcard *.xclbin*)
	$(MAKE) -C $(HW_PATH) clean
	$(MAKE) -C $(KERNELS_PATH) clean