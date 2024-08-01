#include <hls_stream.h>

#include "common.h"

void loopback(hls::stream<application_word> &in, 
              hls::stream<application_word> &out);
