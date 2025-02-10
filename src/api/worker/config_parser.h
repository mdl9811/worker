//
// Copyright (c) 2025 mdl. All rights reserved.
//

#ifndef API_WORKER_CONFIG_PARSER_H_
#define API_WORKER_CONFIG_PARSER_H_

#include <map>
#include <memory>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>
#include "base/files/file_path.h"
#include "base/json/values_util.h"
#include "base/values.h"
#include <set>

namespace worker {
class ConfigParserBase {
 public:
  explicit ConfigParserBase(const base::FilePath& path);
  ConfigParserBase(const base::FilePath& path, base::Value::Dict dict);
  ~ConfigParserBase();

  bool Ok() const { return ok_; }
  bool Exist() const { return config_exist_; }

  bool Parse();

  base::Value::Dict& dict() { return dict_; }

  void Set(std::string_view key, std::string_view value);
  void Set(std::string_view key, int value);
  void Set(std::string_view key, const base::Value::Dict& dict);
  void Remove(std::string_view id);

  // 写入json
  bool Reset(const base::Value::Dict& dict);

  // 构建json {{}} 添加新的dict 到全局字典中
  bool Build(std::string_view id, const base::Value::Dict& dict);
  bool Build();

  void Clear();

 protected:
  base::Value::Dict::iterator Find(std::string_view id);

 protected:
  const base::FilePath path_;
  base::Value::Dict dict_;

  bool ok_;
  bool config_exist_ = false;
};

class ConfigParser final : public ConfigParserBase {
 public:
  ConfigParser(const base::FilePath& path, std::string_view id);
  ConfigParser(base::Value::Dict dict, std::string_view id);
  ~ConfigParser();

  bool Parse();

  void SetName(std::string_view name) { name_ = name; }
  void SetPid(int pid);

  std::vector<int> GetPidList();

  void RmmovePid(int pid);

  const std::string& name() const { return name_; }
  const std::string& id() const { return id_; }

 private:
  std::string name_;
  const std::string id_;
};

class GConfigParser final : public ConfigParserBase {
 public:
  explicit GConfigParser(const base::FilePath& path);
  ~GConfigParser();

  void SetParser(ConfigParser* parser);
  bool Ids(std::vector<std::string>* known_ids) const;
  bool TempIds(std::vector<std::string>* known_ids) const;
  bool ContainerIds(std::vector<std::string>* known_ids) const;
  std::set<std::string> Names() const;

  std::unique_ptr<ConfigParser> Get(std::string_view id) const;

 private:
  bool KnownIds(std::string_view key,
                std::vector<std::string>* known_ids) const;
};
}  // namespace worker

#endif