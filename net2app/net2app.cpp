
#include <ap_int.h>
#include <hls_stream.h>

#include "net2app.h"

void net2app(hls::stream<control_word> &controller,
             hls::stream<application_word> &application,
             hls::stream<network_word> &network,
             hls::stream<cmd_word> &mm2s_cmd, hls::stream<sts_word> &mm2s_sts,
             hls::stream<cmd_word> &s2mm_cmd, hls::stream<sts_word> &s2mm_sts,
             hls::stream<data_word> &dm_in, hls::stream<data_word> &dm_out) {
#pragma HLS INTERFACE mode = axis port = controller
#pragma HLS INTERFACE mode = axis port = application
#pragma HLS INTERFACE mode = axis port = network
#pragma HLS INTERFACE mode = axis port = mm2s_cmd
#pragma HLS INTERFACE mode = axis port = mm2s_sts
#pragma HLS INTERFACE mode = axis port = s2mm_cmd
#pragma HLS INTERFACE mode = axis port = s2mm_sts
#pragma HLS INTERFACE mode = axis port = dm_in
#pragma HLS INTERFACE mode = axis port = dm_out
#pragma HLS INTERFACE ap_ctrl_none port = return

  // pkts
  data_word dm_data;
  application_word app_data;
  network_word net_data;

  // structs
  control_word inst;
  cmd_word command;
  sts_word status;

  inst = controller.read();
  ap_uint<3> operation = control_op(inst);

  switch (operation) {
    /*
      Recv Operation:
        - Receives from the Network stack and stores in a memory mapped space
    */
  case recv: {
    ap_uint<29> npackets = control_len(inst) / 64;
    ap_uint<64> address = control_address(inst);

    command_address(command) = address;
    command_len(command) = control_len(inst);
    s2mm_cmd.write(command);

    // data attribution
  recv_loop:
    for (ap_uint<29> i = 0; i < npackets; i++) { // in bytes
#pragma HLS pipeline II=1
      dm_data.data = network.read().data;
      dm_in.write(dm_data);
    }

    status = s2mm_sts.read();
  } break;
    /*
      Stream From Operation:
        - Receives from the Network stack and redirects to the application
    */
  case stream_from: {
    ap_uint<29> npackets = control_len(inst) / 64;

 stream_from_loop:
    for (ap_uint<29> i = 0; i < npackets; i++) { // in bytes
#pragma HLS pipeline II=1
      app_data.data = network.read().data;
      application.write(app_data);
    }
  } break;
    /*
      Mem 2 Stream Operation:
        - Localy reads from a memory mapped space and streams to application
    */
  case mem2stream: {
    int len = control_len(inst);
    uintptr_t address = control_address(inst);

    command_address(command) = address;
    command_len(command) = len;
    mm2s_cmd.write(command);

  stream2mem_loop:
    for (ap_uint<29> i = 0; i < len / 64; i++) { // in bytes
#pragma HLS pipeline II=1
      application.write(dm_out.read());
    }

    status = mm2s_sts.read();
  } break;
  default:
    break;
  }
}