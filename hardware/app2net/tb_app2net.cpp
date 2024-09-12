#include "app2net.h"

bool cmd_checker(cmd_word cmd, ap_uint<64> add, ap_uint<29> len) {
  bool is_correct =
      cmd.data.range(63, 0) == add && cmd.data.range(92, 64) == len;
  return is_correct;
}

void pre_send(int send_value, ap_uint<29> len,
              hls::stream<handshake_word> &s_arbiter_hs,
              hls::stream<sts_word> &mm2s_sts, hls::stream<data_word> &dm_out) {
  ap_uint<29> len_bytes = len * sizeof(int);

  // confirm handshake - bit 63 should be 1
  handshake_word hs_data;
  hs_data.data.bit(63) = 1;
  s_arbiter_hs.write(hs_data);

  // send data - stream is 512 bits - len is in bytes - mem buffers have 4 bytes
  data_word dm_data;
  for (int i = 0; i < len_bytes / 64; i++) { // total of 8 packets
    dm_data.data = 0;
    for (int j = 0; j < 16; j++) { // reads 16 ints into datamover stream
      dm_data.data.range((j * 32) + 31, j * 32) = send_value;
    }
    dm_out.write(dm_data);
  }

  // send datamover status
  sts_word status;
  status.data = 1;
  mm2s_sts.write(status);
}

int post_send(int send_value, int *output_buffer, ap_uint<29> len,
              hls::stream<handshake_word> &m_arbiter_hs,
              hls::stream<cmd_word> &mm2s_cmd,
              hls::stream<network_word> &network) {
  int sanity_checker = 0;
  ap_uint<29> len_bytes = len * sizeof(int);

  // Check handshake
  ap_uint<16> src = -1;
  ap_uint<16> dst = 1;
  ap_uint<3> op = 0b001;
  handshake_word hs_data;
  hs_data = m_arbiter_hs.read();
  if (src != hs_data.data.range(15, 0) || dst != hs_data.data.range(31, 16) ||
      len_bytes != hs_data.data.range(60, 32) ||
      op != hs_data.data.range(63, 61))
    sanity_checker++;

  // Checks the datamover command the kernel should send
  command_word command = mm2s_cmd.read();
  if (!cmd_check(command, 0x1024, len_bytes))
    sanity_checker++;

  // Receive data - follows the same idea of sending data, as network is 512
  // bits also
  network_word net_data;
  for (int i = 0; i < len_bytes / 64; i++) { // total of 8 packets
    net_data = network.read();
    for (int j = 0; j < 16; j++) { // reads 16 ints into the application stream
      output_buffer[(i * 16) + j] = net_data.data.range((j * 32) + 31, j * 32);
    }
    if (i == ((len_bytes / 64) - 1)) {
      if (net_data.last != 1)
        sanity_checker++;
    } else {
      if (net_data.last != 0)
        sanity_checker++;
    }
  }

  for (int i = 0; i < len; i++)
    if (output_buffer[i] != send_value)
      sanity_checker++;

  return sanity_checker;
}

void pre_stream_to(int stream_value, ap_uint<29> len,
                   hls::stream<handshake_word> &s_arbiter_hs,
                   hls::stream<application_word> &application) {
  ap_uint<29> len_bytes = len * sizeof(int);

  // confirm handshake - bit 63 should be 1
  handshake_word hs_data;
  hs_data.data.bit(63) = 1;
  s_arbiter_hs.write(hs_data);

  // send data - stream is 512 bits - len is in bytes - mem buffers have 4 bytes
  application_word app_data;
  for (int i = 0; i < len_bytes / 64; i++) { // total of 8 packets
    app_data.data = 0;
    for (int j = 0; j < 16; j++) { // reads 16 ints into datamover stream
      app_data.data.range((j * 32) + 31, j * 32) = stream_value;
    }
    application.write(app_data);
  }
}

int post_stream_to(int stream_value, int *output_buffer, ap_uint<29> len,
                   hls::stream<handshake_word> &m_arbiter_hs,
                   hls::stream<network_word> &network) {
  int sanity_checker = 0;
  ap_uint<29> len_bytes = len * sizeof(int);

  // Check handshake
  ap_uint<16> src = -1;
  ap_uint<16> dst = 1;
  ap_uint<3> op = 0b011;
  handshake_word hs_data;
  hs_data = m_arbiter_hs.read();
  if (src != hs_data.data.range(15, 0) || dst != hs_data.data.range(31, 16) ||
      len_bytes != hs_data.data.range(60, 32) ||
      op != hs_data.data.range(63, 61))
    sanity_checker++;

  // Receive data - follows the same idea of sending data, as network is 512
  // bits also
  network_word net_data;
  for (int i = 0; i < len_bytes / 64; i++) { // total of 8 packets
    net_data = network.read();
    for (int j = 0; j < 16; j++) { // reads 16 ints into the application stream
      output_buffer[(i * 16) + j] = net_data.data.range((j * 32) + 31, j * 32);
    }
    if (i == ((len_bytes / 64) - 1)) {
      if (net_data.last != 1)
        sanity_checker++;
    } else {
      if (net_data.last != 0)
        sanity_checker++;
    }
  }

  for (int i = 0; i < len; i++)
    if (output_buffer[i] != stream_value)
      sanity_checker++;

  return sanity_checker;
}

