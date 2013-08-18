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


#include <mitkAbstractFileWriter.h>
#include <usGetModuleContext.h>
#include <itksys/SystemTools.hxx>
#include <fstream>



mitk::AbstractFileWriter::AbstractFileWriter() :
m_Priority (0)
{
}

mitk::AbstractFileWriter::AbstractFileWriter(std::string basedataType, std::string extension, std::string description) :
m_Extension (extension),
m_BasedataType(basedataType),
m_Description (description),
m_Priority (0)
{
}

mitk::AbstractFileWriter::~AbstractFileWriter()
{

}

////////////////// Filenames etc. //////////////////

std::string mitk::AbstractFileWriter::GetFileName() const
{
  return m_FileName;
}

void mitk::AbstractFileWriter::SetFileName(const std::string aFileName)
{
 m_FileName = aFileName;
}

//std::string mitk::AbstractFileWriter::GetFilePrefix() const
//{
//  return m_FilePrefix;
//}
//
//void mitk::AbstractFileWriter::SetFilePrefix(const std::string& aFilePrefix)
//{
//  m_FilePrefix = aFilePrefix;
//}
//
//std::string mitk::AbstractFileWriter::GetFilePattern() const
//{
//  return m_FilePattern;
//}
//
//void mitk::AbstractFileWriter::SetFilePattern(const std::string& aFilePattern)
//{
//  m_FilePattern = aFilePattern;
//}

////////////////////// Writing /////////////////////////

void mitk::AbstractFileWriter::Write(const itk::SmartPointer<BaseData> data, const std::string& path)
{
  if (! itksys::SystemTools::FileExists(path.c_str()))
    mitkThrow() << "File '" + path + "' not found.";
  std::ifstream stream;
  stream.open(path.c_str());
  this->Write(data, stream);
}

//////////// µS Registration & Properties //////////////

void mitk::AbstractFileWriter::RegisterMicroservice(us::ModuleContext* context)
{
  us::ServiceProperties props = this->ConstructServiceProperties();
  m_Registration = context->RegisterService<mitk::IFileWriter>(this, props);
}

void mitk::AbstractFileWriter::UnregisterMicroservice(us::ModuleContext* context)
{
  if (! m_Registration )
  {
    MITK_WARN << "Someone tried to unregister a FileWriter, but it was either not registered or the registration has expired.";
    return;
  }

  m_Registration.Unregister();
}

us::ServiceProperties mitk::AbstractFileWriter::ConstructServiceProperties()
{
  if ( m_Extension == "" )
    MITK_WARN << "Registered a Writer with no extension defined (m_Extension is empty). Writer will not be found by calls from WriterManager.)";
  if ( m_BasedataType == "" )
    MITK_WARN << "Registered a Writer with no BasedataType defined (m_BasedataType is empty). Writer will not be found by calls from WriterManager.)";
  if ( m_Description == "" )
    MITK_WARN << "Registered a Writer with no description defined (m_Description is empty). Writer will have no human readable extension information in FileDialogs.)";

  us::ServiceProperties result;
  result[mitk::IFileWriter::PROP_EXTENSION]    = m_Extension;
  result[mitk::IFileWriter::PROP_DESCRIPTION]    = m_Description;
  result[mitk::IFileWriter::PROP_BASEDATA_TYPE]    = m_BasedataType;
  result[us::ServiceConstants::SERVICE_RANKING()]  = m_Priority;

  for (std::list<std::string>::const_iterator it = m_Options.begin(); it != m_Options.end(); ++it) {
    result[*it] = std::string("true");
  }
  return result;
}

//////////////////////// Options ///////////////////////

std::list< std::string > mitk::AbstractFileWriter::GetSupportedOptions() const
{
  return m_Options;
}

////////////////// MISC //////////////////

bool mitk::AbstractFileWriter::CanWrite(const itk::SmartPointer<BaseData> data, const std::string& path) const
{
  // Default implementation only checks if extension and basedatatype are correct
  std::string pathEnd = path.substr( path.length() - m_Extension.length(), m_Extension.length() );
  if ( !(m_Extension == pathEnd)) return false;

  std::string externalDataType = data->GetNameOfClass();
  return (externalDataType == m_BasedataType);
}

float mitk::AbstractFileWriter::GetProgress() const
{
  // Default implementation always returns 1 (finished)
  return 1;
}

////////////////// µS related Getters //////////////////

int mitk::AbstractFileWriter::GetPriority() const
{
  return m_Priority;
}

std::string mitk::AbstractFileWriter::GetExtension() const
{
  return m_Extension;
}

std::string mitk::AbstractFileWriter::GetDescription() const
{
  return m_Description;
}

std::string mitk::AbstractFileWriter::GetSupportedBasedataType() const
{
  return m_BasedataType;
}
