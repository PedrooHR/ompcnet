#include <cstdio>
#include <cstdlib>

#include <chrono>
#include <string>
#include <thread>

#include <xrt/xrt_bo.h>
#include <xrt/xrt_device.h>
#include <xrt/xrt_kernel.h>
#include <xrt/xrt_uuid.h>

#include <vnx/cmac.hpp>
#include <vnx/networklayer.hpp>

#include "ompcnet.h"

using namespace std;
using namespace std::chrono_literals;

/**==========================================================================**/
/**                              GLOBAL CONFIGS                              **/

#define PACKET_INTS   352
#define N_PACKETS     1
#define REPLAYS       1

/**==========================================================================**/
/**                                   VNX                                    **/
typedef struct vnxIP_ {
  std::string ip_address;
  int port;
  bool valid;
} vnxIP;

constexpr int MAX_SOCKETS = 16;
constexpr int VNX_START_PORT = 5500;

void ConfigureVNX(xrt::device device, int device_id, xrt::uuid uuid,
                  int total_devices, std::unique_ptr<vnx::CMAC> &cmac,
                  std::unique_ptr<vnx::Networklayer> &network_layer) {
  std::vector<vnxIP> ips;
  // Instantiate CMAC kernel
  cmac = std::unique_ptr<vnx::CMAC>(
      new vnx::CMAC(xrt::ip(device, uuid, "cmac_0:{cmac_0}")));
  cmac->set_rs_fec(false);

  // Network kernel
  network_layer = std::unique_ptr<vnx::Networklayer>(new vnx::Networklayer(
      xrt::ip(device, uuid, "networklayer:{networklayer_0}")));

  // See if this sleep is needed
  bool link_status = false;
  for (std::size_t i = 0; i < 5; ++i) {
    auto status = cmac->link_status();
    link_status = status["rx_status"];
    if (link_status) {
      break;
    }
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }

  if (link_status)
    printf("[vnx %d] Found CMAC link\n", device_id);

  // Create IP Table
  for (int i = 0; i < MAX_SOCKETS; i++) {
    ips.push_back({
        .ip_address = "0.0.0.0",
        .port = 0,
        .valid = 0,
    });
  }

  std::string base_ip = "172.155.10.";
  for (int i = 0; i < total_devices; i++) {
    ips[i].ip_address = base_ip + std::to_string(i + 1);
    ips[i].port = VNX_START_PORT + i;
    ips[i].valid = true;
  }

  // set this FPGA ip
  network_layer->update_ip_address(ips[device_id].ip_address);
  for (int i = 0; i < MAX_SOCKETS; i++) {
    if (i != device_id)
      network_layer->configure_socket(i, ips[i].ip_address, ips[i].port,
                                      ips[device_id].port, ips[i].valid);
  }
  std::map<int, vnx::socket_t> socket_map =
      network_layer->populate_socket_table();

  for (auto &entry : socket_map) {
    printf("[vnx %d] - [%d] %s:%d - %d\n", device_id, entry.first,
           entry.second.theirIP.c_str(), entry.second.theirPort,
           entry.second.valid);
  }

  printf("[vnx %d] Finished configuring vnx on device\n", device_id);
}

void ConfigureARP(std::unique_ptr<vnx::Networklayer> &network_layer) {
  network_layer->arp_discovery();
}

void ShowARPTable(int device_id, int total_devices,
                  std::unique_ptr<vnx::Networklayer> &network_layer) {
  network_layer->arp_discovery();
  auto table = network_layer->read_arp_table(16);
  for (const auto &[id, value] : table) {
    printf("[vnx %d] ARP table: [%d] = %s %s\n", device_id, id,
           value.first.c_str(), value.second.c_str());
  }
}

/**==========================================================================**/
/**                                 Helpers                                  **/
int *create_buffer(int val, int size) {
  int *buf = (int *)aligned_alloc(4096, size * sizeof(int));
  for (int i = 0; i < size; i++)
    buf[i] = val;
  return buf;
}

/**==========================================================================**/
/**                                  Test 1                                  **/
void Mem2Kernel2Mem(xrt::device dev, xrt::kernel increment, int32_t *input,
                    int32_t *output, int size, int board_idx) {
  // create OMPC Net Object
  ompcnet::OMPCNet ompc(dev);

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

  // Check
  while (!ompc.isOperationComplete(0) || !ompc.isOperationComplete(1))
    std::this_thread::sleep_for(10ms);

  out.sync(XCL_BO_SYNC_BO_FROM_DEVICE);

  int errors = 0;
  for (int i = 0; i < size; i++)
    if (output[i] != (input[i] + 1))
      errors++;

  printf("[Board %d] - Test 1: Number of errors: %d\n", board_idx, errors);
}

