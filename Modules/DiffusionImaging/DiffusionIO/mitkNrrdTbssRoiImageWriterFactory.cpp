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

#include "mitkNrrdTbssRoiImageWriterFactory.h"

#include "itkCreateObjectFunction.h"
#include "itkVersion.h"

#include <mitkNrrdTbssRoiImageWriter.h>

namespace mitk
{

NrrdTbssRoiImageWriterFactory::NrrdTbssRoiImageWriterFactory()
{
  this->RegisterOverride("IOWriter",
                         "NrrdTbssRoiImageWriter",
                         "NrrdTbssRoiImage ROI Writer",
                         1,
                         itk::CreateObjectFunction< mitk::NrrdTbssRoiImageWriter >::New());
}

NrrdTbssRoiImageWriterFactory::~NrrdTbssRoiImageWriterFactory()
{
}

const char* NrrdTbssRoiImageWriterFactory::GetITKSourceVersion() const
{
  return ITK_SOURCE_VERSION;
}

const char* NrrdTbssRoiImageWriterFactory::GetDescription() const
{
  return "NrrdTbssRoiImageWriterFactory";
}

} // end namespace mitk
