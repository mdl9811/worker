//
// Copyright (c) 2025 mdl. All rights reserved.
//

#ifndef API_WORKER_PROCESS_H_
#define API_WORKER_PROCESS_H_

#include <signal.h>
#include <string>
#include <string_view>
#include <vector>
#include "base/task/sequenced_task_runner.h"

namespace worker {
class Process final {
 public:
  class Delegate {
   public:
    virtual void OnExit(int pid) = 0;
    virtual bool ExecWithStart();
  };

  Process(Delegate* delegate,
          const std::string& id,
          std::vector<std::string> args);

  Process(const Process&) = delete;
  Process(const Process&&) = delete;
  Process& operator=(const Process&) = delete;
  Process&& operator=(const Process&&) = delete;

  ~Process();

  bool Start();
  bool StartNotNamespase();
  bool Ok() const { return ok_; }
  int Pid() const { return pid_; }

  void SetPath(const std::string& rpath, const std::string& mpath);
  void SetFds(const std::vector<int>& fds) { fds_ = fds; }
  void Wait();

 private:
  void DoWait();

  bool Mkdir(const std::string& dir);
  void HandleProcessMessage();

  void CleanUp();

  void SetUidMap(pid_t pid, int inside_uid, int outside_uid);
  void SetGidMap(pid_t pid, int inside_gid, int outside_gid);

  static int RunMain(void* arg);
  static int RunMainNotNamespase(void* arg);

 private:
  // 默认大小 1M
  char* stack_ = nullptr;

  // 在子进程中删除 文件描述符
  std::vector<int> fds_;
  scoped_refptr<base::SequencedTaskRunner> task_runner_;
  base::WeakPtrFactory<Process> weak_factory_{this};

  Delegate* delegate_;
  const std::string id_;  // 容器id
  std::vector<std::string> args_;
  std::string rpath_;
  std::string mpath_;

  int pid_ = -1;
  bool ok_ = false;
};
}  // namespace worker

#endif