DEVICE=xcu55c-fsvh2892-2L-e
TARGET=ip

all: ../arbiter.xo
.PHONY: clean

../arbiter.xo: build.tcl $(wildcard *.cpp) $(wildcard *.h)
	vitis_hls $< -tclargs $(TARGET) $(DEVICE)

clean:
	rm -rf build
	rm -rf ../arbiter.xo
	rm -rf $(wildcard *.log)
