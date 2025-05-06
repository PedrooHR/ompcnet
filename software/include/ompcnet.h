#ifndef OMPCNET_H
#define OMPCNET_H

#include <queue>
#include <stdint.h>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

#include <xrt/xrt_bo.h>
#include <xrt/xrt_device.h>
#include <xrt/xrt_kernel.h>
#include <xrt/xrt_uuid.h>

#include "ompcnet/OPHandler.h"
#include "ompcnet/PathHandler.h"
#include "ompcnet/common.h"

namespace ompcnet {

class OMPCNet {
private:
  xrt::device device;
  xrt::uuid uuid;
  PathHandler *app2net;
  PathHandler *net2app;

  std::mutex map_mtx;
  std::unordered_map<int32_t, OPHandler *> op_map;

public:
  OMPCNet(xrt::device device);
  OMPCNet(xrt::device device, std::string app2net_name,
          std::string net2app_name);
  ~OMPCNet();

  void Send(int32_t src, int32_t recv, int32_t tag, xrt::bo &bo, int32_t len);
  void Recv(int32_t src, int32_t recv, int32_t tag, xrt::bo &bo, int32_t len);
  void StreamTo(int32_t src, int32_t recv, int32_t tag, int32_t len);
  void StreamFrom(int32_t src, int32_t recv, int32_t tag, int32_t len);
  void StreamToMem(int32_t tag, xrt::bo &bo, int32_t len);
  void MemToStream(int32_t tag, xrt::bo &bo, int32_t len);
  bool isOperationComplete(int32_t tag);
};
}; // namespace ompcnet

#endif