#include "controller.h"

constexpr int WRONG_SRC = 99;
constexpr int RIGHT_SRC = 1;
constexpr int WRONG_DST = 99;
constexpr int RIGHT_DST = 1;
constexpr unsigned long WRONG_ADD = 0x1024;
constexpr unsigned long RIGHT_ADD = 0x4096;

bool checker(control_word inst, ap_uint<16> src, ap_uint<16> dst, OPERATION op,
             ap_uint<64> add, ap_uint<29> len) {
  bool is_correct = (int)inst.data.range( 63,   0) == add &&
                    (int)inst.data.range( 79,  64) == src &&
                    (int)inst.data.range( 95,  80) == dst &&
                    (unsigned long)inst.data.range(124,  96) == len &&
                    (int)inst.data.range(127, 125) == op;
  return is_correct;
}

int main() {
  short sanity_checker = 0;
  hls::stream<control_word> app2net;
  hls::stream<control_word> net2app;
  control_word inst;
  int len = 1024;

  // test all operations
  controller(WRONG_SRC, RIGHT_DST, OPERATION::send, RIGHT_ADD, len, app2net,
             net2app); // send
  controller(RIGHT_SRC, WRONG_DST, OPERATION::recv, RIGHT_ADD, len, app2net,
             net2app); // recv
  controller(WRONG_SRC, RIGHT_DST, OPERATION::stream_to, WRONG_ADD, len,
             app2net, net2app); // stream_to
  controller(RIGHT_SRC, WRONG_DST, OPERATION::stream_from, WRONG_ADD, len,
             app2net, net2app); // stream_from
  controller(WRONG_SRC, WRONG_DST, OPERATION::stream2mem, RIGHT_ADD, len,
             app2net, net2app); // send
  controller(WRONG_SRC, WRONG_DST, OPERATION::mem2stream, RIGHT_ADD, len,
             app2net, net2app); // send

  // Check app2net send
  inst = app2net.read();
  if (!checker(inst, -1, RIGHT_DST, OPERATION::send, RIGHT_ADD, len))
    sanity_checker |= 0b000001;

  // Check net2app recv
  inst = net2app.read();
  if (!checker(inst, RIGHT_SRC, -1, OPERATION::recv, RIGHT_ADD, len))
    sanity_checker |= 0b000010;

  // Check app2net stream_to
  inst = app2net.read();
  if (!checker(inst, -1, RIGHT_DST, OPERATION::stream_to, -1, len))
    sanity_checker |= 0b000100;

  // Check net2app stream_from
  inst = net2app.read();
  if (!checker(inst, RIGHT_SRC, -1, OPERATION::stream_from, -1, len))
    sanity_checker |= 0b001000;

  // Check app2net stream2mem
  inst = app2net.read();
  if (!checker(inst, -1, -1, OPERATION::stream2mem, RIGHT_ADD, len))
    sanity_checker |= 0b010000;

  // Check net2app mem2stream
  inst = net2app.read();
  if (!checker(inst, -1, -1, OPERATION::mem2stream, RIGHT_ADD, len))
    sanity_checker |= 0b100000;

  return sanity_checker;
}