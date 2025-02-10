#include "api/gen/message_build.h"
#include "base/json/json_writer.h"

[[maybe_unused]] const char kType[] = "type";
[[maybe_unused]] const char kMessage[] = "message";
[[maybe_unused]] const char kKeyPid[] = "pid";
[[maybe_unused]] const char kKeyTTY[] = "tty";
[[maybe_unused]] const char kKeyID[] = "id";
[[maybe_unused]] const char kKeyArgs[] = "Args";

namespace worker {
MessageBuild::MessageBuild(Type type) : type_(type) {
  dict_.Set(kType, type_);
}
MessageBuild::~MessageBuild() = default;

std::string MessageBuild::Build(Type type, const std::string& message) {
  base::Value::Dict dict;
  dict.Set(kType, type);
  dict.Set(kMessage, message);
  std::string result;
  base::JSONWriter::Write(dict, &result);
  return result;
}

MessageBuild& MessageBuild::Set(const std::string& key,
                                const std::string& message) {
  dict_.Set(key, message);
  return *this;
}

MessageBuild& MessageBuild::Set(const std::string& key, int message) {
  dict_.Set(key, message);
  return *this;
}

MessageBuild& MessageBuild::Set(const std::string& key,
                                base::Value::List&& list) {
  dict_.Set(key, std::move(list));
  return *this;
}

bool MessageBuild::Build(std::string* result) {
  return base::JSONWriter::Write(dict_, result);
}

}  // namespace worker