/**==========================================================================**/
/**                                  Test 2                                  **/
void MemSend(xrt::device dev, int32_t *input, int32_t size, int32_t board_idx) {
  // create OMPC Net Object
  ompcnet::OMPCNet ompc(dev);

  // Create Input Buffers
  xrt::bo bo = xrt::bo(dev, input, size * 4, xrt::bo::flags::normal, 0);
  bo.sync(XCL_BO_SYNC_BO_TO_DEVICE);

  // Dispatch OMPC Operations
  ompc.Send(0, 1, 0, bo, size * sizeof(int));

  while (!ompc.isOperationComplete(0))
    std::this_thread::sleep_for(10ms);

  printf("[Board %d] - Test 2: Finished operations\n", board_idx);
}

void MemRecv(xrt::device dev, int32_t *output, int32_t size,
             int32_t board_idx) {
  // create OMPC Net Object
  ompcnet::OMPCNet ompc(dev);

  // Create Input Buffers
  xrt::bo bo = xrt::bo(dev, output, size * 4, xrt::bo::flags::normal, 0);

  // Dispatch OMPC Operations
  ompc.Recv(0, 1, 0, bo, size * sizeof(int)); // Receiver Path

  while (!ompc.isOperationComplete(0))
    std::this_thread::sleep_for(10ms);

  bo.sync(XCL_BO_SYNC_BO_FROM_DEVICE);

  int errors = 0;
  for (int i = 0; i < size; i++)
    if (output[i] != 2)
      errors++;
      
  printf("[Board %d] - Test 2: Number of errors: %d\n", board_idx, errors);
}

/**==========================================================================**/
/**                                  Test 3                                  **/
void MemRecv_StreamSend(xrt::device dev, xrt::kernel increment, int32_t *input,
                        int32_t size, int32_t board_idx) {
  // create OMPC Net Object
  ompcnet::OMPCNet ompc(dev);

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

  // Check
  while (!ompc.isOperationComplete(0) || !ompc.isOperationComplete(1))
    std::this_thread::sleep_for(10ms);

  printf("[Board %d] - Test 3: Finished operations\n", board_idx);
}

void StreamRecv_MemSend(xrt::device dev, xrt::kernel increment, int32_t *output,
                        int32_t size, int32_t board_idx) {
  // create OMPC Net Object
  ompcnet::OMPCNet ompc(dev);

  // Create Input Buffers
  xrt::bo bo = xrt::bo(dev, output, size * 4, xrt::bo::flags::normal, 0);

  // Dispatch OMPC Operations
  ompc.StreamFrom(0, 1, 0, size * sizeof(int)); // Receiver Path
  ompc.StreamToMem(1, bo, size * sizeof(int));  // Sender Path

  // Execute kernel increment
  xrt::run r(increment);
  r.set_arg(0, size * sizeof(int));
  r.start();
  r.wait();

  // Check
  while (!ompc.isOperationComplete(0) || !ompc.isOperationComplete(1))
    std::this_thread::sleep_for(10ms);

  bo.sync(XCL_BO_SYNC_BO_FROM_DEVICE);

  int errors = 0;
  for (int i = 0; i < size; i++)
    if (output[i] != 3) 
      errors++;

  printf("[Board %d] - Test 3: Number of errors: %d\n", board_idx, errors);
}

