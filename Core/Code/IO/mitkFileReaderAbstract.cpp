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


#include "mitkFileReaderAbstract.h"


mitk::FileReaderAbstract::FileReaderAbstract() :
m_CanReadFromMemory (false),
m_ReadFromMemory (false)
{
}

mitk::FileReaderAbstract::~FileReaderAbstract()
{
}

////////////////// Filenames etc. //////////////////

std::string mitk::FileReaderAbstract::GetFileName() const
{
  return m_FileName;
}

void mitk::FileReaderAbstract::SetFileName(const std::string aFileName)
{
 m_FileName = aFileName;
}

std::string mitk::FileReaderAbstract::GetFilePrefix() const
{
  return m_FilePrefix;
}

void mitk::FileReaderAbstract::SetFilePrefix(const std::string& aFilePrefix)
{
  m_FilePrefix = aFilePrefix;
}

std::string mitk::FileReaderAbstract::GetFilePattern() const
{
  return m_FilePattern;
}

void mitk::FileReaderAbstract::SetFilePattern(const std::string& aFilePattern)
{
  m_FilePattern = aFilePattern;
}

////////////////// Memory Reading //////////////////

bool mitk::FileReaderAbstract::CanReadFromMemory(  )
{
  return m_CanReadFromMemory;
}

void mitk::FileReaderAbstract::SetReadFromMemory( bool read )
{
  m_ReadFromMemory = read;
}
bool mitk::FileReaderAbstract::GetReadFromMemory(  )
{
  return m_ReadFromMemory;
}
void mitk::FileReaderAbstract::SetMemoryBuffer(const std::string dataArray, unsigned int size)
{
  m_MemoryBuffer = dataArray;
  m_MemorySize   = size;
}


//////////// µS Registration & Properties //////////////

void mitk::FileReaderAbstract::RegisterMicroservice(mitk::ModuleContext* context)
{
  ServiceProperties props = this->ConstructServiceProperties();
  itk::LightObject* lightObject = dynamic_cast<itk::LightObject*> (this);
  if (lightObject == 0)
     mitkThrow() << "Tried to register reader that is not a lightObject. All readers must inherit from itk::LightObject when used as a Microservice.";
  m_Registration = context->RegisterService<mitk::FileReaderInterface>(lightObject, props);
}

mitk::ServiceProperties mitk::FileReaderAbstract::ConstructServiceProperties()
{
  if ( m_Extension == "" )
    MITK_WARN << "Registered a Reader with no extension defined (m_Extension is empty). Reader will not be found by calls from ReaderManager.)";
  mitk::ServiceProperties result;
  result[mitk::FileReaderInterface::US_EXTENSION]    = m_Extension;
  result[mitk::ServiceConstants::SERVICE_RANKING()]  = m_Priority;
  result[mitk::FileReaderInterface::US_CAN_READ_FROM_MEMORY]  = m_CanReadFromMemory;

  for (std::list<std::string>::const_iterator it = m_Options.begin(); it != m_Options.end(); ++it) {
    result[*it] = std::string("true");
  }
  return result;
}

//////////////////////// Options ///////////////////////

std::list< std::string > mitk::FileReaderAbstract::GetSupportedOptions() const
{
  return m_Options;
}

////////////////// MISC //////////////////

bool mitk::FileReaderAbstract::CanRead(const std::string& path) const
{
  // Default implementation only checks if extension is correct
  std::string pathEnd = path.substr( path.length() - m_Extension.length(), m_Extension.length() );
  return (m_Extension == pathEnd);
}

float mitk::FileReaderAbstract::GetProgress() const
{
  // Default implementation always returns 1 (finished)
  return 1;
}

////////////////// µS related Getters //////////////////

int mitk::FileReaderAbstract::GetPriority() const
{
  return m_Priority;
}

std::string mitk::FileReaderAbstract::GetExtension() const
{
  return m_Extension;
}
