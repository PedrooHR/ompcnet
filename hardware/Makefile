IPS=app2net net2app datamover arbiter

all: $(IPS) 
.PHONY: $(IPS) clean

app2net: 
	$(MAKE) -C app2net

net2app: 
	$(MAKE) -C net2app

datamover:
	$(MAKE) -C datamover

arbiter:
	$(MAKE) -C arbiter

clean:
	rm -rf $(wildcard *.xo)
	$(MAKE) -C app2net clean
	$(MAKE) -C net2app clean
	$(MAKE) -C datamover clean
	$(MAKE) -C arbiter clean
