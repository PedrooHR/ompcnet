# /*******************************************************************************
#  Copyright (C) 2021 Advanced Micro Devices, Inc
#
#  Licensed under the Apache License, Version 2.0 (the "License");
#  you may not use this file except in compliance with the License.
#  You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
#  Unless required by applicable law or agreed to in writing, software
#  distributed under the License is distributed on an "AS IS" BASIS,
#  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#  See the License for the specific language governing permissions and
#  limitations under the License.
#
# *******************************************************************************/

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
