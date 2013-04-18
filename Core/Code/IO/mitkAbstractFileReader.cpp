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


#include "mitkAbstractFileReader.h"
#include "usGetModuleContext.h"


mitk::AbstractFileReader::AbstractFileReader() :
m_Priority (0),
m_Extension ("")
{
}

mitk::AbstractFileReader::~AbstractFileReader()
{
  UnregisterMicroservice(mitk::GetModuleContext());
}

////////////////// Filenames etc. //////////////////

std::string mitk::AbstractFileReader::GetFileName() const
{
  return m_FileName;
}

void mitk::AbstractFileReader::SetFileName(const std::string aFileName)
{
 m_FileName = aFileName;
}

std::string mitk::AbstractFileReader::GetFilePrefix() const
{
  return m_FilePrefix;
}

void mitk::AbstractFileReader::SetFilePrefix(const std::string& aFilePrefix)
{
  m_FilePrefix = aFilePrefix;
}

std::string mitk::AbstractFileReader::GetFilePattern() const
{
  return m_FilePattern;
}

void mitk::AbstractFileReader::SetFilePattern(const std::string& aFilePattern)
{
  m_FilePattern = aFilePattern;
}


//////////// µS Registration & Properties //////////////

void mitk::AbstractFileReader::RegisterMicroservice(mitk::ModuleContext* context)
{
  ServiceProperties props = this->ConstructServiceProperties();
  itk::LightObject* lightObject = dynamic_cast<itk::LightObject*> (this);
  if (lightObject == 0)
     mitkThrow() << "Tried to register reader that is not a lightObject. All readers must inherit from itk::LightObject when used as a Microservice.";
  m_Registration = context->RegisterService<mitk::IFileReader>(lightObject, props);
}

void mitk::AbstractFileReader::UnregisterMicroservice(mitk::ModuleContext* context)
{
  if (! m_Registration )
  {
    MITK_WARN << "Someone tried to unregister a FileReader, but it was either not registered or the registration has expired.";
    return;
  }

  itk::LightObject* lightObject = dynamic_cast<itk::LightObject*> (this);
  if (lightObject == 0)
     mitkThrow() << "Tried to unregister reader that is not a lightObject. All readers must inherit from itk::LightObject when used as a Microservice. However, you somehow registered the object. If you read this error message, something is very wrong.";
  m_Registration.Unregister();
}

mitk::ServiceProperties mitk::AbstractFileReader::ConstructServiceProperties()
{
  if ( m_Extension == "" )
    MITK_WARN << "Registered a Reader with no extension defined (m_Extension is empty). Reader will not be found by calls from ReaderManager.)";
  mitk::ServiceProperties result;
  result[mitk::IFileReader::PROP_EXTENSION]    = m_Extension;
  result[mitk::ServiceConstants::SERVICE_RANKING()]  = m_Priority;

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
