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
#include <usGetModuleContext.h>
#include <mitkIOUtil.h>

//#include <mitkBaseData.h>

mitk::LegacyFileReaderService::LegacyFileReaderService(std::string extension)
{
  if (extension == "") mitkThrow() << "LegacyFileReaderWrapper cannot be initialized without FileExtension." ;
  m_Extension = extension;
  m_Priority = 0; // Default priority for legacy reader
  RegisterMicroservice(mitk::GetModuleContext());
}

mitk::LegacyFileReaderService::~LegacyFileReaderService()
{

}

////////////////////// Reading /////////////////////////

itk::SmartPointer<mitk::BaseData> mitk::LegacyFileReaderService::Read(const std::string& path)
{
  return mitk::IOUtil::LoadDataNode(path)->GetData();
}

itk::SmartPointer<mitk::BaseData> mitk::LegacyFileReaderService::Read(const std::istream& stream )
{
  mitkThrow () << "Streaming is not supported in Legacy Wrappers.";
  return 0;
}
