#include "api/rtc/url_request.h"
#include <cctype>
#include <memory>
#include <string>
#include "base/check.h"
#include "base/files/file_path.h"
#include "base/files/file_util.h"
#include "base/functional/bind.h"
#include "base/location.h"
#include "base/memory/scoped_refptr.h"
#include "base/strings/sys_string_conversions.h"
#include "base/task/sequenced_task_runner.h"
#include "net/base/mime_util.h"
#include "url_request.h"
#include "api/rtc/url_fetcher.h"
#include "build/build_config.h"
#include "build/buildflag.h"

namespace {
const char kDelimiterEqualChar = '=';
const char kDelimiterAddressChar = '&';
const char kUploadDataMIMEType[] = "multipart/form-data; boundary=";
}  // namespace
namespace worker {

URLRequest::URLPayload::URLPayload()
    : boundary_(net::GenerateMimeMultipartBoundary()) {}

URLRequest::URLPayload::~URLPayload() = default;

// application/x-www-form-urlencoded 保留字母数组 如果是空格替换成+
// 保留四个特殊字符
//- _ . ~ 其余的字符转成%加上相应字符的16进制
const std::string URLRequest::URLPayload::Encode(const std::string& data) {
  std::stringstream encoded;
  for (char c : data) {
    switch (std::isalnum(c)) {
      case 0: {
        switch (c) {
          case '-':
          case '_':
          case '.':
          case '~': {
            encoded << c;
          } break;
          case ' ': {
            encoded << '+';
          } break;
          default: {
            encoded << '%' << std::hex
                    << static_cast<int>(static_cast<unsigned char>(c));
          } break;
        }
      } break;
      default:
        encoded << c;
        break;
    }
  }
  return encoded.str();
}

template <typename V>
void URLRequest::URLPayload::AddJson(const std::string& key, const V& value) {}

void URLRequest::URLPayload::AddUrlEncodPayload(const std::string& key,
                                                const std::string& value) {
  if (start_) {
    start_ = false;
  } else {
    payload_.append(&kDelimiterAddressChar, 1);
  }
  payload_.append(Encode(key));
  payload_.append(&kDelimiterEqualChar, 1);
  payload_.append(Encode(value));
}

void URLRequest::URLPayload::AddFormData(const std::string& name,
                                         const std::string& value,
                                         const std::string& content_type) {
  net::AddMultipartValueForUpload(name, value, boundary_, content_type,
                                  &payload_);
}

void URLRequest::URLPayload::AddFormDataWithFileName(
    const std::string& name,
    const std::string& file_name,
    const std::string& value,
    const std::string& content_type) {
  net::AddMultipartValueForUploadWithFileName(name, file_name, value, boundary_,
                                              content_type, &payload_);
}

void URLRequest::URLPayload::AddFormDataEnd() {
  net::AddMultipartFinalDelimiterForUpload(boundary_, &payload_);
}

bool URLRequest::URLPayload::ReadFileToString(const std::string& path,
                                              std::string& error_reason,
                                              std::string* contents) {
#if BUILDFLAG(IS_WIN)
  base::FilePath file_path(base::SysNativeMBToWide(path));
#else
  base::FilePath file_path(path);
#endif
  if (!base::PathExists(file_path)) {
    error_reason = "Upload directory is missing the file";
    return false;
  }
  if (!base::ReadFileToString(file_path, contents)) {
    error_reason = "Failed to read file contents";
    return false;
  }
  return true;
}

std::string URLRequest::URLPayload::form_data_value() {
  return kUploadDataMIMEType + boundary_;
}

void URLRequest::URLPayload::reset() {
  start_ = true;
  payload_.clear();
}

class URLRequest::Context {
 public:
  scoped_refptr<base::SequencedTaskRunner> task_runner;
  std::unique_ptr<URLFetcher> url_fetcher;
  FetchCallback callback;
};

void URLRequest::Get(const GURL& url,
                     const absl::optional<net::HttpRequestHeaders>& headers,
                     FetchCallback cb,
                     base::TimeDelta timeout) {
  Context* context = new Context;
  context->task_runner = base::SequencedTaskRunner::GetCurrentDefault();
  context->callback = std::move(cb);
  auto url_fetcher = std::make_unique<URLFetcher>(base::BindOnce(
      &URLRequest::OnRequestCompleted, base::Unretained(context)));
  url_fetcher->Get(url, headers, timeout);
  context->url_fetcher = std::move(url_fetcher);
}

void URLRequest::Post(const GURL& url,
                      const absl::optional<net::HttpRequestHeaders>& headers,
                      const absl::optional<std::string>& post_data,
                      FetchCallback cb,
                      base::TimeDelta timeout) {
  Context* context = new Context;
  context->task_runner = base::SequencedTaskRunner::GetCurrentDefault();
  context->callback = std::move(cb);
  auto url_fetcher = std::make_unique<URLFetcher>(base::BindOnce(
      &URLRequest::OnRequestCompleted, base::Unretained(context)));
  url_fetcher->Post(url, headers, post_data, timeout);
  context->url_fetcher = std::move(url_fetcher);
}

std::pair<int, std::string> URLRequest::SyncGet(
    const GURL& url,
    const absl::optional<net::HttpRequestHeaders>& headers,
    base::TimeDelta timeout) {
  return URLFetcher().SyncGet(url, headers, timeout);
}

std::pair<int, std::string> URLRequest::SyncPost(
    const GURL& url,
    const absl::optional<net::HttpRequestHeaders>& headers,
    const absl::optional<std::string>& post_data,
    base::TimeDelta timeout) {
  return URLFetcher().SyncPost(url, headers, std::move(post_data), timeout);
}

void URLRequest::OnRequestCompleted(Context* context,
                                    int response_code,
                                    const std::string& response) {
  DCHECK(context);
  context->task_runner->PostTask(
      FROM_HERE,
      base::BindOnce(&URLRequest::OnResponse, std::move(context->callback),
                     response_code, std::move(response)));
  context->task_runner->PostTask(
      FROM_HERE, base::BindOnce(&URLRequest::RemoveContext, context));
}

void URLRequest::OnResponse(FetchCallback cb,
                            int response_code,
                            const std::string& response) {
  std::move(cb).Run(response_code, response);
}

void URLRequest::RemoveContext(Context* context) {
  delete context;
}

}  // namespace rtc
