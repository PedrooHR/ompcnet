#include <hls_stream.h>

#include "common.h"

void app2net(ap_uint<16> src, ap_uint<16> dst, ap_uint<3> op, ap_uint<64> add,
             ap_uint<32> len, hls::stream<application_word> &application,
             hls::stream<network_word> &network,
             hls::stream<cmd_word> &mm2s_cmd, hls::stream<sts_word> &mm2s_sts,
             hls::stream<cmd_word> &s2mm_cmd, hls::stream<sts_word> &s2mm_sts,
             hls::stream<data_word> &dm_in, hls::stream<data_word> &dm_out);
