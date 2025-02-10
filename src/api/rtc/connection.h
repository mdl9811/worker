//
// Copyright (c) 2025 mdl. All rights reserved.
//

#ifndef API_RTC_CONNECTION_H_
#define API_RTC_CONNECTION_H_

#include "api/rtc/io_buffer.h"
#include "api/utils/queue.h"
#include "base/sequence_checker.h"

namespace worker {
struct Link {
  QUEUE connection_queue;
};

class Connection : public Link {
 public:
  Connection();
  virtual ~Connection();

  virtual void Send(scoped_refptr<IOBuffer> buffer) = 0;
  virtual void Close() = 0;

 private:
  SEQUENCE_CHECKER(sequence_checker_);
};

}  // namespace worker

#endif