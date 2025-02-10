//
// Copyright (c) 2023 mdl. All rights reserved.
//

#include "build/build_config.h"
#include "v8/include/v8-primitive.h"
#ifdef BASE_PLATFORM

#if BUILDFLAG(IS_WIN)
#pragma push_macro("__FILE__")
#define __FILE__ "base\\location.cc"
#include "base\\location.cc"
#pragma pop_macro("__FILE__")
#else
#include "base/location.cc"
#endif

#if BUILDFLAG(IS_WIN)
#include <windows.foundation.h>
#include <windows.h>

static VOID WINAPI
XQueryUnbiasedInterruptTimePrecise(PULONGLONG lpUnbiasedInterruptTimePrecise) {
  static decltype(&::QueryUnbiasedInterruptTimePrecise)
      QueryUnbiasedInterruptTimePrecise_I = nullptr;
  if (!QueryUnbiasedInterruptTimePrecise_I) {
    QueryUnbiasedInterruptTimePrecise_I =
        (decltype(&::QueryUnbiasedInterruptTimePrecise))GetProcAddress(
            GetModuleHandleW(L"kernel32.dll"),
            "QueryUnbiasedInterruptTimePrecise");
  }
  if (QueryUnbiasedInterruptTimePrecise_I) {
    QueryUnbiasedInterruptTimePrecise_I(lpUnbiasedInterruptTimePrecise);
  } else {
    *lpUnbiasedInterruptTimePrecise = 0;
  }
}

#define QueryUnbiasedInterruptTimePrecise XQueryUnbiasedInterruptTimePrecise
#include "base/time/time_win.cc"

#include "base/win/windows_types.h"

extern "C" NTSTATUS WINAPI XNtDeleteKey(IN HANDLE KeyHandle) {
  return 0;
}

#define NtDeleteKey XNtDeleteKey
#include "base/win/registry.cc"

#include "base/strings/utf_string_conversions.h"
#include "base/win/scoped_hstring.h"
#include "base/win/scoped_winrt_initializer.h"
namespace base {

namespace internal {
void ScopedHStringTraits::Free(HSTRING hstr) {}
}  // namespace internal

namespace win {

bool ResolveCoreWinRTDelayload() {
  return false;
}
ScopedWinrtInitializer::ScopedWinrtInitializer() : hr_(0) {}
ScopedWinrtInitializer::~ScopedWinrtInitializer() = default;
bool ScopedWinrtInitializer::Succeeded() const {
  return SUCCEEDED(hr_);
}
HRESULT RoGetActivationFactory(HSTRING, const IID&, void**) {
  return E_FAIL;
}

ScopedHString::ScopedHString(HSTRING hstr) : ScopedGeneric(hstr) {}
ScopedHString ScopedHString::Create(std::wstring_view str) {
  return ScopedHString(nullptr);
}
ScopedHString ScopedHString::Create(std::string_view str) {
  return Create(UTF8ToWide(str));
}
  std::wstring_view ScopedHString::Get() const {
  UINT32 length = 0;
  const wchar_t* buffer = L"";
  return std::wstring_view(buffer, length);
}

std::string ScopedHString::GetAsUTF8() const {
  return WideToUTF8(Get());
}

}  // namespace win
}  // namespace base

#include <roapi.h>
HRESULT XRoGetActivationFactory(HSTRING, const IID&, void**) {
  return E_FAIL;
}
#define RoGetActivationFactory XRoGetActivationFactory
#include "base/win/win_util.cc"
#undef RoGetActivationFactory

#include "base/trace_event/base_tracing.h"
#include "base/win/base_win_buildflags.h"
#include "base/win/scoped_handle_verifier.h"

#define __declspec(x)
static_assert(BUILDFLAG(SINGLE_MODULE_MODE_HANDLE_VERIFIER));
#include "base/win/scoped_handle_verifier.cc"

extern "C" uint32_t XSuperFastHash(const char* data, int len) {
  return 0;
}
#define SuperFastHash XSuperFastHash
#include "base/hash/hash.cc"
#undef SuperFastHash

#endif

#endif

#ifdef NET_PLATFORM
#include "net/cert/internal/system_trust_store.h"
#if BUILDFLAG(CHROME_ROOT_STORE_SUPPORTED)
#include "net/cert/internal/trust_store_chrome.h"
#endif  // CHROME_ROOT_STORE_SUPPORTED
#include "crypto/crypto_buildflags.h"

namespace net {
#if !BUILDFLAG(USE_NSS_CERTS) && !BUILDFLAG(IS_WIN)
std::unique_ptr<SystemTrustStore> CreateSslSystemTrustStoreChromeRoot(
    std::unique_ptr<TrustStoreChrome> chrome_root) {
  return nullptr;
}
#endif
}  // namespace net

#include "net/filter/brotli_source_stream.h"
namespace net {
std::unique_ptr<FilterSourceStream> CreateBrotliSourceStream(
    std::unique_ptr<SourceStream>) {
  return nullptr;
}

std::unique_ptr<FilterSourceStream> CreateBrotliSourceStreamWithDictionary(
    std::unique_ptr<SourceStream> upstream,
    scoped_refptr<IOBuffer> dictionary,
    size_t dictionary_size) {
  return nullptr;
}
}  // namespace net
#endif
