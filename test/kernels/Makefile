IPS=network increment

all: $(IPS) 
.PHONY: $(IPS) clean

network: 
	$(MAKE) -C network

increment: 
	$(MAKE) -C increment

clean:
	rm -rf $(wildcard *.xo)
	rm -rf $(wildcard *.log)
	rm -rf $(wildcard *.xml)
	$(MAKE) -C network clean
	$(MAKE) -C increment clean
