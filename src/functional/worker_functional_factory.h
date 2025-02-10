//
// Copyright (c) 2025 mdl. All rights reserved.
//

#ifndef FUNCTIONAL_WORKER_FUNCTIONAL_FACTORY_H_
#define FUNCTIONAL_WORKER_FUNCTIONAL_FACTORY_H_

#include <memory>
#include <string>
#include <string_view>
#include "functional/functional_interface.h"

namespace worker {
class FunctionalFactory {
 public:
  static std::unique_ptr<FunctionalInterface> CreateWorkerFunctional(
      std::string_view command,
      const std::vector<std::string>& commands,
      std::string* error_message,
      int uid);
};
}  // namespace worker

#endif