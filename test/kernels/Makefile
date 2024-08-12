IPS=network increment

all: $(IPS) 
.PHONY: $(IPS)

network: 
	$(MAKE) -C network

increment: 
	$(MAKE) -C increment

clean:
	rm -rf *.xo
	rm -rf *.log
	rm -rf *.xml
	$(MAKE) -C network clean
	$(MAKE) -C increment clean
