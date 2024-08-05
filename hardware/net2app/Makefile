DEVICE=xcu55c-fsvh2892-2L-e
TARGET=ip

all: net2app

net2app: build.tcl *.cpp *.h
	vitis_hls $< -tclargs $(TARGET) $(DEVICE)

clean:
	rm -rf build
	rm -rf *.log
