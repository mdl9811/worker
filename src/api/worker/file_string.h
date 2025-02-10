//
// Copyright (c) 2025 mdl. All rights reserved.
//

#ifndef API_WORKER_STRING_H_
#define API_WORKER_STRING_H_

namespace worker {
extern const char kEtcHostPath[];
extern const char kEtcHostsFormat[];
extern const char kMPath[];
extern const char kContainerId[];
extern const char kId[];
extern const char kName[];
extern const char kContainerRoot[];
extern const char kPids[];

extern const char kImage[];
extern const char kCommand[];
extern const char kCreated[];
extern const char kStatus[];
extern const char kType[];
extern const char kPorts[];

bool write_file(const char* file, const char* data, int size);
}  // namespace worker

#endif