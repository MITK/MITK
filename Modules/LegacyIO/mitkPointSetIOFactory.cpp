/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkPointSetIOFactory.h"
#include "mitkIOAdapter.h"
#include "mitkPointSetReader.h"

#include "itkVersion.h"

namespace mitk
{
  PointSetIOFactory::PointSetIOFactory()
  {
    this->RegisterOverride("mitkIOAdapter",
                           "mitkPointSetReader",
                           "mitk PointSet IO",
                           true,
                           itk::CreateObjectFunction<IOAdapter<PointSetReader>>::New());
  }

  PointSetIOFactory::~PointSetIOFactory() {}
  const char *PointSetIOFactory::GetITKSourceVersion() const { return ITK_SOURCE_VERSION; }
  const char *PointSetIOFactory::GetDescription() const
  {
    return "PointSet IO Factory, allows the loading of MITK pointsets";
  }

} // end namespace mitk
