#include "datamover.h"

void s2mm(hls::stream<cmd_word> &s2mm_cmd,   // s2mm command stream
          hls::stream<sts_word> &s2mm_sts,   // s2mm status stream
          hls::stream<data_word> &s2mm_axis, // AXI4 stream for data
          mem_word *s2mm_axi                 // AXI4 for writting mem
) {
  // parse command
  cmd_word cmd = s2mm_cmd.read();
  uintptr_t address = cmd.data.range(63, 0) / sizeof(mem_word);
  int length = cmd.data.range(95, 64);

  // execute operation
  for (int i = 0; i < length; i++) {
    mem_word data = s2mm_axis.read().data;
    s2mm_axi[address + i] = data;
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
  uintptr_t address = cmd.data.range(63, 0) / sizeof(mem_word);
  int length = cmd.data.range(95, 64);

  // execute operation
  for (int i = 0; i < length; i++) {
    data_word data;
    data.data = mm2s_axi[address + i];
    mm2s_axis.write(data);
  }

  // send status
  sts_word sts;
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
#pragma HLS INTERFACE axis port = mm2s_cmd
#pragma HLS INTERFACE axis port = mm2s_sts
#pragma HLS INTERFACE axis port = mm2s_axis
#pragma HLS INTERFACE m_axi port = mm2s_axi
// S2MM Ports
#pragma HLS INTERFACE axis port = s2mm_cmd
#pragma HLS INTERFACE axis port = s2mm_sts
#pragma HLS INTERFACE axis port = s2mm_axis
#pragma HLS INTERFACE m_axi port = s2mm_axi
// Function return
#pragma HLS INTERFACE ap_ctrl_none port = return

#pragma HLS DATAFLOW
  s2mm(s2mm_cmd, s2mm_sts, s2mm_axis, s2mm_axi);
  mm2s(mm2s_cmd, mm2s_sts, mm2s_axis, mm2s_axi);
}