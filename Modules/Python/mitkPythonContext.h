/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#ifndef mitkPythonService_h
#define mitkPythonService_h
#define PY_SSIZE_T_CLEAN

#include <itkLightObject.h>
#include <mitkImage.h>
#ifdef _DEBUG
#undef _DEBUG
#include <Python.h>
#define _DEBUG
#else
#include <Python.h>
#endif
#include <MitkPythonExports.h>
#include "mitkCommon.h"

namespace mitk
{

  class MITKPYTHON_EXPORT PythonContext : public itk::LightObject
  {
  public:
    mitkClassMacroItkParent(PythonContext, itk::LightObject);
    itkFactorylessNewMacro(Self);
    
    PythonContext();
    ~PythonContext();
    void Activate(); // deactivate
    mitk::Image::Pointer LoadImageFromPython(const std::string &filePath);
    void TransferBaseDataToPython(mitk::BaseData *mitkImage);
    const char *ExecuteString(const std::string &pyCommands);
    const char *GetStdOut();
    void SetVenvPath(std::string &); // site-package

  private:
    PyThreadState *m_ThreadState;
    PyObject *m_GlobalDictionary;
    PyObject *m_LocalDictionary;
  };
} // namespace mitk
#endif
