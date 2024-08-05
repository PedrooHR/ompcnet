#include <hls_stream.h>

#include "common.h"

void net2app(hls::stream<control_word> &controller,
             hls::stream<application_word> &application, 
             hls::stream<network_word> &network,
             hls::stream<cmd_word> &mm2s_cmd,
             hls::stream<sts_word> &mm2s_sts,
             hls::stream<cmd_word> &s2mm_cmd,
             hls::stream<sts_word> &s2mm_sts,
             hls::stream<data_word> &dm_in,
             hls::stream<data_word> &dm_out);
