#include "functional/functional_interface.h"

namespace worker {

FunctionalInterface::FunctionalInterface(Type type) : type_(type) {}
FunctionalInterface::~FunctionalInterface() = default;

void FunctionalInterface::AddErrorMessage(const std::string_view message) {
  error_message_.append(message);
}

void FunctionalInterface::AddLine(const std::string& message) {
  message_.append(message);
  message_.append("\n");
}

void FunctionalInterface::AddLine() {
  message_.append("\n");
}

bool FunctionalInterface::Ok() const {
  return ok_;
}

void FunctionalInterface::ErrorMessage(std::string* message) {
  *message = error_message_;
}

void FunctionalInterface::AddArg(const std::string& arg) {
  args_vector_.push_back(arg);
}

bool FunctionalInterface::ParseCommand(const std::vector<std::string>& commands,
                                       int* pos) {
  return true;
}

bool FunctionalInterface::InitializeCommand(
    const std::vector<std::string>& commands) {
  const std::string command = commands[0];
  if (command == "--help") {
    return Help();
  } else {
    int index = 0;
    if (!ParseCommand(commands, &index))
      return false;
  }
  return true;
}

void FunctionalInterface::HandleCommand(
    const std::vector<std::string>& commands) {
  if (commands.size() <= 0)
    return AddErrorMessage(
        "run command parameter is empty. Please provide a valid command.");
  if (!InitializeCommand(commands))
    return;
  ok_ = true;
}

bool FunctionalInterface::Help() {
  return false;
}

void FunctionalInterface::OnExit(int pid) {}

}  // namespace worker