//
// Copyright (c) 2025 mdl. All rights reserved.
//

#ifndef ENGINE_COMMAND_LINE_H_
#define ENGINE_COMMAND_LINE_H_
#include <memory>
#include <string>
#include <string_view>
#include "base/memory/scoped_refptr.h"
#include "base/memory/weak_ptr.h"
#include "base/task/sequenced_task_runner.h"
#include "engine/command_type.h"

namespace worker {
class FunctionalInterface;

class CommandLine final {
 public:
  class Delegate {
   public:
    virtual void OnCommand(
        std::unique_ptr<FunctionalInterface> worker_functional) = 0;
    virtual void OnCompletion(const std::string& message) = 0;
  };

  explicit CommandLine(Delegate* delegate);

  CommandLine(const CommandLine&) = delete;
  CommandLine(const CommandLine&&) = delete;
  CommandLine& operator=(const CommandLine&) = delete;
  CommandLine&& operator=(const CommandLine&&) = delete;

  ~CommandLine();

  void ParseCommandLine(const std::vector<std::string>& commands, int uid);

 private:
  void OnCommand(std::unique_ptr<FunctionalInterface> worker_functional);
  void Help();
  void Version();

 private:
  Delegate* delegate_;
  scoped_refptr<base::SequencedTaskRunner> task_runner_;
  base::WeakPtrFactory<CommandLine> weak_factory_{this};
};
}  // namespace worker

#endif