#include "increment.h"

int main() {
  int sanity_checker = 0;

  hls::stream<data_type> input;
  hls::stream<data_type> output;

  int len = 128;
  int len_bytes = 128 * sizeof(int);
  int input_buffer[128];
  int output_buffer[128];

  for (int i = 0; i < len; i++) {
    input_buffer[i] = i;
    output_buffer[i] = 0;
  }

  data_type value;
  for (int i = 0; i < len_bytes / 64; i++) {
    value.data = 0;
    for (int j = 0; j < 16; j++) {
      value.data.range((j * 32) + 31, j * 32) = input_buffer[(i * 16) + j];
    }
    input.write(value);
  }

  increment(len_bytes, input, output);

  for (int i = 0; i < len_bytes / 64; i++) {
    value = output.read();
    for (int j = 0; j < 16; j++) {
      output_buffer[(i * 16) + j] = value.data.range((j * 32) + 31, j * 32);
    }
  }

  for (int i = 0; i < len; i++) {
    if (output_buffer[i] != (input_buffer[i] + 1)) {
      sanity_checker++;
    }
  }

  return sanity_checker;
}