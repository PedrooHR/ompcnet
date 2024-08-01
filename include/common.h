#include <ap_axi_sdata.h>
#include <ap_int.h>
#include <hls_stream.h>

#include <stdint.h>

#define range_s(j) j * 8
#define range_f(j) range_s(j) + 7

enum OPERATION {
  send = 1,
  recv = 2,
  stream_to = 3,
  stream_from = 4,
  stream2mem = 5,
  mem2stream = 6,
};

// Macros for control word
#define control_src(x)      x.data.range( 31,   0)
#define control_dst(x)      x.data.range( 63,  32)
#define control_op(x)       x.data.range( 71,  64)
#define control_address(x)  x.data.range(135,  72)
#define control_len(x)      x.data.range(167, 136)

// Macros for cmd word
#define command_address(x)  x.data.range( 63,   0)
#define command_len(x)      x.data.range( 95,  64)

// Stream pkt definitions
typedef ap_axiu<8, 0, 0, 0> application_word;
typedef ap_axiu<512, 0, 0, 16> network_word;
typedef ap_axiu<168, 0, 0, 0> control_word;
typedef ap_axiu<96, 0, 0, 0> cmd_word;
typedef ap_axiu<32, 0, 0, 0> sts_word;
typedef ap_axiu<8, 0, 0, 0> data_word;
typedef ap_int<8> mem_word;
