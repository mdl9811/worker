//
// Copyright (c) 2025 mdl. All rights reserved.
//

#ifndef API_RTC_TCP_SERVER_CONNECTION_H_
#define API_RTC_TCP_SERVER_CONNECTION_H_
#include <memory>
#include "api/rtc/connection.h"
#include "base/memory/scoped_refptr.h"
#include "base/timer/timer.h"
#include "api/rtc/http1_processor.h"

namespace net {
class StreamSocket;
class GrowableIOBuffer;
class DrainableIOBuffer;
}  // namespace net

namespace worker {
class TCPServer;
class IOBuffer;
class Channel;

class TCPServerConnection final : public Connection {
 public:
  explicit TCPServerConnection(TCPServer* server,
                               std::unique_ptr<net::StreamSocket> socket);
  ~TCPServerConnection();

 private:
  void DidConsume(int bytes);
  void HandleError(int rv);
  void OnHandshakeTimeout();
  void OnLiveCheckTimer();
  void StartIdentify();
  void OnHandshakeDone(int rv);

  void StartReading();
  void StartHttpReading();
  void StartWebSocketReading();

  void HandleReadResult(int result);
  void HandleHttpReadResult(int result);
  void HandleWebSocketReadResult(int result);

  void DoWrite();
  void HandleWriteResult(int result);
  void DoWebSocketWrite();
  void HandleWebSocketWriteResult(int result);

  void OnWebsocketPing();
  void DeliverConnected();
  void HandleHttpResponse(const HttpResponseInfo& response_info);

 private:
  void Send(scoped_refptr<IOBuffer> buffer) override;
  void Close() override;

 private:
  TCPServer* server_;
  int sync_read_count_ = 0;
  int read_req_;
  bool secure_ = false;
  bool pending_request_ = false;
  bool shutdown_read_;
  bool shutdown_write_;
  std::string ws_pending_frame_;
  using MessageWriter = void (TCPServerConnection::*)();
  MessageWriter message_writer_;

  scoped_refptr<Channel> channel_;
  std::unique_ptr<net::StreamSocket> socket_;
  scoped_refptr<net::GrowableIOBuffer> input_buffer_;
  scoped_refptr<net::DrainableIOBuffer> output_buffer_;
  std::unique_ptr<base::OneShotTimer> handshake_timeout_;
  std::vector<scoped_refptr<IOBuffer>> output_queue_;
  std::unique_ptr<base::RepeatingTimer> live_check_timer_;
  std::unique_ptr<base::RepeatingTimer> ws_ping_timer_;
  base::Time last_recv_time_;

  base::WeakPtrFactory<TCPServerConnection> weak_factory_{this};
  SEQUENCE_CHECKER(sequence_checker_);
};

}  // namespace worker

#endif