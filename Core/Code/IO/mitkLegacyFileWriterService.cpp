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


#include <mitkLegacyFileWriterService.h>

#include <mitkDataNode.h>

#include <usGetModuleContext.h>

mitk::LegacyFileWriterService::LegacyFileWriterService(mitk::FileWriter::Pointer legacyWriter,
                                                       const std::string& basedataType,
                                                       const std::string& extension,
                                                       const std::string& description)
  : AbstractFileWriter(basedataType, extension, description)
  , m_LegacyWriter(legacyWriter)
{
  if (extension.empty()) mitkThrow() << "LegacyFileWriterWrapper cannot be initialized without FileExtension." ;

  RegisterService();
}

////////////////////// Writing /////////////////////////

void mitk::LegacyFileWriterService::Write(const BaseData* data, const std::string& path)
{
  if (m_LegacyWriter.IsNull())
    mitkThrow() << "LegacyFileWriterService was incorrectly initialized: Has no LegacyFileWriter.";

  m_LegacyWriter->SetFileName(path.c_str());
  mitk::DataNode::Pointer node = mitk::DataNode::New();
  node->SetData(const_cast<BaseData*>(data));
  m_LegacyWriter->SetInput(node);
  m_LegacyWriter->Write();
}

void mitk::LegacyFileWriterService::Write(const BaseData* data, std::ostream& stream)
{
  mitk::AbstractFileWriter::Write(data, stream);
}

mitk::LegacyFileWriterService* mitk::LegacyFileWriterService::Clone() const
{
  return new LegacyFileWriterService(*this);
}
