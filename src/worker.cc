#include <malloc.h>
#include "base/at_exit.h"
#include "base/command_line.h"
#include "base/logging.h"
#include "base/message_loop/message_pump_type.h"
#include "base/run_loop.h"
#include "base/task/single_thread_task_executor.h"
#include "base/task/thread_pool/thread_pool_instance.h"
#include <sys/types.h>
#include <unistd.h>
#include <cstdio>
#include "engine/session.h"
#include "engine/worker_engine.h"
#include "base/path_service.h"
#include "base/files/file_path.h"
#include "base/files/file_util.h"


int main(int argc, const char* argv[]) {
  base::CommandLine::Init(argc, argv);
  logging::LoggingSettings settings;
  base::AtExitManager at_exit_manager;
  base::SingleThreadTaskExecutor io_task_executor(base::MessagePumpType::IO);
  settings.logging_dest = logging::LOG_TO_STDERR;
  settings.log_file_path = ".";

  base::FilePath path;
  if (!base::PathService::Get(base::FILE_EXE, &path)) {
    LOG(ERROR) << "exe path get faild!";
    return 0;
  }
  path = path.DirName().AppendASCII("../config/");
  path = path.AppendASCII("worker.cfg");
  if (!base::PathExists(path)) {
    printf("workerd not start\n");
    return EXIT_FAILURE;
  }

  worker::ConfigParserBase config_parser(path);
  if (!config_parser.Parse()) {
    printf("glable config parse faild!\n");
    return EXIT_FAILURE;
  }
  auto& dict = config_parser.dict();
  auto port = dict.FindInt("Port");
  if (!port) {
    printf("glable config port not exist!\n");
    return EXIT_FAILURE;
  }

  logging::InitLogging(settings);
  base::ThreadPoolInstance::Create("worker");
  base::RunLoop run_loop;
  worker::Session(run_loop).Run(argc, argv, port.value());
  return EXIT_SUCCESS;
}