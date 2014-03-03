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

#include "mitkLabelSetImageIOFactory.h"
#include "mitkIOAdapter.h"
#include "mitkLabelSetImageReader.h"

#include "itkVersion.h"


namespace mitk
{
  LabelSetImageIOFactory::LabelSetImageIOFactory()
  {
    this->RegisterOverride("mitkIOAdapter",
      "mitkLabelSetImageReader",
      "mitk LabelSetImage IO",
      1,
      itk::CreateObjectFunction<IOAdapter<LabelSetImageReader> >::New());
  }

  LabelSetImageIOFactory::~LabelSetImageIOFactory()
  {
  }

  const char* LabelSetImageIOFactory::GetITKSourceVersion() const
  {
    return ITK_SOURCE_VERSION;
  }

  const char* LabelSetImageIOFactory::GetDescription() const
  {
    return "LabelSetImage IO Factory, allows the loading of MITK LabelSetImages";
  }

}
