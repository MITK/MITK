/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkPythonContext.h>
#include <mitkPythonHelper.h>
#include <mitkPythonUtil.h>
#include <mitkIOUtil.h>

#include <pybind11/embed.h>

#include <algorithm>
#include <fstream>
#include <sstream>

namespace py = pybind11;

namespace
{
  void ActivateVirtualEnv(const std::string& venvName)
  {
    if (!mitk::PythonHelper::VirtualEnvExists(venvName))
    {
      MITK_INFO << "Create virtual environment: " << venvName;
      const auto venvPath = mitk::PythonHelper::CreateVirtualEnv(venvName);

      if (venvPath.empty())
        mitkThrow() << "Could not create virtual environment: "
                    << mitk::PythonHelper::GetVirtualEnvPath(venvName);
    }

    if (!mitk::PythonHelper::ActivateVirtualEnv(venvName))
      mitkThrow() << "Could not activate virtual environment: " << venvName;

    MITK_INFO << "Using virtual environment: " << venvName;
  }

  // Keeps the user's own Python setup out of the interpreter: packages
  // installed with "pip install --user" (and the .pth files that come with
  // them) and the current working directory would otherwise precede the
  // site-packages of MITK and its virtual environments on sys.path.
  void InitializeInterpreter()
  {
    PyConfig config;
    PyConfig_InitPythonConfig(&config);

    config.parse_argv = 0;
    config.user_site_directory = 0;

#if defined(__APPLE__)
    // Python cannot derive its home from an executable inside an app bundle.
    const auto home = mitk::PythonHelper::GetHomePath();

    if (home.empty() || PyStatus_Exception(PyConfig_SetBytesString(&config, &config.home, home.string().c_str())))
    {
      PyConfig_Clear(&config);
      mitkThrow() << "Could not determine the home of the Python interpreter.";
    }
#endif

    py::initialize_interpreter(&config, 0, nullptr, false);
  }

  // Inserts a key into a dict on construction and removes it on
  // destruction. GIL must be held by the caller for both. Used to scope
  // __file__/__name__ during ExecuteFile so they never leak into the
  // shared dictionary, even if py::exec or dict assignment throws.
  class ScopedDictKey
  {
  public:
    ScopedDictKey(py::dict& dict, const char* key, py::object value)
      : m_Dict(dict), m_Key(key)
    {
      m_Dict[py::str(m_Key)] = std::move(value);
    }

    ~ScopedDictKey() noexcept
    {
      if (m_Dict.contains(m_Key))
        PyDict_DelItemString(m_Dict.ptr(), m_Key);

      if (PyErr_Occurred())
        PyErr_Clear();
    }

    ScopedDictKey(const ScopedDictKey&) = delete;
    ScopedDictKey& operator=(const ScopedDictKey&) = delete;

  private:
    py::dict& m_Dict;
    const char* m_Key;
  };
}

struct mitk::PythonContext::Impl
{
  // Python copies the process environment once, when the interpreter starts.
  // A venv that is activated through the environment afterwards, e.g. a second
  // one in the same run, would never show up in os.environ. The context keeps
  // the path itself instead of reading it back from there.
  std::string VirtualEnvPath;

  py::object GetVariable(const std::string& varName)
  {
    py::gil_scoped_acquire gil;

    if (this->Dictionary.contains(varName))
      return this->Dictionary[py::str(varName)];

    return py::none();
  }

  template <typename T>
  std::optional<T> GetVariableAs(const std::string& varName)
  {
    auto var = this->GetVariable(varName);

    if (var.is_none())
      return std::nullopt;

    try
    {
      return var.cast<T>();
    }
    catch (const py::cast_error&)
    {
      return std::nullopt;
    }
  }

  py::dict Dictionary;
};

mitk::PythonContext::PythonContext(const std::string& venvName)
{
  if (!venvName.empty())
    ActivateVirtualEnv(venvName);

  if (!Py_IsInitialized())
    InitializeInterpreter();

  m_Impl = std::make_unique<Impl>();
  m_Impl->VirtualEnvPath = mitk::PythonHelper::GetVirtualEnvPath(venvName).string();
}

