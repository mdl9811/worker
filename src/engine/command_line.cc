#include "engine/command_line.h"
#include <cstdio>
#include <string>
#include <string_view>
#include <utility>
#include <vector>
#include "absl/types/span.h"
#include "base/check.h"
#include "base/strings/stringprintf.h"
#include "base/task/sequenced_task_runner.h"
#include "base/logging.h"
#include "engine/command_type.h"
#include "functional/functional_interface.h"
#include "version.h"
#include "functional/worker_functional_factory.h"
#include "api/gen/message_build.h"

namespace {
void AddLine(std::string* line, const std::string& message) {
  line->append(message);
  line->append("\n");
}
void AddLine(std::string* line) {
  line->append("\n");
}
}  // namespace

namespace worker {
#define TF(run, ...)                                                           \
  task_runner_->PostTask(                                                      \
      FROM_HERE, base::BindOnce(&CommandLine::run, weak_factory_.GetWeakPtr(), \
                                ##__VA_ARGS__))

CommandLine::CommandLine(Delegate* delegate)
    : delegate_(delegate),
      task_runner_(base::SequencedTaskRunner::GetCurrentDefault()) {
  DCHECK(delegate_);
}

CommandLine::~CommandLine() = default;

void CommandLine::ParseCommandLine(const std::vector<std::string>& commands,
                                   int uid) {
  const std::string_view command =
      commands.size() == 0 ? kHelp : std::string_view(commands[0]);
#define kcompare(c, v) c.compare(v) == 0
  if (kcompare(command, kHelp)) {
    Help();
    return;
  } else if (kcompare(command, kVersion)) {
    delegate_->OnCompletion(MessageBuild::Build(
        MessageBuild::kEcho,
        base::StringPrintf("Worker Engine version:            %s\n",
                           WORKER_VERSION)));
    return;
  } else if (kcompare(command, kCVersion)) {
    Version();
    return;
  } else {
    std::string error_message;

    std::string message;
    auto worker_functional = FunctionalFactory::CreateWorkerFunctional(
        command, {commands.begin() + 1, commands.end()}, &error_message, uid);
    if (!worker_functional->Ok()) {
      if (worker_functional->type() != FunctionalInterface::kEmpty) {
        AddLine(&message, base::StringPrintf("Please See 'worker %s' --help.",
                                             command.data()));
        AddLine(&message,
                base::StringPrintf("faild cause: %s", error_message.c_str()));
      } else {
        AddLine(&message,
                base::StringPrintf("workder: '%s' is not a workder command",
                                   command.data()));
        AddLine(&message, "See 'worker --help");
      }
      delegate_->OnCompletion(
          MessageBuild::Build(MessageBuild::kEcho, message));
    } else {
      TF(OnCommand, std::move(worker_functional));
    }
  }
#undef kcompare
}

void CommandLine::OnCommand(
    std::unique_ptr<FunctionalInterface> worker_functional) {
  delegate_->OnCommand(std::move(worker_functional));
}

void CommandLine::Help() {
  std::string message;
  AddLine(&message, "Usage: worker [OPTION] COMMAND");
  AddLine(&message, "Common Commands:");
  AddLine(&message,
          "version   Display the worker version information and Quit.");
  AddLine(&message,
          "run       Run the worker engine and run a new process in this "
          "container.");
  AddLine(&message, "rm       Delete container.");
  AddLine(&message, "ps       List containers.");
  AddLine(&message,
          "exec       Execute a command in a running container.");
  AddLine(&message);
  AddLine(&message, "Global Options:");
  AddLine(&message, "--help     Display help information.");
  AddLine(&message, "--version  Display version information and Quit.");
  delegate_->OnCompletion(MessageBuild::Build(MessageBuild::kEcho, message));
}

void CommandLine::Version() {
  std::string message;
  AddLine(&message, "Client : Worker Engine - Professional.");
  AddLine(&message,
          base::StringPrintf(" Version:               %s", WORKER_VERSION));
  AddLine(&message,
          base::StringPrintf(" API Version:           %s", WORKER_API_VERSION));
  AddLine(&message, base::StringPrintf(" Chromium Kerner:       %s",
                                       WORKER_CHROMIUM_MAJOR));
  AddLine(&message, base::StringPrintf(" Compatible Version:    %s",
                                       WORKER_VERSION_MAJOR));
  AddLine(&message,
          base::StringPrintf(" OS/Arch::              %s", WORKER_ARCH_OS));
  delegate_->OnCompletion(MessageBuild::Build(MessageBuild::kEcho, message));
}

}  // namespace worker