#include "arbiter.h"

/*
 The TX engine is always composed of the following operations: HS Req, HS Res,
 Data Transmission. *The ordering of HS Req and HS Res are interchangeable.
  - HS Req: Is the requisition handshake from this side of the communication to
 the other side.
  - HS Res: Is the response from the handshake requisition from the other side
 of the communication.
  - Data Trasmission: This is the data transmission, will be the length
 according to the Requisitions and Responses during the handshake.

 Reads will always be 512 bits. Stream keep is used to parse packets:
  - Keep == -1 means it is the data in transmission
  - Keep != -1 means it is a handshake operation
*/
void tx(hls::stream<network_word> &tx_network,
        hls::stream<network_word> &tx_handshake,
        hls::stream<network_word> &rx_handshake) {
  // Check if there is and out tx handshake packet
  network_word data;
  
  


}

/*
 The RX engine receives and parses 
*/
void rx(hls::stream<network_word> &rx_network,
        hls::stream<64b_word> &rx_internal) {}

void handshake(hls::stream<handshake_word> &m_handshake,
               hls::stream<handshake_word> &s_handshake) {
  ap_uint<16> src, dst;
  ap_uint<29> length;
  ap_uint<3> operation;

  // wait for a handshake start
  handshake_word request, response;
  request = s_handshake.read();

  // There is a two round handshake, we will receive a response, and also a
  // request to send a response.

  // This tells back the controller to start the operation
  response.data = request.data;
  response.data.range(63, 61) = 0b111; // okay
}

void arbiter(
    /* app2net */
    hls::stream<handshake_word> &m_tx_handshake, // Master TX Handshake
    hls::stream<handshake_word> &s_tx_handshake, // Slave TX Handshake
    hls::stream<network_word> &tx_app2net,       // Connection from app2net
    hls::stream<network_word> &tx_network,       // Connection to Netlayer
    /* net2app */
    hls::stream<handshake_word> &m_rx_handshake, // Master RX Handshake
    hls::stream<handshake_word> &s_rx_handshake, // Slave RX Handshake
    hls::stream<network_word> &rx_net2app,       // Connection to net2app
    hls::stream<network_word> &rx_network,       // Connection from Netlayer
) {
  /* app2net */
#pragma HLS INTERFACE axis port = m_tx_handshake
#pragma HLS INTERFACE axis port = s_tx_handshake
#pragma HLS INTERFACE axis port = tx_app2net
#pragma HLS INTERFACE axis port = tx_network
/* net2app */
#pragma HLS INTERFACE axis port = m_rx_handshake
#pragma HLS INTERFACE axis port = s_rx_handshake
#pragma HLS INTERFACE axis port = rx_net2app
#pragma HLS INTERFACE axis port = rx_network
  // Function return
#ifndef COSIM
#pragma HLS INTERFACE ap_ctrl_none port = return
#else
#pragma HLS INTERFACE s_axilite port = return
#endif

  // Internal streams

#pragma HLS DATAFLOW
  s2mm();
  mm2s();
}