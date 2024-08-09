#include "network.h"

int main() {
  int sanity_checker = 0;

  hls::stream<network_word> input_1;
  hls::stream<network_word> input_2;
  hls::stream<network_word> input_3;
  hls::stream<network_word> output_1;
  hls::stream<network_word> output_2;
  hls::stream<network_word> output_3;

  int len = 128;
  int len_bytes = 128 * sizeof(int);
  int input_buffer_1[128];
  int output_buffer_1[128];
  int input_buffer_2[128];
  int output_buffer_2[128];
  int input_buffer_3[128];
  int output_buffer_3[128];

  for (int i = 0; i < len; i++) {
    input_buffer_1[i] = i * 2;
    input_buffer_2[i] = i * 3;
    input_buffer_3[i] = i * 4;

    output_buffer_1[i] = 0;
    output_buffer_2[i] = 0;
    output_buffer_3[i] = 0;
  }

  network_word value_1, value_2, value_3;
  for (int i = 0; i < len_bytes / 64; i++) {
    value_1.data = 0;
    value_2.data = 0;
    value_3.data = 0;
    for (int j = 0; j < 16; j++) {
      value_1.data.range((j * 32) + 31, j * 32) = input_buffer_1[(i * 16) + j];
      value_2.data.range((j * 32) + 31, j * 32) = input_buffer_2[(i * 16) + j];
      value_3.data.range((j * 32) + 31, j * 32) = input_buffer_3[(i * 16) + j];
    }
    input_1.write(value_1);
    input_2.write(value_2);
    input_3.write(value_3);
  }

  network(2, 3, 1, len_bytes, input_1, output_1, input_2, output_2, input_3,
          output_3);

  for (int i = 0; i < len_bytes / 64; i++) {
    value_1 = output_1.read();
    value_2 = output_2.read();
    value_3 = output_3.read();
    for (int j = 0; j < 16; j++) {
      output_buffer_1[(i * 16) + j] = value_1.data.range((j * 32) + 31, j * 32);
      output_buffer_2[(i * 16) + j] = value_2.data.range((j * 32) + 31, j * 32);
      output_buffer_3[(i * 16) + j] = value_3.data.range((j * 32) + 31, j * 32);
    }
  }

  for (int i = 0; i < len; i++) {
    if (output_buffer_1[i] != input_buffer_3[i]) {
      sanity_checker++;
    }
    if (output_buffer_2[i] != input_buffer_1[i]) {
      sanity_checker++;
    }
    if (output_buffer_3[i] != input_buffer_2[i]) {
      sanity_checker++;
    }
  }

  return sanity_checker;
}