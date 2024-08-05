IPS=controller app2net net2app datamover

all: $(IPS) 
.PHONY: $(IPS)

controller: 
	$(MAKE) -C controller

app2net: 
	$(MAKE) -C app2net

net2app: 
	$(MAKE) -C net2app

datamover:
	$(MAKE) -C datamover

clean:
	rm -rf *.xo
	rm -rf *.log
	rm -rf *.xml
	$(MAKE) -C controller clean
	$(MAKE) -C app2net clean
	$(MAKE) -C net2app clean
	$(MAKE) -C datamover clean
