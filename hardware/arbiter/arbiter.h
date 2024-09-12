#include <ap_axi_sdata.h>
#include <ap_int.h>
#include <hls_stream.h>

#include "common.h"

constexpr unsigned PACKET_SIZE = 22;
constexpr unsigned RX_FIFO_DEPTH = PACKET_SIZE * 100;
constexpr unsigned THRESHOLD_MAX = RX_FIFO_DEPTH - (PACKET_SIZE * 10);
constexpr unsigned THRESHOLD_MIN = PACKET_SIZE * 10;

void arbiter(hls::stream<network_word> &app2net_data_tx,
             hls::stream<network_word> &net2app_data_rx,
             hls::stream<network_word> &network_tx,
             hls::stream<network_word> &network_rx,
             hls::stream<handshake_word> &app2net_hs_tx,
             hls::stream<handshake_word> &app2net_hs_rx,
             hls::stream<handshake_word> &net2app_hs_tx,
             hls::stream<handshake_word> &net2app_hs_rx);