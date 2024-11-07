#include <ap_axi_sdata.h>
#include <ap_int.h>
#include <hls_stream.h>

#include "common.h"

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

constexpr unsigned PACKET_SIZE = 22;
constexpr unsigned RX_FIFO_DEPTH = PACKET_SIZE * 8;
constexpr unsigned THRESHOLD_MAX = RX_FIFO_DEPTH - (PACKET_SIZE * 2);
constexpr unsigned THRESHOLD_MIN = PACKET_SIZE * 2;

void arbiter(hls::stream<network_word> &app2net_data_tx,
             hls::stream<network_word> &net2app_data_rx,
             hls::stream<network_word> &network_tx,
             hls::stream<network_word> &network_rx,
             hls::stream<app_cmd_word> &app_cmd_rx);