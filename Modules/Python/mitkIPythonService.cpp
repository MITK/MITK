/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#define PY_SSIZE_T_CLEAN

#include "mitkIPythonService.h"
#include <Python.h>
#ifndef WIN32
#include <dlfcn.h>
#include "PythonPath.h"
#endif
#include <boost/algorithm/string.hpp>

mitk::IPythonService::IPythonService()
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
}
