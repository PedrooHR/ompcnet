DEVICE=xcu55c-fsvh2892-2L-e
TARGET=ip

all: controller

controller: build.tcl *.cpp *.h
	vitis_hls $< -tclargs $(TARGET) $(DEVICE)

clean:
	rm -rf build
	rm -rf *.log
