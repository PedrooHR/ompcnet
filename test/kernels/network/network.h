#include <ap_axi_sdata.h>
#include <ap_int.h>
#include <hls_stream.h>

typedef ap_axiu<512, 1, 1, 16> network_word;

#ifndef COSIM

void network(hls::stream<network_word> &input_1,
             hls::stream<network_word> &output_1,
             hls::stream<network_word> &input_2,
             hls::stream<network_word> &output_2,
             hls::stream<network_word> &input_3,
             hls::stream<network_word> &output_3);

#else // if compiling for COSIM

void network(int len_bytes,
             hls::stream<network_word> &input_1,
             hls::stream<network_word> &output_1,
             hls::stream<network_word> &input_2,
             hls::stream<network_word> &output_2,
             hls::stream<network_word> &input_3,
             hls::stream<network_word> &output_3);

#endif