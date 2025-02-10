#include "engine/session.h"
#include <cstdio>
#include <vector>
#include "api/gen/message_build.h"
#include "api/rtc/url_request.h"
#include "base/functional/bind.h"
#include "base/json/json_writer.h"
#include "base/location.h"
#include "base/strings/stringprintf.h"
#include "base/task/sequenced_task_runner.h"
#include "base/types/expected.h"
#include "base/values.h"
#include "url/gurl.h"
#include "base/json/json_reader.h"
#include "api/gen/message_build.h"
#include <sched.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>


namespace  {
static const char kCommand[] = "command";
static const char kPMessage[] = "pmessage";
}

namespace worker {

Session::Session(base::RunLoop& run_loop) : run_loop_(run_loop) {
  quit_closure_ = run_loop_.QuitWhenIdleClosure();
}

Session::~Session() = default;

void Session::Reponse(int http_response_code, const std::string& data) {
  if (http_response_code != 200) {
    printf("workerd not start or port faild!\n");
    quit_closure_.Run();
    return;
  }
  OnCompletion(data);
}

void Session::OnCompletion(const std::string& message) {
  auto json = base::JSONReader::Read(message);
  if (!json)
    return;
  auto& dict = json->GetDict();
  auto type = dict.FindInt(::kType);
  if (!type)
    return;
  switch (type.value()) {
    case MessageBuild::kEcho:
      printf("%s\n", dict.FindString(::kMessage)->c_str());
      quit_closure_.Run();
      break;
    case MessageBuild::kRun:
      Run(&dict);
      break;
    case MessageBuild::kExec:
      Exec(&dict);
      break;
  }
}

bool Session::EnterNamespace(const char* type, int pid) {
  char ns_path[256];
  snprintf(ns_path, sizeof(ns_path), "/proc/%d/ns/%s", pid, type);
  int ns_fd = open(ns_path, O_RDONLY);
  if (ns_fd == -1) {
    perror("open ns failed");
    return false;
  }

  // 进入指定命名空间
  if (setns(ns_fd, 0) == -1) {
    perror("setns failed");
    close(ns_fd);
    return false;
  }
  close(ns_fd);
  return true;
}

void Session::RunBash(int flags,
                      int pid,
                      const std::string& id,
                      base::Value::Dict* dict) {
  if (flags) {
    pid_ = pid;
    auto list = dict->FindList(kKeyArgs);
    if (!list && list->empty()) {
      printf("Parameter error\n");
      return;
    }

    std::vector<std::string> argv;
    for (auto it = list->begin(); it != list->end(); it++) {
      argv.push_back(it->GetString());
    }

    process_ = std::make_unique<Process>(this, "", argv);
    process_->StartNotNamespase();
    process_->Wait();

    std::string body;
    base::Value::Dict dict;
    // 容器id
    dict.Set("cid", id);
    // 进程pid
    dict.Set("pid", process_->Pid());
    base::JSONWriter::Write(dict, &body);
    Requeset(kPMessage, body);
    return;
  }
  quit_closure_.Run();
}

void Session::Exec(base::Value::Dict* dict) {
  if (!dict->FindString(::kMessage))
    return;
  printf("%s\n", dict->FindString(::kMessage)->c_str());
  RunBash(dict->FindInt(::kKeyTTY).value(), dict->FindInt(::kKeyPid).value(),
          *dict->FindString(kKeyID), dict);
}

void Session::Run(base::Value::Dict* dict) {
  if (!dict->FindString(::kMessage))
    return;
  printf("%s\n", dict->FindString(::kMessage)->c_str());
  RunBash(dict->FindInt(::kKeyTTY).value(), dict->FindInt(::kKeyPid).value(),
          *dict->FindString(kKeyID), dict);
}

void Session::OnExit(int pid) {
  quit_closure_.Run();
}

bool Session::ExecWithStart() {
  return EnterNamespace("pid", pid_) && EnterNamespace("ipc", pid_) &&
         EnterNamespace("net", pid_) && EnterNamespace("uts", pid_) &&
         EnterNamespace("user", pid_) && EnterNamespace("mnt", pid_);
}

void Session::ParseCommands(int argc, const char* argv[], std::string* body) {
  base::Value::Dict dict;
  base::Value::List list;
  for (int i = 1; i < argc; i++)
    list.Append(argv[i]);

  dict.Set("Args", std::move(list));
  dict.Set("Uid", static_cast<int>(getuid()));
  base::JSONWriter::Write(dict, body);
}

void Session::Requeset(const std::string& path, const std::string& body) {
  worker::URLRequest::Post(
      GURL(base::StringPrintf("http://127.0.0.1:%d/%s", port_, path.c_str())),
      net::HttpRequestHeaders(), body,
      base::BindOnce(&Session::Reponse, base::Unretained(this)),
      base::Seconds(1));
}

void Session::Run(int argc, const char* argv[], int port) {
  port_ = port;
  std::string body;
  ParseCommands(argc, argv, &body);
  Requeset(kCommand, body);
  run_loop_.Run();
}

}  // namespace worker