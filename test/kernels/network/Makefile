DEVICE=xcu55c-fsvh2892-2L-e
TARGET=ip

all: ../network.xo
.PHONY: clean

../network.xo: build.tcl $(wildcard *.cpp) $(wildcard *.h)
	vitis_hls $< -tclargs $(TARGET) $(DEVICE)

clean:
	rm -rf build
	rm -rf $(wildcard *.log)
