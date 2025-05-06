#include "memcpy.h"

void memcpy(char *src, char *dst, int size) {
#pragma HLS interface m_axi port = src
#pragma HLS interface m_axi port = dst
#pragma HLS interface s_axilite port = size
#pragma HLS interface s_axilite port = return

	ap_uint<512> *src_wide = reinterpret_cast<ap_uint<512> *>(src);
	ap_uint<512> *dst_wide = reinterpret_cast<ap_uint<512> *>(dst);

	int wide_size = size / 64;

	for (int i = 0; i < wide_size; i++)
		dst_wide[i] = src_wide[i];
}
