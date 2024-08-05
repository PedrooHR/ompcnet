#include "controller.h"

void controller(ap_int<16> src,
                ap_int<16> dst,
                ap_uint< 3> operation,
                ap_uint<64> address,
                ap_uint<29> size,
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
    inst.data.range( 63,   0) = address;
    inst.data.range( 79,  64) = -1;
    inst.data.range( 95,  80) = dst;
    inst.data.range(124,  96) = size;
    inst.data.range(127, 125) = send;

    app2net.write(inst);
    break;
  case recv:
    inst.data.range( 63,   0) = address;
    inst.data.range( 79,  64) = src;
    inst.data.range( 95,  80) = -1;
    inst.data.range(124,  96) = size;
    inst.data.range(127, 125) = recv;

    net2app.write(inst);
    break;
  case stream_to:
    inst.data.range( 63,   0) = -1;
    inst.data.range( 79,  64) = -1;
    inst.data.range( 95,  80) = dst;
    inst.data.range(124,  96) = size;
    inst.data.range(127, 125) = stream_to;

    app2net.write(inst);
    break;
  case stream_from:
    inst.data.range( 63,   0) = -1;
    inst.data.range( 79,  64) = src;
    inst.data.range( 95,  80) = -1;
    inst.data.range(124,  96) = size;
    inst.data.range(127, 125) = stream_from;

    net2app.write(inst);
    break;
  case stream2mem: // This outputs application stream into mem values (local)
    inst.data.range( 63,   0) = address;
    inst.data.range( 79,  64) = -1;
    inst.data.range( 95,  80) = -1;
    inst.data.range(124,  96) = size;
    inst.data.range(127, 125) = stream2mem;

    app2net.write(inst);
    break;
  case mem2stream: // This inputs mem values into application stream (local)
    inst.data.range( 63,   0) = address;
    inst.data.range( 79,  64) = -1;
    inst.data.range( 95,  80) = -1;
    inst.data.range(124,  96) = size;
    inst.data.range(127, 125) = mem2stream;

    net2app.write(inst);
    break;
  default:
    break;
  }
}