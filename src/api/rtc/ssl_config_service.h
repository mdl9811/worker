//
// Copyright (c) 2025 mdl. All rights reserved.
//

#ifndef API_RTC_SSL_CONFIG_SERVICE_H_
#define API_RTC_SSL_CONFIG_SERVICE_H_

#include <string_view>
#include "net/ssl/ssl_config_service.h"

namespace worker {

class SSLConfigService : public net::SSLConfigService {
 public:
  explicit SSLConfigService(const net::SSLContextConfig& config);
  ~SSLConfigService() override;

  void UpdateSSLConfigAndNotify(const net::SSLContextConfig& config);

  net::SSLContextConfig GetSSLContextConfig() override;
  bool CanShareConnectionWithClientCerts(
      const std::string_view hostname) const override;

 private:
  net::SSLContextConfig config_;
};

}  // namespace rtc

#endif
