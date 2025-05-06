#include "datamover.h"

void s2mm(hls::stream<cmd_word> &s2mm_cmd,   // s2mm command stream
          hls::stream<sts_word> &s2mm_sts,   // s2mm status stream
          hls::stream<data_word> &s2mm_axis, // AXI4 stream for data
          mem_word *s2mm_axi                 // AXI4 for writting mem
) {
  // parse command
  cmd_word cmd = s2mm_cmd.read();
  ap_uint<64> address = cmd.data.range(63, 0) / 64;
  ap_uint<29> length = cmd.data.range(92, 64) / 64; 
  // length comes in bytes, axi is 4 bytes, dm stream is 64 bytes

  // execute operation
  data_word value;
s2mm_loop:
  for (ap_int<29> i = 0; i < length; i++) {
    value = s2mm_axis.read();
    s2mm_axi[address + i] = value.data;
  }

  // send status
  sts_word sts;
  sts.data = 1;
  s2mm_sts.write(sts);
}

void mm2s(hls::stream<cmd_word> &mm2s_cmd,   // mm2s command stream
          hls::stream<sts_word> &mm2s_sts,   // mm2s status stream
          hls::stream<data_word> &mm2s_axis, // AXI4 stream for data
          mem_word *mm2s_axi                 // AXI4 for reading mem
) {
  // parse command
  cmd_word cmd = mm2s_cmd.read();
  ap_uint<64> address = cmd.data.range(63, 0) / 64;
  ap_uint<29> length = cmd.data.range(92, 64) / 64;
  // length comes in bytes, axi is 64 bytes, dm stream is 64 bytes

  // execute operation
  data_word value;
  value.keep = -1;
  value.last = 0;
  for (ap_int<29> i = 0; i < length; i++) {
    value.data = mm2s_axi[address + i];
    if (i == length - 1)
      value.last = 1;
    mm2s_axis.write(value);
  }

  // send status
  sts_word sts;
  sts.last = 1;
  sts.data = 1;
  mm2s_sts.write(sts);
}

void datamover(
    /* MM2S Part */
    hls::stream<cmd_word> &mm2s_cmd,   // mm2s command stream
    hls::stream<sts_word> &mm2s_sts,   // mm2s status stream
    hls::stream<data_word> &mm2s_axis, // AXI4 stream for data
    mem_word *mm2s_axi,                // AXI4 for reading mem
    /* S2MM Part */
    hls::stream<cmd_word> &s2mm_cmd,   // s2mm command stream
    hls::stream<sts_word> &s2mm_sts,   // s2mm status stream
    hls::stream<data_word> &s2mm_axis, // AXI4 stream for data
    mem_word *s2mm_axi                 // AXI4 for writting mem
) {
// MM2S Ports
#pragma HLS INTERFACE axis port = mm2s_cmd depth = 8
#pragma HLS INTERFACE axis port = mm2s_sts depth = 8
#pragma HLS INTERFACE axis port = mm2s_axis depth = 64
#pragma HLS INTERFACE m_axi port = mm2s_axi depth = 8 max_read_burst_length = 64 num_read_outstanding = 16
// S2MM Ports
#pragma HLS INTERFACE axis port = s2mm_cmd depth = 8
#pragma HLS INTERFACE axis port = s2mm_sts depth = 8
#pragma HLS INTERFACE axis port = s2mm_axis depth = 64
#pragma HLS INTERFACE m_axi port = s2mm_axi depth = 8 max_write_burst_length = 64 num_write_outstanding = 16
  // Function return
#ifndef COSIM
#pragma HLS INTERFACE ap_ctrl_none port = return
#else
#pragma HLS INTERFACE s_axilite port = return
#endif

#pragma HLS DATAFLOW
  s2mm(s2mm_cmd, s2mm_sts, s2mm_axis, s2mm_axi);
  mm2s(mm2s_cmd, mm2s_sts, mm2s_axis, mm2s_axi);
}