#include <ap_int.h>
#include <hls_stream.h>

#include "arbiter.h"

#include <hls_print.h>

/* Besides data packets, the arbiter can parse and deal with small packets, we
  call the arbiter packets. Arbiter packets are used for operatation handshake
  and operation stalling.

  The packet is composed by 128 bits:

  packet[127:112]: 16 bits: Source rank
  packet[111: 96]: 16 bits: Destination rank
  packet[ 95: 88]:  8 bits: Operation (TX/RX Hs & Stall)
  packet[ 87: 80]:  8 bits: Reserved - Errors
  packet[ 79:  0]: 80 bits: Payload according to the operation

  Handshake operation bits:
  packet[79:72]:  8 bits: Reserved
  packet[71:64]:  8 bits: Operation (send/recv, etc)
  packet[63:48]: 16 bits: Operation source rank
  packet[47:32]: 16 bits: Operation dest rank
  packet[31: 0]: 32 bits: Operation length (29 used)

  Stall operatation bits:
  packet[79: 0]: 80 bits: Reserved
*/

void rx_arbiter(hls::stream<handshake_word> &app2net_hs_rx,
                hls::stream<handshake_word> &net2app_hs_rx,
                hls::stream<ap_uint<1>> &stall_control, network_word ar_word) {
  ap_uint<8> op = ar_word.data.range(95, 88);

  switch (op) {
  case ARB_OPERATION::stall_start: {
    // Another rank cannot receive data - Not implemented yet
    stall_control.write(0b1);
  } break;
  case ARB_OPERATION::stall_stop: {
    // Another rank can receive data - Not implemented yet
    stall_control.write(0b0);
  } break;
  case ARB_OPERATION::tx_request: {
    // Another rank wants to send data to me - Redirect to net2app
    net2app_hs_rx.write(ar_word);
  } break;
  case ARB_OPERATION::rx_response: {
    // Another rank acknowledged my request to send data - Redirect to app2net
    app2net_hs_rx.write(ar_word);
  } break;
  default:
    break;
  }
}

void tx_engine(hls::stream<handshake_word> &app2net_hs_tx,
               hls::stream<handshake_word> &net2app_hs_tx,
               hls::stream<network_word> &application_tx,
               hls::stream<network_word> &network_tx,
               hls::stream<ap_uint<512>> &rmt_stall_control,
               hls::stream<ap_uint<1>> &stall_control) {
  network_word tx_word;
  handshake_word hs_word;
  ap_uint<512> rmt_stall_value;
  static ap_uint<1> stall_value = 0;
  static ap_uint<5> packet_part = 0;
  static bool stall_tx = false;

  if (packet_part == 0 && !stall_control.empty()) {
    // if not in the middle of packet and received a stall control, evaluate it
    stall_value = stall_control.read();
    if (stall_value == 0b1)
      stall_tx = true; // stall transmissions that should be stalled
    else if (stall_value == 0b0)
      stall_tx = false; // enable transmission again
  } else if (!stall_tx && packet_part == 0 && !app2net_hs_tx.empty()) {
    // If not in the middle of packet and not stalled, proceed with the hs
    network_tx.write(app2net_hs_tx.read());
  } else if (!stall_tx && packet_part == 0 && !net2app_hs_tx.empty()) {
    // If not in the middle of packet and not stalled, proceed with the hs
    network_tx.write(net2app_hs_tx.read());
  } else if (packet_part == 0 && !rmt_stall_control.empty()) {
    // If not in the middle of packet, proceed with the remote stall control
    rmt_stall_value = rmt_stall_control.read();
    tx_word.last = 1;
    tx_word.keep = 0xFFFF;
    tx_word.dest = rmt_stall_value.range(111, 96);
    tx_word.data = rmt_stall_value;
    network_tx.write(tx_word);
  } else if (!stall_tx && !application_tx.empty()) {
    // if not stalled, proceed with data transmission
    network_tx.write(application_tx.read());
    packet_part++;
    if (packet_part >= 22)
      packet_part = 0;
  }
}

