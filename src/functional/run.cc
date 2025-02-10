#include "functional/run.h"
#include <cstdio>
#include <string>
#include <string_view>
#include <vector>
#include "api/worker/config_parser.h"
#include "api/worker/container.h"
#include "api/worker/file_string.h"
#include "api/worker/process.h"
#include "base/check.h"
#include "base/command_line.h"
#include "base/files/file_path.h"
#include "base/files/file_util.h"
#include "base/logging.h"
#include "base/path_service.h"
#include "base/strings/stringprintf.h"
#include "base/time/time.h"
#include "base/values.h"
#include "functional/functional_interface.h"
#include "api/gen/message_build.h"
#include "engine/status_code.h"

namespace worker {
Run::Run(int uid) : FunctionalInterface(FunctionalInterface::kRun), uid_(uid) {}
Run::~Run() = default;

bool Run::ParseCommand(const std::vector<std::string>& commands, int* pos) {
  int& index = *pos;
  while (index < commands.size()) {
    const std::string command = commands[index];
    if (command.find("-") == std::string::npos)
      break;
    if (command == "-i")
      optional_.tty = true;
    else {
      AddErrorMessage("worker run Incorrect parameter.");
      return false;
    }

    index++;
  }
  return true;
}

bool Run::InitializeCommand(const std::vector<std::string>& commands) {
  const std::string command = commands[0];
  if (command == "--help") {
    Help();
  } else {
    int index = 0;
    if (!ParseCommand(commands, &index))
      return false;
    if (index >= commands.size()) {
      AddErrorMessage("worker run Incorrect parameter.");
      return false;
    }
    const std::string exec_command = commands[index++];

    base::FilePath path(exec_command);

    if (!base::PathExists(path)) {
      AddErrorMessage("The process path does not exist.");
      return false;
    }
    if (base::DirectoryExists(path)) {
      AddErrorMessage("The process path is a directory, not a file.");
      return false;
    }
    if (access(exec_command.c_str(), X_OK) != 0) {
      AddErrorMessage("The process path is not executable.");
      return false;
    }

    args_vector_.push_back(exec_command);

    while (index < commands.size())
      args_vector_.push_back(commands[index++]);
  }
  return true;
}

bool Run::Check(Container* container) {
  if (!container->InitializeTempContainer(&config_parser_)) {
    delegate_->OnCompletion(MessageBuild::Build(
        MessageBuild::kEcho, "container initialize faild!\n"));
    return false;
  }
  std::vector<std::string> argv;
  base::FilePath path;
  if (!base::PathService::Get(base::FILE_EXE, &path)) {
    delegate_->OnCompletion(
        MessageBuild::Build(MessageBuild::kEcho, "exe path not exist!\n"));
    return false;
  }
  path = path.DirName().AppendASCII("mdl");
  if (!base::PathExists(path)) {
    delegate_->OnCompletion(
        MessageBuild::Build(MessageBuild::kEcho, "mdl 1 process not exist!\n"));
    return false;
  }
  argv.push_back(path.value());
  auto process = std::make_unique<Process>(this, container->id(), argv);
  process->SetPath(container->rpath(), container->mpath());
  if (!process->Start()) {
    LOG(ERROR) << "process start faild!";
    return false;
  }
  config_parser_->SetPid(process->Pid());
  config_parser_->Set(kImage, "/");
  config_parser_->Set(kCommand, args()[0]);

  std::ostringstream os;
  os << base::Time::Now();
  config_parser_->Set(kCreated, os.str());
  config_parser_->Set(kStatus, ContainerCode::Runing);
  config_parser_->Set(kType, "TEMP");

  process->Wait();
  optional_.ids.push_back(container->id());
  process_ = std::move(process);
  MessageBuild message_build(MessageBuild::kRun);
  message_build.Set(kKeyPid, process_->Pid());
  message_build.Set(kMessage,
                    base::StringPrintf("Ok ID:%s", config_parser_->id()));
  message_build.Set(kKeyID, config_parser_->id());

  base::Value::List list;
  int i = 0;
  while (i < args().size())
    list.Append(args()[i++]);

  message_build.Set(kKeyArgs, std::move(list));

  if (optional_.tty)
    message_build.Set(kKeyTTY, 1);
  else
    message_build.Set(kKeyTTY, 0);
  message_build.Build(&message_);
  delegate_->OnCompletion(message_);
  return true;
}

bool Run::Start(uint32_t id, Delegate* delegate) {
  DCHECK(delegate);
  if (quit) {
    delegate->OnCompletion(MessageBuild::Build(MessageBuild::kEcho, message_));
    return delegate->OnExit(id);
  }

  if (uid_ != 0) {
    delegate->OnCompletion(
        MessageBuild::Build(MessageBuild::kEcho, "permission denied"));
    return delegate->OnExit(id);
  }

  delegate_ = delegate;
  id_ = id;
  Container& container = delegate->container();
  return Check(&container);
}

void Run::OnExit(int pid) {
  Container& container = delegate_->container();
  container.Remove(optional_);
  delegate_->OnExit(id_);
}

bool Run::Help() {
  AddLine("Usage: worker run [OPTION] PATH [args].");
  AddLine("Aliases:");
  AddLine("  worker container run, a new process.");
  AddLine("Options:");
  AddLine("  -i     Enter the container distribution terminal");
  return quit = true;
}

}  // namespace worker