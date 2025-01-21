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

#include "mitkIPythonService.h"
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

  class MITKPYTHON_EXPORT PythonContext : public itk::LightObject, public mitk::IPythonService
  {
  public:
    mitkClassMacroItkParent(PythonContext, itk::LightObject);
    itkFactorylessNewMacro(Self);
    
    PythonContext();
    ~PythonContext();
    void Activate() override;
    mitk::Image::Pointer LoadImageFromPython(const std::string &filePath) override;
    void TransferBaseDataToPython(mitk::BaseData *mitkImage, const std::string &varName = "_mitk_image") override;
    std::string ExecuteString(const std::string &pyCommands) override;
    std::string ExecuteFile(const std::string &filePath);
    const char *GetStdOut() override;
    void SetVirtualEnvironmentPath(const std::string &absolutePath) override; // site-package
    void ClearVirtualEnvironmentPath() override;

  private:
    std::string m_CurrentVenvEnvPath;
    PyThreadState *m_ThreadState;
    PyObject *m_GlobalDictionary;
    PyObject *m_LocalDictionary;
  };
} // namespace mitk
#endif
