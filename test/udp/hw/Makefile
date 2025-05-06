PLATFORM?=xilinx_u55c_gen3x16_xdma_3_202210_1
IPS=hardware kernels test.xclbin

HW_PATH=../../../hardware
KERNELS_PATH=../../kernels
TASK_BENCH_PATH=/home/pedro.rosso/TRETS/researchartifacts/task-bench/fpga_kernel

CONFIG_FILE?=hw.cfg
FREQUENCY?=200

XCLBIN=test.xclbin

HAS_HBM?=yes

# OMPCNET XOS
APP2NET_XO 	= $(HW_PATH)/app2net.xo
NET2APP_XO 	= $(HW_PATH)/net2app.xo
DATAMOVER_XO 	= $(HW_PATH)/datamover.xo
ARBITER_XO 	= $(HW_PATH)/arbiter.xo
TASK_BENCH_XO	= $(TASK_BENCH_PATH)/task_bench.xo
LOOPBACK_XO 	= $(KERNELS_PATH)/loopback.xo
MEMCPY_XO 	= $(KERNELS_PATH)/memcpy.xo
EVERY_XO = $(NET2APP_XO) $(APP2NET_XO) $(DATAMOVER_XO) $(ARBITER_XO) $(TASK_BENCH_XO) $(LOOPBACK_XO) $(MEMCPY_XO)

## VNX Configs
VNX=xup_vitis_network_example

NETLAYERDIR = $(VNX)/NetLayers
CMACDIR     = $(VNX)/Ethernet
NETLAYERHLS = $(NETLAYERDIR)/100G-fpga-network-stack-core
ETH_IF=1
CMAC_UDP_XO = $(VNX)/Ethernet/_x.$(PLATFORM)/cmac_$(ETH_IF).xo
UDP_XO=$(VNX)/NetLayers/_x.$(PLATFORM)/networklayer.xo
NETLAYERXML = $(NETLAYERDIR)/kernel.xml

MAX_SOCKETS?=16
ifeq ($(shell test $(MAX_SOCKETS) -gt 63; echo $$?),0)
    $(error Error: maximum number of sockets is 63)
endif
UDP_THEIR_IP_OFFSET = 0x820
UDP_THEIR_PORT_OFFSET = $(shell printf "0x%04X\n" $$(($(UDP_THEIR_IP_OFFSET) + 8*$(MAX_SOCKETS))))
UDP_MY_PORT_OFFSET = $(shell printf "0x%04X\n" $$(($(UDP_THEIR_PORT_OFFSET) + 8*$(MAX_SOCKETS))))
UDP_VALID_OFFSET = $(shell printf "0x%04X\n" $$(($(UDP_MY_PORT_OFFSET) + 8*$(MAX_SOCKETS))))

HLS_IP_FOLDER ?= $(shell readlink -f ./$(NETLAYERHLS)/synthesis_results_noHBM)
ifeq (yes,$(HAS_HBM))
	HLS_IP_FOLDER = $(shell readlink -f ./$(NETLAYERHLS)/synthesis_results_HBM)
endif

CONFIG := --config $(CONFIG_FILE)
CONFIG += --advanced.param compiler.userPostSysLinkOverlayTcl=$(shell pwd)/$(VNX)/Ethernet/post_sys_link.tcl
NET_XO = $(CMAC_UDP_XO) $(UDP_XO)
CONFIG += --user_ip_repo_paths $(HLS_IP_FOLDER)

## TARGETS
all: hardware kernels $(XCLBIN) vnxx
.PHONY: clean vnxx
vnxx: $(CMAC_UDP_XO) $(UDP_XO)

hardware: 
	$(MAKE) -C $(HW_PATH)

kernels:
	$(MAKE) -C $(KERNELS_PATH) loopback

$(CMAC_UDP_XO) &:
	git submodule update --init --recursive $(VNX)
	$(MAKE) -C $(CMACDIR) DEVICE=$(PLATFORM) INTERFACE=$(ETH_IF) all

$(NETLAYERXML):
	sed -i 's/define NUMBER_SOCKETS 16/define NUMBER_SOCKETS $(MAX_SOCKETS)/' ./$(NETLAYERHLS)/hls/UDP/udp.hpp
	sed -i 's/port=numberSockets/port=numberSockets offset=0x10/' ./$(NETLAYERHLS)/hls/UDP/udp.cpp
	cat ./$(NETLAYERDIR)/template.xml | sed 's/UDP_TP_PLACEHOLDER/$(UDP_THEIR_PORT_OFFSET)/' \
	                                  | sed 's/UDP_MP_PLACEHOLDER/$(UDP_MY_PORT_OFFSET)/' \
	                                  | sed 's/UDP_VL_PLACEHOLDER/$(UDP_VALID_OFFSET)/' > ./$@

$(UDP_XO): $(NETLAYERXML)
	git submodule update --init --recursive $(VNX)
	$(MAKE) -C $(NETLAYERDIR) DEVICE=$(PLATFORM) all

$(XCLBIN): $(EVERY_XO) $(NET_XO) $(CONFIG_FILE) vnxx
	v++ --link --platform $(PLATFORM) --kernel_frequency $(FREQUENCY) $(CONFIG) -o $@ $(EVERY_XO) $(NET_XO) 

clean:
	rm -rf *.log
	rm -rf *.xml
	rm -rf *_x
	rm -rf *.Xil
	rm -rf *.ipcache
	rm -rf *.xclbin*
	rm -rf *.ltx
	$(MAKE) -C $(HW_PATH) clean
	$(MAKE) -C $(KERNELS_PATH) clean