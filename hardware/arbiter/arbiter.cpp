#include <ap_int.h>
#include <hls_stream.h>

#include "arbiter.h"

/* ============================== RX FUNCTIONS ============================== */
void rx_network(hls::stream<network_word> &network_rx,
                hls::stream<packet_word> &iarbiter_rx) {
#pragma HLS inline off
#pragma HLS pipeline II = 1 style=flp

  network_word read_word;
  packet_word write_word;
  ap_uint<1> stall_word;

  write_word.data = 0;
  write_word.keep = 0;
  write_word.last = 0;

  read_word = network_rx.read();

  write_word.data = read_word.data;
  write_word.keep = read_word.keep;
  write_word.last = read_word.last;
  write_word.dest = read_word.dest;

  iarbiter_rx.write(write_word);
}

void rx_arbiter(hls::stream<packet_word> &iarbiter_rx,
                hls::stream<packet_word> &irx_queue,
                hls::stream<ap_uint<1>> &iqueue_in,
                hls::stream<ap_uint<1>> &istall_tx) {
#pragma HLS inline off
#pragma HLS pipeline II = 1 style=flp

  packet_word read_word;
  packet_word write_word;
  ap_uint<1> stall_word;

  write_word.data = 0;
  write_word.keep = 0;
  write_word.last = 0;

  read_word = iarbiter_rx.read();

  write_word.data = read_word.data;
  write_word.keep = read_word.keep;
  write_word.last = read_word.last;
  write_word.dest = read_word.dest;

  // Decide where to send
  if (write_word.keep.bit(15) & !write_word.keep.bit(63)) {
    // If the 15th bit is asserted, but not the 63rd, it is a handshake
    // (0xFFFF)
    stall_word = read_word.data.bit(0);
    istall_tx.write(stall_word);
  } else if (write_word.keep.bit(63)) {
    // If the 63rd bit is asserted, it is a data packet
    irx_queue.write(write_word);
    iqueue_in.write(write_word.last);
  } // drop otherwise
}

void rx_application(hls::stream<packet_word> &irx_queue,
                    hls::stream<network_word> &application_tx,
                    hls::stream<ap_uint<1>> &iqueue_out) {
#pragma HLS inline off
#pragma HLS pipeline II = 1 style=flp
  packet_word read_word;
  network_word write_word;

  read_word = irx_queue.read();

  write_word.data = read_word.data;
  write_word.keep = read_word.keep;
  write_word.dest = read_word.dest;
  write_word.last = read_word.last;

  application_tx.write(write_word);
  iqueue_out.write(write_word.last);
}

/* ================================= STALL ================================= */
void rx_cmd(hls::stream<app_cmd_word> &app_cmd_rx,
            hls::stream<ap_uint<16>> &iapp_cmd_rx) {
#pragma HLS inline off
#pragma HLS pipeline II = 1 style=flp
  app_cmd_word read_word;

  read_word = app_cmd_rx.read();
  iapp_cmd_rx.write(read_word.data);
}

void rx_stall_control(hls::stream<ap_uint<1>> &iqueue_in,
                      hls::stream<ap_uint<1>> &iqueue_out,
                      hls::stream<ap_uint<1>> &istall_rx) {
#pragma HLS inline off
#pragma HLS pipeline II = 1 style=flp

  static int queue_size = 0;
  static ap_uint<1> stalled = 0;
  static int in_words = 0;
  static int out_words = 0;
  ap_uint<1> queue_in_status = 0, queue_out_status = 0;

  if (!iqueue_in.empty()) {
    queue_in_status = iqueue_in.read();
    in_words++;
  }
  if (!iqueue_out.empty()) {
    queue_out_status = iqueue_out.read();
    out_words++;
  }

  queue_size = in_words - out_words;

  if (stalled == 0 && queue_size > THRESHOLD_MAX) {
    stalled = 1;
    istall_rx.write(1);
  } else if (stalled == 1 && queue_size < THRESHOLD_MIN) {
    stalled = 0;
    istall_rx.write(0);
  }
}

