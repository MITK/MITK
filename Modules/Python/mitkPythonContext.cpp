/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkPythonContext.h"
#include <mitkIOUtil.h>
#include <mitkExceptionMacro.h>
#include <swigpyrun.h>

mitk::PythonContext::PythonContext()
  : m_GlobalDictionary(PyDict_New()), m_LocalDictionary(PyDict_New()) {}

void mitk::PythonContext::Activate()
{
  std::string programPath = mitk::IOUtil::GetProgramPath();
  std::replace(programPath.begin(), programPath.end(), '\\', '/');
  programPath.append("/");
  MITK_INFO << "Program path: " <<  programPath;
  std::string pythonCommand;
  pythonCommand.append("import os, sys, io\n");
  pythonCommand.append("sys.path.append('" + programPath + "')\n");
  pythonCommand.append("import numpy\n");
  pythonCommand.append("import pyMITK\n");
  this->ExecuteString(pythonCommand);
}

mitk::PythonContext::~PythonContext()
{
  PyDict_Clear(m_LocalDictionary.get());
  PyDict_Clear(m_GlobalDictionary.get());
}

std::string mitk::PythonContext::ExecuteFile(const std::string &filePath)
{
  PyGILState_STATE state = PyGILState_Ensure();
  int commandType = Py_file_input;
  FILE *file = fopen(filePath.c_str(), "rb");
  if (NULL == file)
  {
    mitkThrow() << "An error occured while reading python file.";
  }
  PyObjectPtr executionResult(
    PyRun_File(file, "script.py", commandType, m_GlobalDictionary.get(), m_LocalDictionary.get()));
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
    PyRun_String(pyCommands.c_str(), commandType, m_GlobalDictionary.get(), m_LocalDictionary.get()));
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
  PyObject *capture_output = PyDict_GetItemString(m_LocalDictionary.get(), varName.c_str());
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
  PyObject *pyImage = PyDict_GetItemString(m_LocalDictionary.get(), varName.c_str());
  if (pyImage == NULL && !(pyImage = PyDict_GetItemString(m_GlobalDictionary.get(), varName.c_str())))
  {
    mitkThrow() << "Could not get image from Python";
  }
  int res = 0; // status variable to check if result is OK
  void *voidImage;
  swig_type_info *pTypeInfo = nullptr;
  pTypeInfo = SWIG_TypeQuery("_p_mitk__Image");
  res = SWIG_ConvertPtr(pyImage, &voidImage, pTypeInfo, 0); // get image from Python as void pointer
  if (!SWIG_IsOK(res))
  {
    mitkThrow() << "Could not cast image to C++ type";
  }
  mitk::Image *mitkImage = reinterpret_cast<mitk::Image *>(voidImage); // cast void pointer to mitk::Image
  MITK_INFO << "C++ received image has dimension: " << mitkImage->GetDimension();
  PyGILState_Release(state);
  return mitkImage;
}

void mitk::PythonContext::TransferBaseDataToPython(mitk::BaseData *mitkBaseData, const std::string &varName)
{
  PyGILState_STATE state = PyGILState_Ensure();
  std::string pythonCommand;
  pythonCommand.append(varName);
  pythonCommand.append(" = None\n");
  pythonCommand.append("def _receive(image_from_cxx):\n"
                       "    print('receive called with %s', image_from_cxx)\n"
                       "    print('Python transferred image has dimension', image_from_cxx.GetDimension())\n"
                       "    global " + varName + "\n"
                       "    " + varName + " = image_from_cxx\n");
  this->ExecuteString(pythonCommand.c_str());
  int owned = 0;
  swig_type_info *pTypeInfo = nullptr;
  if (dynamic_cast<mitk::Image *>(mitkBaseData))
  {
    pTypeInfo = SWIG_TypeQuery("_p_mitk__Image");
  }
  else
  {
    MITK_INFO << "Object is of unsupported type";
  }
  PyObject *pInstance = SWIG_NewPointerObj(reinterpret_cast<void *>(mitkBaseData), pTypeInfo, owned);
  if (nullptr == pInstance)
  {
    MITK_ERROR << "Something went wrong creating the Python instance of the image\n";
  }
  PyObject *receive = PyDict_GetItemString(m_LocalDictionary.get(), "_receive");
  PyObjectPtr result(PyObject_CallFunctionObjArgs(receive, pInstance, NULL));
  if (nullptr == result)
  {
    MITK_ERROR << "Something went wrong setting the image in Python\n";
  }
  PyGILState_Release(state);
}

void mitk::PythonContext::SetVirtualEnvironmentPath(const std::string& absolutePath)
{
  m_CurrentUserSite.clear();
  std::string pythonCommand;
  pythonCommand.append("import site\n");
  pythonCommand.append("site.addsitedir('" + absolutePath + "')\n");
  this->ExecuteString(pythonCommand);
  m_CurrentUserSite = absolutePath;
}

void mitk::PythonContext::ClearVirtualEnvironmentPath()
{
  if (!m_CurrentUserSite.empty())
  {
    std::string pythonCommand;
    pythonCommand.append("from pathlib import Path\n");
    pythonCommand.append("_venv_path = Path('" + m_CurrentUserSite + "')\n");
    pythonCommand.append("sys_paths = list(sys.path)\n");
    pythonCommand.append("root_path = str(_venv_path.parent.parent)\n");
    pythonCommand.append("for _path in sys_paths:\n");
    pythonCommand.append("  if _path.startswith(root_path):\n");
    pythonCommand.append("    sys.path.remove(_path)\n");
    this->ExecuteString(pythonCommand);
    m_CurrentUserSite.clear();
  }
}

bool mitk::PythonContext::IsVariableExists(const std::string &varName)
{
  PyObject *pyVar = PyDict_GetItemString(m_LocalDictionary.get(), varName.c_str());
  if (pyVar == NULL && !(pyVar = PyDict_GetItemString(m_GlobalDictionary.get(), varName.c_str())))
  {
    return false;
  }
  return true;
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
      pFormatExceptionMethod.get(), pExceptionType.get(), pException.get(), pExceptionTypeTb.get(), NULL));
#else
  PyObject *ptype, *pvalue, *ptraceback;
  PyErr_Fetch(&ptype, &pvalue, &ptraceback); // Deprecated since python 3.12
  PyObjectPtr pTracebackMod(PyImport_ImportModule("traceback"));
  PyObjectPtr pFormatExceptionMethod(PyObject_GetAttrString(pTracebackMod.get(), "format_exception"));
  if (NULL != ptype)
  {
    PyObjectPtr pExceptionList(
      PyObject_CallFunctionObjArgs(pFormatExceptionMethod.get(), ptype, pvalue, ptraceback, NULL));
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
