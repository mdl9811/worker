#include "functional/empty.h"
#include "functional/functional_interface.h"

namespace worker {
Empty::Empty() : FunctionalInterface(FunctionalInterface::kEmpty) {}
Empty::~Empty() = default;

bool Empty::ParseCommand(const std::vector<std::string>& commands, int* pos) {
  return false;
}

bool Empty::Start(uint32_t id, Delegate* delegate) {
  return false;
}

}  // namespace worker