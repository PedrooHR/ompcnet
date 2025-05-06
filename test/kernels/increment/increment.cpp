#include "increment.h"

typedef ap_axiu<512, 0, 0, 0> data_type;

void increment(hls::stream<data_type> &input,
               hls::stream<data_type> &output) {
#pragma HLS interface axis port = input
#pragma HLS interface axis port = output
#pragma HLS interface ap_ctrl_none port = return

  data_type value;
  value = input.read();
  for (int i = 0; i < 16; i++)
    #pragma HLS unroll factor=16
    value.data.range((i * 32) + 31, (i * 32)) =
        value.data.range((i * 32) + 31, (i * 32)) + 1;

  output.write(value);
}
