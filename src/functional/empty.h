//
// Copyright (c) 2025 mdl. All rights reserved.
//

#ifndef FUNCTIONAL_EMPTY_H_
#define FUNCTIONAL_EMPTY_H_
#include "functional/functional_interface.h"
#include <string_view>

namespace worker {
class Empty : public FunctionalInterface {
 public:
  explicit Empty();
  ~Empty();

  bool ParseCommand(const std::vector<std::string>& commands,
                    int* pos) override;
  bool Start(uint32_t id, Delegate* delegate) override;
};
}  // namespace worker

#endif