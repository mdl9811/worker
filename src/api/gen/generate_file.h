//
// Copyright (c) 2025 mdl. All rights reserved.
//

#ifndef API_GEN_GENERATE_FILE_H_
#define API_GEN_GENERATE_FILE_H_

#include <string>
#include <string_view>

namespace worker {
class GenerateFile {
 public:
  explicit GenerateFile(const std::string& path);
  ~GenerateFile();

  void AddLine(std::string_view line);
  void AddLine();

  void Clear();
  bool Build();

  void ResetPath(const std::string& path);

 private:
  std::string file_path_;
  std::string contents_;
};
}  // namespace worker

#endif