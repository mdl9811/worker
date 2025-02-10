//
// Copyright (c) 2025 mdl. All rights reserved.
//

#ifndef FUNCTIONAL_FUNCTIONAL_INTERFACE_H_
#define FUNCTIONAL_FUNCTIONAL_INTERFACE_H_
#include <cstdint>
#include <memory>
#include <string>
#include <string_view>
#include <vector>
#include "api/worker/config_parser.h"
#include "api/worker/process.h"

namespace worker {
class Container;

class FunctionalInterface : public Process::Delegate {
 public:
  using ArgsVector = std::vector<std::string>;

  enum Type {
    kEmpty,
    kRun,
    kRm,
    kExec,
    kPs,
  };

  struct Optional {
    bool destroy = false;
    bool tty = false;
    bool remove_path = true;
    std::vector<std::string> ids;
  };

  class Delegate {
   public:
    virtual bool OnExit(uint32_t id) = 0;
    virtual void OnCompletion(const std::string& message) = 0;
    virtual Container& container() = 0;
  };

  explicit FunctionalInterface(Type type);
  virtual ~FunctionalInterface();

  const ArgsVector& args() const { return args_vector_; }
  bool Ok() const;

  void HandleCommand(const std::vector<std::string>& commands);
  void ErrorMessage(std::string* message);
  void AddArg(const std::string& arg);
  ConfigParser* config_parser() const { return config_parser_.get(); }

  Type type() const {return type_;}

  const Optional& optional() const { return optional_; }

  virtual bool Start(uint32_t id, Delegate* delegate) = 0;

 protected:
  virtual bool InitializeCommand(const std::vector<std::string>& commands);
  virtual bool ParseCommand(const std::vector<std::string>& commands, int* pos);

  void AddErrorMessage(const std::string_view message);
  void AddLine(const std::string& message);
  void AddLine();

  virtual bool Help();
  virtual void OnExit(int pid) override;

 protected:
  Delegate* delegate_;
  const Type type_;
  uint32_t id_;

  std::string error_message_;
  std::string message_;

  ArgsVector args_vector_;

  std::unique_ptr<Process> process_;

  std::unique_ptr<ConfigParser> config_parser_;

  bool ok_ = false;
  bool quit = false;
  mutable FunctionalInterface::Optional optional_;
};
}  // namespace worker

#endif