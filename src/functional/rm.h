//
// Copyright (c) 2025 mdl. All rights reserved.
//

#ifndef FUNCTIONAL_RM_H_
#define FUNCTIONAL_RM_H_

#include "functional/functional_interface.h"

namespace worker {
class Rm : public FunctionalInterface {
 public:
  explicit Rm();
  ~Rm();

 private:
  bool Check(Container* container);
  bool ParseCommand(const std::vector<std::string>& commands,
                    int* pos) override;

  bool Start(uint32_t id, Delegate* delegate) override;
  bool Help() override;

 private:
  bool all_ = false;
  bool link_ = false;
  bool volumes_ = false;
};
}  // namespace worker

#endif