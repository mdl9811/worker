#include <malloc.h>
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
#include <sys/types.h>
#include <cstdio>
#include <memory>
#include <vector>
#include "api/worker/process.h"

const char kEmpty[] = "";

class ProcessTest : public worker::Process::Delegate {
 public:
  ProcessTest() {}
  void Run(std::vector<std::string> args) {
    process.reset(new worker::Process(this, "", args));
    process->Wait();
  }

  void OnExit(int pid) override { LOG(INFO) << "on exit pid:" << pid; }

  std::unique_ptr<worker::Process> process;
  std::string rpath_;
  std::string mpath_;
};

int main(int argc, const char* argv[]) {
  logging::LoggingSettings settings;
  base::AtExitManager at_exit_manager;
  base::SingleThreadTaskExecutor io_task_executor(base::MessagePumpType::IO);
  settings.logging_dest = logging::LOG_TO_FILE | logging::LOG_TO_STDERR;
  settings.log_file_path = ".";
  logging::InitLogging(settings);
  base::ThreadPoolInstance::Create("worker");
  std::vector<std::string> args;
  args.push_back("/bin/bash");
  ProcessTest test;
  test.Run(args);
  base::RunLoop().Run();
  return EXIT_SUCCESS;
}
