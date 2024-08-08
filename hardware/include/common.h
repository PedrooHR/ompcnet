#include <ap_axi_sdata.h>
#include <ap_int.h>
#include <hls_stream.h>

#include <stdint.h>

enum OPERATION {
  send        = 0b001,
  recv        = 0b010,
  stream_to   = 0b011,
  stream_from = 0b100,
  stream2mem  = 0b101,
  mem2stream  = 0b110,
};

// Macros for cmd word
#define command_address(x)  x.data.range( 63,   0)
#define command_len(x)      x.data.range( 92,  64)

// Stream pkt definitions
typedef ap_uint<32> mem_word;
typedef ap_axiu<512, 0, 0, 0> application_word;
typedef ap_axiu<512, 0, 0, 0> data_word;
typedef ap_axiu<512, 1, 1, 16> network_word;
typedef ap_axiu<93, 0, 0, 0> cmd_word;
typedef ap_axiu<1, 0, 0, 0> sts_word;
