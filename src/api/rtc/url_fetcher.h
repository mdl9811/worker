//
// Copyright (c) 2025 mdl. All rights reserved.
//

#ifndef API_RTC_URL_FETCHER_H_
#define API_RTC_URL_FETCHER_H_

#include <memory>
#include <string>
#include <utility>

#include "base/functional/callback.h"
#include "base/time/time.h"
#include "net/http/http_request_headers.h"
#include "third_party/abseil-cpp/absl/types/optional.h"
#include "url/gurl.h"

namespace net {
class URLRequestContext;
}  // namespace net

namespace worker {
class URLFetcher {
 public:
  using FetchCallback =
      base::OnceCallback<void(int http_response_code, const std::string& data)>;
  URLFetcher();

  explicit URLFetcher(FetchCallback cb);

  ~URLFetcher();

  void Get(const GURL& url,
           const absl::optional<net::HttpRequestHeaders>& headers,
           base::TimeDelta timeout = base::TimeDelta());
  void Post(const GURL& url,
            const absl::optional<net::HttpRequestHeaders>& headers,
            const absl::optional<std::string>& post_data,
            base::TimeDelta timeout = base::TimeDelta());
  std::pair<int, std::string> SyncGet(
      const GURL& url,
      const absl::optional<net::HttpRequestHeaders>& headers,
      base::TimeDelta timeout = base::TimeDelta());

  std::pair<int, std::string> SyncPost(
      const GURL& url,
      const absl::optional<net::HttpRequestHeaders>& headers,
      const absl::optional<std::string>& post_data,
      base::TimeDelta timeout = base::TimeDelta());

 private:
  //@return code data
  std::pair<int, std::string> Start(
      const GURL& url,
      const std::string& method,
      const absl::optional<net::HttpRequestHeaders>& headers,
      const absl::optional<std::string>& post_data,
      bool sync,
      base::TimeDelta timeout);
  void ProcessResponse(int response_code, const std::string& response);

 private:
  class URLLoader;

  FetchCallback cb_;
  std::unique_ptr<net::URLRequestContext> context_;
  std::unique_ptr<URLLoader> loader_;
  SEQUENCE_CHECKER(sequence_checker_);
};
}  // namespace rtc

#endif  // WELINK_UTILS_URL_FETCHER_H_
