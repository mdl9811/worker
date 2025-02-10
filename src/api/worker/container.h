//
// Copyright (c) 2025 mdl. All rights reserved.
//

#ifndef API_WORKER_CONTAINER_H_
#define API_WORKER_CONTAINER_H_
#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include "api/worker/process.h"
#include "base/task/sequenced_task_runner.h"
#include "functional/functional_interface.h"

namespace worker {
class GenerateContainerID;
class GConfigParser;
class ConfigParser;

class Container final : public FunctionalInterface::Delegate {
 public:
  class Delegate {
   public:
    virtual void OnClose() = 0;
    virtual void OnCompletion(const std::string& message) = 0;
  };

  Container(Delegate* delegate,
            GConfigParser* gconfig_parser,
            GenerateContainerID* generator_container_id);
  ~Container();

  void Run(std::unique_ptr<FunctionalInterface> functional);
  void Quit();

  bool InitializeTempContainer(std::unique_ptr<ConfigParser>* config_parser);
  void Remove(const FunctionalInterface::Optional& optional);
  void RemoveNoneContainer();

  const std::string& id() const { return id_; }
  const std::string& rpath() const { return rpath_; }
  // 挂载路径
  const std::string& mpath() const { return mpath_; }

  GConfigParser* gconfig_parser() { return gconfig_parser_; }

 private:
  bool InitializeContainer(const std::string& name,
                           const std::string& id,
                           const std::string& container_id,
                           const std::string& root,
                           std::unique_ptr<ConfigParser>* config_parser);

  bool Mkdir(const std::string& dir);
  void DoRun(std::unique_ptr<FunctionalInterface> functional);
  void DoQuit();
  void HandleContainerInfo(ConfigParser* config_parser);
  bool Release(const std::string& id, bool remove_path);

 private:
  bool OnExit(uint32_t id) override;
  void OnCompletion(const std::string& message) override;
  Container& container() override { return *this; }

 private:
  Delegate* delegate_;
  GConfigParser* gconfig_parser_;
  GenerateContainerID* generator_container_id_;
  scoped_refptr<base::SequencedTaskRunner> task_runner_;
  base::WeakPtrFactory<Container> weak_factory_{this};

  std::unordered_map<uint32_t, std::unique_ptr<FunctionalInterface>>
      functional_;

  std::string container_id_;
  std::string id_;

  std::string rpath_;
  std::string mpath_;
  std::string container_root_path_;
  bool initialze_ = false;
};
}  // namespace worker

#endif