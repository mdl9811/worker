#include "functional/rm.h"
#include <vector>
#include "api/gen/message_build.h"
#include "api/worker/config_parser.h"
#include "api/worker/container.h"
#include "base/logging.h"

namespace worker {
Rm::Rm() : FunctionalInterface(FunctionalInterface::kRm) {}
Rm::~Rm() = default;

bool Rm::ParseCommand(const std::vector<std::string>& commands, int* pos) {
  int& index = *pos;
  for (; index < commands.size(); index++) {
    const std::string command = commands[index];
    if (command.find("-") == std::string::npos)
      break;
    if (command == "-a") {
      all_ = true;
    } else if (command == "-l") {
      link_ = true;
    } else if (command == "-v") {
      volumes_ = true;
    } else {
      AddErrorMessage("worker run Incorrect parameter.");
      return false;
    }
  }

  while (index < commands.size())
    optional_.ids.push_back(commands[index++]);

  return true;
}

bool Rm::Check(Container* container) {
  if (all_ && link_) {
    AddErrorMessage("Worker rm starts two parameters.[-a -l]");
    delegate_->OnCompletion(
        MessageBuild::Build(MessageBuild::kEcho, error_message_));
    return false;
  }
  if (all_)
    container->gconfig_parser()->Ids(&optional_.ids);

  if (link_)
    container->gconfig_parser()->Ids(&optional_.ids);

  container->Remove(optional_);
  if (volumes_)
    container->RemoveNoneContainer();

  delegate_->OnCompletion(MessageBuild::Build(MessageBuild::kEcho, "Ok"));
  return true;
}

bool Rm::Start(uint32_t id, Delegate* delegate) {
  DCHECK(delegate);
  if (quit) {
    delegate->OnCompletion(MessageBuild::Build(MessageBuild::kEcho, message_));
    return delegate->OnExit(id);
  }

  delegate_ = delegate;
  id_ = id;
  Container& container = delegate_->container();
  Check(&container);

  return delegate->OnExit(id);
}

bool Rm::Help() {
  AddLine("Usage: worker rm [OPTION] CONTAINER [CONTAINER...]");
  AddLine("Aliases:");
  AddLine("   worker container rm, docker container remove, worker rm.");
  AddLine("Options:");
  AddLine("  -a   Delete all containers.");
  AddLine("  -l   Remove all temp containers.");
  AddLine("  -v   Remove anonymous volumes associated with the container.");
  return quit = true;
}

}  // namespace worker