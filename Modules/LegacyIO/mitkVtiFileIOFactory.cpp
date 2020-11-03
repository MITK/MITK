/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkVtiFileIOFactory.h"
#include "mitkIOAdapter.h"
#include "mitkVtiFileReader.h"

#include "itkVersion.h"

namespace mitk
{
  VtiFileIOFactory::VtiFileIOFactory()
  {
    this->RegisterOverride("mitkIOAdapter",
                           "mitkVtiFileReader",
                           "mitk Vti Image IO",
                           true,
                           itk::CreateObjectFunction<IOAdapter<VtiFileReader>>::New());
  }

  VtiFileIOFactory::~VtiFileIOFactory() {}
  const char *VtiFileIOFactory::GetITKSourceVersion() const { return ITK_SOURCE_VERSION; }
  const char *VtiFileIOFactory::GetDescription() const { return "VtiFile IO Factory, allows the loading of vti files"; }
} // end namespace mitk