/**==========================================================================**/
/**                                   FPGA                                   **/
void FPGA_1(std::string xclbin_file) {
  // Global Configs
  int len = PACKET_INTS * N_PACKETS;
  int len_bytes = len * sizeof(int);

  // Device start up
  xrt::device dev(0); // First Device
  xrt::uuid uuid = dev.load_xclbin(xclbin_file);  
  std::this_thread::sleep_for(1s);

  // VNx
  std::unique_ptr<vnx::CMAC> cmac;
  std::unique_ptr<vnx::Networklayer> network_layer;
  ConfigureVNX(dev, 0, uuid, 2, cmac, network_layer);
  ConfigureARP(network_layer);
  ShowARPTable(0, 2, network_layer);  
  std::this_thread::sleep_for(3s);

  // Increment kernel
  xrt::kernel increment = xrt::kernel(dev, uuid, "increment:{increment_0}");

  int *input_buffer;
  int *output_buffer;


  // Test 1
  for (int i = 0; i < REPLAYS; i++) {
    input_buffer = create_buffer(1, len);
    output_buffer = create_buffer(0, len);
    Mem2Kernel2Mem(dev, increment, input_buffer, output_buffer, len, 0);
    delete input_buffer;
    delete output_buffer;
  }


  // Test 2
  for (int i = 0; i < REPLAYS; i++) {
    input_buffer = create_buffer(2, len);
    MemSend(dev, input_buffer, len, 0);
    delete input_buffer;
  }

/*
  // Test 3
  for (int i = 0; i < REPLAYS; i++) {
    input_buffer = create_buffer(1, len);
    MemRecv_StreamSend(dev, increment, input_buffer, len, 0);
    delete input_buffer;
  }
*/

  std::this_thread::sleep_for(2s);
  vnx::stats_t stats = cmac->statistics(true);
  for (auto entry : stats.tx) {
    printf("Board 0: %s = %u\n", entry.first.c_str(), entry.second);
  }
  printf("Board 0: Packets out: %d\n", network_layer->get_udp_out_pkts());
  printf("Board 0: APP Packets out: %d\n",
         network_layer->get_udp_app_out_pkts());
  printf("Board 0: Packets in: %d\n", network_layer->get_udp_in_pkts());
  printf("Board 0: APP Packets in: %d\n", network_layer->get_udp_app_in_pkts());
  printf("\n");

}

void FPGA_2(std::string xclbin_file) {
  // Global Configs
  int len = PACKET_INTS * N_PACKETS;
  int len_bytes = len * sizeof(int);

  // Device start up
  xrt::device dev(1); // Second Device
  xrt::uuid uuid = dev.load_xclbin(xclbin_file);
  std::this_thread::sleep_for(1s);

  // VNx
  std::unique_ptr<vnx::CMAC> cmac;
  std::unique_ptr<vnx::Networklayer> network_layer;
  ConfigureVNX(dev, 1, uuid, 2, cmac, network_layer);
  ConfigureARP(network_layer);
  ShowARPTable(1, 2, network_layer);  
  std::this_thread::sleep_for(2s);

  // Increment kernel
  xrt::kernel increment = xrt::kernel(dev, uuid, "increment:{increment_0}");

  // Testing buffers
  int *input_buffer;
  int *output_buffer;


  // Test 1
  for (int i = 0; i < REPLAYS; i++) {
    input_buffer = create_buffer(1, len);
    output_buffer = create_buffer(0, len);
    Mem2Kernel2Mem(dev, increment, input_buffer, output_buffer, len, 1);
    delete input_buffer;
    delete output_buffer;
  }


  // Test 2
  for (int i = 0; i < REPLAYS; i++) {
    output_buffer = create_buffer(0, len);
    MemRecv(dev, output_buffer, len, 1);
    delete output_buffer;
  }

/*
  // Test 3
  for (int i = 0; i < REPLAYS; i++) {
    output_buffer = create_buffer(0, len);
    StreamRecv_MemSend(dev, increment, output_buffer, len, 1);
    delete output_buffer;
  }
*/

  std::this_thread::sleep_for(3s);
  vnx::stats_t stats = cmac->statistics(true);
  for (auto entry : stats.rx) {
    printf("Board 1: %s = %u\n", entry.first.c_str(), entry.second);
  }
  printf("Board 1: Packets out: %d\n", network_layer->get_udp_out_pkts());
  printf("Board 1: APP Packets out: %d\n",
         network_layer->get_udp_app_out_pkts());
  printf("Board 1: Packets in: %d\n", network_layer->get_udp_in_pkts());
  printf("Board 1: APP Packets in: %d\n", network_layer->get_udp_app_in_pkts());

}

/**==========================================================================**/
/**                                   MAIN                                   **/
int main(int argc, char *argv[]) {
  if (argc < 2) {
    fprintf(stderr, "Program should receive one argument containing the xclbin "
                    "file path\n");
    return 1;
  }

  std::thread fpga1(FPGA_1, std::string(argv[1]));
  std::thread fpga2(FPGA_2, std::string(argv[1]));

  fpga1.join();
  fpga2.join();

  return 0;
}
