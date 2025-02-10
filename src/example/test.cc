#include <malloc.h>
#include <sys/types.h>
#include <cstdio>
#include <memory>
#include <ostream>
#include <sstream>
#include <vector>
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


bool _log_message(int severity,
                  const char* file,
                  int line,
                  size_t message_start,
                  const std::string& str) {
  printf("%s", str.substr(message_start).c_str());
  return true;
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

  LOG(INFO) << "test";
  base::Time::Exploded explode;
  auto s = base::Time::Now();

  base::Time time;
  base::Time::FromUTCString("2025-01-22 11:00:44.523034 UTC",&time);
  base::Time::Now().UTCExplode(&explode);

  std::ostringstream os;
  os << time;
  LOG(INFO) << os.str();
  LOG(INFO) << (s - time).InSeconds();
  // base::RunLoop().Run();
  return EXIT_SUCCESS;
}
