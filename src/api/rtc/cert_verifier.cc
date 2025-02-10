//
// Copyright (c) 2022 mdl. All rights reserved.
//

#include "api/rtc/cert_verifier.h"

#include <memory>
#include <utility>

#include "base/callback_list.h"
#include "base/functional/bind.h"
#include "base/location.h"
#include "base/memory/ref_counted.h"
#include "base/memory/weak_ptr.h"
#include "base/strings/pattern.h"
#include "base/strings/string_util.h"
#include "base/task/sequenced_task_runner.h"
std::unordered_map<std::string, int> error_codes({
#define NET_ERROR(label, value) {#label, value},
#include "net/base/net_error_list.h"
#undef NET_ERROR
});
#include "net/base/net_errors.h"
#include "net/cert/cert_status_flags.h"
#include "net/cert/cert_verify_result.h"
#include "net/cert/x509_certificate.h"

namespace worker {

net::CertStatus MapNetErrorToCertStatus(int error) {
  switch (error) {
    case net::ERR_CERT_COMMON_NAME_INVALID:
      return net::CERT_STATUS_COMMON_NAME_INVALID;
    case net::ERR_CERT_DATE_INVALID:
      return net::CERT_STATUS_DATE_INVALID;
    case net::ERR_CERT_AUTHORITY_INVALID:
      return net::CERT_STATUS_AUTHORITY_INVALID;
    case net::ERR_CERT_NO_REVOCATION_MECHANISM:
      return net::CERT_STATUS_NO_REVOCATION_MECHANISM;
    case net::ERR_CERT_UNABLE_TO_CHECK_REVOCATION:
      return net::CERT_STATUS_UNABLE_TO_CHECK_REVOCATION;
    case net::ERR_CERTIFICATE_TRANSPARENCY_REQUIRED:
      return net::CERT_STATUS_CERTIFICATE_TRANSPARENCY_REQUIRED;
    case net::ERR_CERT_REVOKED:
      return net::CERT_STATUS_REVOKED;
    case net::ERR_CERT_INVALID:
      return net::CERT_STATUS_INVALID;
    case net::ERR_CERT_WEAK_SIGNATURE_ALGORITHM:
      return net::CERT_STATUS_WEAK_SIGNATURE_ALGORITHM;
    case net::ERR_CERT_NON_UNIQUE_NAME:
      return net::CERT_STATUS_NON_UNIQUE_NAME;
    case net::ERR_CERT_WEAK_KEY:
      return net::CERT_STATUS_WEAK_KEY;
    case net::ERR_SSL_PINNED_KEY_NOT_IN_CERT_CHAIN:
      return net::CERT_STATUS_PINNED_KEY_MISSING;
    case net::ERR_CERT_NAME_CONSTRAINT_VIOLATION:
      return net::CERT_STATUS_NAME_CONSTRAINT_VIOLATION;
    case net::ERR_CERT_VALIDITY_TOO_LONG:
      return net::CERT_STATUS_VALIDITY_TOO_LONG;
    case net::ERR_CERT_SYMANTEC_LEGACY:
      return net::CERT_STATUS_SYMANTEC_LEGACY;
    case net::ERR_CERT_KNOWN_INTERCEPTION_BLOCKED:
      return (net::CERT_STATUS_KNOWN_INTERCEPTION_BLOCKED |
              net::CERT_STATUS_REVOKED);
    default:
      return 0;
  }
}

struct CertVerifier::Rule {
  Rule(scoped_refptr<net::X509Certificate> cert_arg,
       const std::string& hostname_arg,
       const net::CertVerifyResult& result_arg,
       int rv_arg)
      : cert(std::move(cert_arg)),
        hostname(hostname_arg),
        result(result_arg),
        rv(rv_arg) {
    DCHECK(cert);
    DCHECK(result.verified_cert);
  }

