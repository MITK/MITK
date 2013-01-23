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

#include "mitkContourModelIOFactory.h"
#include "mitkIOAdapter.h"
#include "mitkContourModelReader.h"

#include "itkVersion.h"


namespace mitk
{
  ContourModelIOFactory::ContourModelIOFactory()
  {
    this->RegisterOverride("mitkIOAdapter",
      "mitkContourModelReader",
      "mitk ContourModel IO",
      1,
      itk::CreateObjectFunction<IOAdapter<ContourModelReader> >::New());
  }

  ContourModelIOFactory::~ContourModelIOFactory()
  {
  }

  const char* ContourModelIOFactory::GetITKSourceVersion() const
  {
    return ITK_SOURCE_VERSION;
  }

  const char* ContourModelIOFactory::GetDescription() const
  {
    return "ContourModel IO Factory, allows the loading of MITK ContourModels";
  }

}
