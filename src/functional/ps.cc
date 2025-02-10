#include "functional/ps.h"
#include <string>
#include <vector>
#include "api/gen/message_build.h"
#include "api/worker/config_parser.h"
#include "api/worker/container.h"
#include "api/worker/file_string.h"
#include "base/logging.h"
#include "base/strings/stringprintf.h"
#include "base/time/time.h"
#include "engine/status_code.h"

namespace worker {
Ps::Ps() : FunctionalInterface(FunctionalInterface::kPs) {}
Ps::~Ps() = default;

bool Ps::ParseCommand(const std::vector<std::string>& commands, int* pos) {
  int& index = *pos;
  for (; index < commands.size(); index++) {
    const std::string command = commands[index];
    if (command.find("-") == std::string::npos)
      break;
    if (command == "-a") {
      all_ = true;
    } else if (command == "-d") {
      designated_ = true;
    } else {
      AddErrorMessage("worker run Incorrect parameter.");
      return false;
    }
  }

  return true;
}

bool Ps::All(Container* container) {
  if (!all_)
    return false;
  auto& dict = container->gconfig_parser()->dict();
  AddLine(
      "CONTAINER ID  IMAGE     COMMAND    CREATED          STATUS   TYPE   "
      "PORTS "
      "NAMES");
  for (auto it = dict.begin(); it != dict.end(); it++) {
    auto& d = it->second;
    auto& dd = d.GetDict();
    std::string id = *dd.FindString(kId);
    std::string image = *dd.FindString(kImage);
    std::string command = *dd.FindString(kCommand);
    std::string created = *dd.FindString(kCreated);

    base::Time time;
    if (base::Time::FromUTCString(created.c_str(), &time)) {
    }
    auto now = base::Time::Now() - time;

    std::string ctime;
    if (now.InDays() > 1) {
      ctime = base::StringPrintf("%d Day ago", now.InDays());
    } else {
      ctime = base::StringPrintf("%d minute ago", now.InMinutes());
    }

    auto code = dd.FindInt(kStatus);
    std::string status;
    switch (code.value()) {
      case ContainerCode::Created:
        status = "Created";
        break;
      case ContainerCode::Runing:
        status = "Runing";
        break;
      case ContainerCode::Exit:
        status = "Exit";
        break;
    }
    std::string port_str;
    auto port = dd.FindInt(kPorts);
    if (port)
      port_str = std::to_string(port.value());
    std::string type = *dd.FindString(kType);
    std::string name = *dd.FindString(kName);
    AddLine(base::StringPrintf("%s  %s         %s  %s     %s  %s %s   %s",
                               id.c_str(), image.c_str(), command.c_str(),
                               ctime.c_str(), status.c_str(), port_str.c_str(),
                               type.c_str(), name.c_str()));
  }
  return true;
}

bool Ps::Designated(Container* container) {
  if (!designated_)
    return false;
  return true;
}

bool Ps::Check(Container* container) {
  if (all_ && designated_) {
    delegate_->OnCompletion(MessageBuild::Build(
        MessageBuild::kEcho, "-a -d Cannot be used simultaneously"));
    return false;
  }
  return All(container) || Designated(container);
}

bool Ps::Start(uint32_t id, Delegate* delegate) {
  DCHECK(delegate);
  if (quit) {
    delegate->OnCompletion(MessageBuild::Build(MessageBuild::kEcho, message_));
    return delegate->OnExit(id);
  }

  delegate_ = delegate;
  id_ = id;
  Container& container = delegate_->container();
  Check(&container);
  delegate_->OnCompletion(MessageBuild::Build(MessageBuild::kEcho, message_));
  return delegate->OnExit(id);
}

bool Ps::Help() {
  AddLine("Usage: worker Ps [OPTION]");
  AddLine("Aliases:");
  AddLine(
      "   worker container ls, worker container list, worker container ps, "
      "worker ps");
  AddLine("Options:");
  AddLine("  -a   Show all containers.");
  AddLine("  -d   Designated container.[-p [id]]");
  return quit = true;
}
}  // namespace worker