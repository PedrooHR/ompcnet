DEVICE=xcu55c-fsvh2892-2L-e
TARGET=ip

all: ../net2app.xo
.PHONY: clean

../net2app.xo: build.tcl $(wildcard *.cpp) $(wildcard *.h)
	vitis_hls $< -tclargs $(TARGET) $(DEVICE)

clean:
	rm -rf build
	rm -rf ../net2app.xo
	rm -rf $(wildcard *.log)
