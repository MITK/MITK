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
#include "Python.h"
#pragma pop_macro("slots")
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
    void Activate();
    bool IsVariableExists(const std::string &varName);
    mitk::Image::Pointer LoadImageFromPython(const std::string &filePath);
    void TransferBaseDataToPython(mitk::BaseData *mitkImage, const std::string &varName = "_mitk_image");
    std::string ExecuteString(const std::string &pyCommands);
    std::string ExecuteFile(const std::string &filePath);
    const char *GetStdOut();
    void SetVirtualEnvironmentPath(const std::string &absolutePath); // site-package
    void ClearVirtualEnvironmentPath();

  private:
    std::string m_CurrentVenvEnvPath;
    PyThreadState *m_ThreadState;
    PyObject *m_GlobalDictionary;
    PyObject *m_LocalDictionary;
  };
} // namespace mitk
#endif
