#include "ompcnet.h"

namespace ompcnet {
OMPCNet::OMPCNet(xrt::device device) : device(device) {
  uuid = device.get_xclbin_uuid();

  app2net = new PathHandler(xrt::kernel(device, uuid, "app2net"), PH_TYPE::TX);
  net2app = new PathHandler(xrt::kernel(device, uuid, "net2app"), PH_TYPE::RX);
}

OMPCNet::OMPCNet(xrt::device device, std::string app2net_name,
                 std::string net2app_name)
    : device(device) {
  uuid = device.get_xclbin_uuid();

  app2net =
      new PathHandler(xrt::kernel(device, uuid, app2net_name), PH_TYPE::TX);
  net2app =
      new PathHandler(xrt::kernel(device, uuid, net2app_name), PH_TYPE::RX);
}

OMPCNet::~OMPCNet() {
  delete app2net;
  delete net2app;

  for (auto entry : op_map)
    delete entry.second;
}

void OMPCNet::Send(int32_t src, int32_t dst, int32_t tag, xrt::bo &bo,
                   int32_t len) {
  OPHandler *new_op =
      new OPHandler(src, dst, OPERATION::send, bo.address(), len);
  app2net->AddOperation(new_op);
  std::lock_guard<std::mutex> lk(map_mtx);
  op_map[tag] = new_op;
}

void OMPCNet::Recv(int32_t src, int32_t dst, int32_t tag, xrt::bo &bo,
                   int32_t len) {
  OPHandler *new_op =
      new OPHandler(src, dst, OPERATION::recv, bo.address(), len);
  net2app->AddOperation(new_op);
  std::lock_guard<std::mutex> lk(map_mtx);
  op_map[tag] = new_op;
}

void OMPCNet::StreamTo(int32_t src, int32_t dst, int32_t tag, int32_t len) {
  OPHandler *new_op = new OPHandler(src, dst, OPERATION::stream_to, 0, len);
  app2net->AddOperation(new_op);
  std::lock_guard<std::mutex> lk(map_mtx);
  op_map[tag] = new_op;
}

void OMPCNet::StreamFrom(int32_t src, int32_t dst, int32_t tag, int32_t len) {
  OPHandler *new_op = new OPHandler(src, dst, OPERATION::stream_from, 0, len);
  net2app->AddOperation(new_op);
  std::lock_guard<std::mutex> lk(map_mtx);
  op_map[tag] = new_op;
}

void OMPCNet::StreamToMem(int32_t tag, xrt::bo &bo, int32_t len) {
  OPHandler *new_op =
      new OPHandler(-1, -1, OPERATION::stream2mem, bo.address(), len);
  app2net->AddOperation(new_op);
  std::lock_guard<std::mutex> lk(map_mtx);
  op_map[tag] = new_op;
}

void OMPCNet::MemToStream(int32_t tag, xrt::bo &bo, int32_t len) {
  OPHandler *new_op =
      new OPHandler(-1, -1, OPERATION::mem2stream, bo.address(), len);
  net2app->AddOperation(new_op);
  std::lock_guard<std::mutex> lk(map_mtx);
  op_map[tag] = new_op;
}

bool OMPCNet::isOperationComplete(int32_t tag) {
  std::lock_guard<std::mutex> lk(map_mtx);
  auto op_handle = op_map.find(tag);
  if (op_handle != op_map.end()) {
    if (op_handle->second->getStatus() == STATUS::COMPLETED) {
      delete op_handle->second;
      op_map.erase(op_handle);
      return true;
    }
  } else {
    return true; // Return true if no operation
  }

  return false;
}
}; // namespace ompcnet
