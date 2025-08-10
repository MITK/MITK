/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkPythonHelper.h>
#include <mitkEnvironment.h>
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
    if (path.empty() || !fs::exists(path) || !fs::is_directory(path))
      return false;

    const auto testFile = path / ".is_writable.tmp";
    std::ofstream stream(testFile.string());

    if (!stream.is_open())
      return false;

    stream << "delete me";
    stream.close();

    std::error_code error;
    return fs::remove(testFile, error) || !error;
  }

  size_t HashAppPath()
  {
    auto appPath = mitk::IOUtil::GetAppBundlePath(mitk::IOUtil::AppBundlePath::Self);

    if (!appPath.empty())
      return std::hash<std::string>{}(appPath.string());

    return {};
  }

  std::string GetHashAsString(size_t hash)
  {
    std::stringstream stream;
    stream << std::hex << std::uppercase << std::setw(16) << std::setfill('0') << hash;
    return stream.str();
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
    return installedPython.lexically_normal();

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
  fs::path venvPath;
  fs::path execPath;

  if (const auto VIRTUAL_ENV = GetEnv("VIRTUAL_ENV"); VIRTUAL_ENV.has_value())
    venvPath = VIRTUAL_ENV.value();

  if (fs::exists(venvPath / "pyvenv.cfg"))
  {
#if defined(_WIN32)
    execPath = venvPath / "Scripts" / "python.exe";
#else
    execPath = venvPath / "bin" / "python3";
#endif
  }
  else
  {
    const auto homePath = GetHomePath();

    if (homePath.empty())
      return {};

#if defined(_WIN32)
    execPath = homePath / "python.exe";
#else
    execPath = homePath / "bin" / "python3";
#endif
  }

  if (fs::exists(execPath))
    return execPath;

  return {};
}

fs::path mitk::PythonHelper::GetVirtualEnvBasePath()
{
  fs::path basePath;

#if defined(_WIN32)
  basePath = mitk::GetEnv("LocalAppData").value_or("");
#elif defined(__APPLE__)
  basePath = mitk::GetEnv("HOME").value_or("");

  if (!basePath.empty())
    basePath /= "Library/Application Support";
#else
  basePath = mitk::GetEnv("XDG_DATA_HOME").value_or("");

  if (basePath.empty())
  {
    const fs::path homePath = mitk::GetEnv("HOME").value_or("");

    if (!homePath.empty())
      basePath = homePath / ".local/share";
  }
#endif

  if (!basePath.empty())
  {
    if (const auto hash = HashAppPath(); hash != 0)
    {
      const auto hashString = GetHashAsString(hash);
      const fs::path venvPath = basePath / "mitk_venvs" / hashString;
      std::error_code error;

      if (!fs::exists(venvPath))
        fs::create_directories(venvPath, error);

      if (!error && IsDirectoryWritable(venvPath))
        return venvPath;
    }
  }

  return {};
}

fs::path mitk::PythonHelper::GetVirtualEnvPath(const std::string& name)
{
  if (name.empty())
    return {};

  const auto basePath = GetVirtualEnvBasePath();

  if (basePath.empty())
    return {};

  return basePath / name;
}

bool mitk::PythonHelper::VirtualEnvExists(const std::string& name)
{
  const auto venvPath = GetVirtualEnvPath(name);

  if (venvPath.empty())
    return false;

  return fs::exists(venvPath / "pyvenv.cfg");
}

fs::path mitk::PythonHelper::CreateVirtualEnv(const std::string& name)
{
  const auto venvPath = GetVirtualEnvPath(name);

  if (venvPath.empty())
    return {};

  if (!fs::exists(venvPath))
  {
    std::error_code error;
    fs::create_directory(venvPath, error);

    if (error)
      return {};
  }

  const auto venvConfigPath = venvPath / "pyvenv.cfg";

  if (fs::exists(venvConfigPath))
    return venvPath;

  const auto execPath = GetExecutablePath();

  if (execPath.empty())
    return {};

  auto venvCommand = "\"" + execPath.string() + "\" -m venv \"" + venvPath.string() + "\"";

#if defined(_WIN32)
  // Workaround for std::system() on Windows:
  //   If the command starts with a quote, it may be misinterpreted or truncated.
  //   Prepend a no-op to ensure the full command is preserved.
  venvCommand = "if 1==1 " + venvCommand;
#endif

  if (auto result = std::system(venvCommand.c_str()); result != 0)
    return {};

  if (fs::exists(venvConfigPath))
    return venvPath;

  return {};
}

bool mitk::PythonHelper::ActivateVirtualEnv(const std::string& name)
{
  return ActivateVirtualEnv(GetVirtualEnvPath(name));
}

bool mitk::PythonHelper::ActivateVirtualEnv(const fs::path& path)
{
  if (path.empty() || !fs::exists(path / "pyvenv.cfg"))
    return false;

  if (!SetEnv("VIRTUAL_ENV", path.string()))
    return false;

#if defined(_WIN32)
  const auto pythonExecDir = path / "Scripts";
#else
  const auto pythonExecDir = path / "bin";
#endif

  if (!AddPathEnv(pythonExecDir))
  {
    UnsetEnv("VIRTUAL_ENV");
    return false;
  }

  return true;
}
