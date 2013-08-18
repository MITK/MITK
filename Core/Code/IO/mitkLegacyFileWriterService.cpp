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
#include <usGetModuleContext.h>
#include <mitkIOUtil.h>
#include <mitkDataNode.h>
#include <mitkCoreObjectFactory.h>
#include <mitkImageWriter.h>

//#include <mitkBaseData.h>

mitk::LegacyFileWriterService::LegacyFileWriterService()
{

}

mitk::LegacyFileWriterService::LegacyFileWriterService(mitk::FileWriter::Pointer legacyWriter, std::string basedataType, std::string extension, std::string description)
{
  if (extension == "") mitkThrow() << "LegacyFileWriterWrapper cannot be initialized without FileExtension." ;
  m_Extension = extension;
  m_LegacyWriter = legacyWriter;
  m_Description = description;
  m_BasedataType = basedataType;
  m_Priority = 0; // Default priority for legacy Writer
  RegisterMicroservice(us::GetModuleContext());
}

mitk::LegacyFileWriterService::~LegacyFileWriterService()
{

}

////////////////////// Writing /////////////////////////

void mitk::LegacyFileWriterService::Write(const itk::SmartPointer<BaseData> data, const std::string& path)
{
  if (m_LegacyWriter.IsNull())
    mitkThrow() << "LegacyFileWriterService was incorrectly initialized: Has no LegacyFileWriter.";

  m_LegacyWriter->SetFileName(path.c_str());
  mitk::DataNode::Pointer node = mitk::DataNode::New();
  node->SetData(data);
  m_LegacyWriter->SetInput(node);
  m_LegacyWriter->Write();
}

void mitk::LegacyFileWriterService::Write(const itk::SmartPointer<BaseData> data, const std::istream& stream )
{
  mitkThrow () << "Streaming is not supported in Legacy Wrappers.";
  //return 0;
}

bool mitk::LegacyFileWriterService::CanWrite(const itk::SmartPointer<BaseData> data, const std::string& path) const
{
  // Default implementation only checks if extension and basedatatype are correct
  std::string pathEnd = path.substr( path.length() - m_Extension.length(), m_Extension.length() );
  if ( !(m_Extension == pathEnd)) return false;

  std::string externalDataType = data->GetNameOfClass();
  return (externalDataType == m_BasedataType);
}
