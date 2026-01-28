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

#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include <pybind11/pybind11.h>

namespace py = pybind11;

namespace
{
  struct PyObjectDeleter
  {
    void operator()(PyObject *obj) const { Py_XDECREF(obj); }
  };

  using PyObjectPtr = std::unique_ptr<PyObject, PyObjectDeleter>;

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
  PyObjectPtr GetVariable(const std::string& varName)
  {
    auto obj = PyDict_GetItemString(LocalDictionary.get(), varName.c_str());

    if (!obj)
      obj = PyDict_GetItemString(GlobalDictionary.get(), varName.c_str());

    if (obj)
      Py_INCREF(obj); // Promote from borrowed to owned reference.

    return PyObjectPtr(obj);
  }

  PyObjectPtr GlobalDictionary;
  PyObjectPtr LocalDictionary;
};

mitk::PythonContext::PythonContext(const std::string& venvName)
  : m_Impl(std::make_unique<Impl>())
{
  if (!venvName.empty())
    ActivateVirtualEnv(venvName);

  if (!Py_IsInitialized())
    Py_Initialize();

  m_Impl->GlobalDictionary.reset(PyDict_New());
  m_Impl->LocalDictionary.reset(PyDict_New());
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

  this->ExecuteString(pyCommands.str());
}

mitk::PythonContext::~PythonContext()
{
  PyDict_Clear(m_Impl->LocalDictionary.get());
  PyDict_Clear(m_Impl->GlobalDictionary.get());
}

std::string mitk::PythonContext::ExecuteFile(const std::string &filePath)
{
  PyGILState_STATE state = PyGILState_Ensure();
  int commandType = Py_file_input;
  FILE *file = fopen(filePath.c_str(), "rb");
  if (nullptr == file)
  {
    mitkThrow() << "An error occured while reading python file.";
  }
  PyObjectPtr executionResult(
    PyRun_File(file, "script.py", commandType, m_Impl->GlobalDictionary.get(), m_Impl->LocalDictionary.get()));
  if (!executionResult)
  {
    // PyErr_Print(); prints stacktrace on console
    std::string traceback = this->GetPythonExceptionTraceback();
    mitkThrow() << "An error occured while running the Python code. " << traceback;
  }
  PyGILState_Release(state);
  return std::string(this->GetStdOut());
}

std::string mitk::PythonContext::ExecuteString(const std::string &pyCommands)
{
  PyGILState_STATE state = PyGILState_Ensure();
  int commandType = Py_file_input;
  PyObjectPtr executionResult(
    PyRun_String(pyCommands.c_str(), commandType, m_Impl->GlobalDictionary.get(), m_Impl->LocalDictionary.get()));
  if (!executionResult)
  {
    //PyErr_Print();  prints stacktrace on console
    std::string traceback = this->GetPythonExceptionTraceback();
    mitkThrow() << "An error occured while running the Python code. " << traceback;
  }
  PyGILState_Release(state);
  return this->GetStdOut();
}

std::string mitk::PythonContext::GetStdOut(const std::string &varName)
{
  PyGILState_STATE state = PyGILState_Ensure();
  std::string _mitk_stdout;
  PyObject *capture_output = PyDict_GetItemString(m_Impl->LocalDictionary.get(), varName.c_str());
  if (capture_output)
  {
    PyObjectPtr output_val(PyObject_CallMethod(capture_output, "getvalue", nullptr));
    _mitk_stdout = PyUnicode_AsUTF8(output_val.get());
    MITK_INFO << varName << " : " << _mitk_stdout;
  }
  else
  {
    PyErr_Print();
  }
  PyGILState_Release(state);
  return _mitk_stdout;
}

mitk::Image* mitk::PythonContext::LoadImageFromPython(const std::string &varName)
{
  PyGILState_STATE state = PyGILState_Ensure();

  try
  {
    py::dict localDict = py::reinterpret_borrow<py::dict>(m_Impl->LocalDictionary.get());
    py::dict globalDict = py::reinterpret_borrow<py::dict>(m_Impl->GlobalDictionary.get());

    py::object pyImage;

    if (localDict.contains(varName.c_str()))
    {
      pyImage = localDict[varName.c_str()];
    }
    else if (globalDict.contains(varName.c_str()))
    {
      pyImage = globalDict[varName.c_str()];
    }
    else
    {
      mitkThrow() << "Cloud not get image from Python: " << varName;
    }

    auto image = pyImage.cast<Image::Pointer>();

    MITK_INFO << "C++ received image has dimension: " << image->GetDimension();

    PyGILState_Release(state);
    return image;
  }
  catch (const py::cast_error& e)
  {
    PyGILState_Release(state);
    mitkThrow() << "Could not cast Python object to mitk::Image: " << e.what();
  }
}

