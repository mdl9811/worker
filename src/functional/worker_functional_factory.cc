#include "functional/worker_functional_factory.h"
#include <memory>
#include "engine/command_type.h"
#include "functional/empty.h"
#include "functional/rm.h"
#include "functional/run.h"
#include "functional/exec.h"
#include "functional/ps.h"

namespace worker {
std::unique_ptr<FunctionalInterface> FunctionalFactory::CreateWorkerFunctional(
    std::string_view command,
    const std::vector<std::string>& commands,
    std::string* error_message,
    int uid) {
  std::unique_ptr<FunctionalInterface> interface;
#define kcompare(c, v) c.compare(v) == 0
  if (kcompare(command, kRun)) {
    interface = std::make_unique<Run>(uid);
  } else if (kcompare(command, kRm)) {
    interface = std::make_unique<Rm>();
  } else if (kcompare(command, kExec)) {
    interface = std::make_unique<Exec>(uid);
  } else if (kcompare(command, kPs)) {
    interface = std::make_unique<Ps>();
  } else {
    interface = std::make_unique<Empty>();
  }

#undef kcompare
  interface->HandleCommand(commands);
  interface->ErrorMessage(error_message);

  return interface;
}
}  // namespace worker