void mitk::PythonContext::Activate(bool importBindings)
{
  std::ostringstream pyCommands; pyCommands
    << "import os, site, sys\n"
    << "def add_site_packages(base_path):\n"
    << "    if os.name == 'nt':\n"
    << "        site_packages = os.path.join(base_path, 'Lib', 'site-packages')\n"
    << "    else:\n"
    << "        version = f'python{sys.version_info.major}.{sys.version_info.minor}'\n"
    << "        site_packages = os.path.join(base_path, 'lib', version, 'site-packages')\n"
    << "    if site_packages not in sys.path:\n"
    << "        site.addsitedir(site_packages)\n"
    << "add_site_packages(sys.base_prefix)\n";

  // Importing NumPy (and the MITK module, which pulls it in) maps the venv's
  // compiled extensions on Linux, where the venv is sys.prefix. Skip it for
  // metadata-only contexts so an "is any venv module loaded?" check stays
  // honest. See Activate()'s documentation.
  if (importBindings)
  {
    std::string appPath = IOUtil::GetAppBundlePath(IOUtil::AppBundlePath::Parent).string();

#if defined(_WIN32)
    std::replace(appPath.begin(), appPath.end(), '\\', '/');
#endif

    pyCommands
      << "import numpy as np\n"
      << "app_dir = '" << appPath << "'\n"
      << "if app_dir not in sys.path:\n"
      << "    sys.path.insert(0, app_dir)\n"
      << "import mitk\n";
  }

  if (!m_Impl->VirtualEnvPath.empty())
    pyCommands << "add_site_packages(" << PyQuote(m_Impl->VirtualEnvPath) << ")\n";

  this->Execute(pyCommands.str());
}

mitk::PythonContext::~PythonContext()
{
  if (!Py_IsInitialized())
  {
    // The interpreter is already finalized. Destroying the py::dict in
    // m_Impl would Py_DECREF objects backed by a dead interpreter (UB).
    // Leak instead; this only happens during abnormal shutdown.
    (void)m_Impl.release();
    return;
  }

  py::gil_scoped_acquire gil;
  m_Impl.reset();
}

void mitk::PythonContext::Execute(const std::string &expression)
{
  py::gil_scoped_acquire gil;

  try
  {
    py::exec(expression, m_Impl->Dictionary);
  }
  catch (py::error_already_set& e)
  {
    mitkThrow() << "An error occurred while executing Python code: " << e.what();
  }
}

void mitk::PythonContext::ExecuteFile(const fs::path& filePath)
{
  const auto normalizedPath = fs::absolute(filePath).lexically_normal();

  std::ifstream stream(normalizedPath, std::ios::binary);

  if (!stream.is_open())
    mitkThrow() << "Could not open Python file: " << normalizedPath.string();

  std::ostringstream buffer;
  buffer << stream.rdbuf();

  py::gil_scoped_acquire gil;

  // __file__ and __name__ are script-scoped: the guards below insert them
  // for the duration of execution so scripts can use Path(__file__) and
  // `if __name__ == "__main__"`, then remove them on destruction so
  // subsequent Execute() calls don't see stale values. User-defined
  // globals intentionally persist in the shared dictionary.
  ScopedDictKey fileKey(m_Impl->Dictionary, "__file__",
                        py::str(normalizedPath.generic_string()));
  ScopedDictKey nameKey(m_Impl->Dictionary, "__name__",
                        py::str("__main__"));

  try
  {
    py::exec(buffer.str(), m_Impl->Dictionary);
  }
  catch (py::error_already_set& e)
  {
    mitkThrow() << "An error occurred while executing Python file \""
                << normalizedPath.string() << "\": " << e.what();
  }
}

void mitk::PythonContext::BindImage(Image* image, const std::string& varName)
{
  py::gil_scoped_acquire gil;

  if (image == nullptr)
  {
    m_Impl->Dictionary[py::str(varName)] = py::none();
    return;
  }

  try
  {
    m_Impl->Dictionary[py::str(varName)] = py::cast(image, py::return_value_policy::reference);
  }
  catch (const py::error_already_set& e)
  {
    mitkThrow() << "Could not bind image to Python variable \"" << varName << "\": " << e.what();
  }
}

void mitk::PythonContext::BindFunction(const std::string& varName, std::function<bool(int, int)> function)
{
  py::gil_scoped_acquire gil;

  if (!function)
  {
    m_Impl->Dictionary[py::str(varName)] = py::none();
    return;
  }

  try
  {
    m_Impl->Dictionary[py::str(varName)] = py::cpp_function(std::move(function), py::name(varName.c_str()));
  }
  catch (const py::error_already_set& e)
  {
    mitkThrow() << "Could not bind function to Python variable \"" << varName << "\": " << e.what();
  }
}

bool mitk::PythonContext::HasVariable(const std::string &varName)
{
  py::gil_scoped_acquire gil;

  return m_Impl->Dictionary.contains(varName);
}

std::optional<bool> mitk::PythonContext::GetVariableAsBool(const std::string& varName)
{
  return m_Impl->GetVariableAs<bool>(varName);
}

std::optional<int> mitk::PythonContext::GetVariableAsInt(const std::string& varName)
{
  return m_Impl->GetVariableAs<int>(varName);
}

std::optional<double> mitk::PythonContext::GetVariableAsDouble(const std::string& varName)
{
  return m_Impl->GetVariableAs<double>(varName);
}

std::optional<std::string> mitk::PythonContext::GetVariableAsString(const std::string& varName)
{
  return m_Impl->GetVariableAs<std::string>(varName);
}
