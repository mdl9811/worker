//
// Copyright (c) 2025 mdl. All rights reserved.
//

#ifndef API_RTC_IO_BUFFER_H_
#define API_RTC_IO_BUFFER_H_

#include <atomic>
#include <string>
#include <string_view>

#include "base/memory/ref_counted.h"

namespace worker {

class IOBuffer {
 public:
  enum Type : uint8_t {
    DATA = 0,
    APP = 1,
    WEBSOCKET_FRAME = 2,
  };

  static scoped_refptr<IOBuffer> New(const void* p, size_t n, Type type = DATA);
  static scoped_refptr<IOBuffer> New(std::string_view data, Type type = DATA) {
    return New(data.data(), data.size(), type);
  }
  static scoped_refptr<IOBuffer> New(int size, Type type = DATA);
  static scoped_refptr<IOBuffer> NewWebsocketFrame(std::string_view data,
                                                   bool text = true);

  void AddRef() { ref_count_.fetch_add(1, std::memory_order_relaxed); }
  void Release() {
    if (ref_count_.fetch_sub(1, std::memory_order_acq_rel) == 1)
      delete[] reinterpret_cast<const char*>(this);
  }

  template <typename T = char>
  T* data() {
    return reinterpret_cast<T*>(this + 1);
  }
  int size() const { return size_; }
  Type type() const { return type_; }
  void type(uint8_t type) { type_ = static_cast<Type>(type); }

 private:
  IOBuffer() = delete;
  IOBuffer(const IOBuffer&) = delete;
  IOBuffer& operator=(const IOBuffer&) = delete;

  std::atomic_int ref_count_;
  int32_t size_;
  Type type_;
};
}  // namespace welink::rtc

#endif  // WELINK_RTC_IO_BUFFER_H_
