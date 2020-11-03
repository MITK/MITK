/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkParRecFileIOFactory.h"
#include "mitkIOAdapter.h"
#include "mitkParRecFileReader.h"

#include "itkVersion.h"

namespace mitk
{
  ParRecFileIOFactory::ParRecFileIOFactory()
  {
    this->RegisterOverride("mitkIOAdapter",
                           "mitkParRecFileReader",
                           "mitk ParRec Image IO",
                           true,
                           itk::CreateObjectFunction<IOAdapter<ParRecFileReader>>::New());
  }

  ParRecFileIOFactory::~ParRecFileIOFactory() {}
  const char *ParRecFileIOFactory::GetITKSourceVersion() const { return ITK_SOURCE_VERSION; }
  const char *ParRecFileIOFactory::GetDescription() const
  {
    return "ParRecFile IO Factory, allows the loading of ParRec images";
  }

} // end namespace mitk
