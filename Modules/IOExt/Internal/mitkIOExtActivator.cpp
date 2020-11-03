/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkIOExtActivator.h"

#include "mitkObjFileReaderService.h"
#include "mitkPlyFileReaderService.h"
#include "mitkPlyFileWriterService.h"
#include "mitkSceneFileReader.h"
#include "mitkVtkUnstructuredGridReader.h"

namespace mitk
{
  void IOExtActivator::Load(us::ModuleContext *)
  {
    m_SceneReader.reset(new SceneFileReader());

    m_VtkUnstructuredGridReader.reset(new VtkUnstructuredGridReader());
    m_ObjReader.reset(new ObjFileReaderService());

    m_PlyReader.reset(new PlyFileReaderService());
    m_ObjWriter.reset(new PlyFileWriterService());
  }

  void IOExtActivator::Unload(us::ModuleContext *) {}
}

US_EXPORT_MODULE_ACTIVATOR(mitk::IOExtActivator)
