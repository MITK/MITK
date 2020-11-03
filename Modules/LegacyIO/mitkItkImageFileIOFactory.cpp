/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkItkImageFileIOFactory.h"
#include "mitkIOAdapter.h"
#include "mitkItkImageFileReader.h"

#include "itkVersion.h"

namespace mitk
{
  ItkImageFileIOFactory::ItkImageFileIOFactory()
  {
    this->RegisterOverride("mitkIOAdapter",
                           "mitkItkImageFileReader",
                           "itk Image IO",
                           true,
                           itk::CreateObjectFunction<IOAdapter<ItkImageFileReader>>::New());
  }

  ItkImageFileIOFactory::~ItkImageFileIOFactory() {}
  const char *ItkImageFileIOFactory::GetITKSourceVersion() const { return ITK_SOURCE_VERSION; }
  const char *ItkImageFileIOFactory::GetDescription() const
  {
    return "ItkImageFile IO Factory, allows the loading of images supported by ITK";
  }

} // end namespace mitk
