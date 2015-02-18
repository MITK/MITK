/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#include "mitkIOExtActivator.h"

#include "mitkSceneFileReader.h"
#include "mitkObjFileReaderService.h"
#include "mitkVtkUnstructuredGridReader.h"
#include "mitkPlyFileWriterService.h"
#include "mitkPlyFileReaderService.h"


namespace mitk {

void IOExtActivator::Load(us::ModuleContext*)
{
  m_SceneReader.reset(new SceneFileReader());

  m_VtkUnstructuredGridReader.reset(new VtkUnstructuredGridReader());
  m_ObjReader.reset(new ObjFileReaderService());

  m_PlyReader.reset(new PlyFileReaderService());
  m_ObjWriter.reset(new PlyFileWriterService());
}

void IOExtActivator::Unload(us::ModuleContext*)
{
}

}

US_EXPORT_MODULE_ACTIVATOR(mitk::IOExtActivator)
