//
// Copyright (c) 2025 mdl. All rights reserved.
//

#ifndef API_GEN_MESSAGE_BUILD_H_
#define API_GEN_MESSAGE_BUILD_H_

#include <string>
#include "base/values.h"

extern const char kType[];
extern const char kMessage[];
extern const char kKeyPid[];
extern const char kKeyTTY[];
extern const char kKeyID[];
extern const char kKeyArgs[];

namespace worker {
class MessageBuild {
 public:
  enum Type { kEcho, kRun, kRm, kExec, kPs };
  explicit MessageBuild(Type type);
  ~MessageBuild();

  static std::string Build(Type type, const std::string& message);

  MessageBuild& Set(const std::string& key, const std::string& message);
  MessageBuild& Set(const std::string& key, int message);
  MessageBuild& Set(const std::string& key, base::Value::List&& list);

  // 结束
  bool Build(std::string* result);

 private:
  Type type_;
  base::Value::Dict dict_;
};
}  // namespace worker

#endif