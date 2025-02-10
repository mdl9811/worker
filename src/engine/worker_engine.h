//
// Copyright (c) 2025 mdl. All rights reserved.
//

#ifndef ENGINE_WROKER_ENGINE_H_
#define ENGINE_WROKER_ENGINE_H_
#include <sys/stat.h>
#include <memory>
#include <string>
#include <vector>
#include "api/rtc/tcp_server.h"
#include "base/timer/timer.h"
#include "engine/command_line.h"
#include "api/worker/container.h"

#include "base/functional/callback.h"
#include "base/functional/callback_forward.h"
#include "base/run_loop.h"

#include "base/files/file_path.h"

namespace worker {
class GenerateContainerID;
class GConfigParser;
class TCPServer;

class WorkerEngine final : public CommandLine::Delegate,
                           public Container::Delegate,
                           public TCPServer::Delegate {
 public:
  explicit WorkerEngine(base::RunLoop& run_loop);

  WorkerEngine(const WorkerEngine&) = delete;
  WorkerEngine(const WorkerEngine&&) = delete;
  WorkerEngine& operator=(const WorkerEngine&) = delete;
  WorkerEngine&& operator=(const WorkerEngine&&) = delete;

  ~WorkerEngine();

  void Run(int argc, const char* argv[], int port);

 private:
  void Setup();
  bool Initialize(int argc, const char* argv[], int port);
  bool InitializeContainer(int port);

  base::FilePath DefaultConfigPath();

  void CheckConfig();
  void ScanConfig(GConfigParser* config_parser);
  void ScanConfig(ConfigParser* config_parser);

  void OnCommand(
      std::unique_ptr<FunctionalInterface> worker_functional) override;
  void OnCompletion(const std::string& message) override;
  void OnClose() override;

 private:
  // http
  using OnCompletionCallback = Http1Processor::OnCompletionCallback;
  void HandleCommand(const worker::HttpRequestInfo& request,
                     OnCompletionCallback callback);
  void HandleProcessMessage(const worker::HttpRequestInfo& request,
                            OnCompletionCallback callback);

  OnCompletionCallback callback_;

 private:
  std::vector<std::string> commands;

  scoped_refptr<base::SequencedTaskRunner> task_runner_;
  base::WeakPtrFactory<WorkerEngine> weak_factory_{this};
  std::unique_ptr<worker::CommandLine> command_line_;
  base::RunLoop& run_loop_;

  base::RepeatingClosure quit_closure_;

  base::RepeatingTimer check_config_timer_;

  worker::Http1Processor http1_processor_;
  std::unique_ptr<TCPServer> tcp_server_;
  std::unique_ptr<GConfigParser> config_parser_;
  std::unique_ptr<GenerateContainerID> generate_container_id_;
  std::unique_ptr<Container> container_;
};
}  // namespace worker

#endif