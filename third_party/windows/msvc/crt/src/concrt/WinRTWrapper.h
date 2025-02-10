// ==++==
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
// ==--==
// =+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+
//
// WinRTWrapper.h
//
// Dynamic wrappers around Windows Runtime functions.
//
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#pragma once

#define DEFINE_STATIC_WRAPPER_FN(classname, name, rt) \
    classname ::Pfn_ ## name classname ::s_pfn_ ## name;\
    rt classname :: name() \
    {\
        Pfn_ ## name pfn = (Pfn_ ## name) Security::DecodePointer(s_pfn_ ## name); \
        ASSERT(pfn != NULL);\
        return pfn();\
    }

#define DECLARE_STATIC_WRAPPER_FN(name, rt) \
    public:\
        static rt name();\
    private:\
        typedef rt (WINAPI *Pfn_ ## name)();\
        static Pfn_ ## name s_pfn_ ## name;

#define DEFINE_STATIC_WRAPPER_FN_1(classname, name, rt, a1t) \
    classname ::Pfn_ ## name classname ::s_pfn_ ## name;\
    rt classname :: name(a1t a1) \
    {\
        Pfn_ ## name pfn = (Pfn_ ## name) Security::DecodePointer(s_pfn_ ## name); \
        ASSERT(pfn != NULL);\
        return pfn(a1);\
    }

#define DECLARE_STATIC_WRAPPER_FN_1(name, rt, a1t) \
    public:\
        static rt name(a1t);\
    private:\
        typedef rt (WINAPI *Pfn_ ## name)(a1t);\
        static Pfn_ ## name s_pfn_ ## name;

#define GETPROC_FROM_MODULE_HANDLE(module_handle, name) \
    {\
        Pfn_ ## name pfn = (Pfn_ ## name)(GetProcAddress(module_handle, #name));\
        if (pfn == NULL) throw scheduler_resource_allocation_error(HRESULT_FROM_WIN32(GetLastError()));\
        s_pfn_ ## name = (Pfn_ ## name) Security::EncodePointer(pfn);\
    }

namespace Concurrency
{
namespace details
{
    class WinRT
    {
    public:

        /// <summary>
        ///     Initializes all static function pointers to Windows Runtime functions.  We do not call or link against these for distribution
        ///     against OS's below Win8.
        /// </summary>
        static void Initialize();

        /// <summary>
        ///     Returns whether or not the Windows Runtime wrappers have been initialized yet.
        /// </summary>
        static bool Initialized();

#if !defined(_ONECORE)
        DECLARE_STATIC_WRAPPER_FN_1(RoInitialize, HRESULT, RO_INIT_TYPE);
        DECLARE_STATIC_WRAPPER_FN(RoUninitialize, void);
#else
        static HRESULT RoInitialize(RO_INIT_TYPE);
        static void RoUninitialize();
#endif // !defined(_ONECORE)

    private:

        static HMODULE m_hModule;
        static volatile LONG s_fInitialized;

    };
} // namespace details
} // namespace Concurrency
