/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkManualSelectingDICOMReaderService.h"

mitk::ManualSelectingDICOMReaderService::ManualSelectingDICOMReaderService()
  : BaseDICOMReaderService("MITK DICOM Reader v2 (manual)"), m_Selector(DICOMFileReaderSelector::New())
{
  Options defaultOptions;

  m_Selector->LoadBuiltIn3DConfigs();
  m_Selector->LoadBuiltIn3DnTConfigs();

  auto readers = m_Selector->GetAllConfiguredReaders();

  std::vector<std::string> configs;
  for (const auto& reader : readers)
  {
    configs.push_back(reader->GetConfigurationLabel());
  }
  defaultOptions["Configuration"] = configs;

  this->SetDefaultOptions(defaultOptions);

  this->RegisterService();
}

mitk::DICOMFileReader::Pointer mitk::ManualSelectingDICOMReaderService::GetReader(const mitk::StringList& /*relevantFiles*/) const
{
  const auto label = this->GetOption("Configuration").ToString();

  DICOMFileReader::Pointer selectedReader;
  
  auto readers = m_Selector->GetAllConfiguredReaders();
  for (const auto& reader : readers)
  {
    if (label == reader->GetConfigurationLabel())
    {
      selectedReader = reader;
    }
  }

  return selectedReader;
}

mitk::ManualSelectingDICOMReaderService* mitk::ManualSelectingDICOMReaderService::Clone() const
{
  return new ManualSelectingDICOMReaderService(*this);
}