void mitk::PythonContext::TransferBaseDataToPython(mitk::BaseData* baseData, const std::string& varName)
{
  auto state = PyGILState_Ensure();

  try
  {
    auto globals = py::reinterpret_borrow<py::dict>(m_Impl->GlobalDictionary.get());

    if (auto image = dynamic_cast<Image*>(baseData))
    {
      globals[varName.c_str()] = py::cast(image, py::return_value_policy::reference);
    }
    else
    {
      MITK_ERROR << "Unsupported BaseData type";
    }
  }
  catch (const py::error_already_set& e)
  {
    MITK_ERROR << "Python error: " << e.what();
  }

  PyGILState_Release(state);
}

bool mitk::PythonContext::HasVariable(const std::string &varName)
{
  PyObject *pyVar = PyDict_GetItemString(m_Impl->LocalDictionary.get(), varName.c_str());
  if (pyVar == nullptr && !(pyVar = PyDict_GetItemString(m_Impl->GlobalDictionary.get(), varName.c_str())))
  {
    return false;
  }
  return true;
}

template <>
std::optional<bool> mitk::PythonContext::GetVariableAs<bool>(const std::string& varName)
{
  auto obj = m_Impl->GetVariable(varName);

  if (!obj)
    return std::nullopt;

  auto isTrue = PyObject_IsTrue(obj.get());

  if (isTrue == -1)
    return std::nullopt;

  return isTrue != 0;
}

template <>
std::optional<int> mitk::PythonContext::GetVariableAs<int>(const std::string& varName)
{
  PyObjectPtr obj = m_Impl->GetVariable(varName);

  if (!obj || !PyLong_Check(obj.get()))
    return std::nullopt;

  return static_cast<int>(PyLong_AsLong(obj.get()));
}

template <>
std::optional<double> mitk::PythonContext::GetVariableAs<double>(const std::string& varName)
{
  PyObjectPtr obj = m_Impl->GetVariable(varName);

  if (!obj || !PyFloat_Check(obj.get()))
    return std::nullopt;

  return PyFloat_AsDouble(obj.get());
}

template <>
std::optional<std::string> mitk::PythonContext::GetVariableAs<std::string>(const std::string& varName)
{
  PyObjectPtr obj = m_Impl->GetVariable(varName);

  if (!obj || !PyUnicode_Check(obj.get()))
    return std::nullopt;

  auto str = PyUnicode_AsUTF8(obj.get());

  if (str == nullptr)
    return std::nullopt;

  return str;
}

std::string mitk::PythonContext::GetPythonExceptionTraceback()
{
  std::string errorMessage;
#if PY_MINOR_VERSION >= 12
  PyObjectPtr pException(PyErr_GetRaisedException());
  PyObjectPtr pTracebackMod(PyImport_ImportModule("traceback"));
  PyObjectPtr pFormatExceptionMethod(PyObject_GetAttrString(pTracebackMod.get(), "format_exception"));
  if (nullptr != pException)
  {
    PyObjectPtr pExceptionType(PyObject_Type(pException.get()));
    PyObjectPtr pExceptionTypeTb(PyException_GetTraceback(pException.get()));
    PyObjectPtr pExceptionList(PyObject_CallFunctionObjArgs(
      pFormatExceptionMethod.get(), pExceptionType.get(), pException.get(), pExceptionTypeTb.get(), nullptr));
#else
  PyObject* ptype, * pvalue, * ptraceback;
  PyErr_Fetch(&ptype, &pvalue, &ptraceback); // Deprecated since python 3.12
  PyObjectPtr pTracebackMod(PyImport_ImportModule("traceback"));
  PyObjectPtr pFormatExceptionMethod(PyObject_GetAttrString(pTracebackMod.get(), "format_exception"));
  if (nullptr != ptype)
  {
    PyObjectPtr pExceptionList(
      PyObject_CallFunctionObjArgs(pFormatExceptionMethod.get(), ptype, pvalue, ptraceback, nullptr));
#endif
    if (pExceptionList && PyList_Check(pExceptionList.get()))
    {
      PyObjectPtr pExceptionText(PyUnicode_Join(PyUnicode_FromString(""), pExceptionList.get()));
      errorMessage = pExceptionText ? PyUnicode_AsUTF8(pExceptionText.get()) : "UnknownError Caught.";
    }
    else
    {
      errorMessage = "Failed to format exception";
    }
  }
  return errorMessage;
}

