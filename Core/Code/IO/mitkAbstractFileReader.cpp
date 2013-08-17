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


#include <mitkAbstractFileReader.h>

#include <usGetModuleContext.h>
#include <usModuleContext.h>

#include <itksys/SystemTools.hxx>

#include <fstream>


mitk::AbstractFileReader::AbstractFileReader() :
m_Priority (0)
{
}

mitk::AbstractFileReader::AbstractFileReader(const std::string& extension, const std::string& description) :
m_Extension (extension),
m_Description (description),
m_Priority (0)
{
}


////////////////// Filenames etc. //////////////////

std::string mitk::AbstractFileReader::GetFileName() const
{
  return m_FileName;
}

void mitk::AbstractFileReader::SetFileName(const std::string& aFileName)
{
 m_FileName = aFileName;
}


// FILE PATTERN mechanic is currently deactivated until decision has been reached on how to handle patterns //


//std::string mitk::AbstractFileReader::GetFilePrefix() const
//{
//  return m_FilePrefix;
//}
//
//void mitk::AbstractFileReader::SetFilePrefix(const std::string& aFilePrefix)
//{
//  m_FilePrefix = aFilePrefix;
//}
//
//std::string mitk::AbstractFileReader::GetFilePattern() const
//{
//  return m_FilePattern;
//}
//
//void mitk::AbstractFileReader::SetFilePattern(const std::string& aFilePattern)
//{
//  m_FilePattern = aFilePattern;
//}

////////////////////// Reading /////////////////////////

std::list< itk::SmartPointer<mitk::BaseData> > mitk::AbstractFileReader::Read(const std::string& path, mitk::DataStorage* /*ds*/)
{
  if (! itksys::SystemTools::FileExists(path.c_str()))
    mitkThrow() << "File '" + path + "' not found.";
  std::ifstream stream;
  stream.open(path.c_str());
  return this->Read(stream);
}

//////////// µS Registration & Properties //////////////

void mitk::AbstractFileReader::RegisterMicroservice(us::ModuleContext* context)
{
  us::ServiceProperties props = this->ConstructServiceProperties();
  m_Registration = context->RegisterService<mitk::IFileReader>(this, props);
}

void mitk::AbstractFileReader::UnregisterMicroservice(us::ModuleContext* /*context*/)
{
  if (! m_Registration )
  {
    MITK_WARN << "Someone tried to unregister a FileReader, but it was either not registered or the registration has expired.";
    return;
  }

  m_Registration.Unregister();
}

us::ServiceProperties mitk::AbstractFileReader::ConstructServiceProperties()
{
  if ( m_Extension.empty() )
    MITK_WARN << "Registered a Reader with no extension defined (m_Extension is empty). Reader will not be found by calls from ReaderManager.)";
  if ( m_Description.empty() )
    MITK_WARN << "Registered a Reader with no description defined (m_Description is empty). Reader will have no human readable extension information in FileDialogs.)";
  std::transform(m_Extension.begin(), m_Extension.end(), m_Extension.begin(), ::tolower);

  us::ServiceProperties result;
  result[mitk::IFileReader::PROP_EXTENSION]    = m_Extension;
  result[mitk::IFileReader::PROP_DESCRIPTION]    = m_Description;
  result[us::ServiceConstants::SERVICE_RANKING()]  = m_Priority;

  for (std::list<std::string>::const_iterator it = m_Options.begin(); it != m_Options.end(); ++it) {
    result[*it] = std::string("true");
  }
  return result;
}

//////////////////////// Options ///////////////////////

std::list< std::string > mitk::AbstractFileReader::GetSupportedOptions() const
{
  return m_Options;
}

////////////////// MISC //////////////////

bool mitk::AbstractFileReader::CanRead(const std::string& path) const
{
  // Default implementation only checks if extension is correct
  std::string pathEnd = path.substr( path.length() - m_Extension.length(), m_Extension.length() );
  return (m_Extension == pathEnd);
}

float mitk::AbstractFileReader::GetProgress() const
{
  // Default implementation always returns 1 (finished)
  return 1;
}

////////////////// µS related Getters //////////////////

int mitk::AbstractFileReader::GetPriority() const
{
  return m_Priority;
}

std::string mitk::AbstractFileReader::GetExtension() const
{
  return m_Extension;
}

std::string mitk::AbstractFileReader::GetDescription() const
{
  return m_Description;
}
