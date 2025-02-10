//
// Copyright (c) 2022 mdl. All rights reserved.
//

#include "api/rtc/ssl_config_service.h"

namespace worker {

SSLConfigService::SSLConfigService(const net::SSLContextConfig& config)
    : config_(config) {}

SSLConfigService::~SSLConfigService() = default;

net::SSLContextConfig SSLConfigService::GetSSLContextConfig() {
  return config_;
}

bool SSLConfigService::CanShareConnectionWithClientCerts(
    const std::string_view hostname) const {
  return false;
}

void SSLConfigService::UpdateSSLConfigAndNotify(
    const net::SSLContextConfig& config) {
  config_ = config;
  NotifySSLContextConfigChange();
}
}  // namespace rtc
