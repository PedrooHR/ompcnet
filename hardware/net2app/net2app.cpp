
#include <ap_int.h>
#include <hls_stream.h>

#include "net2app.h"

void net2app(ap_uint<16> src, ap_uint<16> dst, ap_uint<3> op, ap_uint<64> add,
             ap_uint<32> len, hls::stream<application_word> &application,
             hls::stream<network_word> &network,
             hls::stream<cmd_word> &mm2s_cmd, hls::stream<sts_word> &mm2s_sts,
             hls::stream<cmd_word> &s2mm_cmd, hls::stream<sts_word> &s2mm_sts,
             hls::stream<data_word> &dm_in, hls::stream<data_word> &dm_out) {
#pragma HLS INTERFACE s_axilite port = src bundle = control
#pragma HLS INTERFACE s_axilite port = dst bundle = control
#pragma HLS INTERFACE s_axilite port = op bundle = control
#pragma HLS INTERFACE s_axilite port = add bundle = control
#pragma HLS INTERFACE s_axilite port = len bundle = control
#pragma HLS INTERFACE axis port = application
#pragma HLS INTERFACE axis port = network
#pragma HLS INTERFACE axis port = mm2s_cmd
#pragma HLS INTERFACE axis port = mm2s_sts
#pragma HLS INTERFACE axis port = s2mm_cmd
#pragma HLS INTERFACE axis port = s2mm_sts
#pragma HLS INTERFACE axis port = dm_in
#pragma HLS INTERFACE axis port = dm_out
#pragma HLS INTERFACE s_axilite port = return bundle = control

  // pkts
  data_word dm_data;
  application_word app_data;
  network_word net_data;

  // structs
  cmd_word command;
  sts_word status;

  ap_uint<32> npackets = len / 64;

  switch (op) {
    /*
      Recv Operation:
        - Receives from the Network stack and stores in a memory mapped space
    */
  case recv: {
    command_address(command) = add;
    command_len(command) = len;
    s2mm_cmd.write(command);

    // data attribution
  recv_loop:
    for (ap_uint<32> i = 0; i < npackets; i++) { // in bytes
#pragma HLS pipeline II = 1
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
  stream_from_loop:
    for (ap_uint<32> i = 0; i < npackets; i++) { // in bytes
#pragma HLS pipeline II = 1
      app_data.data = network.read().data;
      application.write(app_data);
    }
  } break;
    /*
      Mem 2 Stream Operation:
        - Localy reads from a memory mapped space and streams to application
    */
  case mem2stream: {
    command_address(command) = add;
    command_len(command) = len;
    mm2s_cmd.write(command);

  stream2mem_loop:
    for (ap_uint<32> i = 0; i < npackets; i++) { // in bytes
#pragma HLS pipeline II = 1
      application.write(dm_out.read());
    }

    status = mm2s_sts.read();
  } break;
  default:
    break;
  }
}