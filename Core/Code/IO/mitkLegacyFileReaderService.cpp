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

//#include <mitkBaseData.h>

mitk::LegacyFileReaderService::LegacyFileReaderService(std::string extension, mitk::FileReader* reader)
{
  if (extension == "") mitkThrow() << "LegacyFileReaderWrapper cannot be initialized without FileExtension." ;
  if (reader == 0) mitkThrow() << "LegacyFileReaderWrapper cannot be initialized without a FileReader to Wrap." ;
  m_Extension = extension;
  m_Reader = reader;
  RegisterMicroservice(mitk::GetModuleContext());
}

mitk::LegacyFileReaderService::~LegacyFileReaderService()
{

}

////////////////////// Reading /////////////////////////

itk::SmartPointer<mitk::BaseData> mitk::LegacyFileReaderService::Read(const std::string& path)
{
//  m_Reader->SetFileName(path);
//  m_Reader->
  return 0;
}

itk::SmartPointer<mitk::BaseData> mitk::LegacyFileReaderService::Read(const std::istream& stream )
{
  mitkThrow () << "Streaming is not supported in Legacy Wrappers.";
  return 0;
}

bool mitk::LegacyFileReaderService::CanRead(const std::string& path) const
{
  // Default implementation only checks if extension is correct
  std::string pathEnd = path.substr( path.length() - m_Extension.length(), m_Extension.length() );
  return (m_Extension == pathEnd);
}
