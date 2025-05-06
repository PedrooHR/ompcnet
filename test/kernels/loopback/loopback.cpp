#include "loopback.h"

typedef ap_axiu<512, 0, 0, 0> data_type;

void loopback(hls::stream<data_type> &input,
               hls::stream<data_type> &output) {
#pragma HLS interface axis port = input
#pragma HLS interface axis port = output
#pragma HLS interface ap_ctrl_none port = return

data_type tmp;

do{
#pragma HLS PIPELINE II=1
	tmp = input.read();
	output.write(tmp);
} while(tmp.last == 0);

}
