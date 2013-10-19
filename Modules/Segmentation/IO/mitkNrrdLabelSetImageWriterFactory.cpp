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

#include "mitkNrrdLabelSetImageWriterFactory.h"

#include "itkCreateObjectFunction.h"
#include "itkVersion.h"

#include <mitkNrrdLabelSetImageWriter.h>

namespace mitk
{

NrrdLabelSetImageWriterFactory::NrrdLabelSetImageWriterFactory()
{
  this->RegisterOverride("IOWriter",
                         "NrrdLabelSetImageWriter",
                         "Nrrd LabelSet Image Writer",
                         1,
                         itk::CreateObjectFunction< mitk::NrrdLabelSetImageWriter >::New());
}

NrrdLabelSetImageWriterFactory::~NrrdLabelSetImageWriterFactory()
{
}

const char* NrrdLabelSetImageWriterFactory::GetITKSourceVersion() const
{
  return ITK_SOURCE_VERSION;
}

const char* NrrdLabelSetImageWriterFactory::GetDescription() const
{
  return "NrrdLabelSetImageWriterFactory";
}

} // end namespace mitk
