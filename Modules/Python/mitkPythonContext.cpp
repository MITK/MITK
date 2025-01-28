/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkPythonContext.h"
#include <mitkIOUtil.h>
#ifdef _DEBUG
#undef _DEBUG
#include <Python.h>
#define _DEBUG
#else
#include <Python.h>
#endif

#include "PythonPath.h"
#include <mitkExceptionMacro.h>
#include <swigpyrun.h>

namespace 
{
  int refCount = 0;
}

mitk::PythonContext::PythonContext()
  : m_ThreadState(nullptr), m_GlobalDictionary(PyDict_New()), m_LocalDictionary(PyDict_New())
{
  ::refCount++;
}

void mitk::PythonContext::Activate()
{
  PyGILState_STATE state = PyGILState_Ensure();
  MITK_INFO << "python activate: " << ::refCount;
  std::string programPath = mitk::IOUtil::GetProgramPath();
  std::replace(programPath.begin(), programPath.end(), '\\', '/');
  programPath.append("/");
  MITK_INFO << programPath;
  MITK_INFO << "EP_DLL_DIR " << std::string(EP_DLL_DIR);
  std::string pythonCommand;
  // execute a string which imports packages that are needed and sets paths to directories
  pythonCommand.append("import os, sys, io\n");
  pythonCommand.append("sys.path.append('" + programPath + "')\n");
  //   pythonCommand.append("sys.path.append('" + std::string(BIN_DIR) + "')\n");
  //   pythonCommand.append("sys.path.append('" +std::string(EXTERNAL_DIST_PACKAGES) + "')\n");
  //   pythonCommand.append("\nsite.addsitedir('"+std::string(EXTERNAL_SITE_PACKAGES)+"')\n");
  // in python 3.8 onwards, the path system variable is not longer used to find dlls
  // that's why the dlls that are needed for swig wrapping need to be searched manually
#ifdef WIN32
  std::string searchForDll = "os.add_dll_directory('" + std::string(EP_DLL_DIR) + "')\n";
  pythonCommand.append(searchForDll);
#endif
  pythonCommand.append("import numpy\n");
  pythonCommand.append("import SimpleITK as sitk\n");
  pythonCommand.append("import pyMITK\n");
  this->ExecuteString(pythonCommand);
  //m_ThreadState = PyEval_SaveThread();
  PyGILState_Release(state);
}

mitk::PythonContext::~PythonContext()
{
  ::refCount--;
  Py_XDECREF(m_GlobalDictionary);
  Py_XDECREF(m_LocalDictionary);
}

std::string mitk::PythonContext::ExecuteFile(const std::string &filePath)
{
  PyGILState_STATE state = PyGILState_Ensure();
  int commandType = Py_file_input;
  PyObject *executionResult;
  try
  {
    FILE *file = fopen(filePath.c_str(), "rb");
    executionResult = PyRun_File(file, "script.py", commandType, m_GlobalDictionary, m_LocalDictionary);
  }
  catch (const mitk::Exception &)
  {
    PyErr_Print();
    //m_ThreadState = PyEval_SaveThread();
    throw;
  }
  Py_XDECREF(executionResult);
  PyGILState_Release(state);
  return std::string(this->GetStdOut());
}

std::string mitk::PythonContext::ExecuteString(const std::string &pyCommands)
{
  PyGILState_STATE state = PyGILState_Ensure();
  int commandType = Py_file_input;
  PyObject *executionResult = PyRun_String(pyCommands.c_str(), commandType, m_GlobalDictionary, m_LocalDictionary);

  MITK_INFO << "Working on the following keys:";
  PyObject *key;
  PyObject *keys = PyDict_Keys(m_GlobalDictionary);
  for (int i = 0; i < PyList_Size(keys); i++)
  {
    key = PyList_GetItem(keys, i);
    MITK_INFO << " Key :" << PyUnicode_AsUTF8(key);
    // Py_DECREF(key);
  }
  MITK_INFO << "----";
  // Py_DECREF(keys);
  if (executionResult)
  {
    PyObject *objectsRepresentation = PyObject_Repr(executionResult);
    const char *resultChar = PyUnicode_AsUTF8(objectsRepresentation);
    MITK_INFO << "executionResult: " << std::string(resultChar);
    //m_ThreadState = PyEval_SaveThread();
  }
  else
  {
    PyErr_Print();
    mitkThrow() << "An error occured while running the Python code";
  }
  Py_XDECREF(executionResult);
  PyGILState_Release(state);
  return std::string(this->GetStdOut());
}

const char *mitk::PythonContext::GetStdOut()
{
  PyGILState_STATE state = PyGILState_Ensure();
  const char *result = "nothing";
  PyObject *capture_output = PyDict_GetItemString(m_LocalDictionary, "_mitk_stdout");
  if (capture_output != NULL)
  {
    PyObject *output_val = PyObject_CallMethod(capture_output, "getvalue", nullptr);
    result = PyUnicode_AsUTF8(output_val);
    MITK_INFO << "result: " << result;
    Py_XDECREF(output_val);
  }
  else
  {
    PyErr_Print();
  }
  PyGILState_Release(state);
  return result;
}

mitk::Image::Pointer mitk::PythonContext::LoadImageFromPython(const std::string &varName)
{
  PyGILState_STATE state = PyGILState_Ensure();
  PyObject *pyImage = PyDict_GetItemString(m_LocalDictionary, varName.c_str());
  if (pyImage == NULL &&
    !(pyImage = PyDict_GetItemString(m_GlobalDictionary, varName.c_str())))
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
  if (pInstance == NULL)
  {
    MITK_ERROR << "Something went wrong creating the Python instance of the image\n";
  }
  PyObject *receive = PyDict_GetItemString(m_LocalDictionary, "_receive");
  PyObject *result = PyObject_CallFunctionObjArgs(receive, pInstance, NULL);
  if (result == NULL)
  {
    MITK_ERROR << "Something went wrong setting the image in Python\n";
  }
  //PyRun_SimpleString("print('The MITK image is available via variable _mitk_image')");
  // PyRun_SimpleString(
  //     "numpy_image = _mitk_image.GetAsNumpy()\n"
  //     "print('numpy image shape',numpy_image.shape)\n"
  //     "numpy_image[:]=0."
  // );
  //m_ThreadState = PyEval_SaveThread();
  Py_XDECREF(result);
  PyGILState_Release(state);
}

void mitk::PythonContext::SetVirtualEnvironmentPath(const std::string& absolutePath)
{
  PyGILState_STATE state = PyGILState_Ensure();
  m_CurrentVenvEnvPath.clear();
  std::string pythonCommand;
  pythonCommand.append("import sys\n"); // TODO: delete other imports are reimport
  pythonCommand.append("sys.path.insert(0, '" + absolutePath + "')\n");
  this->ExecuteString(pythonCommand);
  m_CurrentVenvEnvPath = absolutePath;
  PyGILState_Release(state);
}

void mitk::PythonContext::ClearVirtualEnvironmentPath()
{
  PyGILState_STATE state = PyGILState_Ensure();
  std::string pythonCommand;
  pythonCommand.append("import sys\n");
  pythonCommand.append("sys.path.remove('" + m_CurrentVenvEnvPath + "')\n");
  this->ExecuteString(pythonCommand);
  m_CurrentVenvEnvPath.clear();
  PyGILState_Release(state);
}
