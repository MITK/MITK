/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkVtkImageIOFactory.h"
#include "mitkIOAdapter.h"
#include "mitkVtkImageReader.h"

#include "itkVersion.h"

namespace mitk
{
  VtkImageIOFactory::VtkImageIOFactory()
  {
    this->RegisterOverride("mitkIOAdapter",
                           "mitkVtkImageReader",
                           "mitk Vtk Image IO",
                           true,
                           itk::CreateObjectFunction<IOAdapter<VtkImageReader>>::New());
  }

  VtkImageIOFactory::~VtkImageIOFactory() {}
  const char *VtkImageIOFactory::GetITKSourceVersion() const { return ITK_SOURCE_VERSION; }
  const char *VtkImageIOFactory::GetDescription() const
  {
    return "VtkImage IO Factory, allows the loading of pvtk files";
  }

} // end namespace mitk
