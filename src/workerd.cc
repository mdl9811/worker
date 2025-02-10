#include <malloc.h>
#include "base/at_exit.h"
#include "base/command_line.h"
#include "base/logging.h"
#include "base/message_loop/message_pump_type.h"
#include "base/run_loop.h"
#include "base/strings/stringprintf.h"
#include "base/task/single_thread_task_executor.h"
#include "base/task/thread_pool/thread_pool_instance.h"
#include <sys/types.h>
#include <unistd.h>
#include <cstddef>
#include "engine/worker_engine.h"
#include "base/path_service.h"
#include "base/files/file_path.h"
#include "base/files/file_util.h"
#include "base/strings/string_number_conversions.h"
#include "api/worker/config_parser.h"
#include "api/gen/generate_file.h"

static const char kPort[] = "port";
static const char kGen[] = "g";

static void generate_configure(base::FilePath path, int port) {
  if (!base::PathExists(path))
    base::CreateDirectory(path);
  path = path.AppendASCII("worker.cfg");
  worker::ConfigParserBase config_parser(path);
  if (!config_parser.Parse()) {
    LOG(ERROR) << "glable config parse faild!";
    return;
  }
  config_parser.Set("Port", port);
  config_parser.Build();
  if (chmod(path.DirName().value().c_str(), 0755) == -1) {
    perror("设置目录权限失败");
    return;
  }
}

int main(int argc, const char* argv[]) {

  base::CommandLine::Init(argc, argv);
  base::CommandLine& command_line = *base::CommandLine::ForCurrentProcess();
  logging::LoggingSettings settings;
  base::AtExitManager at_exit_manager;
  base::SingleThreadTaskExecutor io_task_executor(base::MessagePumpType::IO);
  settings.logging_dest = logging::LOG_TO_FILE | logging::LOG_TO_STDERR;

  base::FilePath path;
  if (!base::PathService::Get(base::FILE_EXE, &path)) {
    LOG(ERROR) << "exe path get faild!";
    return 0;
  }
  std::string port_str = command_line.GetSwitchValueASCII(kPort);

  int port = 8888;
  if (!port_str.empty())
    base::StringToInt(port_str, &port);

  if (command_line.HasSwitch(kGen)) {
    auto service_path = path;
    service_path = service_path.DirName().AppendASCII("../config/");
    service_path = service_path.AppendASCII("workerd.service");
    LOG(INFO) << "exec: sudo cp " << service_path << " /lib/systemd/system";
    worker::GenerateFile gen_file(service_path.value());
    gen_file.AddLine("[Unit]");
    gen_file.AddLine("Description=Worker Service Example");
    gen_file.AddLine("After=network.target");
    gen_file.AddLine();
    gen_file.AddLine("[Service]");
    gen_file.AddLine("Type=simple");
    gen_file.AddLine("Restart=always");
    gen_file.AddLine("Restart=on-failure");
    gen_file.AddLine("TimeoutStartSec=30");
    gen_file.AddLine("RestartSec=5s");
    gen_file.AddLine("ExecStart=" + base::StringPrintf("%s --port=%d",
                                                       path.value().c_str(),
                                                       port));
    gen_file.AddLine("ExecStop=/bin/kill $MAINPID");
    gen_file.AddLine();
    gen_file.AddLine("[Install]");
    gen_file.AddLine("WantedBy=multi-user.target");
    gen_file.Build();
    return EXIT_SUCCESS;
  }

  auto log_path = path;
  log_path = log_path.DirName().AppendASCII("../log/");
  if (!base::PathExists(log_path))
    base::CreateDirectory(log_path);
  log_path = log_path.AppendASCII("workerd.log");
  settings.log_file_path = log_path.value();
  logging::InitLogging(settings);
  base::ThreadPoolInstance::Create("workerd");
  generate_configure(path.DirName().AppendASCII("../config/"), port);

  base::RunLoop run_loop;
  LOG(INFO) << "Start Workerd Engine";
  worker::WorkerEngine(run_loop).Run(argc, argv, port);
  return EXIT_SUCCESS;
}