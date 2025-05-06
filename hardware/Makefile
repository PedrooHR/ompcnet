IPS=app2net net2app datamover

all: $(IPS) 
.PHONY: $(IPS) clean

app2net: 
	$(MAKE) -C app2net

net2app: 
	$(MAKE) -C net2app

datamover:
	$(MAKE) -C datamover

clean:
	rm -rf $(wildcard *.xo)
	$(MAKE) -C app2net clean
	$(MAKE) -C net2app clean
	$(MAKE) -C datamover clean