// monitor receives input every 2 data read and write
void rx_monitor(hls::stream<ap_uint<16>> &rx_in_control,
                hls::stream<ap_uint<1>> &rx_out_control,
                hls::stream<ap_uint<512>> &rmt_stall_control) {
  ap_uint<512> tx_word;
  ap_uint<16> dst;
  ap_uint<1> dummy;
  static unsigned long int status = 0;
  static bool stall = false;
  if (rx_in_control.read_nb(dst)) {
    status += 2;
  }
  if (rx_out_control.read_nb(dummy)) {
    status -= 2;
  }

  if (status >= THRESHOLD_MAX && !stall) {
    stall = true;
    tx_word.range(111, 96) = dst;
    tx_word.range(95, 88) = ARB_OPERATION::stall_start;
    rmt_stall_control.write(tx_word);
  } else if (status <= THRESHOLD_MIN) {
    stall = false;
    tx_word.range(111, 96) = dst;
    tx_word.range(95, 88) = ARB_OPERATION::stall_stop;
    rmt_stall_control.write(tx_word);
  }
}

void rx_out_engine(hls::stream<network_word> &application_rx,
                   hls::stream<ap_uint<512>> &rx_fifo,
                   hls::stream<ap_uint<1>> &rx_out_control) {
  static unsigned long int out_counter = 0;

  network_word packet;
  packet.data = rx_fifo.read();
  application_rx.write(packet);
  out_counter++;

  if (out_counter % 2 == 0)
    rx_out_control.write(0b1);
}

void rx_in_engine(hls::stream<handshake_word> &app2net_hs_rx,
                  hls::stream<handshake_word> &net2app_hs_rx,
                  hls::stream<network_word> &network_rx,
                  hls::stream<ap_uint<512>> &rx_fifo,
                  hls::stream<ap_uint<16>> &rx_in_control,
                  hls::stream<ap_uint<1>> &stall_control) {
  static unsigned int in_counter = 0;

  network_word rx_word;
  rx_word = network_rx.read();
  if (rx_word.keep == 0xFFFF) {
    // 128 bits means it is an arbiter packet
    rx_arbiter(app2net_hs_rx, net2app_hs_rx, stall_control, rx_word);
  } else {
    // Any other keep (usually all 512 bits) is data packet
    rx_fifo.write(rx_word.data);
    in_counter++;
  }

  if (in_counter % 2 == 0)
    rx_in_control.write(rx_word.dest);
}

void arbiter(hls::stream<network_word> &app2net_data_tx,
             hls::stream<network_word> &net2app_data_rx,
             hls::stream<network_word> &network_tx,
             hls::stream<network_word> &network_rx,
             hls::stream<handshake_word> &app2net_hs_tx,
             hls::stream<handshake_word> &app2net_hs_rx,
             hls::stream<handshake_word> &net2app_hs_tx,
             hls::stream<handshake_word> &net2app_hs_rx) {
#pragma HLS INTERFACE axis port = app2net_data_tx
#pragma HLS INTERFACE axis port = net2app_data_rx
#pragma HLS INTERFACE axis port = network_tx
#pragma HLS INTERFACE axis port = network_rx
#pragma HLS INTERFACE axis port = app2net_hs_tx
#pragma HLS INTERFACE axis port = app2net_hs_rx
#pragma HLS INTERFACE axis port = net2app_hs_tx
#pragma HLS INTERFACE axis port = net2app_hs_rx
#pragma HLS INTERFACE ap_ctrl_none port = return

  hls::stream<ap_uint<16>> rx_in_control;
#pragma HLS STREAM variable = rx_in_control type = FIFO depth = 32
  hls::stream<ap_uint<1>> rx_out_control;
#pragma HLS STREAM variable = rx_out_control type = FIFO depth = 32
  hls::stream<ap_uint<1>> stall_control;
#pragma HLS STREAM variable = stall_control type = FIFO depth = 32

  hls::stream<ap_uint<512>> rmt_stall_control;
#pragma HLS STREAM variable = rmt_stall_control type = FIFO depth = 32
#pragma HLS bind_storage variable = rmt_stall_control type = FIFO impl = BRAM

  hls::stream<ap_uint<512>> rx_fifo;
#pragma HLS STREAM variable = rx_fifo type = FIFO depth = RX_FIFO_DEPTH
#pragma HLS bind_storage variable = rx_fifo type = FIFO impl = BRAM

#pragma HLS DATAFLOW
  tx_engine(app2net_hs_tx, net2app_hs_tx, app2net_data_tx, network_tx,
            rmt_stall_control, stall_control);
  rx_monitor(rx_in_control, rx_out_control, rmt_stall_control);
  rx_out_engine(net2app_data_rx, rx_fifo, rx_out_control);
  rx_in_engine(app2net_hs_rx, net2app_hs_rx, network_rx, rx_fifo, rx_in_control,
               stall_control);
}