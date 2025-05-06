IPS=network increment loopback

all: $(IPS) 
.PHONY: $(IPS) clean

network: 
	$(MAKE) -C network

increment: 
	$(MAKE) -C increment

loopback: 
	$(MAKE) -C loopback

clean:
	rm -rf $(wildcard *.xo)
	rm -rf $(wildcard *.log)
	rm -rf $(wildcard *.xml)
	$(MAKE) -C network clean
	$(MAKE) -C increment clean
	$(MAKE) -C loopback clean
