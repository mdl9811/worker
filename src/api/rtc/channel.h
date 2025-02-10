//
// Copyright (c) 2025 mdl. All rights reserved.
//

#ifndef API_RTC_CHANNEL_H_
#define API_RTC_CHANNEL_H_

#include <atomic>
#include <cstddef>
#include <cstdint>

namespace worker {
class Channel;

class Channel {
 public:
  Channel();
  virtual ~Channel();

  void AddRef() { ref_count_.fetch_add(1, std::memory_order_relaxed); }
  void Release() {
    if (ref_count_.fetch_sub(1, std::memory_order_acq_rel) == 1)
      Dispose();
  }
  static void Destruct(const Channel* processor);
  virtual void OnMessage(const uint8_t* data, int size) = 0;
  virtual void OnClose() = 0;
  virtual void Dispose();

 protected:
  std::atomic_int ref_count_;
};

}  // namespace worker

#endif