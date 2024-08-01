
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
#pragma HLS INTERFACE s_axilite port = return

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
      Recv Operation:
        - Receives from the Network stack and stores in a memory mapped space
    */
  case recv: {
    int len = control_len(inst);
    uintptr_t address = control_address(inst);

    command_address(command) = address;
    command_len(command) = len;
    s2mm_cmd.write(command);

    // data attribution
    int i = 0, j = 0;
    while (i < len) {
      j = 0;
      net_data = network.read();
      while (i < len && j < 64) {
        dm_data.data = net_data.data.range(range_f(j), range_s(j));
        dm_in.write(dm_data);
        i++;
        j++;
      }
    }

    status = s2mm_sts.read();
  } break;
    /*
      Stream From Operation:
        - Receives from the Network stack and redirects to the application
    */
  case stream_from: {
    int len = control_len(inst);

    int i = 0, j = 0;
    while (i < len) {
      j = 0;
      net_data = network.read();
      while (i < len && j < 64) {
        app_data.data = net_data.data.range(range_f(j), range_s(j));
        application.write(app_data);
        i++;
        j++;
      }
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

    for (int i = 0; i < len; i++) {
      app_data.data = dm_out.read().data;
      application.write(app_data);
    }

    status = mm2s_sts.read();
  } break;
  case exit:
    break;

  default:
    break;
  }
}