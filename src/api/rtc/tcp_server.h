//
// Copyright (c) 2025 mdl. All rights reserved.
//

#ifndef API_RTC_TCP_SERVER_H_
#define API_RTC_TCP_SERVER_H_
#include <memory>
#include "base/memory/weak_ptr.h"
#include "net/base/ip_endpoint.h"
#include "net/socket/stream_socket.h"
#include "net/socket/tcp_server_socket.h"
#include "api/utils/queue.h"
#include "api/rtc/http1_processor.h"
#include "api/rtc/channel.h"

namespace worker {
class TCPServerConnection;
class Connection;

class TCPServer final {
 public:
  class Delegate {
   public:
    virtual scoped_refptr<Channel> HandleWebSocketRequest(
        TCPServer* server,
        Connection* connection,
        const HttpRequestInfo& request_info);
  };

  explicit TCPServer(Delegate* delegate, Http1Processor* http_processor);
  ~TCPServer();

  int InitializeAndListen(const net::IPEndPoint& endpoint);
  void Run();

  Http1Processor* http_processor() { return http_processor_; }
  scoped_refptr<base::SequencedTaskRunner> task_runner() {
    return task_runner_;
  }

 private:
  friend TCPServerConnection;
  void RemoveConnection(TCPServerConnection* connection);

  scoped_refptr<Channel> HandleWebSocketRequest(
      Connection* connection,
      const HttpRequestInfo& request_info) {
    return delegate_->HandleWebSocketRequest(this, connection, request_info);
  }

 private:
  void DoAccept();
  void OnAcceptCompleted(int rv);
  void HandleAcceptResult(std::unique_ptr<net::StreamSocket> accepted_socket);

 private:
  Delegate* delegate_;
  Http1Processor* http_processor_;
  int sync_accept_count_ = 0;
  scoped_refptr<base::SequencedTaskRunner> task_runner_;
  base::WeakPtrFactory<TCPServer> weak_factory_{this};

  std::unique_ptr<net::TCPServerSocket> listen_socket_;
  std::unique_ptr<net::StreamSocket> accepted_socket_;
  QUEUE tcp_queue_;
};

}  // namespace worker

#endif