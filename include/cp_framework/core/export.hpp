#pragma once

#ifndef DLL_EXPORT
#define DLL_EXPORT __declspec(dllexport)
#endif

#ifndef DLL_IMPORT
#define DLL_IMPORT __declspec(dllimport)
#endif

#ifdef CP_FRAMEWORK_EXPORTS
#define CP_API DLL_EXPORT
#else
#define CP_API DLL_IMPORT
#endif