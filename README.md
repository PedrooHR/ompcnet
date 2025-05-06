# OMPCNet

OMPCNet is the network interface that connects application FPGA kernels with the network FPGA kernels that implement a ethernet stack. OMPCNet implements the basic peer-to-peer operations:
- Buffer Operations: **send** and **recv**
- Stream Operations: **stream_to** and **stream_from**
- Local Operations: **stream2mem** and **mem2stream**

Arbiter Hardware Module should be build with Vivado, exporting as .xo file