//
// Copyright (c) 2025 mdl. All rights reserved.
//

#ifndef VERSION_H_
#define VERSION_H_

// The version of the worker.
// 第一位大版本 第二位小版本 第三位修复bug
// 大版本更新可能会有不兼容的改动 小版本都兼容
#define WORKER_VERSION "1.0.0"

//兼容大版本 例如1 代表版本1和1之前的版本都兼容 之前的都兼容
#define WORKER_VERSION_MAJOR "1"

//chromium 内核版本
#define WORKER_CHROMIUM_MAJOR "132.0.6834.11"

//api版本 用于和worker通信
#define WORKER_API_VERSION "1.0"

//目前支持linux/amd64 未来可能会支持更多平台
#define WORKER_ARCH_OS "linux/amd64"

#endif