DEVICE=xcu55c-fsvh2892-2L-e
TARGET=ip

all: ../datamover.xo
.PHONY: clean

../datamover.xo: build.tcl $(wildcard *.cpp) $(wildcard *.h)
	vitis_hls $< -tclargs $(TARGET) $(DEVICE)

clean:
	rm -rf build
	rm -rf ../datamover.xo
	rm -rf $(wildcard *.log)
