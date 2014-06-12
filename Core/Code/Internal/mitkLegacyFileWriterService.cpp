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


#include "mitkLegacyFileWriterService.h"

#include <mitkDataNode.h>

#include <usGetModuleContext.h>

mitk::LegacyFileWriterService::LegacyFileWriterService(mitk::FileWriterWithInformation::Pointer legacyWriter,
                                                       const std::string& basedataType,
                                                       const std::string& extension,
                                                       const std::string& description)
  : AbstractFileWriter(basedataType, extension, description)
  , m_LegacyWriter(legacyWriter)
{
  if (extension.empty()) mitkThrow() << "LegacyFileWriterWrapper cannot be initialized without FileExtension." ;

  m_ServiceRegistration = RegisterService();
}

mitk::LegacyFileWriterService::~LegacyFileWriterService()
{
  try
  {
    m_ServiceRegistration.Unregister();
  }
  catch (const std::exception&)
  {}
}

////////////////////// Writing /////////////////////////

void mitk::LegacyFileWriterService::Write(const BaseData* data, const std::string& path)
{
  if (m_LegacyWriter.IsNull())
    mitkThrow() << "LegacyFileWriterService was incorrectly initialized: Has no LegacyFileWriter.";

  m_LegacyWriter->SetFileName(path.c_str());
  m_LegacyWriter->DoWrite(mitk::BaseData::Pointer(const_cast<BaseData*>(data)));
}

void mitk::LegacyFileWriterService::Write(const BaseData* data, std::ostream& stream)
{
  mitk::AbstractFileWriter::Write(data, stream);
}

bool mitk::LegacyFileWriterService::CanWrite(const mitk::BaseData* data) const
{
  return mitk::AbstractFileWriter::CanWrite(data) &&
      m_LegacyWriter->CanWriteBaseDataType(mitk::BaseData::Pointer(const_cast<BaseData*>(data)));
}

mitk::LegacyFileWriterService* mitk::LegacyFileWriterService::Clone() const
{
  return new LegacyFileWriterService(*this);
}
