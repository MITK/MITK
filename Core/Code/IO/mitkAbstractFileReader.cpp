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
#include <usPrototypeServiceFactory.h>

#include <itksys/SystemTools.hxx>

#include <fstream>

mitk::AbstractFileReader::AbstractFileReader()
  : m_Priority (0)
  , m_PrototypeFactory(NULL)
{
}

mitk::AbstractFileReader::~AbstractFileReader()
{
  delete m_PrototypeFactory;
}

mitk::AbstractFileReader::AbstractFileReader(const mitk::AbstractFileReader& other)
  : m_Extension(other.m_Extension)
  , m_Description(other.m_Description)
  , m_Priority(other.m_Priority)
  , m_Options(other.m_Options)
  , m_PrototypeFactory(NULL)
{
}

mitk::AbstractFileReader::AbstractFileReader(const std::string& extension, const std::string& description)
  : m_Extension (extension)
  , m_Description (description)
  , m_Priority (0)
  , m_PrototypeFactory(NULL)
{
}

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

us::ServiceRegistration<mitk::IFileReader> mitk::AbstractFileReader::RegisterService(us::ModuleContext* context)
{
  if (m_PrototypeFactory) return us::ServiceRegistration<mitk::IFileReader>();

  struct PrototypeFactory : public us::PrototypeServiceFactory
  {
    mitk::AbstractFileReader* const m_Prototype;

    PrototypeFactory(mitk::AbstractFileReader* prototype)
      : m_Prototype(prototype)
    {}

    us::InterfaceMap GetService(us::Module* /*module*/, const us::ServiceRegistrationBase& /*registration*/)
    {
      return us::MakeInterfaceMap<mitk::IFileReader>(m_Prototype->Clone());
    }

    void UngetService(us::Module* /*module*/, const us::ServiceRegistrationBase& /*registration*/,
      const us::InterfaceMap& service)
    {
      delete us::ExtractInterface<mitk::IFileReader>(service);
    }
  };

  m_PrototypeFactory = new PrototypeFactory(this);
  us::ServiceProperties props = this->GetServiceProperties();
  return context->RegisterService<mitk::IFileReader>(m_PrototypeFactory, props);
}

us::ServiceProperties mitk::AbstractFileReader::GetServiceProperties()
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

  for (std::list< mitk::IFileReader::FileServiceOption  >::const_iterator it = m_Options.begin(); it != m_Options.end(); ++it) {
    if (it->second)
      result[it->first] = std::string("true");
    else result[it->first] = std::string("false");
  }
  return result;
}

//////////////////////// Options ///////////////////////

std::list< mitk::IFileReader::FileServiceOption > mitk::AbstractFileReader::GetOptions() const
{
  return m_Options;
}

void mitk::AbstractFileReader::SetOptions(std::list< mitk::IFileReader::FileServiceOption > options)
{
  if (options.size() != m_Options.size()) MITK_WARN << "Number of set Options differs from Number of available Options, which is a sign of false usage. Please consult documentation";
  m_Options = options;
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
