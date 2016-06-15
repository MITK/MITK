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

#include "mitkClassicDICOMSeriesReaderService.h"

#include <mitkClassicDICOMSeriesReader.h>

namespace mitk {

  ClassicDICOMSeriesReaderService::ClassicDICOMSeriesReaderService()
  : BaseDICOMReaderService("MITK DICOM Reader v2 (classic config)")
{
  this->RegisterService();
}

  DICOMFileReader::Pointer ClassicDICOMSeriesReaderService::GetReader(const mitk::StringList& relevantFiles) const
{
  mitk::ClassicDICOMSeriesReader::Pointer reader = mitk::ClassicDICOMSeriesReader::New();
  return reader.GetPointer();
};


  ClassicDICOMSeriesReaderService* ClassicDICOMSeriesReaderService::Clone() const
  {
    return new ClassicDICOMSeriesReaderService(*this);
  }

}
