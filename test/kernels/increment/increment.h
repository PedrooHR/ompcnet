#include <ap_axi_sdata.h>
#include <ap_int.h>
#include <hls_stream.h>

typedef ap_axiu<512, 0, 0, 0> data_type;

void increment(ap_uint<32> len_bytes, hls::stream<data_type> &input,
               hls::stream<data_type> &output);