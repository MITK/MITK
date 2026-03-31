/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef USGLOBALCONFIG_H
#define USGLOBALCONFIG_H

//-------------------------------------------------------------------
// Platform defines
//-------------------------------------------------------------------

#if defined(__APPLE__)
  #define US_PLATFORM_APPLE
#endif

#if defined(__linux__)
  #define US_PLATFORM_LINUX
#endif

#if defined(_WIN32) || defined(_WIN64)
  #define US_PLATFORM_WINDOWS
#else
  #define US_PLATFORM_POSIX
#endif

//-------------------------------------------------------------------
// Macros for import/export declarations
//-------------------------------------------------------------------

#if defined(US_PLATFORM_WINDOWS)
  #define US_ABI_EXPORT __declspec(dllexport)
  #define US_ABI_IMPORT __declspec(dllimport)
  #define US_ABI_LOCAL
#else
  #define US_ABI_EXPORT __attribute__ ((visibility ("default")))
  #define US_ABI_IMPORT __attribute__ ((visibility ("default")))
  #define US_ABI_LOCAL  __attribute__ ((visibility ("hidden")))
#endif

//-------------------------------------------------------------------
// Macros for suppressing warnings
//-------------------------------------------------------------------

#ifdef _MSC_VER
#define US_MSVC_PUSH_DISABLE_WARNING(wn) \
__pragma(warning(push))                  \
__pragma(warning(disable:wn))
#define US_MSVC_POP_WARNING \
__pragma(warning(pop))
#define US_MSVC_DISABLE_WARNING(wn) \
__pragma(warning(disable:wn))
#else
#define US_MSVC_PUSH_DISABLE_WARNING(wn)
#define US_MSVC_POP_WARNING
#define US_MSVC_DISABLE_WARNING(wn)
#endif

// Do not warn about the usage of deprecated unsafe functions
US_MSVC_DISABLE_WARNING(4996)

#endif // USGLOBALCONFIG_H
