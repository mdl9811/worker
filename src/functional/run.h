//
// Copyright (c) 2025 mdl. All rights reserved.
//

#ifndef FUNCTIONAL_RUN_H_
#define FUNCTIONAL_RUN_H_

#include <string>
#include <string_view>
#include "functional/functional_interface.h"

namespace worker {
class Container;
class Run : public FunctionalInterface {
 public:
  explicit Run(int uid);
  ~Run();

 private:
  bool InitializeCommand(const std::vector<std::string>& commands) override;
  bool ParseCommand(const std::vector<std::string>& commands,
                    int* pos) override;

  bool Check(Container* container);
  bool Start(uint32_t id, Delegate* delegate) override;
  bool Help() override;

 private:
  Delegate* delegate_;
  int uid_;
  void OnExit(int pid) override;
};
}  // namespace worker

#endif