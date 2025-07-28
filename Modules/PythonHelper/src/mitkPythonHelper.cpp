/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkPythonHelper.h>
#include <mitkIOUtil.h>

namespace
{
  fs::path Up(fs::path path, int levels)
  {
    while (levels-- > 0)
      path = path.parent_path();

    return path;
  }

  bool IsDirectoryWritable(const fs::path& path)
  {
    if (!fs::is_directory(path))
      return false;

    const auto testFile = path / ".is_writable.tmp";
    std::ofstream stream(testFile.string());

    if (!stream.is_open())
      return false;

    stream << "delete me";
    stream.close();

    std::error_code error;
    fs::remove(testFile, error);

    return !error;
  }
}

fs::path mitk::PythonHelper::GetHomePath()
{
  auto appDir = fs::path(mitk::IOUtil::GetProgramPath()).remove_filename();

#if defined(_WIN32)
  auto buildTreePython = Up(appDir, 2) / "python";
#elif defined(__APPLE__)
  auto buildTreePython = Up(appDir, 4) / "python";
#else
  auto buildTreePython = Up(appDir, 1) / "python";
#endif

  if (fs::exists(buildTreePython) && fs::is_directory(buildTreePython))
    return buildTreePython.lexically_normal();

#if defined(__APPLE__)
  auto installedPython = Up(appDir, 1) / "Resources" / "python";
#else
  auto installedPython = Up(appDir, 1) / "python";
#endif

  if (fs::exists(installedPython) && fs::is_directory(installedPython))
  {
    if (IsDirectoryWritable(installedPython))
      return installedPython.lexically_normal();

    // TODO: Copy to writable location / use venv in writable location
  }

  return {};
}

fs::path mitk::PythonHelper::GetLibraryPath()
{
  auto homePath = GetHomePath();

  if (homePath.empty())
    return {};

#if defined(_WIN32)
  auto libPath = homePath / MITK_PYTHON_LIBRARY_FILENAME;
#else
  auto libPath = homePath / "lib" / MITK_PYTHON_LIBRARY_FILENAME;
#endif

  if (fs::exists(libPath))
    return libPath;

  return {};
}

fs::path mitk::PythonHelper::GetExecutablePath()
{
  auto homePath = GetHomePath();

  if (homePath.empty())
    return {};

#if defined(_WIN32)
  auto execPath = homePath / "python.exe";
#else
  auto execPath = homePath / "bin" / "python3";
#endif

  if (fs::exists(execPath))
    return execPath;

  return {};
}

