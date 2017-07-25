#pragma once

#include <string>

#include <QString>

#include <MitkUtilitiesExports.h>

namespace Utilities
{
  MITKUTILITIES_EXPORT std::string preferredPath(const std::string& path);
  MITKUTILITIES_EXPORT std::string convertToLocalEncoding(QString str);
  MITKUTILITIES_EXPORT std::string absPath(const std::string& relpath);
}
