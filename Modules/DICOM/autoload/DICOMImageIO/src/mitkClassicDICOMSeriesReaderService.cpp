/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkClassicDICOMSeriesReaderService.h"

#include <mitkClassicDICOMSeriesReader.h>

namespace mitk {

  ClassicDICOMSeriesReaderService::ClassicDICOMSeriesReaderService()
  : BaseDICOMReaderService("MITK DICOM Reader v2 (classic config)")
{
  this->RegisterService();
}

  DICOMFileReader::Pointer ClassicDICOMSeriesReaderService::GetReader(const mitk::StringList&) const
{
  mitk::ClassicDICOMSeriesReader::Pointer reader = mitk::ClassicDICOMSeriesReader::New();
  return reader.GetPointer();
};


  ClassicDICOMSeriesReaderService* ClassicDICOMSeriesReaderService::Clone() const
  {
    return new ClassicDICOMSeriesReaderService(*this);
  }

}
