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

#include <algorithm>
#include <cctype>
#include <functional>

#if defined(_WIN32)
#include <windows.h>
#include <psapi.h>
#elif defined(__APPLE__)
#include <mach-o/dyld.h>
#else
#include <link.h>
#endif

namespace
{
  fs::path Up(fs::path path, int levels)
  {
    while (levels-- > 0)
      path = path.parent_path();

    return path;
  }

  // The Python interpreter inside a virtual environment. The layout differs by
  // platform (Windows under Scripts/, POSIX under bin/); keeping it in one place
  // lets GetExecutablePath and GetVirtualEnvExecutablePath share the definition.
  fs::path VenvPythonExe(const fs::path& venvPath)
  {
#if defined(_WIN32)
    return venvPath / "Scripts" / "python.exe";
#else
    return venvPath / "bin" / "python3";
#endif
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

    if (appPath.empty())
      return {};

    // Canonicalize before hashing so the hash is invariant to how the
    // executable was launched. On Windows in particular, GetModuleFileName
    // returns the path with whatever case and short/long form was used at
    // load time; weakly_canonical resolves both to the form stored on disk.
    std::error_code ec;
    auto canonicalAppPath = fs::weakly_canonical(appPath, ec);

    if (ec || canonicalAppPath.empty())
      canonicalAppPath = appPath;

    return std::hash<std::string>{}(canonicalAppPath.string());
  }

  std::string GetHashAsString(size_t hash)
  {
    std::stringstream stream;
    stream << std::hex << std::uppercase << std::setw(16) << std::setfill('0') << hash;
    return stream.str();
  }

  bool IsPathUnder(const fs::path& child, const fs::path& parent)
  {
    std::error_code ec;
    auto canChild = fs::weakly_canonical(child, ec);

    if (ec || canChild.empty())
      return false;

    auto canParent = fs::weakly_canonical(parent, ec);

    if (ec || canParent.empty())
      return false;

    auto childStr = canChild.string();
    auto parentStr = canParent.string();

#if defined(_WIN32)
    auto toLower = [](unsigned char c) { return static_cast<char>(std::tolower(c)); };
    std::transform(childStr.begin(), childStr.end(), childStr.begin(), toLower);
    std::transform(parentStr.begin(), parentStr.end(), parentStr.begin(), toLower);
#endif

    return childStr.size() > parentStr.size() &&
           childStr.compare(0, parentStr.size(), parentStr) == 0;
  }

  bool ForEachLoadedModule(const std::function<bool(const fs::path&)>& pred)
  {
#if defined(_WIN32)
    HANDLE process = GetCurrentProcess();
    std::vector<HMODULE> modules(1024);
    DWORD bytesNeeded = 0;

    if (!EnumProcessModules(process, modules.data(),
                            static_cast<DWORD>(modules.size() * sizeof(HMODULE)),
                            &bytesNeeded))
    {
      return false;
    }

    if (bytesNeeded > modules.size() * sizeof(HMODULE))
    {
      modules.resize(bytesNeeded / sizeof(HMODULE));

      if (!EnumProcessModules(process, modules.data(),
                              static_cast<DWORD>(modules.size() * sizeof(HMODULE)),
                              &bytesNeeded))
      {
        return false;
      }
    }

    const size_t count = bytesNeeded / sizeof(HMODULE);

    for (size_t i = 0; i < count; ++i)
    {
      wchar_t buffer[MAX_PATH];

      if (GetModuleFileNameExW(process, modules[i], buffer, MAX_PATH) &&
          pred(fs::path(buffer)))
      {
        return true;
      }
    }

    return false;
#elif defined(__APPLE__)
    const uint32_t count = _dyld_image_count();

    for (uint32_t i = 0; i < count; ++i)
    {
      if (const char* name = _dyld_get_image_name(i); name != nullptr && *name != '\0')
      {
        if (pred(fs::path(name)))
          return true;
      }
    }

    return false;
#else
    struct Ctx
    {
      const std::function<bool(const fs::path&)>* pred;
      bool found;
    };

    Ctx ctx{&pred, false};

    dl_iterate_phdr(
      [](struct dl_phdr_info* info, size_t, void* data) -> int
      {
        auto* c = static_cast<Ctx*>(data);

        if (info->dlpi_name != nullptr && info->dlpi_name[0] != '\0' &&
            (*c->pred)(fs::path(info->dlpi_name)))
        {
          c->found = true;
          return 1;
        }

        return 0;
      },
      &ctx);

    return ctx.found;
#endif
  }
}

fs::path mitk::PythonHelper::GetHomePath()
{
  auto appDir = fs::path(mitk::IOUtil::GetProgramPath()).remove_filename();

#if defined(_WIN32)
  auto buildTreePython = Up(appDir, 2) / "python";
#elif defined(__APPLE__)
  auto buildTreePython = Up(appDir, 4) / "python";

  if (!fs::exists(buildTreePython) || !fs::is_directory(buildTreePython))
    buildTreePython = Up(appDir, 1) / "python";
#else
  auto buildTreePython = Up(appDir, 1) / "python";
#endif

  if (fs::exists(buildTreePython) && fs::is_directory(buildTreePython))
    return buildTreePython.lexically_normal();

#if defined(__APPLE__)
  auto installedPython = Up(appDir, 1) / "Frameworks" / "Python.framework" / "Versions" / "A";
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
    execPath = VenvPythonExe(venvPath);
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

fs::path mitk::PythonHelper::GetVirtualEnvExecutablePath(const std::string& name)
{
  const auto venvPath = GetVirtualEnvPath(name);

  if (venvPath.empty() || !fs::exists(venvPath / "pyvenv.cfg"))
    return {};

  const auto execPath = VenvPythonExe(venvPath);

  if (fs::exists(execPath))
    return execPath;

  return {};
}

fs::path mitk::PythonHelper::GetVirtualEnvScriptPath(const std::string& name, const std::string& scriptName)
{
  const auto venvPath = GetVirtualEnvPath(name);

  if (venvPath.empty())
    return {};

  // Same platform layout as VenvPythonExe (Windows: Scripts/, POSIX: bin/), but
  // for an arbitrary console script rather than the interpreter.
#if defined(_WIN32)
  return venvPath / "Scripts" / (scriptName + ".exe");
#else
  return venvPath / "bin" / scriptName;
#endif
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

bool mitk::PythonHelper::RemoveVirtualEnv(const std::string& name)
{
  return RemoveVirtualEnv(GetVirtualEnvPath(name));
}

bool mitk::PythonHelper::RemoveVirtualEnv(const fs::path& path)
{
  if (path.empty() || !fs::exists(path))
    return false;

  std::error_code error;
  fs::remove_all(path, error);

  return !error && !fs::exists(path);
}

bool mitk::PythonHelper::IsAnyVirtualEnvModuleLoaded(const std::string& name)
{
  return IsAnyVirtualEnvModuleLoaded(GetVirtualEnvPath(name));
}

bool mitk::PythonHelper::IsAnyVirtualEnvModuleLoaded(const fs::path& path)
{
  if (path.empty() || !fs::exists(path))
    return false;

  return ForEachLoadedModule(
    [&path](const fs::path& modulePath) { return IsPathUnder(modulePath, path); });
}