void rx_stall_cmd(hls::stream<ap_uint<16>> &iapp_cmd_rx,
                  hls::stream<ap_uint<1>> &istall_rx,
                  hls::stream<ap_uint<17>> &istall_cmd) {
#pragma HLS inline off
#pragma HLS pipeline II = 1 style=flp

  enum txStallStateEnum { CMD, STALL };
  static txStallStateEnum txStallState = CMD;
  static ap_uint<16> dest = 99;
  ap_uint<17> stall_word;

  switch (txStallState) {
  case CMD: {
    if (!iapp_cmd_rx.empty()) {
      ap_uint<16> ldest = iapp_cmd_rx.read();
      dest = ldest;
    }
    txStallState = STALL;
  } break;
  case STALL: {
    if (!istall_rx.empty()) {
      stall_word.range(15, 0) = dest;
      stall_word.bit(16) = istall_rx.read();
      istall_cmd.write(stall_word);
    }
    txStallState = CMD;
  } break;
  }
}

/* ============================== TX FUNCTIONS ============================== */
void tx_application(hls::stream<network_word> &application_tx,
                    hls::stream<packet_word> &itx_app) {
#pragma HLS inline off
#pragma HLS pipeline II = 1 style=flp
  network_word read_word;
  packet_word write_word;

  read_word = application_tx.read();

  write_word.data = read_word.data;
  write_word.keep = read_word.keep;
  write_word.dest = read_word.dest;
  write_word.last = read_word.last;

  itx_app.write(write_word);
}

void tx_stall_arbiter(hls::stream<packet_word> &itx_sapp,
                      hls::stream<ap_uint<1>> &istall_tx,
                      hls::stream<packet_word> &itx_capp) {
#pragma HLS inline off
#pragma HLS pipeline II = 1 style=flp

  enum txStallArbiterStateEnum { STALL, DATA };
  static txStallArbiterStateEnum StallArbiterState = STALL;

  static ap_uint<1> stall = 0;
  static ap_uint<1> idle = 1;

  packet_word read_word;
  packet_word write_word;

  write_word.data = 0;
  write_word.keep = 0;
  write_word.last = 0;

  switch (StallArbiterState) {
  case STALL: {
    if (idle == 1 && !istall_tx.empty())
      stall = istall_tx.read();

    StallArbiterState = DATA;
  } break;
  case DATA: {
    if (stall == 0 && !itx_sapp.empty()) {
      read_word = itx_sapp.read();

      write_word.data = read_word.data;
      write_word.keep = read_word.keep;
      write_word.last = read_word.last;
      write_word.dest = read_word.dest;

      itx_capp.write(write_word);

      if (read_word.last) {
        idle = 1;
        StallArbiterState = STALL;
      } else {
        idle = 0;
      }
    } else {
      StallArbiterState = STALL;
    }
  } break;
  }
}

void tx_cmd_arbiter(hls::stream<packet_word> &itx_capp,
                    hls::stream<ap_uint<17>> &istall_cmd,
                    hls::stream<packet_word> &itx_net) {
#pragma HLS inline off
#pragma HLS pipeline II = 1 style=flp

  enum txCMDArbiterStateEnum { STALL_MSG, DATA };
  static txCMDArbiterStateEnum CMDArbiterState = STALL_MSG;

  packet_word read_word;
  packet_word write_word;

  static ap_uint<1> idle = 1;

  write_word.data = 0;
  write_word.keep = 0;
  write_word.last = 0;

  switch (CMDArbiterState) {
  case STALL_MSG: {
    if (!istall_cmd.empty()) {
      ap_uint<17> stall_word = istall_cmd.read();
      write_word.data.bit(0) = stall_word.bit(16);
      write_word.keep = 0xFFFF;
      write_word.last = 1;
      write_word.dest = stall_word.range(15, 0);
      itx_net.write(write_word);
    }

    CMDArbiterState = DATA;
  } break;

  case DATA: {
    if (!itx_capp.empty()) {
      read_word = itx_capp.read();

      write_word.data = read_word.data;
      write_word.keep = read_word.keep;
      write_word.last = read_word.last;
      write_word.dest = read_word.dest;

      itx_net.write(write_word);

      if (read_word.last)
        CMDArbiterState = STALL_MSG;
    } else {
      CMDArbiterState = STALL_MSG;
    }
  } break;
  }
}

