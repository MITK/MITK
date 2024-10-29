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

mitk::PythonContext::PythonContext()
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
  PyGILState_Ensure();
}

void mitk::PythonContext::Activate()
{
  PyGILState_Ensure();
  std::string programPath = mitk::IOUtil::GetProgramPath();
  std::replace(programPath.begin(), programPath.end(), '\\', '/');
  programPath.append("/");
  MITK_INFO << programPath;
  MITK_INFO << "EP_DLL_DIR " << std::string(EP_DLL_DIR);
  std::string pythonCommand;
  // execute a string which imports packages that are needed and sets paths to directories
  pythonCommand.append("import numpy\n");
  pythonCommand.append("import os, site, sys, io\n");
  pythonCommand.append("import os\n");
  pythonCommand.append("sys.path.append('" + programPath + "')\n");
  //   pythonCommand.append("sys.path.append('" + std::string(BIN_DIR) + "')\n");
  //   pythonCommand.append("sys.path.append('" +std::string(EXTERNAL_DIST_PACKAGES) + "')\n");
  //   pythonCommand.append("\nsite.addsitedir('"+std::string(EXTERNAL_SITE_PACKAGES)+"')\n");
  // in python 3.8 onwards, the path system variable is not longer used to find dlls
  // that's why the dlls that are needed for swig wrapping need to be searched manually
  std::string searchForDll = "os.add_dll_directory('"+std::string(EP_DLL_DIR)+"')\n";
  pythonCommand.append(searchForDll);
  pythonCommand.append("import SimpleITK as sitk\n");
  pythonCommand.append("import pyMITK\n");
  if (PyRun_SimpleString(pythonCommand.c_str()) == -1)
  {
    MITK_ERROR << "Something went wrong in setting the path in Python";
  }
  PyRun_SimpleString("_mitk_stdout = io.StringIO()\n"
                     "sys.stdout = _mitk_stdout\n");
  PyObject *main = PyImport_AddModule("__main__");
  m_GlobalDictionary = PyModule_GetDict(main);
  m_LocalDictionary = m_GlobalDictionary;
  m_ThreadState = PyEval_SaveThread();
}

mitk::PythonContext::~PythonContext()
{
  if (Py_IsInitialized())
  {
    Py_Finalize();
  }
}

const char *mitk::PythonContext::ExecuteString(const std::string &pyCommands)
{
  PyGILState_Ensure();
  if (PyRun_SimpleString(pyCommands.c_str()) == -1)
  {
    MITK_ERROR << "Something went wrong in Python";
  }
  return this->GetStdOut();
}

const char *mitk::PythonContext::GetStdOut()
{
  PyGILState_Ensure();
  PyObject *main_module = PyImport_AddModule("__main__");
  PyObject *capture_output = PyObject_GetAttrString(main_module, "_mitk_stdout");
  PyObject *output_string = PyObject_CallMethod(capture_output, "getvalue", nullptr);
  const char *result = PyUnicode_AsUTF8(output_string);
  Py_DECREF(output_string);
  Py_DECREF(capture_output);
  return result;
}

mitk::Image::Pointer mitk::PythonContext::LoadImageFromPython(const std::string &filePath)
{
  PyGILState_Ensure();
  mitk::Image *mitkImage;
  std::string convertToMITKImage = "_mitk_image_list = pyMITK.IOUtil.imread('" + filePath +"')\n"
                                   "_mitk_image = image_list[0]\n";
  // after executing this, the desired mitk image is available in the Python context with the variable name _mitk_image
  if (PyRun_SimpleString(convertToMITKImage.c_str()) == -1)
  {
    MITK_ERROR << "Something went wrong in Python";
  }
  PyGILState_Ensure();
  // get dictionary with variables from main context
  PyObject *main = PyImport_AddModule("__main__");
  PyObject *globals = PyModule_GetDict(main);
  // get mitk_image from Python context
  PyObject *pyImage = PyDict_GetItemString(globals, "_mitk_image");
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
  mitkImage = reinterpret_cast<mitk::Image *>(voidImage);
  MITK_INFO << "C++ received image has dimension: " << mitkImage->GetDimension();
  m_ThreadState = PyEval_SaveThread();
  return mitkImage;
}

void mitk::PythonContext::TransferBaseDataToPython(mitk::BaseData *mitkBaseData)
{
  PyGILState_Ensure();
  PyRun_SimpleString("_mitk_image = None");
  PyRun_SimpleString("def _receive(image_from_cxx):\n"
                     "    print ('receive called with %s', image_from_cxx)\n"
                     "    print('Python transferred image has dimension', image_from_cxx.GetDimension())\n"
                     "    global _mitk_image\n"
                     "    _mitk_image = image_from_cxx\n");
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
  PyGILState_Ensure();
  PyRun_SimpleString("print('The MITK image is available via variable _mitk_image')");
  // PyRun_SimpleString(
  //     "numpy_image = _mitk_image.GetAsNumpy()\n"
  //     "print('numpy image shape',numpy_image.shape)\n"
  //     "numpy_image[:]=0."
  // );
  m_ThreadState = PyEval_SaveThread();
}

void mitk::PythonContext::SetVirtualEnvironmentPath(const std::string& absolutePath)
{
  PyGILState_Ensure();
  m_CurrentVenvEnvPath.clear();
  std::string pythonCommand;
  pythonCommand.append("import sys\n"); // TODO: delete other imports are reimport
  pythonCommand.append("sys.path.insert(0, '" + absolutePath + "')\n");
  if (PyRun_SimpleString(pythonCommand.c_str()) == -1)
  {
    MITK_ERROR << "Something went wrong in setting the path in Python";
  }
  m_CurrentVenvEnvPath = absolutePath;
}

void mitk::PythonContext::ClearVirtualEnvironmentPath()
{
  PyGILState_Ensure();
  std::string pythonCommand;
  pythonCommand.append("import sys\n");
  pythonCommand.append("sys.path.remove('" + m_CurrentVenvEnvPath + "')\n");
  if (PyRun_SimpleString(pythonCommand.c_str()) == -1)
  {
    MITK_ERROR << "Something went wrong in clearing the path in Python";
  }
  m_CurrentVenvEnvPath.clear();
}
