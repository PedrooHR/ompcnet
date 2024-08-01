IPS=controller app2net net2app datamover loopback

all: $(IPS) 
.PHONY: $(IPS)

controller: 
	$(MAKE) -C controller

app2net: 
	$(MAKE) -C app2net

net2app: 
	$(MAKE) -C net2app

loopback: 
	$(MAKE) -C loopback

datamover:
	$(MAKE) -C datamover

clean:
	rm -rf *.xo
	rm -rf *.log
	rm -rf *.xml
	$(MAKE) -C controller clean
	$(MAKE) -C app2net clean
	$(MAKE) -C net2app clean
	$(MAKE) -C loopback clean
	$(MAKE) -C datamover clean
