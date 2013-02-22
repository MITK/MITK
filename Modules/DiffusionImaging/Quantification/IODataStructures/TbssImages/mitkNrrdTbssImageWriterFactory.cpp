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

#include "mitkNrrdTbssImageWriterFactory.h"

#include "itkCreateObjectFunction.h"
#include "itkVersion.h"

#include <mitkNrrdTbssImageWriter.h>

namespace mitk
{

NrrdTbssImageWriterFactory::NrrdTbssImageWriterFactory()
{
  typedef float TbssPixelType;
  this->RegisterOverride("IOWriter",
                         "NrrdTbssImageWriter",
                         "NrrdTbssImage Writer",
                         1,
                         itk::CreateObjectFunction< mitk::NrrdTbssImageWriter >::New());
}

NrrdTbssImageWriterFactory::~NrrdTbssImageWriterFactory()
{
}

const char* NrrdTbssImageWriterFactory::GetITKSourceVersion() const
{
  return ITK_SOURCE_VERSION;
}

const char* NrrdTbssImageWriterFactory::GetDescription() const
{
  return "NrrdTbssImageWriterFactory";
}

} // end namespace mitk
