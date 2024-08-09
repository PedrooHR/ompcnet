#ifndef PATHHANDLER_H
#define PATHHANDLER_H

#include <condition_variable>
#include <thread>
#include <unordered_map>

#include <xrt/xrt_kernel.h>

#include "ompcnet/OPHandler.h"
#include "ompcnet/common.h"

namespace ompcnet {
class PathHandler {
private:
  xrt::kernel path_kernel;
  SafeQueue<OPHandler *> operation_queue;
  std::thread execution_thread;
  std::condition_variable execution_cv;
  std::mutex execution_mtx;
  std::atomic<bool> is_finished;

  void ProcessQueue();
  void Executor(OPHandler *op);

public:
  PathHandler(xrt::kernel path_kernel);
  ~PathHandler();

  void AddOperation(OPHandler *op);
};
}; // namespace ompcnet

#endif