void pre_stream2mem(int mem_value, ap_uint<29> len,
                    hls::stream<sts_word> &s2mm_sts,
                    hls::stream<application_word> &application) {
  ap_uint<29> len_bytes = len * sizeof(int);

  // send data - stream is 512 bits - len is in bytes - mem buffers have 4 bytes
  application_word app_data;
  for (int i = 0; i < len_bytes / 64; i++) { // total of 8 packets
    app_data.data = 0;
    for (int j = 0; j < 16; j++) { // reads 16 ints into datamover stream
      app_data.data.range((j * 32) + 31, j * 32) = mem_value;
    }
    application.write(app_data);
  }

  // send datamover status
  sts_word status;
  status.data = 1;
  s2mm_sts.write(status);
}

int post_stream2mem(int mem_value, int *output_buffer, ap_uint<29> len,
                    hls::stream<cmd_word> &s2mm_cmd,
                    hls::stream<data_word> &dm_in) {
  int sanity_checker = 0;
  ap_uint<29> len_bytes = len * sizeof(int);
  // Checks the datamover command the kernel should send
  command_word command = s2mm_cmd.read();
  if (!cmd_check(command, 0x2048, len_bytes))
    sanity_checker++;

  // Receive data - follows the same idea of sending data, as network is 512
  // bits also
  data_word dm_data;
  for (int i = 0; i < len_bytes / 64; i++) { // total of 8 packets
    dm_data = dm_in.read();
    for (int j = 0; j < 16; j++) { // reads 16 ints into the application stream
      output_buffer[(i * 16) + j] = dm_data.data.range((j * 32) + 31, j * 32);
    }
  }

  for (int i = 0; i < len; i++)
    if (output_buffer[i] != mem_value)
      sanity_checker++;

  return sanity_checker;
}

int main() {
  short sanity_checker = 0;

  // streams
  hls::stream<application_word> application("application");
  hls::stream<network_word> network("network");
  hls::stream<handshake_word> m_arbiter_hs("m_arbiter_hs");
  hls::stream<handshake_word> s_arbiter_hs("s_arbiter_hs");
  hls::stream<cmd_word> mm2s_cmd("mm2s_cmd");
  hls::stream<sts_word> mm2s_sts("mm2s_sts");
  hls::stream<cmd_word> s2mm_cmd("s2mm_cmd");
  hls::stream<sts_word> s2mm_sts("s2mm_sts");
  hls::stream<data_word> dm_in("dm_in");
  hls::stream<data_word> dm_out("dm_out");
  cmd_word cmd;
  int len = 128; // in bytes
  ap_uint<29> len_bytes = len * sizeof(len);
  int send_buffer[128];
  int stream_to_buffer[128];
  int stream2mem_buffer[128];

  for (int i = 0; i < 128; i++) {
    send_buffer[i] = 0;
    stream_to_buffer[i] = 0;
    stream2mem_buffer[i] = 0;
  }

  // Test Send
  pre_send(1, len, s_arbiter_hs, mm2s_sts, dm_out);
  app2net(-1, 1, OPERATION::send, 0x1024, len_bytes, application, network,
          m_arbiter_hs, s_arbiter_hs, mm2s_cmd, mm2s_sts, s2mm_cmd, s2mm_sts,
          dm_in, dm_out);
  sanity_checker +=
      post_send(1, send_buffer, len, m_arbiter_hs, mm2s_cmd, network);

  // Test Stream 2 Mem
  pre_stream2mem(3, len, s2mm_sts, application);
  app2net(-1, -1, OPERATION::stream2mem, 0x2048, len_bytes, application,
          network, m_arbiter_hs, s_arbiter_hs, mm2s_cmd, mm2s_sts, s2mm_cmd,
          s2mm_sts, dm_in, dm_out);
  sanity_checker += post_stream2mem(3, stream2mem_buffer, len, s2mm_cmd, dm_in);

  // Test Stream To
  pre_stream_to(2, len, s_arbiter_hs, application);
  app2net(-1, 1, OPERATION::stream_to, 0, len_bytes, application, network,
          m_arbiter_hs, s_arbiter_hs, mm2s_cmd, mm2s_sts, s2mm_cmd, s2mm_sts,
          dm_in, dm_out);
  sanity_checker +=
      post_stream_to(2, stream_to_buffer, len, m_arbiter_hs, network);


  return sanity_checker;
}