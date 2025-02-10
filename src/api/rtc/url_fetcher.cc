#include "api/rtc/url_fetcher.h"

#include <utility>

#include "base/functional/bind.h"
#include "base/run_loop.h"
#include "base/strings/stringprintf.h"
#include "base/time/time.h"
#include "net/base/elements_upload_data_stream.h"
#include "net/base/io_buffer.h"
#include "net/base/load_flags.h"
#include "net/base/net_errors.h"
#include "net/base/upload_bytes_element_reader.h"
#include "net/cookies/cookie_monster.h"
#include "net/http/http_auth_handler_factory.h"
#include "net/http/http_response_headers.h"
#include "net/proxy_resolution/configured_proxy_resolution_service.h"
#include "net/traffic_annotation/network_traffic_annotation.h"
#include "net/url_request/redirect_info.h"
#include "net/url_request/static_http_user_agent_settings.h"
#include "net/url_request/url_request_context.h"
#include "net/url_request/url_request_context_builder.h"
#include "api/rtc/cert_verifier.h"
#include "api/rtc/ssl_config_service.h"

namespace worker {

namespace {

constexpr int kBufferSize = 2048;
constexpr int kMaxResponseSizeBytes = 4 * 1048576;

const net::NetworkTrafficAnnotationTag kTrafficAnnotation =
    net::DefineNetworkTrafficAnnotation(
        "URLFetcher",
        "semantics {\n"
        "  sender: \"Welink HTTP Client\"\n"
        "  description: \"Welink HTTP Client\"\n"
        "  trigger: \"N/A\"\n"
        "  data: \"No user data.\"\n"
        "  destination: OTHER\n"
        "  destination_other: \"N/A\"\n"
        "}\n"
        "policy {\n"
        "  cookies_allowed: NO\n"
        "  setting: \"This request cannot be disabled.\"\n"
        "  policy_exception_justification: \"none\"\n"
        "}");

}  // namespace

class URLFetcher::URLLoader : public net::URLRequest::Delegate {
 public:
  using BodyAsStringCallback =
      base::OnceCallback<void(int response_code,
                              const std::string& response_body)>;

  URLLoader() = default;
  ~URLLoader() = default;

  void DownloadToString(std::unique_ptr<net::URLRequest> request,
                        BodyAsStringCallback body_as_string_callback,
                        base::TimeDelta timeout) {
    request_ = std::move(request);
    cb_ = std::move(body_as_string_callback);
    data_received_.clear();
    read_buffer_ = base::MakeRefCounted<net::IOBufferWithSize>(kBufferSize);
    request_->Start();

    if (!timeout.is_zero()) {
      timeout_timer_.Start(
          FROM_HERE, timeout,
          base::BindOnce(&URLLoader::FinishWithResult,
                         weak_ptr_factory_.GetWeakPtr(), net::ERR_TIMED_OUT));
    }
  }

  void RunUntilComplete(std::unique_ptr<net::URLRequest> request,
                        base::TimeDelta timeout) {
    DownloadToString(std::move(request),
                     base::BindOnce([](int response_code,
                                       const std::string& response_body) {}),
                     timeout);
    base::RunLoop run_loop;
    on_complete_ = run_loop.QuitClosure();
    run_loop.Run();
  }

  const std::string& data_received() const { return data_received_; }
  const int response_code() const { return response_code_; }

  // 默认异步
  void sync() { sync_ = true; }

  // URLRequest::Delegate implementation:

  void OnReceivedRedirect(net::URLRequest* request,
                          const net::RedirectInfo& redirect_info,
                          bool* defer_redirect) override {
    if (!redirect_info.new_url.SchemeIsCryptographic()) {
      request->Cancel();
      return;
    }
  }

  void OnAuthRequired(net::URLRequest* request,
                      const net::AuthChallengeInfo& auth_info) override {
    request->Cancel();
  }

  void OnCertificateRequested(
      net::URLRequest* request,
      net::SSLCertRequestInfo* cert_request_info) override {
    request->Cancel();
  }

  void OnSSLCertificateError(net::URLRequest* request,
                             int net_error,
                             const net::SSLInfo& ssl_info,
                             bool fatal) override {
    request->Cancel();
  }

  void OnResponseStarted(net::URLRequest* request, int net_error) override {
    DCHECK_NE(net::ERR_IO_PENDING, net_error);
    DCHECK_EQ(request, request_.get());

    if (net_error != net::OK) {
      FinishWithResult(net_error);
      return;
    }

    ReadResponse();
  }

  void OnReadCompleted(net::URLRequest* request, int read_result) override {
    DCHECK_NE(net::ERR_IO_PENDING, read_result);
    DCHECK_EQ(request, request_.get());

    if (read_result <= 0) {
      FinishWithResult(read_result);
      return;
    }

    data_received_.append(read_buffer_->data(), read_result);
    if (data_received_.size() > kMaxResponseSizeBytes) {
      FinishWithResult(net::ERR_FILE_TOO_BIG);
      return;
    }

    ReadResponse();
  }