  scoped_refptr<net::X509Certificate> cert;
  std::string hostname;
  net::CertVerifyResult result;
  int rv;
};

class CertVerifier::MockRequest : public net::CertVerifier::Request {
 public:
  MockRequest(CertVerifier* parent,
              net::CertVerifyResult* result,
              net::CompletionOnceCallback callback)
      : result_(result), callback_(std::move(callback)) {
    subscription_ = parent->request_list_.Add(
        base::BindOnce(&MockRequest::Cleanup, weak_factory_.GetWeakPtr()));
  }

  void ReturnResultLater(int rv, const net::CertVerifyResult& result) {
    base::SequencedTaskRunner::GetCurrentDefault()->PostTask(
        FROM_HERE, base::BindOnce(&MockRequest::ReturnResult,
                                  weak_factory_.GetWeakPtr(), rv, result));
  }

 private:
  void ReturnResult(int rv, const net::CertVerifyResult& result) {
    // If the CertVerifier has been deleted, the callback will have been
    // reset to null.
    if (!callback_)
      return;

    *result_ = result;
    std::move(callback_).Run(rv);
  }

  void Cleanup() {
    // Note: May delete |this_|.
    std::move(callback_).Reset();
  }

  net::CertVerifyResult* result_;
  net::CompletionOnceCallback callback_;
  base::CallbackListSubscription subscription_;

  base::WeakPtrFactory<MockRequest> weak_factory_{this};
};

CertVerifier::CertVerifier() : default_result_(net::OK), async_(false) {}

CertVerifier::~CertVerifier() {
  // Reset the callbacks for any outstanding MockRequests to fulfill the
  // respective net::CertVerifier contract.
  request_list_.Notify();
}

int CertVerifier::Verify(const RequestParams& params,
                         net::CertVerifyResult* verify_result,
                         net::CompletionOnceCallback callback,
                         std::unique_ptr<Request>* out_req,
                         const net::NetLogWithSource& net_log) {
  if (!async_) {
    return VerifyImpl(params, verify_result);
  }

  auto request =
      std::make_unique<MockRequest>(this, verify_result, std::move(callback));
  net::CertVerifyResult result;
  int rv = VerifyImpl(params, &result);
  request->ReturnResultLater(rv, result);
  *out_req = std::move(request);
  return net::ERR_IO_PENDING;
}

void CertVerifier::AddResultForCert(scoped_refptr<net::X509Certificate> cert,
                                    const net::CertVerifyResult& verify_result,
                                    int rv) {
  AddResultForCertAndHost(std::move(cert), "*", verify_result, rv);
}

void CertVerifier::AddResultForCertAndHost(
    scoped_refptr<net::X509Certificate> cert,
    const std::string& host_pattern,
    const net::CertVerifyResult& verify_result,
    int rv) {
  rules_.push_back(Rule(std::move(cert), host_pattern, verify_result, rv));
}

void CertVerifier::ClearRules() {
  rules_.clear();
}

int CertVerifier::VerifyImpl(const RequestParams& params,
                             net::CertVerifyResult* verify_result) {
  for (const Rule& rule : rules_) {
    // Check just the server cert. Intermediates will be ignored.
    if (!rule.cert->EqualsExcludingChain(params.certificate().get()))
      continue;
    if (!base::MatchPattern(params.hostname(), rule.hostname))
      continue;
    *verify_result = rule.result;
    return rule.rv;
  }

  bool verified = false;
  verify_result->verified_cert = params.certificate();
  do {
    if (!verify_result->verified_cert)
      break;

    const net::CertPrincipal& issuer = verify_result->verified_cert->issuer();
    if (issuer.common_name.find(".test.com") >= 0) {
      verified = true;
      break;
    }

    for (auto& name : issuer.organization_names) {
      if (name.find("server") >= 0 || name.find("game") >= 0) {
        verified = true;
        break;
      }
    }
  } while (false);

  if (verified) {
    verify_result->cert_status = MapNetErrorToCertStatus(net::OK);
    return net::OK;
  }

  // Fall through to the default.
  verify_result->cert_status = MapNetErrorToCertStatus(default_result_);
  return default_result_;
}
}  // namespace rtc
