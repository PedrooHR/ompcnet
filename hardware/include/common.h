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

enum ARB_OPERATION { // 8 bits
  nop         = 0x0,
  stall_start = 0x11,
  stall_stop  = 0x21,
  tx_request  = 0x31,
  rx_response = 0x41,
};

// Macros for cmd word
#define command_address(x)  x.data.range( 63,   0)
#define command_len(x)      x.data.range( 92,  64)

// Internal types (HLS interface does not handle ap_axi* types)
typedef ap_uint<512> mem_word;
typedef ap_uint<512> wide_word;

typedef struct packet_word {
  ap_uint<512> data;
  ap_uint<64> keep;
  ap_uint<16> dest;
  ap_uint<1> last;
} packet_word;

// Interface types
typedef ap_axiu<512, 0, 0, 16> handshake_word;
typedef ap_axiu<512, 0, 0, 0> application_word;
typedef ap_axiu<512, 0, 0, 0> data_word;
typedef ap_axiu<512, 0, 0, 16> network_word;
typedef ap_axiu<93, 0, 0, 0> cmd_word;
typedef ap_axiu<1, 0, 0, 0> sts_word;
typedef ap_axiu<16, 0, 0, 0> app_cmd_word;
