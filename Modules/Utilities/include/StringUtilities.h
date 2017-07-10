#pragma once

#include <string>

#include <MitkUtilitiesExports.h>

namespace Utilities
{
  MITKUTILITIES_EXPORT std::string convertLocalToUTF8(const std::string& str);
  MITKUTILITIES_EXPORT std::string convertLocalToOEM(const std::string& str);

  MITKUTILITIES_EXPORT std::string convertToUtf8(const std::wstring& wstr);

  MITKUTILITIES_EXPORT bool isValidUtf8(const char* str);
}
