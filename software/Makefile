CXX?=g++

TARGETS=ompcnet

INCLUDES_PATH=-I$(XILINX_XRT)/include -I./include
LIBRARIES_PATH=-L$(XILINX_XRT)/lib
LIBRARIES=-lxrt_coreutil -luuid
STANDARD=-std=c++17
PREFIX?=/usr/local

all: $(TARGETS) 
.PHONY: install

ompcnet: src/*.cpp include/*.h
	$(CXX) -shared  $(INCLUDES_PATH) $(LIBRARIES_PATH) $(STANDARD) -fPIC $^ -o lib$@.so $(LIBRARIES)

install: ompcnet
	install -d $(DESTDIR)$(PREFIX)/lib/
	install -m 644 *.so $(DESTDIR)$(PREFIX)/lib/
	install -d $(DESTDIR)$(PREFIX)/include/
	install -m 644 include/*.h $(DESTDIR)$(PREFIX)/include/
	install -d $(DESTDIR)$(PREFIX)/include/ompcnet
	install -m 644 include/ompcnet/*.h $(DESTDIR)$(PREFIX)/include/ompcnet

clean:
	rm *.so
