DEVICE=xcu55c-fsvh2892-2L-e
TARGET=ip

all: ../memcpy.xo
.PHONY: clean

../memcpy.xo: build.tcl $(wildcard *.cpp) $(wildcard *.h)
	vitis_hls $< -tclargs $(TARGET) $(DEVICE)

clean:
	rm -rf build
	rm -rf $(wildcard *.log)
	rm -rf ../memcpy.xo
