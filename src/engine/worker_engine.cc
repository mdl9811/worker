#include "engine/worker_engine.h"
#include <csignal>
#include <cstring>
#include <memory>
#include <string>
#include <utility>
#include <vector>
#include "absl/types/span.h"
#include "api/gen/generate_container_id.h"
#include "api/rtc/tcp_server.h"
#include "api/worker/file_string.h"
#include "base/check.h"
#include "base/command_line.h"
#include "base/files/file_path.h"
#include "base/functional/bind.h"
#include "base/json/json_reader.h"
#include "base/location.h"
#include "base/logging.h"
#include "base/task/sequenced_task_runner.h"
#include "base/time/time.h"
#include "engine/command_line.h"
#include "functional/functional_interface.h"
#include "api/worker/config_parser.h"
#include "base/path_service.h"
#include "base/files/file_util.h"
#include "api/rtc/io_buffer.h"
#include "engine/status_code.h"

namespace {
[[maybe_unused]] const char kMimeType_Text[] = "text/plain";
[[maybe_unused]] const char kMimeType_Json[] = "application/json";
}  // namespace

namespace worker {
WorkerEngine::WorkerEngine(base::RunLoop& run_loop)
    : task_runner_(base::SequencedTaskRunner::GetCurrentDefault()),
      run_loop_(run_loop) {
  quit_closure_ = run_loop_.QuitWhenIdleClosure();
}

WorkerEngine::~WorkerEngine() = default;

bool WorkerEngine::InitializeContainer(int port) {
  std::vector<std::string> ids, container_ids;
  config_parser_->Ids(&ids);
  config_parser_->ContainerIds(&container_ids);
  generate_container_id_.reset(
      new GenerateContainerID(std::move(ids), std::move(container_ids)));
  container_.reset(
      new Container(this, config_parser_.get(), generate_container_id_.get()));
  tcp_server_.reset(new TCPServer(this, &http1_processor_));
  LOG(INFO) << "listen port:" << port;
  auto endpoint = net::IPEndPoint(net::IPAddress::IPv4AllZeros(), port);
  if (tcp_server_->InitializeAndListen(endpoint) != net::OK)
    return false;
  Setup();
  tcp_server_->Run();

  check_config_timer_.Start(FROM_HERE, base::Seconds(1), this,
                            &WorkerEngine::CheckConfig);
  return true;
}

bool WorkerEngine::Initialize(int argc, const char* argv[], int port) {
  config_parser_.reset(new GConfigParser(DefaultConfigPath()));
  if (!config_parser_->Parse())
    return false;
  ScanConfig(config_parser_.get());
  command_line_.reset(new CommandLine(this));
  return InitializeContainer(port);
}

void WorkerEngine::CheckConfig() {
  ScanConfig(config_parser_.get());
}

void WorkerEngine::ScanConfig(ConfigParser* config_parser) {
  auto pid_list = config_parser->dict().FindList(kPids);
  for (auto it = pid_list->begin(); it != pid_list->end();) {
    auto pid = it->GetInt();
    if (kill(pid, 0) == 0) {
      it++;
    } else {
      it = pid_list->erase(it);
    }
  }
  if (pid_list->empty())
    config_parser->Set(kStatus, ContainerCode::Exit);
}

void WorkerEngine::ScanConfig(GConfigParser* config_parser) {
  auto& dict = config_parser->dict();
  for (auto it = dict.begin(); it != dict.end();) {
    auto& d = it->second;
    auto& cdict = d.GetDict();
    auto config_path = *cdict.FindString(kContainerRoot) + "/worker.db";
    if (base::PathExists(base::FilePath(config_path))) {
      auto parser = std::make_unique<ConfigParser>(base::FilePath(config_path),
                                                   it->first);
      parser->SetName(*cdict.FindString(kName));
      if (parser->Parse()) {
        ScanConfig(parser.get());
        parser->Build();
        config_parser->SetParser(parser.get());
      }
      it++;
    } else {
      it = dict.erase(it);
    }
  }
  config_parser->Build();
}

void WorkerEngine::Setup() {
#define SET_HANDLER(path, secure, function) \
  http1_processor_.SetHandler(              \
      path, secure,                         \
      base::BindRepeating(&WorkerEngine::function, base::Unretained(this)));
  SET_HANDLER("/command", false, HandleCommand);
  SET_HANDLER("/pmessage", false, HandleProcessMessage);
}

void WorkerEngine::HandleProcessMessage(const worker::HttpRequestInfo& request,
                                        OnCompletionCallback callback) {
  HttpResponseInfo response;
  response.status_code(net::HTTP_OK);

  auto body = request.body;
  if (body) {
    auto json = base::JSONReader::Read(body.value());
    auto& dict = json->GetDict();
    std::string id = *dict.FindString("cid");
    int pid = dict.FindInt("pid").value();
    auto parser = config_parser_->Get(id);
    if (!parser) {
      response.SetContentHeaders(IOBuffer::New("Faild"), kMimeType_Text);
    } else {
      parser->dict().FindList(kPids)->Append(pid);
      parser->Build();
      config_parser_->SetParser(parser.get());
      config_parser_->Build();
      response.SetContentHeaders(IOBuffer::New("OK"), kMimeType_Text);
    }
  }

  std::move(callback).Run(response);
  return;
}

void WorkerEngine::HandleCommand(const worker::HttpRequestInfo& request,
                                 OnCompletionCallback callback) {
  callback_ = std::move(callback);
  auto body = request.body;
  commands.clear();
  do {
    if (!body)
      break;
    auto json = base::JSONReader::Read(body.value());
    if (!json)
      break;
    auto dict = json->GetIfDict();
    if (!dict)
      break;
    auto list = dict->FindList("Args");
    if (!list)
      break;
    for (auto it = list->begin(); it != list->end(); it++)
      commands.push_back(it->GetString());
    command_line_->ParseCommandLine(commands, dict->FindInt("Uid").value());
  } while (false);
}

void WorkerEngine::Run(int argc, const char* argv[], int port) {
  if (!Initialize(argc, argv, port))
    return;
  run_loop_.Run();
}

void WorkerEngine::OnCommand(
    std::unique_ptr<FunctionalInterface> worker_functional) {
  container_->Run(std::move(worker_functional));
}

void WorkerEngine::OnCompletion(const std::string& message) {
  HttpResponseInfo response;
  response.status_code(net::HTTP_OK);
  response.SetContentHeaders(IOBuffer::New(message), kMimeType_Text);
  std::move(callback_).Run(response);
}

void WorkerEngine::OnClose() {
  task_runner_->PostTask(
      FROM_HERE,
      base::BindOnce([](WorkerEngine* engine) { engine->quit_closure_.Run(); },
                     base::Unretained(this)));
}

base::FilePath WorkerEngine::DefaultConfigPath() {
  base::FilePath path;
  if (!base::PathService::Get(base::FILE_EXE, &path)) {
    LOG(ERROR) << "exe path get faild!";
    return path;
  }
  path = path.DirName().AppendASCII("../config/");
  if (!base::PathExists(path))
    base::CreateDirectory(path);
  path = path.AppendASCII("worker.db");
  return path;
}

}  // namespace worker