//
// Copyright (c) 2025 mdl. All rights reserved.
//

#ifndef ENGINE_SESSION_H_
#define ENGINE_SESSION_H_
#include <memory>
#include "base/run_loop.h"
#include "base/values.h"
#include "api/worker/process.h"

namespace worker {
class Session : public Process::Delegate {
 public:
  explicit Session(base::RunLoop& run_loop);
  ~Session();

  void Run(int argc, const char* argv[], int port);

 private:
  void ParseCommands(int argc, const char* argv[], std::string* body);
  void Reponse(int http_response_code, const std::string& data);

  void OnCompletion(const std::string& message);

 private:
  void Run(base::Value::Dict* dict);
  void Exec(base::Value::Dict* dict);

  bool EnterNamespace(const char* type, int pid);
  void RunBash(int flags,
               int pid,
               const std::string& id,
               base::Value::Dict* dict);

  void Requeset(const std::string& path, const std::string& body);

 private:
  void OnExit(int pid) override;
  bool ExecWithStart() override;

 private:
  int pid_;
  int port_ = 0;
  base::RepeatingClosure quit_closure_;

  std::unique_ptr<Process> process_;

  base::RunLoop& run_loop_;
};
}  // namespace worker

#endif