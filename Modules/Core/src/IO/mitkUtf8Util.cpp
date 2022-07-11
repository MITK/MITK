/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkUtf8Util.h>
#include <mitkExceptionMacro.h>

#include <usGlobalConfig.h>

#ifdef US_PLATFORM_WINDOWS

#include <Windows.h>

namespace
{
  std::wstring MultiByteToWideChar(const std::string& mbString, UINT codePage)
  {
    auto numChars = ::MultiByteToWideChar(codePage, 0, mbString.data(), mbString.size(), nullptr, 0);

    if (0 >= numChars)
      mitkThrow() << "Failure to convert multi-byte character string to wide character string";

    std::wstring wString;
    wString.resize(numChars);

    ::MultiByteToWideChar(codePage, 0, mbString.data(), mbString.size(), &wString[0], static_cast<int>(wString.size()));

    return wString;
  }

  std::string WideCharToMultiByte(const std::wstring& wString, UINT codePage)
  {
    auto numChars = ::WideCharToMultiByte(codePage, 0, wString.data(), wString.size(), nullptr, 0, nullptr, nullptr);

    if (0 >= numChars)
      mitkThrow() << "Failure to convert wide character string to multi-byte character string";

    std::string mbString;
    mbString.resize(numChars);

    ::WideCharToMultiByte(codePage, 0, wString.data(), wString.size(), &mbString[0], static_cast<int>(mbString.size()), nullptr, nullptr);

    return mbString;
  }
}

#endif

std::string mitk::Utf8Util::Local8BitToUtf8(const std::string& local8BitStr)
{
#ifdef US_PLATFORM_WINDOWS
  try
  {
    return CP_UTF8 != GetOEMCP()
      ? WideCharToMultiByte(MultiByteToWideChar(local8BitStr, CP_ACP), CP_UTF8)
      : local8BitStr;
  }
  catch (const mitk::Exception&)
  {
    MITK_WARN << "String conversion from current code page to UTF-8 failed. Input string is returned unmodified.";
  }
#endif

  return local8BitStr;
}

std::string mitk::Utf8Util::Utf8ToLocal8Bit(const std::string& utf8Str)
{
#ifdef US_PLATFORM_WINDOWS
  try
  {
    return CP_UTF8 != GetOEMCP()
      ? WideCharToMultiByte(MultiByteToWideChar(utf8Str, CP_UTF8), CP_ACP)
      : utf8Str;
  }
  catch (const mitk::Exception&)
  {
    MITK_WARN << "String conversion from UTF-8 to current code page failed. Input string is returned unmodified.";
  }
#endif

  return utf8Str;
}
