/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkVtkSurfaceIOFactory.h"
#include "mitkIOAdapter.h"
#include "mitkVtkSurfaceReader.h"

#include "itkVersion.h"

namespace mitk
{
  VtkSurfaceIOFactory::VtkSurfaceIOFactory()
  {
    this->RegisterOverride("mitkIOAdapter",
                           "mitkVtkSurfaceReader",
                           "mitk Vtk Surface IO",
                           true,
                           itk::CreateObjectFunction<IOAdapter<VtkSurfaceReader>>::New());
  }

  VtkSurfaceIOFactory::~VtkSurfaceIOFactory() {}
  const char *VtkSurfaceIOFactory::GetITKSourceVersion() const { return ITK_SOURCE_VERSION; }
  const char *VtkSurfaceIOFactory::GetDescription() const
  {
    return "VtkSurface IO Factory, allows the loading of Vtk files";
  }

} // end namespace mitk
