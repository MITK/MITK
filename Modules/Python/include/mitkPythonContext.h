/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#ifndef mitkPythonContext_h
#define mitkPythonContext_h

#define PY_SSIZE_T_CLEAN

#include <itkLightObject.h>
#include <mitkImage.h>

#pragma push_macro("slots")
#undef slots
#include <Python.h>
#pragma pop_macro("slots")

#include <MitkPythonExports.h>
#include <mitkCommon.h>

namespace mitk
{
  struct PyObjectDeleter
  {
    void operator()(PyObject *obj) const { Py_XDECREF(obj); }
  };

  using PyObjectPtr = std::unique_ptr<PyObject, PyObjectDeleter>;

  class MITKPYTHON_EXPORT PythonContext : public itk::LightObject
  {
  public:
    mitkClassMacroItkParent(PythonContext, itk::LightObject);
    itkFactorylessNewMacro(Self);

    PythonContext();
    ~PythonContext();

    /**
     * @brief Imports essential python packages: numpy, os, sys, io
     * and pyMITK. Also adds current bin folder to path.
     */
    void Activate();

    /**
     * @brief Check if given variable exists in the current context defined by
     * `globals` (m_GlobalDictionary) & `locals` (m_LocalDictionary) namespaces.
     */
    bool HasVariable(const std::string &varName);

    /**
     * @brief Provides view into mitk::Image type object in Python
     * as mitk::Image* pointer in MITK.
     */
    mitk::Image* LoadImageFromPython(const std::string &varName);

    /**
     * @brief Creates view of mitk::BaseData pointer in MITK into corresponding SWIG proxy
     * type object in Python.
     */
    void TransferBaseDataToPython(mitk::BaseData *mitkImage, const std::string &varName = "_mitk_image");

    /**
     * @brief Executes the given python syntax in the current context defined by
     * `globals` (m_GlobalDictionary) & `locals` (m_LocalDictionary) namespaces.
     */
    std::string ExecuteString(const std::string &pyCommands);

    /**
     * @brief Executes the given python file in the current context defined by
     * `globals` (m_GlobalDictionary) & `locals` (m_LocalDictionary)
     */
    std::string ExecuteFile(const std::string &filePath);

    /**
     * @brief Returns any exception stacktrace occured in python as string back
     * to MITK.
     */
    std::string GetPythonExceptionTraceback();

    /**
     * @brief Returns value from the given string stream object.
     * See: https://docs.python.org/3/library/io.html#io.StringIO.getvalue
     */
    std::string GetStdOut(const std::string &varName = "_mitk_stdout");

  private:
    PyObjectPtr m_GlobalDictionary;
    PyObjectPtr m_LocalDictionary;
  };
}

#endif

