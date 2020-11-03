/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkSTLFileIOFactory.h"
#include "mitkIOAdapter.h"
#include "mitkSTLFileReader.h"

#include "itkVersion.h"

namespace mitk
{
  STLFileIOFactory::STLFileIOFactory()
  {
    this->RegisterOverride("mitkIOAdapter",
                           "mitkSTLFileReader",
                           "mitk STL Surface IO",
                           true,
                           itk::CreateObjectFunction<IOAdapter<STLFileReader>>::New());
  }

  STLFileIOFactory::~STLFileIOFactory() {}
  const char *STLFileIOFactory::GetITKSourceVersion() const { return ITK_SOURCE_VERSION; }
  const char *STLFileIOFactory::GetDescription() const { return "STLFile IO Factory, allows the loading of STL files"; }
} // end namespace mitk
