#include <malloc.h>
#include <sys/types.h>
#include <cstdio>
#include <memory>
#include <vector>
#include "api/gen/generate_container_id.h"
#include "api/worker/container.h"
#include "base/at_exit.h"
#include "base/command_line.h"
#include "base/logging.h"
#include "base/message_loop/message_pump_type.h"
#include "base/run_loop.h"
#include "base/task/single_thread_task_executor.h"
#include "base/task/thread_pool/thread_pool_instance.h"
#include "functional/functional_interface.h"

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
  auto generate_container_id = std::make_unique<worker::GenerateContainerID>();
  worker::Container container(nullptr, nullptr, generate_container_id.get());
  base::RunLoop().Run();
  return EXIT_SUCCESS;
}