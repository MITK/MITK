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

//#include <mitkBaseData.h>

mitk::LegacyFileWriterService::LegacyFileWriterService(std::string extension, std::string description)
{
  if (extension == "") mitkThrow() << "LegacyFileWriterWrapper cannot be initialized without FileExtension." ;
  m_Extension = extension;
  m_Description = description;
  m_Priority = 0; // Default priority for legacy Writer
  RegisterMicroservice(mitk::GetModuleContext());
}

mitk::LegacyFileWriterService::~LegacyFileWriterService()
{

}

////////////////////// Reading /////////////////////////

void mitk::LegacyFileWriterService::Write(const itk::SmartPointer<BaseData> data, const std::string& path)
{
 // return mitk::IOUtil::LoadDataNode(path)->GetData();
}

void mitk::LegacyFileWriterService::Write(const itk::SmartPointer<BaseData> data, const std::istream& stream )
{
  mitkThrow () << "Streaming is not supported in Legacy Wrappers.";
  //return 0;
}
