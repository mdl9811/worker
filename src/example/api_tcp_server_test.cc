#include "api/rtc/tcp_server.h"
#include <malloc.h>
#include <sys/types.h>
#include <cstdio>
#include <memory>
#include <ostream>
#include <sstream>
#include <vector>
#include "api/rtc/http1_processor.h"
#include "api/worker/process.h"
#include "base/at_exit.h"
#include "base/command_line.h"
#include "base/functional/bind.h"
#include "base/location.h"
#include "base/logging.h"
#include "base/message_loop/message_pump_type.h"
#include "base/run_loop.h"
#include "base/task/sequenced_task_runner.h"
#include "base/task/single_thread_task_executor.h"
#include "base/task/thread_pool/thread_pool_instance.h"
#include "base/time/time.h"
#include "base/functional/callback.h"
#include "net/base/ip_endpoint.h"
#include "net/base/net_errors.h"
#include "api/rtc/io_buffer.h"

class TCPServerTest final : public worker::TCPServer::Delegate {
 public:
  TCPServerTest() {  }
  ~TCPServerTest() = default;

  bool Initialize(const net::IPEndPoint& endpoint) {
    http_processor_.reset(new worker::Http1Processor);
    tcp_server_.reset(new worker::TCPServer(this, http_processor_.get()));
    if (tcp_server_->InitializeAndListen(endpoint) != net::OK)
      return false;
    return true;
  }

  void Run() { tcp_server_->Run(); }

  void Setup();

  void HandleCommand(const worker::HttpRequestInfo& request,
                     worker::HttpResponseInfo* response);
  std::unique_ptr<worker::TCPServer> tcp_server_;
  std::unique_ptr<worker::Http1Processor> http_processor_;
};

void TCPServerTest::Setup() {
#define SET_HANDLER(path, secure, function) \
  http_processor_->SetHandler(              \
      path, secure,                         \
      base::BindRepeating(&TCPServerTest::function, base::Unretained(this)));

  SET_HANDLER("/command", false, HandleCommand);
}

void TCPServerTest::HandleCommand(const worker::HttpRequestInfo& request,
                                  worker::HttpResponseInfo* response) {
  response->status_code(net::HTTP_OK);

  auto data = worker::IOBuffer::New("HandleCommand");
  // const char kMimeType_Json[] = "application/json";
  const char kMimeType_TextPlain[] = "text/plain";
  response->SetContentHeaders(data, kMimeType_TextPlain);
  LOG(INFO) << "HandleCommand";
}

int main(int argc, const char* argv[]) {
  logging::LoggingSettings settings;
  base::AtExitManager at_exit_manager;
  base::SingleThreadTaskExecutor io_task_executor(base::MessagePumpType::IO);
  settings.logging_dest = logging::LOG_TO_FILE | logging::LOG_TO_STDERR;
  settings.log_file_path = ".";
  logging::InitLogging(settings);
  //logging::SetLogMessageHandler(_log_message);
  base::ThreadPoolInstance::Create("worker");
  TCPServerTest tcp_server;
  auto endpoint = net::IPEndPoint(net::IPAddress::IPv4AllZeros(), 8888);
  if (!tcp_server.Initialize(endpoint))
    return -1;
  tcp_server.Setup();
  tcp_server.Run();

  base::RunLoop().Run();
  return EXIT_SUCCESS;
}
