#include <ap_axi_sdata.h>
#include <ap_int.h>
#include <hls_stream.h>

#include "common.h"

void datamover(
    /* MM2S Part */
    hls::stream<cmd_word> &mm2s_cmd,   // mm2s command stream
    hls::stream<sts_word> &mm2s_sts,   // mm2s status stream
    hls::stream<data_word> &mm2s_axis, // AXI4 stream for data
    mem_word *mm2s_axi,                // AXI4 for reading mem
    /* S2MM Part */
    hls::stream<cmd_word> &s2mm_cmd,   // s2mm command stream
    hls::stream<sts_word> &s2mm_sts,   // s2mm status stream
    hls::stream<data_word> &s2mm_axis, // AXI4 stream for data
    mem_word *s2mm_axi                 // AXI4 for writting mem
);