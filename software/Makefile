CXX?=g++

TARGETS=libompcnet.so

INCLUDES_PATH=-I$(XILINX_XRT)/include -I./include
LIBRARIES_PATH=-L$(XILINX_XRT)/lib
LIBRARIES=-lxrt_coreutil -luuid
STANDARD=-std=c++17
PREFIX?=/usr/local

all: $(TARGETS) 
.PHONY: clean install

libompcnet.so: $(wildcard src/*.cpp)
	$(CXX) -g -c $(INCLUDES_PATH) $(LIBRARIES_PATH) $(STANDARD) -fPIC $^
	$(CXX) -g -shared  $(INCLUDES_PATH) $(LIBRARIES_PATH) $(STANDARD) -fPIC *.o -o $@ $(LIBRARIES)

install-lib: libompcnet.so
	mkdir -p $(DESTDIR)$(PREFIX)/lib/
	cp -r $< $(DESTDIR)$(PREFIX)/lib/

install-include: $(wildcard include/*)
	mkdir -p $(DESTDIR)$(PREFIX)/include/
	cp -r $^ $(DESTDIR)$(PREFIX)/include/

install: install-lib install-include
	echo "Installing Software Driver"

clean:
	rm -rf *.so
	rm -rf *.o
