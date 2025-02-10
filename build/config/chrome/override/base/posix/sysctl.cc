// Copyright 2023 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/posix/sysctl.h"

#include <sys/sysctl.h>

#include <initializer_list>
#include <optional>
#include <string>

#include "base/check_op.h"
#include "base/functional/function_ref.h"
#include "base/numerics/safe_conversions.h"
#include "build/build_config.h"

namespace {

std::optional<std::string> StringSysctlImpl(
    base::FunctionRef<int(char* /*out*/, size_t* /*out_len*/)> sysctl_func) {
  size_t buf_len;
  int result = sysctl_func(nullptr, &buf_len);
  if (result < 0 || buf_len < 1) {
    return std::nullopt;
  }

  std::string value(buf_len - 1, '\0');
  result = sysctl_func(&value[0], &buf_len);
  if (result < 0) {
    return std::nullopt;
  }
  CHECK_LE(buf_len - 1, value.size());
  CHECK_EQ(value[buf_len - 1], '\0');
  value.resize(buf_len - 1);

  return value;
}

#if BUILDFLAG(IS_LINUX)
int sysctlbyname(const char* name,
                 void* oldp,
                 size_t* oldlenp,
                 const void* newp,
                 size_t newlen) {
  char path[256];
  FILE* fp;

  // 构建文件路径
  snprintf(path, sizeof(path), "/proc/sys/%s", name);

  // 如果要获取参数
  if (oldp != NULL) {
    // 打开文件以读取
    fp = fopen(path, "r");
    if (fp == NULL) {
      return -1;  // 失败
    }

    // 读取值
    if (fread(oldp, 1, *oldlenp, fp) <= 0) {
      fclose(fp);
      return -1;  // 失败
    }

    fclose(fp);

    // 如果 oldlenp 不为 NULL，更新其值的大小
    if (oldlenp != NULL) {
      *oldlenp = strlen((char*)oldp);
    }
  }

  // 如果要设置参数
  if (newp != NULL) {
    // 打开文件以写入
    fp = fopen(path, "w");
    if (fp == NULL) {
      return -1;  // 失败
    }

    // 写入新值
    if (fwrite(newp, 1, newlen, fp) < newlen) {
      fclose(fp);
      return -1;  // 失败
    }

    fclose(fp);
  }

  return 0;  // 成功
}
#endif

}  // namespace

namespace base {

std::optional<std::string> StringSysctl(const std::initializer_list<int>& mib) {
  return StringSysctlImpl([mib](char* out, size_t* out_len) {
    return sysctl(const_cast<int*>(std::data(mib)),
                  checked_cast<unsigned int>(std::size(mib)), out, out_len,
                  nullptr, 0);
  });
}

#if !BUILDFLAG(IS_OPENBSD)
std::optional<std::string> StringSysctlByName(const char* name) {
  return StringSysctlImpl([name](char* out, size_t* out_len) {
    return sysctlbyname(name, out, out_len, nullptr, 0);
  });
}
#endif

}  // namespace base
