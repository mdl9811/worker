//
// Copyright (c) 2025 mdl. All rights reserved.
//

#ifndef FUNCTIONAL_PS_H_
#define FUNCTIONAL_PS_H_

#include "functional/functional_interface.h"

namespace worker {
class Ps : public FunctionalInterface {
 public:
  explicit Ps();
  ~Ps();

 private:
  bool Check(Container* container);
  bool ParseCommand(const std::vector<std::string>& commands,
                    int* pos) override;

  bool Start(uint32_t id, Delegate* delegate) override;
  bool Help() override;

  bool All(Container* container);
  bool Designated(Container* container);

 private:
  bool all_ = false;
  bool designated_ = false;
};
}  // namespace worker

#endif