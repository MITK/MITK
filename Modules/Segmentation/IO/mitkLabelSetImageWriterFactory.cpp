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

#include "mitkLabelSetImageWriterFactory.h"

#include "itkCreateObjectFunction.h"
#include "itkVersion.h"

#include <mitkLabelSetImageWriter.h>

namespace mitk
{

LabelSetImageWriterFactory::LabelSetImageWriterFactory()
{
  this->RegisterOverride("IOWriter",
                         "LabelSetImageWriter",
                         "Nrrd LabelSet Image Writer",
                         1,
                         itk::CreateObjectFunction< mitk::LabelSetImageWriter >::New());
}

LabelSetImageWriterFactory::~LabelSetImageWriterFactory()
{
}

const char* LabelSetImageWriterFactory::GetITKSourceVersion() const
{
  return ITK_SOURCE_VERSION;
}

const char* LabelSetImageWriterFactory::GetDescription() const
{
  return "LabelSetImageWriterFactory";
}

} // end namespace mitk
