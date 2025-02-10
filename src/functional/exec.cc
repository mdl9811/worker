#include "functional/exec.h"
#include "functional/functional_interface.h"
#include "api/gen/message_build.h"
#include "api/worker/container.h"

namespace worker {
Exec::Exec(int uid)
    : FunctionalInterface(FunctionalInterface::kExec), uid_(uid) {}
Exec::~Exec() = default;

bool Exec::ParseCommand(const std::vector<std::string>& commands, int* pos) {
  int& index = *pos;
  for (; index < commands.size(); index++) {
    const std::string command = commands[index];
    if (command.find("-") == std::string::npos)
      break;
    if (command == "-i") {
      interaction_ = true;
    } else {
      AddErrorMessage("worker exec Incorrect parameter.");
      return false;
    }
  }
  if (index + 2 == commands.size()) {
    optional_.ids.push_back(commands[index++]);
    args_vector_.push_back(commands[index]);
  } else {
    AddErrorMessage("worker exec Incorrect parameter.");
    return false;
  }
  return true;
}

bool Exec::Check(Container* container) {
  if (!optional_.ids.size())
    return false;
  auto config_parser = container->gconfig_parser()->Get(optional_.ids[0]);
  if (!config_parser) {
    AddErrorMessage("container not exist.");
    delegate_->OnCompletion(
        MessageBuild::Build(MessageBuild::kEcho, error_message_));
    return false;
  }

  auto pid_list = config_parser->GetPidList();

  if (pid_list.empty()) {
    delegate_->OnCompletion(
        MessageBuild::Build(MessageBuild::kEcho, "Container Exit!"));
    return false;
  }

  MessageBuild message_build(MessageBuild::kExec);
  base::Value::List list;
  int i = 0;
  while (i < args().size())
    list.Append(args()[i++]);

  message_build.Set(kKeyArgs, std::move(list));
  message_build.Set(kKeyPid, pid_list[0]);
  message_build.Set(kMessage, "Ok");
  message_build.Set(kKeyID, config_parser->id());
  if (interaction_)
    message_build.Set(kKeyTTY, 1);
  else
    message_build.Set(kKeyTTY, 0);
  message_build.Build(&message_);
  delegate_->OnCompletion(message_);
  return true;
}

bool Exec::Start(uint32_t id, Delegate* delegate) {
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
  Container& container = delegate_->container();
  Check(&container);

  return delegate->OnExit(id);
}

bool Exec::Help() {
  AddLine("Usage: worker exec [OPTION] CONTAINER [CONTAINER...]");
  AddLine("Aliases:");
  AddLine("   worker container exec, docker exec");
  AddLine("Options:");
  AddLine("  -i   Keep STDIN open even if not attached");
  return quit = true;
}
}  // namespace worker