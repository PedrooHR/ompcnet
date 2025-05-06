#ifndef OMPCNET_COMMON_H
#define OMPCNET_COMMON_H

#include <mutex>
#include <queue>

namespace ompcnet {
enum OPERATION {
  send        = 0b001,
  recv        = 0b010,
  stream_to   = 0b011,
  stream_from = 0b100,
  stream2mem  = 0b101,
  mem2stream  = 0b110,
};

enum ARGPOS {
  src = 0,
  dst = 1,
  operation = 2,
  address = 3,
  len = 4,
};

enum STATUS {
  QUEUED = 1,
  EXECUTING = 2,
  COMPLETED = 3,
};

template <typename T> class SafeQueue {
private:
  std::queue<T> queue;
  std::mutex mtx;

public:
  SafeQueue() {}

  void Push(T object) {
    std::lock_guard<std::mutex> lk(mtx);
    queue.push(object);
  }

  bool Pop(T *object) {
    std::lock_guard<std::mutex> lk(mtx);
    if (!queue.empty()) {
      *object = queue.front();
      queue.pop();
      return true;
    }
    return false;
  }
};
}; // namespace ompcnet

#endif