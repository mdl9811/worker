#include "api/rtc/connection.h"
namespace worker {
Connection::Connection() {
  QUEUE_INIT(&connection_queue);
}

Connection::~Connection() {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  QUEUE_REMOVE(&connection_queue);
}

}  // namespace worker