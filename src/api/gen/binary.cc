#include "api/gen/binary.h"
#include <fstream>
#include <ios>
#include <iostream>
#include <ostream>
#include "base/files/file_path.h"
#include "base/files/file_util.h"
#include "base/logging.h"
#include "base/path_service.h"
#include "build/build_config.h"
#include "build/buildflag.h"
namespace worker {
static void io_end(std::fstream* io, int k) {
  if (!io || k <= 0)
    return;
  *io << ", ";
  if (k % 8 == 0)
    *io << "\n";
}

static int binary(const char* path,
                  const char* name,
                  const char* data,
                  int size) {
  base::FilePath file_path(path);
  const auto dir_name = file_path.DirName();
  if (!base::PathExists(dir_name))
    base::CreateDirectory(dir_name);

  std::fstream io(path, std::ios::out | std::ios::trunc);
  if (!io.is_open())
    return -1;

  io << "const unsigned char " << name << "[" << size << "] = {" << std::endl;

  int pos = 0;
  while (pos < size) {
    io_end(&io, pos);
    io << "0x" << std::hex << std::setw(2) << std::setfill('0')
       << (static_cast<int>(data[pos++]) & 0xFF);
  }

  io << std::endl << "};" << std::endl;
  io << "const unsigned int k" << name << " = " << std::dec << size << ";"
     << std::endl;

  io.close();
  return 0;
}

int gen_binary(const char* path, const char* name, const char* data, int size) {
  if (!path || !name || !data || (size <= 0))
    return -1;
  return binary(path, name, data, size);
}
}  // namespace worker