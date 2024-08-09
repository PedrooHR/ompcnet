#include "network.h"

void send_packet(hls::stream<network_word> &output_1,
                 hls::stream<network_word> &output_2,
                 hls::stream<network_word> &output_3, network_word value,
                 int dest) {

  switch (dest) {
  case 1:
    output_1.write(value);
    break;
  case 2:
    output_2.write(value);
    break;
  case 3:
    output_3.write(value);
    break;
  default:
    break;
  }
}

void network(int receiver_1, int receiver_2, int receiver_3, int len_bytes,
             hls::stream<network_word> &input_1,
             hls::stream<network_word> &output_1,
             hls::stream<network_word> &input_2,
             hls::stream<network_word> &output_2,
             hls::stream<network_word> &input_3,
             hls::stream<network_word> &output_3) {
#pragma HLS interface s_axilite port = receiver_1 bundle = control
#pragma HLS interface s_axilite port = receiver_2 bundle = control
#pragma HLS interface s_axilite port = receiver_3 bundle = control
#pragma HLS interface s_axilite port = len_bytes bundle = control
#pragma HLS interface axis port = input_1
#pragma HLS interface axis port = output_1
#pragma HLS interface axis port = input_2
#pragma HLS interface axis port = output_2
#pragma HLS interface axis port = input_3
#pragma HLS interface axis port = output_3
#pragma HLS interface s_axilite port = return bundle = control

  int counter_1 = len_bytes;
  int counter_2 = len_bytes;
  int counter_3 = len_bytes;

  network_word data_val;

  while (counter_1 > 0 || counter_2 > 0 || counter_3 > 0) {
    if (input_1.read_nb(data_val)) {
      send_packet(output_1, output_2, output_3, data_val, receiver_1);
      counter_1 = counter_1 - 64;
    }
    if (input_2.read_nb(data_val)) {
      send_packet(output_1, output_2, output_3, data_val, receiver_2);
      counter_2 = counter_2 - 64;
    }
    if (input_3.read_nb(data_val)) {
      send_packet(output_1, output_2, output_3, data_val, receiver_3);
      counter_3 = counter_3 - 64;
    }
  }
}