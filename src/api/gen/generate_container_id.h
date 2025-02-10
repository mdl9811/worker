//
// Copyright (c) 2025 mdl. All rights reserved.
//

#ifndef API_GEN_GENERATE_CONTAINER_ID_H_
#define API_GEN_GENERATE_CONTAINER_ID_H_
#include <set>
#include <string>
#include <string_view>
#include <vector>
#include "absl/types/span.h"

namespace worker {

// 生成唯一ID
class GenerateContainerID final {
 public:
  GenerateContainerID();
  GenerateContainerID(std::vector<std::string>&& known_ids,
                      std::vector<std::string>&& known_dids);
  ~GenerateContainerID();

  std::string GenerateUniqueId() const;
  std::string GenerateUniqueDId() const;
  uint32_t GenerateUniqueUId() const;

  bool RemoveId(const std::string& id);
  bool RemoveDId(const std::string& id);

  std::string RandomUniqueName(std::set<std::string>&& names);

 private:
  std::string GenerateId(std::set<std::string>& known_ids, int len) const;
  std::string CreateRandomNonZeroStringId(int len) const;
  uint32_t CreateRandomId() const;

  bool Remove(std::set<std::string>& known_ids, const std::string& id);
  bool RNGGenerate(void* buf, size_t len) const;

 private:
  // 容器id
  mutable std::set<std::string> known_ids_;
  // 生成目录id
  mutable std::set<std::string> known_dids_;

  mutable std::set<uint32_t> known_uids_;
};
}  // namespace worker

#endif