
#include <ap_int.h>
#include <hls_stream.h>

#include "app2net.h"
/*
#include <hls_print.h>

// packet[127:112]: 16 bits: Source rank
// packet[111: 96]: 16 bits: Destination rank
// packet[ 95: 88]:  8 bits: Operation (TX/RX Hs & Stall)
// packet[ 87: 80]:  8 bits: Reserved - Errors
// packet[ 79: 72]:  8 bits: Reserved
// packet[ 71: 64]:  8 bits: Operation (send/recv, etc)
// packet[ 63: 48]: 16 bits: Operation source rank
// packet[ 47: 32]: 16 bits: Operation dest rank
// packet[ 31:  0]: 32 bits: Operation length (29 used)
void app2net_handshake(hls::stream<handshake_word> &handshake_tx,
                       hls::stream<handshake_word> &handshake_rx,
                       ap_uint<16> src, ap_uint<16> dst, ap_uint<3> op,
                       ap_uint<32> len) {
#pragma HLS inline off
  handshake_word hs_word;
  hs_word.last = 1;
  hs_word.keep = 0xFFFF;
  hs_word.dest = dst;
  hs_word.data.range(127, 112) = src;
  hs_word.data.range(111, 96) = dst;
  hs_word.data.range(95, 88) = ARB_OPERATION::tx_request;
  hs_word.data.range(87, 80) = 0;
  hs_word.data.range(79, 72) = 0;
  hs_word.data.range(66, 64) = op;
  hs_word.data.range(63, 48) = src;
  hs_word.data.range(47, 32) = dst;
  hs_word.data.range(32, 0) = len;

  // Emit Request
  handshake_tx.write(hs_word);

  // Blocking-wait for response --- Should we check? I don't think we need it
  // right now
  hs_word = handshake_rx.read();
}
*/
void app2net(ap_uint<16> src, ap_uint<16> dst, ap_uint<3> op, ap_uint<64> add,
             ap_uint<32> len, hls::stream<application_word> &application,
             hls::stream<network_word> &network,
             /*hls::stream<handshake_word> &handshake_tx,
             hls::stream<handshake_word> &handshake_rx,*/
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
/*#pragma HLS INTERFACE axis port = handshake_tx
#pragma HLS INTERFACE axis port = handshake_rx*/
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
    // app2net_handshake(handshake_tx, handshake_rx, src, dst, op, len);

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
    // app2net_handshake(handshake_tx, handshake_rx, src, dst, op, len);

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