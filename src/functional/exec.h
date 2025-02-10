//
// Copyright (c) 2025 mdl. All rights reserved.
//

#ifndef FUNCTIONAL_EXEC_H_
#define FUNCTIONAL_EXEC_H_

#include <string>
#include "functional/functional_interface.h"

namespace worker {
class Container;
class Exec : public FunctionalInterface {
 public:
  explicit Exec(int uid);
  ~Exec();

 private:
  bool ParseCommand(const std::vector<std::string>& commands,
                    int* pos) override;

  bool Check(Container* container);
  bool Start(uint32_t id, Delegate* delegate) override;
  bool Help() override;

 private:
  Delegate* delegate_;
  int uid_ = 0;
  bool interaction_ = false;
};
}  // namespace worker

#endif