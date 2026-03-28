/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkIOExtActivator_h
#define mitkIOExtActivator_h

#include <usModuleActivator.h>

#include <memory>

namespace mitk
{
  struct IFileReader;
  struct IFileWriter;

  class IOExtActivator : public us::ModuleActivator
  {
  public:
    void Load(us::ModuleContext *context) override;
    void Unload(us::ModuleContext *context) override;

  private:
    std::unique_ptr<IFileReader> m_SceneReader;
    std::unique_ptr<IFileReader> m_VtkUnstructuredGridReader;
    std::unique_ptr<IFileReader> m_ObjReader;
    std::unique_ptr<IFileWriter> m_ObjWriter;

    std::unique_ptr<IFileReader> m_PlyReader;
  };
}

#endif
