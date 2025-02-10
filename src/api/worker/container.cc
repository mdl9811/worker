#include "api/worker/container.h"
#include <errno.h>
#include <fcntl.h>
#include <linux/limits.h>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <csignal>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <memory>
#include <string>
#include <vector>
#include "api/gen/generate_container_id.h"
#include "api/worker/config_parser.h"
#include "api/worker/file_string.h"
#include "api/worker/process.h"
#include "base/check.h"
#include "base/files/file_path.h"
#include "base/files/file_util.h"
#include "base/functional/bind.h"
#include "base/json/json_reader.h"
#include "base/json/json_writer.h"
#include "base/location.h"
#include "base/logging.h"
#include "base/path_service.h"
#include "base/task/sequenced_task_runner.h"
#include "base/values.h"
#include "functional/functional_interface.h"

namespace {
constexpr char kRootPath[] = "/worker";
constexpr char kOverlayfsLowerdir[] = "/";
constexpr char kOverlayfsWorkdir[] = "/work";
constexpr char kOverlayfsUpperdir[] = "/upper";
constexpr char kOverlayfsMergeddir[] = "/merged";
constexpr char kRoot[] = "root";
}  // namespace

namespace worker {

Container::Container(Delegate* delegate,
                     GConfigParser* gconfig_parser,
                     GenerateContainerID* generator_container_id)
    : delegate_(delegate),
      gconfig_parser_(gconfig_parser),
      generator_container_id_(generator_container_id),
      task_runner_(base::SequencedTaskRunner::GetCurrentDefault()) {}

Container::~Container() = default;

bool Container::InitializeContainer(
    const std::string& name,
    const std::string& id,
    const std::string& container_id,
    const std::string& root,
    std::unique_ptr<ConfigParser>* config_parser) {
  if (!base::PathExists(base::FilePath(root))) {
    if (!Mkdir(root))
      return false;
  }

  const std::string container_path = root + "/" + container_id;
  const std::string workdir = container_path + kOverlayfsWorkdir;
  const std::string upperdir = container_path + kOverlayfsUpperdir;
  std::string mergeddir = container_path + kOverlayfsMergeddir;

  if (!Mkdir(container_path))
    return false;
  if (!Mkdir(workdir))
    return false;
  if (!Mkdir(upperdir))
    return false;
  if (!Mkdir(mergeddir))
    return false;
  char mount_data[256];
  snprintf(mount_data, sizeof(mount_data), "lowerdir=%s,upperdir=%s,workdir=%s",
           kOverlayfsLowerdir, upperdir.c_str(), workdir.c_str());
  if (mount("overlay", mergeddir.c_str(), "overlay", 0, mount_data) == -1) {
    perror("mount overlay");
    return false;
  }

  rpath_ = container_path + "/" + kRoot;
  mpath_ = mergeddir;
  container_root_path_ = container_path;
  id_ = id;
  container_id_ = container_id;

  *config_parser = std::make_unique<ConfigParser>(
      base::FilePath(container_path + "/worker.db"), id_);
  auto cparser = (*config_parser).get();
  if (!cparser->Parse())
    return false;
  cparser->Set(kContainerId, container_id);
  cparser->Set(kContainerRoot, container_root_path_);
  cparser->Set(kMPath, mpath_);
  cparser->Set(kName, name);
  cparser->Set(kId, id);

  return true;
}

bool Container::InitializeTempContainer(
    std::unique_ptr<ConfigParser>* config_parser) {
  return InitializeContainer(
      generator_container_id_->RandomUniqueName(gconfig_parser_->Names()),
      generator_container_id_->GenerateUniqueId(),
      generator_container_id_->GenerateUniqueDId(), kRootPath, config_parser);
}

void Container::Run(std::unique_ptr<FunctionalInterface> functional) {
  task_runner_->PostTask(
      FROM_HERE, base::BindOnce(&Container::DoRun, weak_factory_.GetWeakPtr(),
                                std::move(functional)));
}

void Container::Quit() {
  task_runner_->PostTask(FROM_HERE, base::BindOnce(&Container::DoQuit,
                                                   weak_factory_.GetWeakPtr()));
}

void Container::HandleContainerInfo(ConfigParser* config_parser) {
  if (!config_parser || !gconfig_parser_ || !gconfig_parser_->Ok())
    return;

  config_parser->Build();

  gconfig_parser_->SetParser(config_parser);
  gconfig_parser_->Build();
}

void Container::DoRun(std::unique_ptr<FunctionalInterface> functional) {
  uint32_t id = generator_container_id_->GenerateUniqueUId();
  auto pair = functional_.insert({id, std::move(functional)});
  auto interface = (*pair.first).second.get();
  if (!(interface)->Start(id, this))
    return;

  task_runner_->PostTask(
      FROM_HERE,
      base::BindOnce(&Container::HandleContainerInfo,
                     weak_factory_.GetWeakPtr(), interface->config_parser()));
}

void Container::DoQuit() {
  delegate_ ? delegate_->OnClose() : void();
}

void Container::RemoveNoneContainer() {
  if (!gconfig_parser_ || !gconfig_parser_->Ok())
    return;
  std::vector<std::string> ids;
  gconfig_parser_->Ids(&ids);
  auto& dict = gconfig_parser_->dict();
  for (auto& id : ids) {
    auto container_path = dict.FindDict(id)->FindString(kContainerRoot);
    if (!base::PathExists(base::FilePath(*container_path)))
      gconfig_parser_->Remove(id);
  }
  gconfig_parser_->Build();
}

bool Container::Release(const std::string& id, bool remove_path) {
  if (!gconfig_parser_ || !gconfig_parser_->Ok())
    return false;
  auto& dict = gconfig_parser_->dict();
  if (!dict.FindDict(id)) {
    LOG(WARNING) << "Container ID Faild! " << id;
    return false;
  }

  auto current_dict = dict.FindDict(id);
  auto mpath = current_dict->FindString(kMPath);
  auto container_path = current_dict->FindString(kContainerRoot);

  if (!mpath || !container_path) {
    LOG(ERROR) << "config not exist mpath or container paht";
    return false;
  }
  if (!base::PathExists(base::FilePath(*mpath)) ||
      !base::PathExists(base::FilePath(*container_path))) {
    return false;
  }
  auto config_parser = std::make_unique<ConfigParser>(
      base::FilePath(*container_path + "/worker.db"), id);
  if (!config_parser->Parse()) {
    LOG(ERROR) << "container config lost";
    return false;
  }
  auto pids = config_parser->GetPidList();

  for (auto& pid : pids)
    kill(pid, SIGTERM);

  if (remove_path) {
    umount(mpath->c_str());
    base::FilePath path(*container_path);
    if (!base::DeletePathRecursively(path)) {
      LOG(ERROR) << "delete file faild! PATH: " << path;
      return false;
    }
    gconfig_parser_->Remove(id);
    gconfig_parser_->Build();
    LOG(INFO) << "Remove Container ID:" << id;
  }
  return true;
}

void Container::Remove(const FunctionalInterface::Optional& optional) {
  for (int i = 0; i < optional.ids.size(); i++)
    Release(optional.ids[i], optional.remove_path);
  return;
}

void Container::OnCompletion(const std::string& message) {
  delegate_->OnCompletion(message);
}

bool Container::OnExit(uint32_t id) {
  auto iter = functional_.find(id);
  if (iter == functional_.end())
    return false;
  functional_.erase(iter);
  return true;
}

bool Container::Mkdir(const std::string& dir) {
  base::FilePath path(dir);
  if (!base::CreateDirectory(base::FilePath(path))) {
    LOG(ERROR) << "mkdir faild:" << dir;
    return false;
  }
  return true;
}

}  // namespace worker