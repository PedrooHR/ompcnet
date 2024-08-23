#include "ompcnet/PathHandler.h"

namespace ompcnet {
PathHandler::PathHandler(xrt::kernel path_kernel, PH_TYPE type)
    : path_kernel(path_kernel), type(type) {
  is_finished = false;

  execution_thread = std::thread(&PathHandler::ProcessQueue, this);
}

PathHandler::~PathHandler() {
  is_finished = true;
  execution_cv.notify_all();
  execution_thread.join();
}

void PathHandler::AddOperation(OPHandler *op) {
  operation_queue.Push(op);
  op->setStatus(STATUS::QUEUED);
  execution_cv.notify_all();
}

void PathHandler::ProcessQueue() {
  do {
    // wait for a notification if the queue is empty
    if (operation_queue.IsEmpty()) {
      std::unique_lock<std::mutex> lk(execution_mtx);
      execution_cv.wait(lk);

      if (is_finished)
        break;
    }

    OPHandler *op = operation_queue.Pop();

    op->setStatus(STATUS::EXECUTING);
    if (type == PH_TYPE::RX)
      RX_Executor(op);
    else
      RX_Executor(op);
    op->setStatus(STATUS::COMPLETED);
  } while (!is_finished);
}

void PathHandler::RX_Executor(OPHandler *op) {
  xrt::run run_op = xrt::run(path_kernel);

  run_op.set_arg(ARGPOS::src, op->getSrc());
  run_op.set_arg(ARGPOS::dst, op->getDst());
  run_op.set_arg(ARGPOS::address, op->getAdd());
  run_op.set_arg(ARGPOS::operation, op->getOp());
  run_op.set_arg(ARGPOS::len, op->getLen());

  run_op.start();
  run_op.wait();
}

void PathHandler::TX_Executor(OPHandler *op) {
  xrt::run run_op = xrt::run(path_kernel);

  int remaining_len = op->getLen();
  unsigned long current_address = op->getAdd();

  int i = 0;
  while (remaining_len > 0) {
    int len =
        (remaining_len > MAX_PACKET_BYTES) ? MAX_PACKET_BYTES : remaining_len;

    run_op.set_arg(ARGPOS::src, op->getSrc());
    run_op.set_arg(ARGPOS::dst, op->getDst());
    run_op.set_arg(ARGPOS::address, current_address);
    run_op.set_arg(ARGPOS::operation, op->getOp());
    run_op.set_arg(ARGPOS::len, len);

    run_op.start();
    run_op.wait();

    remaining_len -= len;
    current_address += MAX_PACKET_BYTES;
  }
}

} // namespace ompcnet