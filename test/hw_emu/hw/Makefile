IPS=hardware
all: $(IPS) 
.PHONY: $(IPS)

hardware: 
	$(MAKE) -C ../../hardware

clean:
	rm -rf *.xo
	rm -rf *.log
	rm -rf *.xml
	$(MAKE) -C ../../hardware clean