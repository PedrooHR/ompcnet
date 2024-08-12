#include "datamover.h"

int main() {
  int sanity_check = 0;

  /* MM2S Part */
  hls::stream<cmd_word> mm2s_cmd("mm2s_cmd");    // mm2s command stream
  hls::stream<sts_word> mm2s_sts("mm2s_sts");    // mm2s status stream
  hls::stream<data_word> mm2s_axis("mm2s_axis"); // AXI4 stream for data
  int mm2s_axi[128];                             // AXI4 for reading mem
  /* S2MM Part */
  hls::stream<cmd_word> s2mm_cmd("s2mm_cmd");    // s2mm command stream
  hls::stream<sts_word> s2mm_sts("s2mm_sts");    // s2mm status stream
  hls::stream<data_word> s2mm_axis("s2mm_axis"); // AXI4 stream for data
  int s2mm_axi[128];                             // AXI4 for writting mem

  sts_word status;
  cmd_word command;
  data_word value;

  for (int i = 0; i < 128; i++) {
    mm2s_axi[i] = 64 + i;
    s2mm_axi[i] = 0;
  }

  mem_word *wide_mm2s = reinterpret_cast<mem_word *>(mm2s_axi);
  mem_word *wide_s2mm = reinterpret_cast<mem_word *>(s2mm_axi);

  command.data.range(63, 0) = 0;
  command.data.range(92, 64) = 128 * 4;
  s2mm_cmd.write(command);
  mm2s_cmd.write(command);

  for (int i = 0; i < 16; i++) {
    value.data.range((i * 32) + 31, i * 32) = 32 + i;
  }

  for (int i = 0; i < 8; i++) {
    s2mm_axis.write(value);
  }

  datamover(mm2s_cmd, mm2s_sts, mm2s_axis, wide_mm2s, s2mm_cmd, s2mm_sts,
            s2mm_axis, wide_s2mm);

  // stream from kernel
  int data_from_stream[128];
  for (int i = 0; i < 8; i++) {
    value = mm2s_axis.read();
    for (int j = 0; j < 16; j++) {
      data_from_stream[i * 16 + j] =
          (int)value.data.range((j * 32) + 31, j * 32);
    }
  }

  // check
  for (int i = 0; i < 128; i++)
    if (s2mm_axi[i] != (64 + i))
      sanity_check++;

  for (int i = 0; i < 8; i++)
    for (int j = 0; j < 16; j++)
      if (data_from_stream[i * 16 + j] != (32 + j))
        sanity_check++;

  status.data = 0;
  status = s2mm_sts.read();
  if ((int)status.data != 1)
    sanity_check++;

  status.data = 0;
  status = mm2s_sts.read();
  if ((int)status.data != 1)
    sanity_check++;

  return sanity_check;
}