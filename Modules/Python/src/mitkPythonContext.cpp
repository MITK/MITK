/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkPythonContext.h>
#include <mitkPythonHelper.h>
#include <mitkIOUtil.h>

#include <pybind11/embed.h>

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
}

struct mitk::PythonContext::Impl
{
  py::object GetVariable(const std::string& varName)
  {
    py::gil_scoped_acquire gil;

    if (this->LocalDictionary.contains(varName))
      return this->LocalDictionary[py::str(varName)];

    if (this->GlobalDictionary.contains(varName))
      return this->GlobalDictionary[py::str(varName)];

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

  py::dict GlobalDictionary;
  py::dict LocalDictionary;
};

mitk::PythonContext::PythonContext(const std::string& venvName)
{
  if (!venvName.empty())
    ActivateVirtualEnv(venvName);

  if (!Py_IsInitialized())
    py::initialize_interpreter();

  m_Impl = std::make_unique<Impl>();
}

void mitk::PythonContext::Activate()
{
  std::string appPath = IOUtil::GetAppBundlePath(IOUtil::AppBundlePath::Parent).string();

#if defined(_WIN32)
  std::replace(appPath.begin(), appPath.end(), '\\', '/');
#endif

  std::ostringstream pyCommands; pyCommands
    << "import os, site, sys\n"
    << "def add_site_packages(base_path):\n"
    << "    import os, site, sys\n"
    << "    if os.name == 'nt':\n"
    << "        site_packages = os.path.join(base_path, 'Lib', 'site-packages')\n"
    << "    else:\n"
    << "        version = f'python{sys.version_info.major}.{sys.version_info.minor}'\n"
    << "        site_packages = os.path.join(base_path, 'lib', version, 'site-packages')\n"
    << "    if site_packages not in sys.path:\n"
    << "        site.addsitedir(site_packages)\n"
    << "add_site_packages(sys.base_prefix)\n"
    << "import numpy as np\n"
    << "app_dir = '" << appPath << "'\n"
    << "if app_dir not in sys.path:\n"
    << "    sys.path.insert(0, app_dir)\n"
    << "import mitk\n"
    << "venv = os.environ.get('VIRTUAL_ENV')\n"
    << "if venv:\n"
    << "    add_site_packages(venv)\n";

  this->Execute(pyCommands.str());
}

mitk::PythonContext::~PythonContext()
{
  m_Impl->LocalDictionary.clear();
  m_Impl->GlobalDictionary.clear();
}

void mitk::PythonContext::Execute(const std::string &expression)
{
  py::gil_scoped_acquire gil;

  try
  {
    py::exec(expression, m_Impl->GlobalDictionary, m_Impl->LocalDictionary);
  }
  catch (py::error_already_set& e)
  {
    mitkThrow() << "An error occurred while executing Python code: " << e.what();
  }
}

void mitk::PythonContext::BindImage(Image* image, const std::string& varName)
{
  py::gil_scoped_acquire gil;

  if (image == nullptr)
  {
    m_Impl->GlobalDictionary[py::str(varName)] = py::none();
    return;
  }

  try
  {
    m_Impl->GlobalDictionary[py::str(varName)] = py::cast(image, py::return_value_policy::reference);
  }
  catch (const py::error_already_set& e)
  {
    mitkThrow() << "Could not bind image to Python variable \"" << varName << "\": " << e.what();
  }
}

bool mitk::PythonContext::HasVariable(const std::string &varName)
{
  py::gil_scoped_acquire gil;

  return m_Impl->LocalDictionary.contains(varName) ||
         m_Impl->GlobalDictionary.contains(varName);
}

std::optional<bool> mitk::PythonContext::GetVariableAsBool(const std::string& varName)
{
  return m_Impl->GetVariableAs<bool>(varName);
}

std::optional<int> mitk::PythonContext::GetVariableAsInt(const std::string& varName)
{
  return m_Impl->GetVariableAs<int>(varName);
}

std::optional<std::string> mitk::PythonContext::GetVariableAsString(const std::string& varName)
{
  return m_Impl->GetVariableAs<std::string>(varName);
}
