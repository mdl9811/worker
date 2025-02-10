//
// Copyright (c) 2025 mdl. All rights reserved.
//

#ifndef API_RTC_URL_REQUEST_H_
#define API_RTC_URL_REQUEST_H_

#include <string>
#include <utility>

#include "base/functional/callback.h"
#include "base/time/time.h"
#include "net/http/http_request_headers.h"
#include "third_party/abseil-cpp/absl/types/optional.h"
#include "url/gurl.h"

namespace worker {
class URLRequest {
 public:
  using FetchCallback =
      base::OnceCallback<void(int http_response_code, const std::string& data)>;

  // 目前不支持 JSON
  class URLPayload {
   public:
    explicit URLPayload();

    ~URLPayload();

    void AddUrlEncodPayload(const std::string& key, const std::string& value);
    void AddFormData(const std::string& name,
                     const std::string& value,
                     const std::string& content_type);
    void AddFormDataWithFileName(const std::string& name,
                                 const std::string& file_name,
                                 const std::string& value,
                                 const std::string& content_type);

    void AddFormDataEnd();

    bool ReadFileToString(const std::string& path,
                          std::string& error_reason,
                          std::string* contents);

    const std::string& string() const { return payload_; }
    void clear() { payload_.clear(); }

    void reset();

    std::string form_data_value();

   private:
    template <typename V>
    void AddJson(const std::string& key, const V& value);
    const std::string Encode(const std::string& data);

   private:
    bool start_ = true;
    std::string payload_;
    std::string boundary_;
  };

  static void Get(const GURL& url,
                  const absl::optional<net::HttpRequestHeaders>& headers,
                  FetchCallback cb,
                  base::TimeDelta timeout = base::TimeDelta());
  static void Post(const GURL& url,
                   const absl::optional<net::HttpRequestHeaders>& headers,
                   const absl::optional<std::string>& post_data,
                   FetchCallback cb,
                   base::TimeDelta timeout = base::TimeDelta());

  static std::pair<int, std::string> SyncGet(
      const GURL& url,
      const absl::optional<net::HttpRequestHeaders>& headers,
      base::TimeDelta timeout = base::TimeDelta());
  static std::pair<int, std::string> SyncPost(
      const GURL& url,
      const absl::optional<net::HttpRequestHeaders>& headers,
      const absl::optional<std::string>& post_data,
      base::TimeDelta timeout = base::TimeDelta());

 private:
  class Context;
  static void OnRequestCompleted(Context* context,
                                 int response_code,
                                 const std::string& response);
  static void OnResponse(FetchCallback cb,
                         int response_code,
                         const std::string& response);
  static void RemoveContext(Context* contexg);
};

}  // namespace rtc

#endif