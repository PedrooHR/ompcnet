
#include <ap_int.h>
#include <hls_stream.h>

#include "app2net.h"

void app2net(hls::stream<control_word> &controller,
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
      Send Operation:
        - Sends data from a memory mapped space to the Network stack
    */
  case OPERATION::send: {
    ap_uint<16> dest = control_dst(inst);
    ap_uint<29> npackets = control_len(inst) / 64;
    ap_uint<64> address = control_address(inst);

    // Data mover command
    command_address(command) = address;
    command_len(command) = control_len(inst);
    mm2s_cmd.write(command);

    // network packet
    net_data.dest = control_dst(inst);
    net_data.last = 0;

    // data attribution
  send_loop:
    for (ap_uint<29> i = 0; i < npackets; i++) { // in bytes
#pragma HLS pipeline II=1
      net_data.data = dm_out.read().data;
      if (i == (npackets - 1)) // last pkt
        net_data.last = 1;
      network.write(net_data);
    }

    status = mm2s_sts.read();
  } break;
    /*
      Stream To Operation:
        - Streams from the application to the Network stack
    */
  case OPERATION::stream_to: {
    ap_uint<29> npackets = control_len(inst) / 64;

    net_data.dest = control_dst(inst);
    net_data.last = 0;

    // data attribution
  stream_to_loop:
    for (ap_uint<29> i = 0; i < npackets; i++) { // in bytes
#pragma HLS pipeline II=1
      net_data.data = application.read().data;
      if (i == (npackets - 1)) // last pkt
        net_data.last = 1;
      network.write(net_data);
    }
  } break;
    /*
      Stream 2 Mem Operation:
        - Locally streams from the application to a memory mapped space
    */
  case OPERATION::stream2mem: {
    ap_uint<29> len = control_len(inst);
    ap_uint<64> address = control_address(inst);

    // Data mover command
    command_address(command) = address;
    command_len(command) = len;
    s2mm_cmd.write(command);

  stream2mem_loop:
    for (ap_uint<29> i = 0; i < len / 64; i++) { // in bytes
#pragma HLS pipeline II=1
      dm_in.write(application.read());
    }

    status = s2mm_sts.read();
  } break;
  default:
    break;
  }
}