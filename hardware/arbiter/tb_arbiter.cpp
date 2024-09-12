#include "arbiter.h"

#define PACKET_SIZE 1408
#define PACKET_STREAMS 22
#define PACKET_INTS 352
#define PACKET_NUM 5

int main() {
  int sanity_check = 0;

  /* app2net */
  hls::stream<handshake_word> m_tx_handshake("m_tx_handshake");
  hls::stream<handshake_word> s_tx_handshake("s_tx_handshake");
  hls::stream<network_word> tx_app2net("tx_app2net");
  hls::stream<network_word> tx_network("tx_network");
  /* net2app */
  hls::stream<handshake_word> m_rx_handshake("m_rx_handshake");
  hls::stream<handshake_word> s_rx_handshake("s_rx_handshake");
  hls::stream<network_word> rx_net2app("rx_net2app");
  hls::stream<network_word> rx_network("rx_network");

  int len = 128; // in bytes
  ap_uint<29> len_bytes = len * sizeof(len);
  int sent_tx_buffer[PACKET_NUM * PACKET_INTS];
  int recv_tx_buffer[PACKET_NUM * PACKET_INTS];
  int sent_rx_buffer[PACKET_NUM * PACKET_INTS];
  int recv_rx_buffer[PACKET_NUM * PACKET_INTS];

  // Init arrays
  for (int i = 0; i < PACKET_NUM * PACKET_INTS; i++) {
    sent_tx_buffer[i] = i + 5096;
    recv_tx_buffer[i] = 0;
    sent_rx_buffer[i] = i + 32767;
    recv_rx_buffer[i] = 0;
  }

  // Populate send streams
  for (int i = 0; i < PACKET_NUM * PACKET_INTS; i += 16) {
    network_word tx_value;
    network_word rx_value;
    for (int j = 0; j < 16; j++) {
      tx_value.data.range(32 * j + 31, 32 * j) = sent_tx_buffer[i * 16 + j];
      rx_value.data.range(32 * j + 31, 32 * j) = sent_rx_buffer[i * 16 + j];
    }
    tx_value.keep = -1;
    rx_value.keep = -1;
    tx_app2net.write(tx_value);
    rx_network.write(rx_value);
  }

  // TX Handshake

  arbiter(m_tx_handshake, s_tx_handshake, tx_app2net, tx_network,
          m_rx_handshake, s_rx_handshake, rx_net2app, rx_network);

  for (int i = 0; i < PACKET_NUM * PACKET_INTS; i += 16) {
    network_word tx_value = tx_network.read();
    network_word rx_value = rx_net2app.read();
    for (int j = 0; j < 16; j++) {
      recv_tx_buffer[i * 16 + j] = tx_value.data.range(32 * j + 31, 32 * j);
      recv_rx_buffer[i * 16 + j] = rx_value.data.range(32 * j + 31, 32 * j);
    }
  }

  // comparing
  for (int i = 0; i < PACKET_NUM * PACKET_INTS; i++) {
    if (sent_tx_buffer[i] != recv_tx_buffer[i])
      sanity_check++;
    if (sent_rx_buffer[i] != recv_rx_buffer[i])
      sanity_check++;
  }

  return sanity_check;
}