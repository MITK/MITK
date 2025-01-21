/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#ifndef mitkIPythonService_h
#define mitkIPythonService_h
#define PY_SSIZE_T_CLEAN

#include <mitkServiceInterface.h>
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

  class MITKPYTHON_EXPORT IPythonService
  {
  public:
    virtual void Activate()=0;
    virtual mitk::Image::Pointer LoadImageFromPython(const std::string &filePath)=0;
    virtual void TransferBaseDataToPython(mitk::BaseData *mitkImage, const std::string &varName = "_mitk_image") = 0;
    virtual std::string ExecuteString(const std::string &pyCommands)=0;
    virtual const char *GetStdOut()=0;
    virtual void SetVirtualEnvironmentPath(const std::string &absolutePath)=0; // site-package
    virtual void ClearVirtualEnvironmentPath()=0;

           
    virtual ~IPythonService();
    static std::string ForceLoadModule();

  private:
    std::string m_CurrentVenvEnvPath;
    PyThreadState *m_ThreadState;
    PyObject *m_GlobalDictionary;
    PyObject *m_LocalDictionary;
  };
}
MITK_DECLARE_SERVICE_INTERFACE(mitk::IPythonService, "org.mitk.services.IPythonService")

#endif
