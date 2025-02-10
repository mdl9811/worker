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
#include "api/worker/config_parser.h"
#include "base/path_service.h"
#include "base/files/file_util.h"
#include "api/gen//generate_container_id.h"

int TestAdd(base::FilePath path) {
  const char kContainerId[] = "ContainerId";
  auto generate_container_id = std::make_unique<worker::GenerateContainerID>();
  path = path.DirName().AppendASCII("../testconfig/");
  if (!base::PathExists(path))
    base::CreateDirectory(path);

  auto gpath = path.AppendASCII("test.db");
  worker::GConfigParser gconfig_parser(gpath);
  if (!gconfig_parser.Parse()) {
    LOG(ERROR) << "gconfig parse faild!";
    return -1;
  }

  auto path1 = path.AppendASCII("test1.db");
  worker::ConfigParser config_parser1(
      path1, generate_container_id->GenerateUniqueId());
  if (!config_parser1.Parse()) {
    LOG(ERROR) << "config parser1 faild!";
    return -1;
  }
  config_parser1.SetName(
      generate_container_id->RandomUniqueName(gconfig_parser.Names()));
  config_parser1.Set(kContainerId, generate_container_id->GenerateUniqueDId());
  gconfig_parser.SetParser(&config_parser1);
  auto path2 = path.AppendASCII("test1.db");
  worker::ConfigParser config_parser2(
      path1, generate_container_id->GenerateUniqueId());

  config_parser2.SetName(
      generate_container_id->RandomUniqueName(gconfig_parser.Names()));

  if (!config_parser2.Parse()) {
    LOG(ERROR) << "config parser2 faild!";
    return -1;
  }

  config_parser2.Set(kContainerId, generate_container_id->GenerateUniqueDId());

  gconfig_parser.SetParser(&config_parser2);
  gconfig_parser.Build();
  return 0;
}

int TestRemove(base::FilePath path) {
  path = path.DirName().AppendASCII("../testconfig/");
  if (!base::PathExists(path))
    base::CreateDirectory(path);

  auto gpath = path.AppendASCII("testrm.db");
  worker::GConfigParser gconfig_parser(gpath);
  if (!gconfig_parser.Parse()) {
    LOG(ERROR) << "gconfig parse faild!";
    return -1;
  }
  gconfig_parser.Remove("id");
  //gconfig_parser.Set("id", "mdl");
  gconfig_parser.Build();
  return 0;
}

int main(int argc, const char* argv[]) {
  logging::LoggingSettings settings;
  base::AtExitManager at_exit_manager;
  base::SingleThreadTaskExecutor io_task_executor(base::MessagePumpType::IO);
  settings.logging_dest = logging::LOG_TO_FILE | logging::LOG_TO_STDERR;
  settings.log_file_path = ".";
  logging::InitLogging(settings);
  base::ThreadPoolInstance::Create("worker");

  base::FilePath path;
  if (!base::PathService::Get(base::FILE_EXE, &path)) {
    LOG(ERROR) << "exe path get faild!";
    return -1;
  }
#if 0
  TestAdd(path);
#endif

#if 1
  TestRemove(path);
#endif
  return EXIT_SUCCESS;
}
