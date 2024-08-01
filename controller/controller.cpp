#include "controller.h"

void controller(ap_uint<32> src,
                ap_uint<32> dst,
                ap_uint< 8> operation,
                ap_uint<64> address,
                ap_uint<32> size,
                hls::stream<control_word> &app2net,
                hls::stream<control_word> &net2app) {
#pragma HLS INTERFACE s_axilite port = src bundle = control
#pragma HLS INTERFACE s_axilite port = dst bundle = control
#pragma HLS INTERFACE s_axilite port = operation bundle = control
#pragma HLS INTERFACE s_axilite port = address bundle = control 
#pragma HLS INTERFACE s_axilite port = size bundle = control
#pragma HLS INTERFACE axis port = app2net
#pragma HLS INTERFACE axis port = net2app
#pragma HLS INTERFACE s_axilite port = return bundle = control
  
  control_word inst;

  switch (operation) {
  case send:
    inst.data.range( 31,   0) = -1;
    inst.data.range( 63,  32) = dst;
    inst.data.range( 71,  64) = send;
    inst.data.range(135,  72) = address;
    inst.data.range(167, 136) = size;

    app2net.write(inst);
    break;
  case recv:
    inst.data.range( 31,   0) = src;
    inst.data.range( 63,  32) = -1;
    inst.data.range( 71,  64) = recv;
    inst.data.range(135,  72) = address;
    inst.data.range(167, 136) = size;

    net2app.write(inst);
    break;
  case stream_to:
    inst.data.range( 31,   0) = -1;
    inst.data.range( 63,  32) = dst;
    inst.data.range( 71,  64) = stream_to;
    inst.data.range(135,  72) = -1;
    inst.data.range(167, 136) = size;

    app2net.write(inst);
    break;
  case stream_from:
    inst.data.range( 31,   0) = src;
    inst.data.range( 63,  32) = -1;
    inst.data.range( 71,  64) = stream_from;
    inst.data.range(135,  72) = -1;
    inst.data.range(167, 136) = size;

    net2app.write(inst);
    break;
  case stream2mem: // This outputs application stream into mem values (local)
    inst.data.range( 31,   0) = -1;
    inst.data.range( 63,  32) = -1;
    inst.data.range( 71,  64) = stream2mem;
    inst.data.range(135,  72) = address;
    inst.data.range(167, 136) = size;

    app2net.write(inst);
    break;
  case mem2stream: // This inputs mem values into application stream (local)
    inst.data.range( 31,   0) = -1;
    inst.data.range( 63,  32) = -1;
    inst.data.range( 71,  64) = mem2stream;
    inst.data.range(135,  72) = address;
    inst.data.range(167, 136) = size;

    net2app.write(inst);
    break;
  case exit:    
    inst.data.range( 31,   0) = -1;
    inst.data.range( 63,  32) = -1;
    inst.data.range( 71,  64) = exit;
    inst.data.range(135,  72) = -1;
    inst.data.range(167, 136) = -1;

    app2net.write(inst);
    net2app.write(inst);
    break;
  default:
    break;
  }
}