#include "increment.h"

typedef ap_axiu<512, 0, 0, 0> data_type;

void increment(ap_uint<32> len_bytes, hls::stream<data_type> &input,
               hls::stream<data_type> &output) {
#pragma HLS interface s_axilite port = len_bytes bundle = control
#pragma HLS interface axis port = input
#pragma HLS interface axis port = output
#pragma HLS interface s_axilite port = return bundle = control

  data_type value;
  for (int i = 0; i < len_bytes / 64; i++) {
    value = input.read();
    for (int i = 0; i < 16; i++)
      value.data.range((i * 32) + 31, (i * 32)) =
          value.data.range((i * 32) + 31, (i * 32)) + 1;

    output.write(value);
  }
}