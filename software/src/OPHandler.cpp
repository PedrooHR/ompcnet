#include "ompcnet/OPHandler.h"

namespace ompcnet {

OPHandler::OPHandler(int src = -1, int dst = -1, int op = 0,
                     unsigned long address = 0, int len = 0)
    : src(src), dst(dst), op(op), address(address), len(len) {}

void OPHandler::setStatus(STATUS sts) {
  std::lock_guard<std::mutex> lk(mtx);
  op_status = sts;
}

STATUS OPHandler::getStatus() {
  std::lock_guard<std::mutex> lk(mtx);
  return op_status;
}

int OPHandler::getSrc() { return src; }

int OPHandler::getDst() { return dst; }

int OPHandler::getOp() { return op; }

unsigned long OPHandler::getAdd() { return address; }

int OPHandler::getLen() { return len; }

}; // namespace ompcnet