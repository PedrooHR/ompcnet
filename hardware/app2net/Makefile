DEVICE=xcu55c-fsvh2892-2L-e
TARGET=ip

all: ../app2net.xo
.PHONY: clean

../app2net.xo: build.tcl $(wildcard *.cpp) $(wildcard *.h)
	vitis_hls $< -tclargs $(TARGET) $(DEVICE)

clean:
	rm -rf build
	rm -rf ../app2net.xo
	rm -rf $(wildcard *.log)
