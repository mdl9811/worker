//
// Copyright (c) 2022 Welink. All rights reserved.
//

#include "api/rtc/http1_processor.h"

#include "base/check_op.h"
#include "base/strings/string_util.h"
#include "base/strings/stringprintf.h"
#include "base/task/sequenced_task_runner.h"
#include "net/http/http_request_headers.h"
#include "api/rtc/io_buffer.h"

namespace worker {

HttpRequestInfo::HttpRequestInfo(bool is_secure) : secure(is_secure) {}

HttpRequestInfo::~HttpRequestInfo() = default;

std::string HttpRequestInfo::GetHeaderValue(
    const std::string& header_name) const {
  DCHECK_EQ(base::ToLowerASCII(header_name), header_name);
  decltype(headers)::const_iterator it = headers.find(header_name);
  if (it != headers.end())
    return it->second;
  return std::string();
}

bool HttpRequestInfo::HasHeaderValue(const std::string& header_name,
                                     const std::string& header_value) const {
  DCHECK_EQ(base::ToLowerASCII(header_value), header_value);
  std::string complete_value = base::ToLowerASCII(GetHeaderValue(header_name));

  for (const std::string_view& cur :
       base::SplitStringPiece(complete_value, ",", base::KEEP_WHITESPACE,
                              base::SPLIT_WANT_NONEMPTY)) {
    if (base::TrimString(cur, " \t", base::TRIM_ALL) == header_value)
      return true;
  }
  return false;
}

HttpResponseInfo::HttpResponseInfo() : status_code_(net::HTTP_FORBIDDEN) {}

HttpResponseInfo::~HttpResponseInfo() = default;

void HttpResponseInfo::AddHeader(const std::string& name,
                                 const std::string& value) {
  headers_.emplace_back(std::make_pair(name, value));
}

void HttpResponseInfo::SetContentHeaders(scoped_refptr<IOBuffer> body,
                                         const std::string& content_type) {
  body_ = body;
  AddHeader(net::HttpRequestHeaders::kContentLength,
            base::StringPrintf("%d", body_ ? body_->size() : 0));
  AddHeader(net::HttpRequestHeaders::kContentType, content_type);
}

scoped_refptr<IOBuffer> HttpResponseInfo::Serialize() const {
  std::string response = base::StringPrintf("HTTP/1.1 %d %s\r\n", status_code_,
                                            GetHttpReasonPhrase(status_code_));
  base::StringPairs::const_iterator header;
  for (header = headers_.begin(); header != headers_.end(); ++header)
    response += header->first + ": " + header->second + "\r\n";

  response += "\r\n";

  int body_size = body_ ? body_->size() : 0;
  auto buffer = IOBuffer::New(response.size() + body_size);

  memcpy(buffer->data(), &response[0], response.size());
  if (body_size)
    memcpy(buffer->data() + response.size(), body_->data(), body_size);

  return buffer;
}

class Http1Processor::Handler {
 public:
  Handler(scoped_refptr<base::SequencedTaskRunner> task_runner,
          bool secure,
          HandlerCallback callback)
      : dispath_(task_runner.get() ? &Handler::DispatchTaskrunnerCallback
                                   : &Handler::DispatchCallback),
        task_runner_(task_runner),
        secure_(secure),
        callback_(std::move(callback)) {}
  Handler(bool secure, HandlerAsyncCallback callback)
      : dispath_(&Handler::DispatchAsyncCallback),
        secure_(secure),
        async_callback_(std::move(callback)) {}
  ~Handler() = default;

  bool dispatch(HttpRequestInfo&& request_info, OnCompletionCallback callback) {
    return (this->*dispath_)(std::move(request_info), std::move(callback));
  }

  class AsyncContext {
   public:
    explicit AsyncContext(Handler* handler,
                          HttpRequestInfo&& request_info,
                          OnCompletionCallback callback)
        : handler_(handler),
          callback_(std::move(callback)),
          request_info_(std::move(request_info)) {}
    ~AsyncContext() = default;

    Handler* handler_;
    OnCompletionCallback callback_;
    HttpRequestInfo request_info_;
    HttpResponseInfo response_info_;
  };

  static void HandleAsyncRequest(AsyncContext* context) {
    context->handler_->callback_.Run(context->request_info_,
                                     &context->response_info_);
  }

  static void HandleAsyncResult(std::unique_ptr<AsyncContext> context) {
    std::move(context->callback_).Run(context->response_info_);
  }

  using Dispath = bool (Handler::*)(HttpRequestInfo&& request_info,
                                    OnCompletionCallback callback);
  bool DispatchTaskrunnerCallback(HttpRequestInfo&& request_info,
                                  OnCompletionCallback callback);
  bool DispatchCallback(HttpRequestInfo&& request_info,
                        OnCompletionCallback callback);
  bool DispatchAsyncCallback(HttpRequestInfo&& request_info,
                             OnCompletionCallback callback);

  Dispath dispath_;
  scoped_refptr<base::SequencedTaskRunner> task_runner_;
  bool secure_;
  HandlerCallback callback_;
  HandlerAsyncCallback async_callback_;
};

bool Http1Processor::Handler::DispatchTaskrunnerCallback(
    HttpRequestInfo&& request_info,
    OnCompletionCallback callback) {
  std::unique_ptr<AsyncContext> context = std::make_unique<AsyncContext>(
      this, std::move(request_info), std::move(callback));
  AsyncContext* context_raw = context.get();
  return task_runner_->PostTaskAndReply(
      FROM_HERE,
      base::BindOnce(Http1Processor::Handler::HandleAsyncRequest, context_raw),
      base::BindOnce(Http1Processor::Handler::HandleAsyncResult,
                     std::move(context)));
}

bool Http1Processor::Handler::DispatchCallback(HttpRequestInfo&& request_info,
                                              OnCompletionCallback callback) {
  HttpResponseInfo response_info;
  callback_.Run(request_info, &response_info);
  std::move(callback).Run(response_info);
  return true;
}

bool Http1Processor::Handler::DispatchAsyncCallback(
    HttpRequestInfo&& request_info,
    OnCompletionCallback callback) {
  async_callback_.Run(request_info, std::move(callback));
  return true;
}

Http1Processor::Http1Processor() = default;

Http1Processor::~Http1Processor() = default;

void Http1Processor::SetHandler(
    std::string_view path,
    bool secure,
    HandlerCallback callback,
    scoped_refptr<base::SequencedTaskRunner> task_runner) {
  std::unique_ptr<Handler> handler =
      std::make_unique<Handler>(task_runner, secure, std::move(callback));
  handlers_[std::string(path.data(), path.length())] = std::move(handler);
}

void Http1Processor::SetHandler(std::string_view path,
                               bool secure,
                               HandlerAsyncCallback callback) {
  std::unique_ptr<Handler> handler =
      std::make_unique<Handler>(secure, std::move(callback));
  handlers_[std::string(path.data(), path.length())] = std::move(handler);
}

bool Http1Processor::Process(HttpRequestInfo&& request_info,
                            OnCompletionCallback callback) {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  DCHECK(!request_info.path.empty());
  auto it = handlers_.find(request_info.path);
  if (it != handlers_.end()) {
    auto& handler = it->second;
    if (!request_info.secure && handler->secure_)
      return false;
    return handler->dispatch(std::move(request_info), std::move(callback));
  }
  return false;
}

}  // namespace worker
