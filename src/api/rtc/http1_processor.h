//
// Copyright (c) 2025 mdl. All rights reserved.
//

#ifndef API_RTC_HTTP_PROCESSOR_H_
#define API_RTC_HTTP_PROCESSOR_H_

#include <map>
#include <string_view>
#include <unordered_map>

#include "absl/types/optional.h"
#include "base/functional/callback.h"
#include "base/memory/scoped_refptr.h"
#include "base/sequence_checker.h"
#include "base/strings/string_split.h"
#include "net/http/http_status_code.h"


namespace base {
class SequencedTaskRunner;
}

namespace worker {
class IOBuffer;
class TCPServerConnection;

class HttpRequestInfo final {
 public:
  HttpRequestInfo(bool is_secure);
  ~HttpRequestInfo();

  std::string GetHeaderValue(const std::string& header_name) const;
  bool HasHeaderValue(const std::string& header_name,
                      const std::string& header_value) const;

  enum HTTP_METHOD { GET, POST };
  int method;
  bool secure;
  std::string path;
  absl::optional<std::string> query;
  absl::optional<std::string> body;

  std::map<std::string, std::string> headers;
};

class HttpResponseInfo final {
 public:
  HttpResponseInfo();
  ~HttpResponseInfo();

  void AddHeader(const std::string& name, const std::string& value);
  void SetContentHeaders(scoped_refptr<IOBuffer> body,
                         const std::string& content_type);

  scoped_refptr<IOBuffer> Serialize() const;

  net::HttpStatusCode status_code() const { return status_code_; }
  void status_code(net::HttpStatusCode status_code) {
    status_code_ = status_code;
  }
  const base::StringPairs& headers() { return headers_; }
  IOBuffer* body() { return body_.get(); }

 private:
  net::HttpStatusCode status_code_;
  base::StringPairs headers_;
  scoped_refptr<IOBuffer> body_;
};

class Http1Processor final {
 public:
  using OnCompletionCallback =
      base::OnceCallback<void(const HttpResponseInfo& response_info)>;
  using HandlerCallback =
      base::RepeatingCallback<void(const HttpRequestInfo& request_info,
                                   HttpResponseInfo* response_info)>;
  using HandlerAsyncCallback =
      base::RepeatingCallback<void(const HttpRequestInfo& request_info,
                                   OnCompletionCallback callback)>;
  Http1Processor();
  ~Http1Processor();

  void SetHandler(
      std::string_view path,
      bool secure,
      HandlerCallback cb,
      scoped_refptr<base::SequencedTaskRunner> task_runner = nullptr);
  void SetHandler(std::string_view path, bool secure, HandlerAsyncCallback cb);
  bool Process(HttpRequestInfo&& request_info, OnCompletionCallback cb);

 private:
  class Handler;
  std::unordered_map<std::string, std::unique_ptr<Handler>> handlers_;
  SEQUENCE_CHECKER(sequence_checker_);
};
}  // namespace worker

#endif
