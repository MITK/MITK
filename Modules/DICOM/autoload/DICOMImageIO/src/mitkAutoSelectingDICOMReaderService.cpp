/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkAutoSelectingDICOMReaderService.h"

#include <mitkDICOMFileReaderSelector.h>

namespace mitk {

AutoSelectingDICOMReaderService::AutoSelectingDICOMReaderService()
  : BaseDICOMReaderService("MITK DICOM Reader v2 (autoselect)")
{
  this->SetRanking(5);
  this->RegisterService();
}

DICOMFileReader::Pointer AutoSelectingDICOMReaderService::GetReader(const mitk::StringList& relevantFiles) const
{
  mitk::DICOMFileReaderSelector::Pointer selector = mitk::DICOMFileReaderSelector::New();

  selector->LoadBuiltIn3DConfigs();
  selector->LoadBuiltIn3DnTConfigs();
  selector->SetInputFiles(relevantFiles);

  mitk::DICOMFileReader::Pointer reader = selector->GetFirstReaderWithMinimumNumberOfOutputImages();
  if(reader.IsNotNull())
  {
      //reset tag cache to ensure that additional tags of interest
      //will be regarded by the reader if set later on.
      reader->SetTagCache(nullptr);
  }

  return reader;
};

AutoSelectingDICOMReaderService* AutoSelectingDICOMReaderService::Clone() const
{
  return new AutoSelectingDICOMReaderService(*this);
}

}