// Pass from internal stream to the out network stream
void tx_network(hls::stream<packet_word> &itx_net,
                hls::stream<network_word> &network_tx) {
#pragma HLS inline off
#pragma HLS pipeline II = 1 style=flp
  packet_word read_word;
  network_word write_word;

  read_word = itx_net.read();

  write_word.data = read_word.data;
  write_word.keep = read_word.keep;
  write_word.dest = read_word.dest;
  write_word.last = read_word.last;

  network_tx.write(write_word);
}

void arbiter(hls::stream<network_word> &application_tx,
             hls::stream<network_word> &application_rx,
             hls::stream<network_word> &network_tx,
             hls::stream<network_word> &network_rx,
             hls::stream<app_cmd_word> &app_cmd_rx) {
#pragma HLS DATAFLOW disable_start_propagation

// TX ENGINE INTERFACE
#pragma HLS INTERFACE axis register both port = application_tx
#pragma HLS INTERFACE axis register both port = network_tx
// RX ENGINE INTERFACE
#pragma HLS INTERFACE axis register both port = application_rx
#pragma HLS INTERFACE axis register both port = network_rx
// STALL CMD INTEFACE
#pragma HLS INTERFACE axis register both port = app_cmd_rx
// CONTROL TYPE
#pragma HLS INTERFACE ap_ctrl_none port = return

  // Internal Streams
  static hls::stream<packet_word> iarbiter_rx("iarbiter_rx");
#pragma HLS stream variable = iarbiter_rx type = FIFO depth = 3

  static hls::stream<packet_word> irx_queue("irx_queue");
#pragma HLS stream variable = irx_queue type = FIFO depth = RX_FIFO_DEPTH

  static hls::stream<ap_uint<1>> iqueue_out("iqueue_out");
#pragma HLS stream variable = iqueue_out type = FIFO depth = 3

  static hls::stream<ap_uint<1>> iqueue_in("iqueue_in");
#pragma HLS stream variable = iqueue_in type = FIFO depth = 3

  static hls::stream<ap_uint<16>> iapp_cmd_rx("iapp_cmd_rx");
#pragma HLS stream variable = iapp_cmd_rx type = FIFO depth = 5

  static hls::stream<ap_uint<1>> istall_rx("istall_rx");
#pragma HLS stream variable = istall_rx type = FIFO depth = 3

  static hls::stream<ap_uint<17>> istall_cmd("istall_cmd");
#pragma HLS stream variable = istall_cmd type = FIFO depth = 3

  static hls::stream<ap_uint<1>> istall_tx("istall_tx");
#pragma HLS stream variable = istall_tx type = FIFO depth = 5

  static hls::stream<packet_word> itx_sapp("itx_sapp");
#pragma HLS stream variable = itx_sapp type = FIFO depth = 6

  static hls::stream<packet_word> itx_capp("itx_capp");
#pragma HLS stream variable = itx_capp type = FIFO depth = 6

  static hls::stream<packet_word> itx_net("itx_net");
#pragma HLS stream variable = itx_net type = FIFO depth = 3

  rx_network(network_rx, iarbiter_rx);
  rx_arbiter(iarbiter_rx, irx_queue, iqueue_in, istall_tx);
  rx_application(irx_queue, application_rx, iqueue_out);

  rx_cmd(app_cmd_rx, iapp_cmd_rx);
  rx_stall_control(iqueue_in, iqueue_out, istall_rx);
  rx_stall_cmd(iapp_cmd_rx, istall_rx, istall_cmd);

  tx_application(application_tx, itx_sapp);
  tx_stall_arbiter(itx_sapp, istall_tx, itx_capp);
  tx_cmd_arbiter(itx_capp, istall_cmd, itx_net);
  tx_network(itx_net, network_tx); 
}