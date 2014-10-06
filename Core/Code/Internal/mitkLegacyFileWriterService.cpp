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
#include <mitkIOMimeTypes.h>
#include <mitkCustomMimeType.h>

mitk::LegacyFileWriterService::LegacyFileWriterService(mitk::FileWriter::Pointer legacyWriter,
                                                       const std::string& description)
  : AbstractFileWriter(legacyWriter->GetSupportedBaseData())
  , m_LegacyWriter(legacyWriter)
{
  this->SetMimeTypePrefix(IOMimeTypes::DEFAULT_BASE_NAME() + ".legacy.");
  this->SetDescription(description);

  CustomMimeType customMimeType;
  std::vector<std::string> extensions = legacyWriter->GetPossibleFileExtensions();
  for(std::vector<std::string>::iterator ext = extensions.begin(); ext != extensions.end(); ext++)
  {
    if (ext->empty()) continue;

    std::string extension = *ext;
    if (extension.size() > 1 && extension[0] == '*')
    {
      // remove "*"
      extension = extension.substr(1);
    }
    if (extension.size() > 1 && extension[0] == '.')
    {
      // remove "."
      extension = extension.substr(1);
    }
    std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
    customMimeType.AddExtension(extension);
  }
  this->SetMimeType(customMimeType);

  m_ServiceRegistration = RegisterService();
}

mitk::LegacyFileWriterService::~LegacyFileWriterService()
{
}

////////////////////// Writing /////////////////////////

void mitk::LegacyFileWriterService::Write()
{
  if (m_LegacyWriter.IsNull())
    mitkThrow() << "LegacyFileWriterService was incorrectly initialized: Has no LegacyFileWriter.";

  ValidateOutputLocation();

  LocalFile localFile(this);

  m_LegacyWriter->SetFileName(localFile.GetFileName().c_str());
  m_LegacyWriter->SetInput(const_cast<BaseData*>(this->GetInput()));
  m_LegacyWriter->Write();
}

mitk::IFileWriter::ConfidenceLevel mitk::LegacyFileWriterService::GetConfidenceLevel() const
{
  if (mitk::AbstractFileWriter::GetConfidenceLevel() == Unsupported) return Unsupported;
  DataNode::Pointer node = DataNode::New();
  node->SetData(const_cast<BaseData*>(this->GetInput()));
  return m_LegacyWriter->CanWriteDataType(node) ? Supported : Unsupported;
}

mitk::LegacyFileWriterService* mitk::LegacyFileWriterService::Clone() const
{
  return new LegacyFileWriterService(*this);
}
