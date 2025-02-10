#include "api/worker/config_parser.h"
#include <string>
#include <string_view>
#include <utility>
#include <vector>
#include "base/check.h"
#include "base/files/file_path.h"
#include "base/json/json_reader.h"
#include "base/json/json_writer.h"
#include "base/files/file_util.h"
#include "base/logging.h"
#include "api/worker/file_string.h"
#include "base/values.h"

namespace worker {
// base
ConfigParserBase::ConfigParserBase(const base::FilePath& path) : path_(path) {}
ConfigParserBase::ConfigParserBase(const base::FilePath& path,
                                   base::Value::Dict dict)
    : path_(path), dict_(dict.Clone()) {}
ConfigParserBase::~ConfigParserBase() = default;

bool ConfigParserBase::Parse() {
  if (!base::PathExists(path_))
    return ok_ = true;
  std::string contents;
  if (!base::ReadFileToString(base::MakeAbsoluteFilePath(path_), &contents)) {
    LOG(ERROR) << "ReadFileToString faild!";
    return false;
  }
  auto read_json = base::JSONReader::Read(contents);
  if (!read_json) {
    LOG(ERROR) << "config not json file!";
    return false;
  }
  if (!read_json->GetIfDict()) {
    LOG(ERROR) << "Config not json list!";
    return false;
  }
  dict_ = read_json->GetDict().Clone();
  return ok_ = config_exist_ = true;
}

base::Value::Dict::iterator ConfigParserBase::Find(std::string_view id) {
  for (auto iter = dict_.begin(); iter != dict_.end(); iter++) {
    if (iter->first == id)
      return iter;
  }
  return dict_.end();
}

void ConfigParserBase::Set(std::string_view key, std::string_view value) {
  if (Ok())
    dict_.Set(key, value);
  return;
}

void ConfigParserBase::Set(std::string_view key,
                           const base::Value::Dict& dict) {
  if (Ok())
    dict_.Set(key, dict.Clone());
  return;
}

void ConfigParserBase::Set(std::string_view key, int value) {
  if (Ok())
    dict_.Set(key, value);
  return;
}

void ConfigParserBase::Remove(std::string_view id) {
  if (!Ok() || dict_.empty())
    return;
  auto iter = Find(id);
  if (iter == dict_.end()) {
    return;
  }
  dict_.erase(iter);
  return;
}

bool ConfigParserBase::Reset(const base::Value::Dict& dict) {
  std::string data;
  if (!base::JSONWriter::WriteWithOptions(
          dict, base::JSONWriter::Options::OPTIONS_PRETTY_PRINT, &data) ||
      !base::WriteFile(path_, data)) {
    LOG(ERROR) << "WriteFile faild! path: " << path_.value();
    return false;
  }
  return true;
}

bool ConfigParserBase::Build(std::string_view id,
                             const base::Value::Dict& dict) {
  if (!Ok() || dict.empty())
    return false;
  dict_.Set(id, dict.Clone());
  return Reset(dict_.Clone());
}

bool ConfigParserBase::Build() {
  if (!Ok())
    return false;
  return Reset(dict_.Clone());
}

void ConfigParserBase::Clear() {
  dict_.clear();
}

// current
ConfigParser::ConfigParser(const base::FilePath& path, std::string_view id)
    : ConfigParserBase(path), id_(id) {}
ConfigParser::ConfigParser(base::Value::Dict dict, std::string_view id)
    : ConfigParserBase(
          base::FilePath(*dict.FindString(kContainerRoot) + "/worker.db"),
          dict.Clone()),
      id_(id) {}
ConfigParser::~ConfigParser() = default;

bool ConfigParser::Parse() {
  if (!ConfigParserBase::Parse())
    return false;
  Set(kName, name_);
  if (!dict_.FindList(kPids))
    dict_.Set(kPids, base::Value::List());
  return true;
}

void ConfigParser::SetPid(int pid) {
  if (!Ok())
    return;
  auto list = dict_.FindList(kPids);
  list->Append(pid);
}

void ConfigParser::RmmovePid(int pid) {
  if (!Ok())
    return;
  auto list = dict_.FindList(kPids);
  for (auto it = list->begin(); it != list->end(); it++) {
    if (it->GetInt() == pid) {
      list->erase(it);
      break;
    }
  }
}

std::vector<int> ConfigParser::GetPidList() {
  if (!Ok())
    return {};
  std::vector<int> pids;
  auto list = dict_.FindList(kPids);

  if (!list)
    return pids;
  for (auto it = list->begin(); it != list->end(); it++)
    pids.push_back(it->GetInt());
  return pids;
}

// global
GConfigParser::GConfigParser(const base::FilePath& path)
    : ConfigParserBase(path) {}
GConfigParser::~GConfigParser() = default;

void GConfigParser::SetParser(ConfigParser* parser) {
  Set(parser->id(), parser->dict());
}

bool GConfigParser::Ids(std::vector<std::string>* known_ids) const {
  return KnownIds(kId, known_ids);
}

bool GConfigParser::TempIds(std::vector<std::string>* known_ids) const {
  for (auto iter = dict_.begin(); iter != dict_.end(); iter++) {
    const auto& dict = iter->second;
    if (!dict.is_dict())
      continue;
    if (*dict.GetDict().FindString(kType) != "TEMP")
      continue;
    if (auto id = dict.GetDict().FindString(kId)) {
      known_ids->push_back(*id);
    }
  }
  return true;
}

bool GConfigParser::ContainerIds(std::vector<std::string>* known_ids) const {
  return KnownIds(kContainerId, known_ids);
}

std::set<std::string> GConfigParser::Names() const {
  std::set<std::string> names;
  for (auto iter = dict_.begin(); iter != dict_.end(); iter++) {
    const auto& name = iter->first;
    names.insert(name);
  }
  return names;
}

bool GConfigParser::KnownIds(std::string_view key,
                             std::vector<std::string>* known_ids) const {
  for (auto iter = dict_.begin(); iter != dict_.end(); iter++) {
    const auto& dict = iter->second;
    if (!dict.is_dict())
      continue;
    if (auto id = dict.GetDict().FindString(key)) {
      known_ids->push_back(*id);
    }
  }
  return true;
}

std::unique_ptr<ConfigParser> GConfigParser::Get(std::string_view id) const {
  auto dict = dict_.FindDict(id);
  if (dict)
    return std::make_unique<ConfigParser>(dict->Clone(), id);
  return nullptr;
}
}  // namespace worker
