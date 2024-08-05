#include <hls_stream.h>

#include "common.h"

void controller(ap_uint<16> src,
                ap_uint<16> dst,
                ap_uint< 3> operation,
                ap_uint<64> address,
                ap_uint<29> size,
                hls::stream<control_word> &app2net,
                hls::stream<control_word> &net2app);