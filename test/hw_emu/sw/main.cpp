#include <cstdio>
#include <cstdlib>

#include <chrono>
#include <string>
#include <thread>

#include <xrt/xrt_bo.h>
#include <xrt/xrt_device.h>
#include <xrt/xrt_kernel.h>
#include <xrt/xrt_uuid.h>

#include "ompcnet.h"

using namespace std::chrono_literals;

int *create_buffer(int val, int size) {
  int *buf = (int *)aligned_alloc(4096, size * sizeof(int));
  for (int i = 0; i < size; i++)
    buf[i] = val;
  return buf;
}

/**==========================================================================**/
/**                                  Test 1                                  **/
void Mem2Kernel2Mem(xrt::device dev, xrt::kernel increment, int32_t *input,
                    int32_t *output, int size, std::string app2net,
                    std::string net2app) {
  // create OMPC Net Object
  ompcnet::OMPCNet ompc(dev, app2net, net2app);

  // bo creation
  xrt::bo in = xrt::bo(dev, input, size * 4, xrt::bo::flags::normal, 0);
  xrt::bo out = xrt::bo(dev, output, size * 4, xrt::bo::flags::normal, 0);
  in.sync(XCL_BO_SYNC_BO_TO_DEVICE);
  out.sync(XCL_BO_SYNC_BO_TO_DEVICE);

  // dispatch network operation
  ompc.MemToStream(0, in, size * sizeof(int));
  ompc.StreamToMem(1, out, size * sizeof(int));

  // Kernel Execution
  xrt::run r(increment);
  r.set_arg(0, size * sizeof(int));
  r.start();
  r.wait();

  std::this_thread::sleep_for(1s);

  // Check
  if (ompc.isOperationComplete(0))
    printf("Mem2Stream completed on Mem2Kernel2Mem\n");

  if (ompc.isOperationComplete(1))
    printf("Stream2Mem completed on Mem2Kernel2Mem\n");

  in.sync(XCL_BO_SYNC_BO_FROM_DEVICE);
  out.sync(XCL_BO_SYNC_BO_FROM_DEVICE);

  int errors = 0;
  for (int i = 0; i < size; i++)
    if (output[i] != (input[i] + 1))
      errors++;

  printf("Number of errors: %d\n", errors);
}

/**==========================================================================**/
/**                                  Test 2                                  **/
void MemRecv_StreamSend(xrt::device dev, xrt::kernel increment, int32_t *input,
                        int size, std::string app2net, std::string net2app) {
  // create OMPC Net Object
  ompcnet::OMPCNet ompc(dev, app2net, net2app);

  // Create Input Buffers
  xrt::bo bo = xrt::bo(dev, input, size * 4, xrt::bo::flags::normal, 0);
  bo.sync(XCL_BO_SYNC_BO_TO_DEVICE);

  // Dispatch OMPC Operations
  ompc.MemToStream(0, bo, size * sizeof(int)); // Receiver Path
  ompc.StreamTo(0, 1, 1, size * sizeof(int));  // Sender Path

  // Execute kernel increment
  xrt::run r(increment);
  r.set_arg(0, size * sizeof(int));
  r.start();
  r.wait();

  std::this_thread::sleep_for(1s);

  if (ompc.isOperationComplete(0))
    printf("Mem2Stream completed on MemRecv_StreamSend\n");
  if (ompc.isOperationComplete(1))
    printf("StreamTo completed on MemRecv_StreamSend\n");
}

void StreamRecv_StreamSend(xrt::device dev, xrt::kernel increment, int size,
                           std::string app2net, std::string net2app) {
  // create OMPC Net Object
  ompcnet::OMPCNet ompc(dev, app2net, net2app);

  // Dispatch OMPC Operations
  ompc.StreamFrom(0, 1, 0, size * sizeof(int)); // Receiver Path
  ompc.StreamTo(1, 2, 1, size * sizeof(int));   // Sender Path

  // Execute kernel increment
  xrt::run r(increment);
  r.set_arg(0, size * sizeof(int));
  r.start();
  r.wait();

  std::this_thread::sleep_for(1s);

  if (ompc.isOperationComplete(0))
    printf("StreamFrom completed on StreamRecv_StreamSend\n");
  if (ompc.isOperationComplete(1))
    printf("StreamTo completed on StreamRecv_StreamSend\n");
}

void StreamRecv_MemSend(xrt::device dev, xrt::kernel increment, int32_t *input,
                        int32_t *output, int size, std::string app2net,
                        std::string net2app) {
  // create OMPC Net Object
  ompcnet::OMPCNet ompc(dev, app2net, net2app);

  // Create Input Buffers
  xrt::bo bo = xrt::bo(dev, output, size * 4, xrt::bo::flags::normal, 0);

  // Dispatch OMPC Operations
  ompc.StreamFrom(1, 2, 0, size * sizeof(int)); // Receiver Path
  ompc.StreamToMem(1, bo, size * sizeof(int));  // Sender Path

  // Execute kernel increment
  xrt::run r(increment);
  r.set_arg(0, size * sizeof(int));
  r.start();
  r.wait();

  std::this_thread::sleep_for(1s);

  if (ompc.isOperationComplete(0))
    printf("StreamFrom completed on StreamRecv_MemSend\n");
  if (ompc.isOperationComplete(1))
    printf("Stream2Mem completed on StreamRecv_MemSend\n");

  bo.sync(XCL_BO_SYNC_BO_FROM_DEVICE);

  int errors = 0;
  for (int i = 0; i < size; i++)
    if (output[i] != (input[i] + 3))
      errors++;

  printf("Number of errors: %d\n", errors);
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    fprintf(stderr, "Program should receive one argument containing the xclbin "
                    "file path\n");
    return 1;
  }

  // Global Configs
  int len = 16;
  int len_bytes = len * sizeof(int);

  // FPGA global configs
  xrt::device dev(0); // First Device
  xrt::uuid uuid = dev.load_xclbin(argv[1]);

  // FPGA Kernels
  xrt::kernel increment_0 = xrt::kernel(dev, uuid, "increment:{increment_0}");
  xrt::kernel increment_1 = xrt::kernel(dev, uuid, "increment:{increment_1}");
  xrt::kernel increment_2 = xrt::kernel(dev, uuid, "increment:{increment_2}");

  // Test 1 - mem -> kernel 1 -> kernel 2 -> kernel 3 -> mem
  int *test1_input = create_buffer(1, len);
  int *test1_output = create_buffer(0, len);

  // test 1
  Mem2Kernel2Mem(dev, increment_0, test1_input, test1_output, len,
                 "app2net:{app2net_0}", "net2app:{net2app_0}");
  Mem2Kernel2Mem(dev, increment_1, test1_input, test1_output, len,
                 "app2net:{app2net_1}", "net2app:{net2app_1}");
  Mem2Kernel2Mem(dev, increment_2, test1_input, test1_output, len,
                 "app2net:{app2net_2}", "net2app:{net2app_2}");

  // test 2
  std::thread t1(MemRecv_StreamSend, dev, increment_0, test1_input, len,
                 "app2net:{app2net_0}", "net2app:{net2app_0}");
  std::thread t2(StreamRecv_StreamSend, dev, increment_1, len,
                 "app2net:{app2net_1}", "net2app:{net2app_1}");
  std::thread t3(StreamRecv_MemSend, dev, increment_2, test1_input,
                 test1_output, len, "app2net:{app2net_2}",
                 "net2app:{net2app_2}");

  t1.join();
  t2.join();
  t3.join();

  delete test1_input;
  delete test1_output;

  return 0;
}
