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

#ifndef WIN32
#include <dlfcn.h>
#endif

#include <boost/algorithm/string.hpp>
#include <swigpyrun.h>

namespace 
{
  int refCount = 0;
}

mitk::PythonContext::PythonContext() : m_ThreadState(nullptr)
{
// for Linux, libpython needs to be opened before initializing the Python Interpreter to enable imports
#ifndef WIN32
  std::vector<std::string> pythonIncludeVector;
  // get libpython file to open (taken from "Python3_INCLUDE_DIR" variable from CMake to dynamically do it for different
  // python versions)
  boost::split(pythonIncludeVector, PYTHON_INCLUDE, boost::is_any_of("/"));
  std::string libPython = "lib" + pythonIncludeVector[pythonIncludeVector.size() - 1] + ".so";
  dlopen(libPython.c_str(), RTLD_LAZY | RTLD_GLOBAL);
#endif
  // Initialize Python interpreter and ensure global interpreter lock in order to execute python code safely
  if (!Py_IsInitialized())
  {
    Py_Initialize();
  }
}

void mitk::PythonContext::Activate()
{
  PyGILState_STATE state = PyGILState_Ensure();
  ::refCount++;
  std::string programPath = mitk::IOUtil::GetProgramPath();
  std::replace(programPath.begin(), programPath.end(), '\\', '/');
  programPath.append("/");
  MITK_INFO << programPath;
  MITK_INFO << "EP_DLL_DIR " << std::string(EP_DLL_DIR);
  std::string pythonCommand;
  // execute a string which imports packages that are needed and sets paths to directories
  pythonCommand.append("import os, site, sys, io\n");
  pythonCommand.append("sys.path.append('" + programPath + "')\n");
  //   pythonCommand.append("sys.path.append('" + std::string(BIN_DIR) + "')\n");
  //   pythonCommand.append("sys.path.append('" +std::string(EXTERNAL_DIST_PACKAGES) + "')\n");
  //   pythonCommand.append("\nsite.addsitedir('"+std::string(EXTERNAL_SITE_PACKAGES)+"')\n");
  // in python 3.8 onwards, the path system variable is not longer used to find dlls
  // that's why the dlls that are needed for swig wrapping need to be searched manually
  std::string searchForDll = "os.add_dll_directory('"+std::string(EP_DLL_DIR)+"')\n";
  pythonCommand.append(searchForDll);
  pythonCommand.append("import numpy\n");
  //pythonCommand.append("import SimpleITK as sitk\n");
  pythonCommand.append("import pyMITK\n");
  if (PyRun_SimpleString(pythonCommand.c_str()) == -1)
  {
    MITK_ERROR << "Something went wrong in setting the path in Python";
  }
  PyRun_SimpleString("_mitk_stdout = io.StringIO()\n"
                     "_mitk_stderr = io.StringIO\n"
                     "sys.stdout = sys.stderr = _mitk_stdout\n");
  //m_GlobalDictionary = PyDict_New();
  PyObject *main = PyImport_AddModule("__main__");
  m_GlobalDictionary = PyModule_GetDict(main);
  m_LocalDictionary = m_GlobalDictionary;

  //m_ThreadState = PyEval_SaveThread();
  PyGILState_Release(state);
}

mitk::PythonContext::~PythonContext()
{
  ::refCount--;
  //MITK_INFO << "m_GlobalDictionary: " << m_GlobalDictionary->ob_refcnt;
  //Py_XDECREF(m_GlobalDictionary);
  if (Py_IsInitialized() && ::refCount < 1)
  {
    Py_Finalize();
  }
  //MITK_INFO << "m_GlobalDictionary: " << m_GlobalDictionary->ob_refcnt;
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
    m_ThreadState = PyEval_SaveThread();
    throw;
  }
  Py_XDECREF(executionResult);
  PyGILState_Release(state);
  return std::string(this->GetStdOut());
}


std::string mitk::PythonContext::ExecuteString(const std::string &pyCommands)
{
  PyGILState_STATE state = PyGILState_Ensure();
  if (PyRun_SimpleString(pyCommands.c_str()) == -1)
  {
    MITK_ERROR << "Something went wrong in Python";
  }
  /* int commandType = Py_file_input;
  PyObject *executionResult;
  try
  {
    executionResult = PyRun_String(pyCommands.c_str(), commandType, m_GlobalDictionary, m_LocalDictionary);
  }
  catch (const mitk::Exception &)
  {
   PyErr_Print();
   //m_ThreadState = PyEval_SaveThread();
   throw;
  }
  Py_XDECREF(executionResult);*/
  PyGILState_Release(state);
  return std::string(this->GetStdOut());
}

