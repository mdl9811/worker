#include <malloc.h>
#include <sys/types.h>
#include <cstdio>
#include <memory>
#include <ostream>
#include <sstream>
#include <vector>
#include "absl/types/optional.h"
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
#include "api/rtc/url_request.h"
#include "url/gurl.h"

void Reponse(int http_response_code, const std::string& data) {
  LOG(INFO) << http_response_code << " data: " << data;
}

int main(int argc, const char* argv[]) {
  // 使用url requeset 必须初始化命令行
  base::CommandLine::Init(argc, argv);

  logging::LoggingSettings settings;
  base::AtExitManager at_exit_manager;
  base::SingleThreadTaskExecutor io_task_executor(base::MessagePumpType::IO);
  settings.logging_dest = logging::LOG_TO_FILE | logging::LOG_TO_STDERR;
  settings.log_file_path = ".";
  logging::InitLogging(settings);
  // logging::SetLogMessageHandler(_log_message);
  base::ThreadPoolInstance::Create("worker");

  net::HttpRequestHeaders headers;
  worker::URLRequest::Get(GURL("http://192.168.0.100:8888/command"), headers,
                          base::BindOnce(Reponse), base::Seconds(2));

  base::RunLoop().Run();
  return EXIT_SUCCESS;
}
