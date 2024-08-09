#include "ompcnet.h"

namespace ompcnet {
OMPCNet::OMPCNet(xrt::device device) : device(device) {
  uuid = device.get_xclbin_uuid();

  app2net = new PathHandler(xrt::kernel(device, uuid, "app2net"));
  net2app = new PathHandler(xrt::kernel(device, uuid, "net2app"));
}

OMPCNet::~OMPCNet() {
  delete app2net;
  delete net2app;
}

void OMPCNet::Send(int32_t src, int32_t dst, int32_t tag, xrt::bo &bo,
                   int32_t len) {
  OPHandler *new_op =
      new OPHandler(src, dst, OPERATION::send, bo.address(), len);
  op_map[tag] = new_op;
  app2net->AddOperation(new_op);
}

void OMPCNet::Recv(int32_t src, int32_t dst, int32_t tag, xrt::bo &bo,
                   int32_t len) {
  OPHandler *new_op =
      new OPHandler(src, dst, OPERATION::recv, bo.address(), len);
  op_map[tag] = new_op;
  net2app->AddOperation(new_op);
}

void OMPCNet::StreamTo(int32_t src, int32_t dst, int32_t tag, int32_t len) {
  OPHandler *new_op = new OPHandler(src, dst, OPERATION::stream_to, 0, len);
  op_map[tag] = new_op;
  app2net->AddOperation(new_op);
}

void OMPCNet::StreamFrom(int32_t src, int32_t dst, int32_t tag, int32_t len) {
  OPHandler *new_op = new OPHandler(src, dst, OPERATION::stream_from, 0, len);
  op_map[tag] = new_op;
  net2app->AddOperation(new_op);
}

void OMPCNet::StreamToMem(int32_t tag, xrt::bo &bo, int32_t len) {
  OPHandler *new_op =
      new OPHandler(-1, -1, OPERATION::stream2mem, bo.address(), len);
  op_map[tag] = new_op;
  app2net->AddOperation(new_op);
}

void OMPCNet::MemToStream(int32_t tag, xrt::bo &bo, int32_t len) {
  OPHandler *new_op =
      new OPHandler(-1, -1, OPERATION::mem2stream, bo.address(), len);
  op_map[tag] = new_op;
  net2app->AddOperation(new_op);
}

bool OMPCNet::isOperationComplete(int32_t tag) {
  auto op_handle = op_map.find(tag);
  if (op_handle != op_map.end()) {
    if (op_handle->second->getStatus() == STATUS::COMPLETED) {
      delete op_handle->second;
      op_map.erase(op_handle);
      return true;
    }
  }

  return false;
}
}; // namespace ompcnet