const char *mitk::PythonContext::GetStdOut()
{
  PyGILState_STATE state = PyGILState_Ensure();
  PyObject *main_module = PyImport_AddModule("__main__");
  if (!main_module)
  {
    PyErr_Print();
    main_module = PyImport_AddModule("__main__");
  }
  PyObject *capture_output = PyObject_GetAttrString(main_module, "_mitk_stdout");
  PyObject *output_val = PyObject_CallMethod(capture_output, "getvalue", nullptr);
  const char *result = PyUnicode_AsUTF8(output_val); 
  /* Py_XDECREF(capture_output);
  Py_XDECREF(output_val);
  Py_XDECREF(main_module);*/
  PyGILState_Release(state);
  return result;
}

mitk::Image::Pointer mitk::PythonContext::LoadImageFromPython(const std::string &varName)
{
  MITK_INFO << "LoadImageFromPython";
  PyGILState_STATE state = PyGILState_Ensure();
  PyObject *main = PyImport_AddModule("__main__");
  PyObject *globals = PyModule_GetDict(main);
  PyObject *pyImage = PyDict_GetItemString(globals, varName.c_str());
  if (pyImage == NULL)
  {
    mitkThrow() << "Could not get image from Python";
  }

  // res is status variable to check if result when getting the image from Python context is OK
  int res = 0;
  void *voidImage;
  swig_type_info *pTypeInfo = nullptr;
  pTypeInfo = SWIG_TypeQuery("_p_mitk__Image");
  // get image from Python context as C++ void pointer
  res = SWIG_ConvertPtr(pyImage, &voidImage, pTypeInfo, 0);
  if (!SWIG_IsOK(res))
  {
    mitkThrow() << "Could not cast image to C++ type";
  }
  // cast C++ void pointer to mitk::Image::Pointer
  mitk::Image *mitkImage = reinterpret_cast<mitk::Image *>(voidImage);
  MITK_INFO << "C++ received image has dimension: " << mitkImage->GetDimension();
  PyGILState_Release(state);
  return mitkImage;
}

void mitk::PythonContext::TransferBaseDataToPython(mitk::BaseData *mitkBaseData, const std::string &varName)
{
  MITK_INFO << "TransferBaseDataToPython";
  PyGILState_STATE state = PyGILState_Ensure();
  std::string pythonCommand;
  pythonCommand.append(varName);
  pythonCommand.append(" = None\n");
  pythonCommand.append("def _receive(image_from_cxx):\n"
                       "    print ('receive called with %s', image_from_cxx)\n"
                       "    print('Python transferred image has dimension', image_from_cxx.GetDimension())\n"
                       "    global " + varName + "\n"
                       "    " + varName + " = image_from_cxx\n");
  //PyRun_SimpleString(pythonCommand.c_str());
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
    std::cout << "Something went wrong creating the Python instance of the image\n";
  }
  PyObject *main = PyImport_AddModule("__main__");
  PyObject *receive = PyObject_GetAttrString(main, "_receive");
  PyObject *result = PyObject_CallFunctionObjArgs(receive, pInstance, NULL);
  if (result == NULL)
  {
    std::cout << "Something went wrong setting the image in Python\n";
  }
  PyRun_SimpleString("print('The MITK image is available via variable _mitk_image')");
  // PyRun_SimpleString(
  //     "numpy_image = _mitk_image.GetAsNumpy()\n"
  //     "print('numpy image shape',numpy_image.shape)\n"
  //     "numpy_image[:]=0."
  // );
  //m_ThreadState = PyEval_SaveThread();
  /* Py_XDECREF(receive);
  Py_XDECREF(result);
  Py_XDECREF(main);*/
  PyGILState_Release(state);
}

void mitk::PythonContext::SetVirtualEnvironmentPath(const std::string& absolutePath)
{
  PyGILState_STATE state = PyGILState_Ensure();
  m_CurrentVenvEnvPath.clear();
  std::string pythonCommand;
  pythonCommand.append("import sys\n"); // TODO: delete other imports are reimport
  pythonCommand.append("sys.path.insert(0, '" + absolutePath + "')\n");
  if (PyRun_SimpleString(pythonCommand.c_str()) == -1)
  {
    MITK_ERROR << "Something went wrong in setting the path in Python";
  }
  m_CurrentVenvEnvPath = absolutePath;
  PyGILState_Release(state);
}

void mitk::PythonContext::ClearVirtualEnvironmentPath()
{
  PyGILState_STATE state = PyGILState_Ensure();
  std::string pythonCommand;
  pythonCommand.append("import sys\n");
  pythonCommand.append("sys.path.remove('" + m_CurrentVenvEnvPath + "')\n");
  if (PyRun_SimpleString(pythonCommand.c_str()) == -1)
  {
    MITK_ERROR << "Something went wrong in clearing the path in Python";
  }
  m_CurrentVenvEnvPath.clear();
  PyGILState_Release(state);
}
