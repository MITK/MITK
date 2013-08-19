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


#include <mitkLegacyImageWriterService.h>

#include <mitkImageWriter.h>

mitk::LegacyImageWriterService::LegacyImageWriterService(const std::string& basedataType, const std::string& extension,
                                                         const std::string& description)
  : LegacyFileWriterService(mitk::ImageWriter::New().GetPointer(), basedataType, extension, description)
{
}

mitk::LegacyImageWriterService* mitk::LegacyImageWriterService::Clone() const
{
  return new LegacyImageWriterService(*this);
}
