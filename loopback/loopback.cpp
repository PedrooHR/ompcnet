
#include <hls_stream.h>
#include <ap_int.h>

#include "loopback.h"

void loopback(hls::stream<application_word> &in, 
              hls::stream<application_word> &out) {
#pragma HLS INTERFACE mode=axis port = in
#pragma HLS INTERFACE mode=axis port = out
#pragma HLS INTERFACE s_axilite port = return bundle = control
  
  out.write(in.read());
}