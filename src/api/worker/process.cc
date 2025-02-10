#include "api/worker/process.h"

#include "base/check_op.h"
#include "base/functional/bind.h"
#include "base/location.h"
#include "base/logging.h"
#include "base/task/sequenced_task_runner.h"
#include "base/time/time.h"
#include <sched.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>
#include <sys/wait.h>
#include <signal.h>
#include <unistd.h>
#include <sys/utsname.h>
#include <sys/capability.h>
#include <sched.h>
#include <sys/types.h>
#include <sys/mount.h>
#include <sys/syscall.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <linux/limits.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "api/worker//file_string.h"
#include "base/files/file_path.h"
#include "base/path_service.h"
#include "base/files/file_util.h"

namespace worker {
const int kStackSize = 1024 * 1024;
bool Process::Delegate::ExecWithStart() {
  return true;
}

Process::Process(Delegate* delegate,
                 const std::string& id,
                 std::vector<std::string> args)
    : stack_((char*)malloc(kStackSize)),
      task_runner_(base::SequencedTaskRunner::GetCurrentDefault()),
      delegate_(delegate),
      id_(id),
      args_(args) {}

Process::~Process() {
  CleanUp();
}

void Process::SetUidMap(pid_t pid, int inside_uid, int outside_uid) {
  char map_path[PATH_MAX];
  char map_buf[256];
  int map_fd;
  snprintf(map_path, sizeof(map_path), "/proc/%d/uid_map", pid);
  map_fd = open(map_path, O_WRONLY);
  if (map_fd == -1) {
    perror("open uid_map");
    return;
  }
  snprintf(map_buf, sizeof(map_buf), "%d %d 1000\n", inside_uid, outside_uid);

  if (write(map_fd, map_buf, strlen(map_buf)) == -1) {
    perror("write uid_map");
    return;
  }

  close(map_fd);
}

void Process::SetGidMap(pid_t pid, int inside_gid, int outside_gid) {
  char map_path[PATH_MAX];
  char map_buf[256];
  int map_fd;

  snprintf(map_path, sizeof(map_path), "/proc/%d/gid_map", pid);
  map_fd = open(map_path, O_WRONLY);
  if (map_fd == -1) {
    perror("open gid_map");
    return;
  }

  snprintf(map_buf, sizeof(map_buf), "%d %d 1000\n", inside_gid, outside_gid);
  if (write(map_fd, map_buf, strlen(map_buf)) == -1) {
    perror("write gid_map");
    exit(EXIT_FAILURE);
  }

  close(map_fd);
}

bool Process::Start() {
  pid_ = clone(&Process::RunMain, stack_ + kStackSize,
               CLONE_NEWUTS | CLONE_NEWIPC | CLONE_NEWPID | CLONE_NEWNET |
                   CLONE_NEWNS | CLONE_NEWUSER | SIGCHLD | CLONE_PTRACE,
               this);
  if (pid_ == -1) {
    ok_ = false;
    perror("clone");
    return false;
  }
  SetUidMap(pid_, 0, getuid());
  SetGidMap(pid_, 0, getgid());
  return ok_ = true;
}

bool Process::StartNotNamespase() {
  pid_ = clone(&Process::RunMainNotNamespase, stack_ + kStackSize,
               SIGCHLD | CLONE_PTRACE, this);
  if (pid_ == -1) {
    ok_ = false;
    perror("clone");
    return false;
  }
  return ok_ = true;
}

bool Process::Mkdir(const std::string& dir) {
  if (mkdir(dir.c_str(), 0755) == -1) {
    perror(dir.c_str());
    return false;
  }
  return true;
}

void Process::SetPath(const std::string& rpath, const std::string& mpath) {
  rpath_ = rpath;
  mpath_ = mpath;
}

void Process::HandleProcessMessage() {
  if (!delegate_ || rpath_.empty() || mpath_.empty())
    return;
  if (!Mkdir(rpath_.c_str()))
    return;
  if (mount(mpath_.c_str(), rpath_.c_str(), NULL,
            MS_BIND, NULL) == -1) {
    perror("mount root");
    return;
  }
  const std::string old_root = rpath_ + "/old_root";
  if (!Mkdir(old_root.c_str()))
    return;
  // 切换到新根目录
  if (chdir(rpath_.c_str()) == -1) {
    perror("chdir new root");
    return;
  }
  // 更新 根目录
  if (syscall(SYS_pivot_root, ".", "./old_root") == -1) {
    perror("pivot_root");
    return;
  }
  // 执行 pivot_root，将新根目录设置为 new_root，旧根目录放在 old_root 中
  if (mount("proc", "/proc", "proc", 0, "") == -1) {
    perror("mount proc");
    return;
  }
  // 更改工作目录到新根目录
  if (chdir("/") == -1) {
    perror("chdir /");
    return;
  }

  // 后面俩步操作 隐藏老的root
  //  卸载旧根目录
  if (umount2("/old_root", MNT_DETACH) == -1) {
    perror("umount old root");
    return;
  }

  // 删除旧根目录
  if (rmdir("/old_root") == -1) {
    perror("rmdir old root");
    return;
  }

  base::FilePath path(kEtcHostPath);
  if (base::PathExists(path))
    remove(kEtcHostPath);
  char hosts[1024];
  int size = snprintf(hosts, sizeof(hosts), kEtcHostsFormat, id_.c_str());
  base::WriteFile(path, {hosts, static_cast<size_t>(size)});
  write_file(kEtcHostPath, hosts, size);
  sethostname(id_.c_str(), id_.size());
}

void Process::Wait() {
  if (!Ok())
    return;
  task_runner_->PostTask(
      FROM_HERE, base::BindOnce(&Process::DoWait, weak_factory_.GetWeakPtr()));
}

void Process::DoWait() {
  switch (waitpid(pid_, NULL, WNOHANG)) {
    case 0:
      task_runner_->PostDelayedTask(
          FROM_HERE,
          base::BindOnce(&Process::DoWait, weak_factory_.GetWeakPtr()),
          base::Milliseconds(20));
      break;
    default:
      delegate_ ? delegate_->OnExit(pid_) : void();
  }
}

void Process::CleanUp() {
  stack_ ? free(stack_) : void();
  stack_ = nullptr;
}

int Process::RunMainNotNamespase(void* arg) {
  auto process = reinterpret_cast<Process*>(arg);
  if (!process || process->args_.empty())
    return -1;
  if (!process->delegate_->ExecWithStart())
    return -1;
  for (auto fd : process->fds_)
    close(fd);
  auto& argv = process->args_;

  std::vector<char*> child_args;
  child_args.reserve(argv.size() + 1);

  for (size_t i = 0; i < argv.size(); ++i)
    child_args.push_back(const_cast<char*>(argv[i].c_str()));

  child_args.push_back(nullptr);

  if (execv(child_args[0], child_args.data()) < 0) {
    perror("execv");
  }
  return 0;
}

int Process::RunMain(void* arg) {
  auto process = reinterpret_cast<Process*>(arg);
  if (!process || process->args_.empty())
    return -1;

  process->HandleProcessMessage();
  for (auto fd : process->fds_)
    close(fd);

  auto& argv = process->args_;
  std::vector<char*> child_args;
  child_args.reserve(argv.size() + 1);

  for (size_t i = 0; i < argv.size(); ++i)
    child_args.push_back(const_cast<char*>(argv[i].c_str()));

  child_args.push_back(nullptr);
  int dev_null = open("/dev/null", O_WRONLY);
  if (dev_null == -1) {
    perror("open failed");
    return 1;
  }
  dup2(dev_null, STDOUT_FILENO);
  dup2(dev_null, STDERR_FILENO);
  close(dev_null);
  if (execv(child_args[0], child_args.data()) < 0) {
    perror("execv");
  }

  return 0;
}

}  // namespace worker