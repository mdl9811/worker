//
// Copyright (c) 2025 mdl. All rights reserved.
//

#ifndef API_RTC_CERT_VERIFIER_H_
#define API_RTC_CERT_VERIFIER_H_

#include <list>
#include <memory>
#include <string>

#include "base/callback_list.h"
#include "net/base/completion_once_callback.h"
#include "net/cert/cert_verifier.h"
#include "net/cert/cert_verify_result.h"

namespace worker {
class CertVerifier : public net::CertVerifier {
 public:
  CertVerifier();
  ~CertVerifier() override;

  int Verify(const RequestParams& params,
             net::CertVerifyResult* verify_result,
             net::CompletionOnceCallback callback,
             std::unique_ptr<Request>* out_req,
             const net::NetLogWithSource& net_log) override;
  void SetConfig(const Config& config) override {}
  void AddObserver(Observer* observer) override {}
  void RemoveObserver(Observer* observer) override {}
  void set_default_result(int default_result) {
    default_result_ = default_result;
  }

  void set_async(bool async) { async_ = async; }
  void AddResultForCert(scoped_refptr<net::X509Certificate> cert,
                        const net::CertVerifyResult& verify_result,
                        int rv);

  void AddResultForCertAndHost(scoped_refptr<net::X509Certificate> cert,
                               const std::string& host_pattern,
                               const net::CertVerifyResult& verify_result,
                               int rv);
  void ClearRules();

 private:
  struct Rule;
  using RuleList = std::list<Rule>;
  class MockRequest;
  friend class MockRequest;

  int VerifyImpl(const RequestParams& params,
                 net::CertVerifyResult* verify_result);

  int default_result_;
  RuleList rules_;
  bool async_;

  base::OnceClosureList request_list_;
};
}  // namespace rtc

#endif  // WELINK_RTC_CERT_VERIFIER_H_
