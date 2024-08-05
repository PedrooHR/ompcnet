DEVICE=xcu55c-fsvh2892-2L-e
TARGET=ip

all: app2net

app2net: build.tcl *.cpp *.h
	vitis_hls $< -tclargs $(TARGET) $(DEVICE)

clean:
	rm -rf build
	rm -rf *.log
