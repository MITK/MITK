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
                         1,
                         itk::CreateObjectFunction<IOAdapter<VtkSurfaceReader> >::New());
}

VtkSurfaceIOFactory::~VtkSurfaceIOFactory()
{
}

const char* VtkSurfaceIOFactory::GetITKSourceVersion() const
{
  return ITK_SOURCE_VERSION;
}

const char* VtkSurfaceIOFactory::GetDescription() const
{
  return "VtkSurface IO Factory, allows the loading of Vtk files";
}

} // end namespace mitk
