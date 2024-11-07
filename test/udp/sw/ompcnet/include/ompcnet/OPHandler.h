#ifndef OPHANDLER_H
#define OPHANDLER_H

#include <mutex>
#include <unordered_map>

#include "ompcnet/common.h"

namespace ompcnet {
class OPHandler {
private:
  int src, dst, len, op;
  unsigned long address;
  STATUS op_status;
  std::mutex mtx;

public:
  OPHandler(int src, int dst, int op, unsigned long address, int len);

  void setStatus(STATUS sts);
  STATUS getStatus();
  int getSrc();
  int getDst();
  unsigned long getAdd();
  int getOp();
  int getLen();
};
}; // namespace ompcnet

#endif