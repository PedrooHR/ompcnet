
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
  ap_int<8> operation = control_op(inst);

  switch (operation) {
    /*
      Send Operation:
        - Sends data from a memory mapped space to the Network stack
    */
  case send: {
    int len = control_len(inst);
    uintptr_t address = control_address(inst);

    // network packet
    net_data.dest = control_dst(inst);
    net_data.last = 0;

    // Data mover command
    command_address(command) = address;
    command_len(command) = len;
    mm2s_cmd.write(command);

    // data attribution
    int i = 0, j = 0;
    while (i < len) {
      j = 0;
      while (i < len && j < 64) {
        dm_data = dm_out.read();
        net_data.data.range(range_f(j), range_s(j)) = dm_data.data;
        i++;
        j++;
      }
      if (i == len) // last pkt
        net_data.last = 1;
      network.write(net_data);
    }

    status = mm2s_sts.read();
  } break;
    /*
      Stream To Operation:
        - Streams from the application to the Network stack
    */
  case stream_to: {
    int len = control_len(inst);

    net_data.dest = control_dst(inst);
    net_data.last = 0;

    // data attribution
    int i = 0, j = 0;
    while (i < len) {
      j = 0;
      while (i < len && j < 64) {
        net_data.data.range(range_f(j), range_s(j)) = application.read().data;
        i++;
        j++;
      }
      if (i == len) // last pkt
        net_data.last = 1;
      network.write(net_data);
    }
  } break;
    /*
      Stream 2 Mem Operation:
        - Localy streams from the application to a memory mapped space
    */
  case stream2mem: {
    int len = control_len(inst);
    uintptr_t address = control_address(inst);

    // Data mover command
    command_address(command) = address;
    command_len(command) = len;
    s2mm_cmd.write(command);

    for (int i = 0; i < len; i++) {
      dm_data.data = application.read().data;
      dm_in.write(dm_data);
    }

    status = s2mm_sts.read();
  } break;
  case exit:
    break;

  default:
    break;
  }
}