  void FinishWithResult(int net_error) {
    DCHECK_NE(net::ERR_IO_PENDING, net_error);
    int response_code;

    //timeout_timer_.AbandonAndStop();

    if (net_error != net::OK) {
      // LOG(ERROR) << "request failed, error: " <<
      // net::ErrorToString(net_error);
      response_code = 0;
    } else {
      response_code = request_->GetResponseCode();
      if (request_->GetResponseCode() != 200) {
        LOG(ERROR) << "request failed, bad response: " << response_code;
      }
    }
    request_.reset();
    response_code_ = response_code;
    if (!sync_) {
      std::move(cb_).Run(response_code, data_received_);
    } else {
      std::move(on_complete_).Run();
    }
  }

  void ReadResponse() {
    while (true) {
      int result = request_->Read(read_buffer_.get(), kBufferSize);
      if (result == net::ERR_IO_PENDING)
        return;

      if (result <= 0) {
        FinishWithResult(result);
        return;
      }

      data_received_.append(read_buffer_->data(), result);
    }
  }

 private:
  std::unique_ptr<net::URLRequest> request_;
  scoped_refptr<net::IOBuffer> read_buffer_;
  std::string data_received_;
  int response_code_;
  BodyAsStringCallback cb_;
  base::WeakPtrFactory<URLLoader> weak_ptr_factory_{this};
  base::OneShotTimer timeout_timer_;
  base::OnceClosure on_complete_;
  bool sync_ = false;
  ;
};

URLFetcher::URLFetcher() = default;

URLFetcher::URLFetcher(FetchCallback cb) : cb_(std::move(cb)) {}

URLFetcher::~URLFetcher() = default;

void URLFetcher::Get(const GURL& url,
                     const absl::optional<net::HttpRequestHeaders>& headers,
                     base::TimeDelta timeout) {
  Start(url, "GET", headers, absl::nullopt, false, timeout);
}

void URLFetcher::Post(const GURL& url,
                      const absl::optional<net::HttpRequestHeaders>& headers,
                      const absl::optional<std::string>& post_data,
                      base::TimeDelta timeout) {
  Start(url, "POST", headers, post_data, false, timeout);
}

std::pair<int, std::string> URLFetcher::SyncGet(
    const GURL& url,
    const absl::optional<net::HttpRequestHeaders>& headers,
    base::TimeDelta timeout) {
  return Start(url, "GET", headers, absl::nullopt, true, timeout);
}

std::pair<int, std::string> URLFetcher::SyncPost(
    const GURL& url,
    const absl::optional<net::HttpRequestHeaders>& headers,
    const absl::optional<std::string>& post_data,
    base::TimeDelta timeout) {
  return Start(url, "POST", headers, post_data, true, timeout);
}

std::pair<int, std::string> URLFetcher::Start(
    const GURL& url,
    const std::string& method,
    const absl::optional<net::HttpRequestHeaders>& headers,
    const absl::optional<std::string>& post_data,
    bool sync,
    base::TimeDelta timeout) {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  DCHECK(!loader_);
  if (!context_) {
    net::URLRequestContextBuilder builder;

    builder.set_proxy_resolution_service(
        net::ConfiguredProxyResolutionService::CreateDirect());
    builder.SetCertVerifier(std::make_unique<worker::CertVerifier>());
    builder.set_ssl_config_service(
        std::make_unique<worker::SSLConfigService>(net::SSLContextConfig()));
    builder.SetHttpAuthHandlerFactory(
        net::HttpAuthHandlerFactory::CreateDefault());
    builder.SetHttpServerProperties(
        std::make_unique<net::HttpServerProperties>());
    builder.set_quic_context(std::make_unique<net::QuicContext>());
    builder.SetCookieStore(
        std::make_unique<net::CookieMonster>(nullptr, nullptr));
    builder.set_http_user_agent_settings(
        std::make_unique<net::StaticHttpUserAgentSettings>("en-us",
                                                           std::string()));

    context_ = builder.Build();
  }

  loader_ = std::make_unique<URLLoader>();
  auto request = context_->CreateRequest(url, net::IDLE, loader_.get(),
                                         kTrafficAnnotation);

  request->set_method(method);
  request->SetLoadFlags(net::LOAD_DISABLE_CACHE);
  request->set_allow_credentials(false);

  if (post_data) {
    std::unique_ptr<net::UploadElementReader> reader(
        net::UploadOwnedBytesElementReader::CreateWithString(
            post_data.value()));
    request->set_upload(
        net::ElementsUploadDataStream::CreateWithReader(std::move(reader)));
  }

  if (headers) {
    for (const auto& header : headers.value().GetHeaderVector())
      request->SetExtraRequestHeaderByName(header.key, header.value, true);
  }

  if (!sync) {
    loader_->DownloadToString(
        std::move(request),
        base::BindOnce(&URLFetcher::ProcessResponse, base::Unretained(this)),
        timeout);
    return {0, ""};
  }

  loader_->sync();
  loader_->RunUntilComplete(std::move(request), timeout);

  return {loader_->response_code(), loader_->data_received()};
}

void URLFetcher::ProcessResponse(int response_code,
                                 const std::string& response) {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  std::move(cb_).Run(response_code, response);
}
}  // namespace rtc
