#ifndef PATHHANDLER_H
#define PATHHANDLER_H

#include <condition_variable>
#include <thread>
#include <unordered_map>

#include <xrt/xrt_kernel.h>

#include "ompcnet/OPHandler.h"
#include "ompcnet/common.h"

namespace ompcnet {

enum class PH_TYPE {
  TX = 0,
  RX = 1,
};

constexpr int32_t PACKET_SIZE = 1408; // in bytes
constexpr int32_t MAX_PACKETS = 10;
constexpr uint64_t MAX_PACKET_BYTES = PACKET_SIZE * MAX_PACKETS;

class PathHandler {
private:
  PH_TYPE type;
  xrt::kernel path_kernel;
  SafeQueue<OPHandler *> operation_queue;
  std::thread execution_thread;
  std::condition_variable execution_cv;
  std::mutex execution_mtx;
  std::atomic<bool> is_finished;

  void ProcessQueue();
  void RX_Executor(OPHandler *op);
  void TX_Executor(OPHandler *op);

public:
  PathHandler(xrt::kernel path_kernel, PH_TYPE type);
  ~PathHandler();

  void AddOperation(OPHandler *op);
};
}; // namespace ompcnet

#endif