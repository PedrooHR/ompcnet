#include <ap_axi_sdata.h>
#include <ap_int.h>
#include <hls_stream.h>

#include "common.h"

void arbiter(
    /* app2net */
    hls::stream<handshake_word> &m_tx_handshake, // Master TX Handshake
    hls::stream<handshake_word> &s_tx_handshake, // Slave TX Handshake
    hls::stream<network_word> &tx_network,       // Connection to Netlayer
    /* net2app */
    hls::stream<handshake_word> &m_rx_handshake, // Master RX Handshake
    hls::stream<handshake_word> &s_rx_handshake, // Slave RX Handshake
    hls::stream<network_word> &rx_network,       // Connection from Netlayer
);