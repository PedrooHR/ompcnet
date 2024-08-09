#include <ap_axi_sdata.h>
#include <ap_int.h>
#include <hls_stream.h>

typedef ap_axiu<512, 1, 1, 16> network_word;

void network(int receiver_1, int receiver_2, int receiver_3, int len_bytes,
             hls::stream<network_word> &input_1,
             hls::stream<network_word> &output_1,
             hls::stream<network_word> &input_2,
             hls::stream<network_word> &output_2,
             hls::stream<network_word> &input_3,
             hls::stream<network_word> &output_3);