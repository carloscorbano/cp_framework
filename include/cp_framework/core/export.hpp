#pragma once

#pragma once

// Detect platform
#if defined(_WIN32) || defined(_WIN64)
#define CP_PLATFORM_WINDOWS 1
#else
#define CP_PLATFORM_WINDOWS 0
#endif

// ------------------------------------------------------------
// Visibility (export / import)
// ------------------------------------------------------------
#if CP_PLATFORM_WINDOWS
// Windows: only exports what is explicitly marked
#define CP_API_EXPORT __declspec(dllexport)
#define CP_API_IMPORT __declspec(dllimport)
#else
// Linux / Mac: use GCC/Clang visibility attributes
#define CP_API_EXPORT __attribute__((visibility("default")))
#define CP_API_IMPORT __attribute__((visibility("default")))
#endif

// ------------------------------------------------------------
// CP_API (main macro to expose symbols)
// ------------------------------------------------------------
#ifdef CP_BUILD_DLL
// Building the DLL
#define CP_API CP_API_EXPORT
#else
// Using the DLL
#define CP_API CP_API_IMPORT
#endif

// ------------------------------------------------------------
// Optional: hide internal symbols explicitly
// ------------------------------------------------------------
#if CP_PLATFORM_WINDOWS
// Windows hides everything not exported
#define CP_INTERNAL
#else
// On Linux/macOS, you can force hide visibility
#define CP_INTERNAL __attribute__((visibility("hidden")))
#endif
