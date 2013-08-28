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

#include <mitkLegacyFileReaderService.h>
#include <mitkIOUtil.h>

#include <usGetModuleContext.h>

mitk::LegacyFileReaderService::LegacyFileReaderService(const mitk::LegacyFileReaderService& other)
  : mitk::AbstractFileReader(other)
{
}

mitk::LegacyFileReaderService::LegacyFileReaderService(const std::string& extension, const std::string& description)
  : AbstractFileReader(extension, description)
{
  if (extension.empty()) mitkThrow() << "LegacyFileReaderWrapper cannot be initialized without FileExtension.";
  m_ServiceReg = this->RegisterService();
}

mitk::LegacyFileReaderService::~LegacyFileReaderService()
{
  if (m_ServiceReg) m_ServiceReg.Unregister();
}

////////////////////// Reading /////////////////////////

std::vector< mitk::BaseData::Pointer > mitk::LegacyFileReaderService::Read(const std::string& path, mitk::DataStorage* /*ds*/)
{
  std::vector< mitk::BaseData::Pointer > result;
  result.push_back(mitk::IOUtil::LoadDataNode(path)->GetData());
  return result;
}

std::vector< mitk::BaseData::Pointer > mitk::LegacyFileReaderService::Read(const std::istream& /*stream*/, mitk::DataStorage* /*ds*/)
{
  mitkThrow () << "Streaming is not supported in Legacy Wrappers.";
  std::vector< mitk::BaseData::Pointer > result;
  return result;
}

mitk::LegacyFileReaderService* mitk::LegacyFileReaderService::Clone() const
{
  return new LegacyFileReaderService(*this);
}
