#include "api/gen//generate_container_id.h"
#include "base/check.h"
#include "base/check_op.h"
#include "base/logging.h"

#include <openssl/rand.h>
#include <limits>

namespace worker {

//base64 [/] 修改成 z 和原始的base不一样 修改过的
static const char kBase64[64] = {
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M',
    'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
    'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm',
    'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 's', 'z'};

GenerateContainerID::GenerateContainerID() : known_ids_(), known_dids_() {}
GenerateContainerID::GenerateContainerID(std::vector<std::string>&& known_ids,
                                         std::vector<std::string>&& known_dids)
    : known_ids_(known_ids.begin(), known_ids.end()),
      known_dids_(known_dids.begin(), known_dids.end()) {}
GenerateContainerID::~GenerateContainerID() = default;

std::string GenerateContainerID::GenerateUniqueId() const {
  return GenerateId(known_ids_, 12);
}

std::string GenerateContainerID::GenerateUniqueDId() const {
  return GenerateId(known_ids_, 48);
}

uint32_t GenerateContainerID::GenerateUniqueUId() const {
  while (true) {
    auto pair = known_uids_.insert(CreateRandomId());
    if (pair.second) {
      return *pair.first;
    }
  }
}

std::string GenerateContainerID::RandomUniqueName(
    std::set<std::string>&& names) {
  return GenerateId(names, 10);
}

bool GenerateContainerID::RemoveId(const std::string& id) {
  return Remove(known_ids_, id);
}

bool GenerateContainerID::RemoveDId(const std::string& id) {
  return Remove(known_dids_, id);
}

std::string GenerateContainerID::GenerateId(std::set<std::string>& known_ids,
                                            int len) const {
  while (true) {
    auto pair = known_ids.insert(CreateRandomNonZeroStringId(len));
    if (pair.second) {
      return *pair.first;
    }
  }
}

bool GenerateContainerID::Remove(std::set<std::string>& known_ids,
                                 const std::string& id) {
  auto it = known_ids.find(id);
  if (it == known_ids.end())
    return false;
  known_ids.erase(it);
  return true;
}

std::string GenerateContainerID::CreateRandomNonZeroStringId(int len) const {
  std::unique_ptr<uint8_t[]> bytes(new uint8_t[len]);
  if (!RNGGenerate(bytes.get(), len)) {
    LOG(ERROR) << "Faild to generate random string!";
    return "";
  }

  std::string data;
  data.reserve(len);
  for (size_t i = 0; i < len; ++i) {
    data.push_back(kBase64[bytes[i] % 64]);
  }
  return data;
}

uint32_t GenerateContainerID::CreateRandomId() const {
  uint32_t id;
  CHECK(RNGGenerate(&id, sizeof(id)));
  return id;
}

bool GenerateContainerID::RNGGenerate(void* buf, size_t len) const {
  return (RAND_bytes(reinterpret_cast<unsigned char*>(buf), len) > 0);
}
}  // namespace worker