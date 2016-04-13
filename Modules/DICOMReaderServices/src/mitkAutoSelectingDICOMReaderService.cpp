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

#include "mitkAutoSelectingDICOMReaderService.h"

#include <mitkDICOMFileReaderSelector.h>

namespace mitk {

AutoSelectingDICOMReaderService::AutoSelectingDICOMReaderService()
  : BaseDICOMReaderService("MITK DICOM Reader v2 (autoselect)")
{
  this->RegisterService();
}

DICOMFileReader::Pointer AutoSelectingDICOMReaderService::GetReader(const mitk::StringList& relevantFiles) const
{
  mitk::DICOMFileReaderSelector::Pointer selector = mitk::DICOMFileReaderSelector::New();

  selector->LoadBuiltIn3DConfigs();
  selector->LoadBuiltIn3DnTConfigs();
  selector->SetInputFiles(relevantFiles);

  mitk::DICOMFileReader::Pointer reader = selector->GetFirstReaderWithMinimumNumberOfOutputImages();
  return reader;
};

AutoSelectingDICOMReaderService* AutoSelectingDICOMReaderService::Clone() const
{
  return new AutoSelectingDICOMReaderService(*this);
}

}
