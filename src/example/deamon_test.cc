//服务的例子
/*
*将example/workerd-test.servce copy 到/lib/systemd/system目录下
*执行 systemctl daemon-reload、systemctl start workerd-test
*停止 systemctl stop workerd-test
*/

#include <malloc.h>
#include <sys/types.h>
#include <unistd.h>
#include <cstdio>
#include "base/at_exit.h"
#include "base/functional/bind.h"
#include "base/location.h"
#include "base/logging.h"
#include "base/message_loop/message_pump_type.h"
#include "base/run_loop.h"
#include "base/task/sequenced_task_runner.h"
#include "base/task/single_thread_task_executor.h"
#include "base/task/thread_pool/thread_pool_instance.h"
#include "base/time/time.h"
#include "base/files/file_path.h"
#include "base/path_service.h"
#include "base/files/file_util.h"
#include "api/gen/generate_file.h"
#include "base/command_line.h"

bool _log_message(int severity,
                  const char* file,
                  int line,
                  size_t message_start,
                  const std::string& str) {
  printf("%s", str.substr(message_start).c_str());
  return true;
}

void echo_time() {
  LOG(INFO) << "current time" << base::Time::Now();
  base::SequencedTaskRunner::GetCurrentDefault()->PostDelayedTask(
      FROM_HERE, base::BindOnce(echo_time), base::Seconds(1));
}

int main(int argc, const char* argv[]) {
  if (true) {
    base::CommandLine::Init(argc, argv);
    logging::LoggingSettings settings;
    base::AtExitManager at_exit_manager;
    base::SingleThreadTaskExecutor io_task_executor(base::MessagePumpType::IO);
    base::FilePath path;
    if (!base::PathService::Get(base::FILE_EXE, &path)) {
      LOG(ERROR) << "exe path get faild!";
      return 0;
    }
    base::CommandLine& command_line = *base::CommandLine::ForCurrentProcess();
    if (command_line.HasSwitch("g")) {
      base::FilePath service_path = path;
      service_path = service_path.DirName().AppendASCII("workerd-test.service");
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
      gen_file.AddLine("ExecStart=" + path.value());
      gen_file.AddLine("ExecStop=/bin/kill $MAINPID");
      gen_file.AddLine();
      gen_file.AddLine("[Install]");
      gen_file.AddLine("WantedBy=multi-user.target");
      gen_file.Build();
      return 0;
    }

    settings.logging_dest = logging::LOG_TO_FILE | logging::LOG_TO_STDERR;
    path = path.DirName().AppendASCII("../log/");
    if (!base::PathExists(path))
      base::CreateDirectory(path);
    path = path.AppendASCII("workerd.log");
    settings.log_file_path = path.value();
    logging::InitLogging(settings);
    // logging::SetLogMessageHandler(_log_message);
    base::ThreadPoolInstance::Create("worker");
    base::SequencedTaskRunner::GetCurrentDefault()->PostTask(
        FROM_HERE, base::BindOnce(echo_time));
    base::RunLoop().Run();
  }

  return EXIT_SUCCESS;
}
