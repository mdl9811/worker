//
// Copyright (c) 2023 mdl. All rights reserved.
//

#include <time.h>

#include "corecrt_io.h"
#include "corecrt_internal_lowio.h"
#include "corecrt_internal_time.h"

#ifdef _DEBUG
#include "third_party/windows/sdk/ucrt/heap/debug_heap.cpp"
#include "third_party/windows/sdk/ucrt/heap/debug_heap_hook.cpp"
#include "third_party/windows/sdk/ucrt/misc/dbgrpt.cpp"
#include "third_party/windows/sdk/ucrt/misc/dbgrptt.cpp"
#include "third_party/windows/sdk/ucrt/misc/debug_fill_threshold.cpp"
#endif

extern "C" {
long __cdecl _lseek_nolock(int const fh, long const offset, int const origin);
__int64 __cdecl _lseeki64_nolock(int const fh,
                                 __int64 const offset,
                                 int const origin);

__time32_t __cdecl _time32x(_Out_opt_ __time32_t* _Time) {
  return _time32(_Time);
}

__time64_t __cdecl _time64x(_Out_opt_ __time64_t* _Time) {
  return _time64(_Time);
}

long __cdecl _lseek_nolockx(_In_ int _FileHandle,
                            _In_ long _Offset,
                            _In_ int _Origin) {
  return _lseek_nolock(_FileHandle, _Offset, _Origin);
}

__int64 __cdecl _lseeki64_nolockx(_In_ int _FileHandle,
                                  _In_ __int64 _Offset,
                                  _In_ int _Origin) {
  return _lseeki64_nolock(_FileHandle, _Offset, _Origin);
}

errno_t __cdecl _waccess_sx(_In_z_ const wchar_t* _Filename,
                            _In_ int _AccessMode) {
  return _waccess_s(_Filename, _AccessMode);
}

errno_t __cdecl _access_sx(_In_z_ char const* _FileName, _In_ int _AccessMode) {
  return _access_s(_FileName, _AccessMode);
}

__time32_t __cdecl __loctotime32_tx(int const yr,
                                    int const mo,
                                    int const dy,
                                    int const hr,
                                    int const mn,
                                    int const sc,
                                    int const dstflag) {
  return __loctotime32_t(yr, mo, dy, hr, mn, sc, dstflag);
}

__time64_t __cdecl __loctotime64_tx(int const yr,
                                    int const mo,
                                    int const dy,
                                    int const hr,
                                    int const mn,
                                    int const sc,
                                    int const dstflag) {
  return __loctotime64_t(yr, mo, dy, hr, mn, sc, dstflag);
}

errno_t __cdecl _sopen_nolockx(int* const punlock_flag,
                               int* const pfh,
                               char const* const path,
                               int const oflag,
                               int const shflag,
                               int const pmode,
                               int const secure) {
  return _sopen_nolock(punlock_flag, pfh, path, oflag, shflag, pmode, secure);
}

errno_t __cdecl _wsopen_nolockx(int* const punlock_flag,
                                int* const pfh,
                                wchar_t const* const path,
                                int const oflag,
                                int const shflag,
                                int const pmode,
                                int const secure) {
  return _wsopen_nolock(punlock_flag, pfh, path, oflag, shflag, pmode, secure);
}
}
