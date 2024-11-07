
#include <ap_int.h>
#include <hls_stream.h>

#include "app2net.h"

void app2net(ap_uint<16> src, ap_uint<16> dst, ap_uint<3> op, ap_uint<64> add,
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
      Send Operation:
        - Sends data from a memory mapped space to the Network stack
    */
  case OPERATION::send: {
    // Data mover command
    command_address(command) = add;
    command_len(command) = len;
    mm2s_cmd.write(command);

    // network packet
    net_data.dest = dst;
    net_data.last = 0;
    net_data.keep = -1; // Enable all bytes

    // data attribution
    ap_uint<5> packet_idx = 0;
  send_loop:
    for (ap_uint<32> i = 0; i < npackets; i++) { // in bytes
#pragma HLS pipeline II = 1
      net_data.data = dm_out.read().data;
      if (packet_idx < 21 && i < (npackets - 1)) {
        // still in the packet
        net_data.last = 0;
        packet_idx = packet_idx + 1;
      } else {
        // end of the packet (1408b)
        net_data.last = 1;
        packet_idx = 0;
      }
      network.write(net_data);
    }

    status = mm2s_sts.read();
  } break;
    /*
      Stream To Operation:
        - Streams from the application to the Network stack
    */
  case OPERATION::stream_to: {
    net_data.dest = dst;
    net_data.last = 0;
    net_data.keep = -1; // Enable all bytes

    // data attribution
    ap_uint<5> packet_idx = 0;
  stream_to_loop:
    for (ap_uint<32> i = 0; i < npackets; i++) { // in bytes
#pragma HLS pipeline II = 1
      net_data.data = application.read().data;
      if (packet_idx < 21 && i < (npackets - 1)) {
        // still in the packet
        net_data.last = 0;
        packet_idx = packet_idx + 1;
      } else {
        // end of the packet (1408b)
        net_data.last = 1;
        packet_idx = 0;
      }
      network.write(net_data);
    }
  } break;
    /*
      Stream 2 Mem Operation:
        - Locally streams from the application to a memory mapped space
    */
  case OPERATION::stream2mem: {
    // Data mover command
    command_address(command) = add;
    command_len(command) = len;
    s2mm_cmd.write(command);

  stream2mem_loop:
    for (ap_uint<32> i = 0; i < npackets; i++) { // in bytes
#pragma HLS pipeline II = 1
      dm_in.write(application.read());
    }

    status = s2mm_sts.read();
  } break;
  default:
    break;
  }
}