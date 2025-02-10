#include "api/gen/generate_file.h"
#include "base/files/file.h"
#include "base/files/file_path.h"
#include "base/files/file_util.h"

namespace worker {
GenerateFile::GenerateFile(const std::string& path) : file_path_(path) {}
GenerateFile::~GenerateFile() = default;

void GenerateFile::AddLine(std::string_view line) {
  contents_.append(line);
  contents_.append("\n");
}

void GenerateFile::AddLine() {
  contents_.append("\n");
}

bool GenerateFile::Build() {
  if (file_path_.empty())
    return false;
  base::FilePath path(file_path_);
  if (!base::PathExists(path.DirName()))
    base::CreateDirectory(path.DirName());
  return base::WriteFile(path, contents_);
}

void GenerateFile::Clear() {
  contents_.clear();
}

void GenerateFile::ResetPath(const std::string& path) {
  file_path_ = path;
}

}  // namespace worker