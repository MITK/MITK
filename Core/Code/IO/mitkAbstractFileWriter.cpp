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
#include <mitkBaseData.h>
#include <mitkIOUtil.h>

#include <usGetModuleContext.h>
#include <usModuleContext.h>
#include <usPrototypeServiceFactory.h>

#include <itksys/SystemTools.hxx>

#include <fstream>

mitk::AbstractFileWriter::AbstractFileWriter()
  : m_Priority (0)
  , m_PrototypeFactory(NULL)
{
}

mitk::AbstractFileWriter::~AbstractFileWriter()
{
  delete m_PrototypeFactory;
}

mitk::AbstractFileWriter::AbstractFileWriter(const mitk::AbstractFileWriter& other)
  : m_Extension(other.m_Extension)
  , m_BasedataType(other.m_BasedataType)
  , m_Description(other.m_Description)
  , m_Priority(other.m_Priority)
  , m_Options(other.m_Options)
  , m_PrototypeFactory(NULL)
{
}

mitk::AbstractFileWriter::AbstractFileWriter(const std::string& basedataType, const std::string& extension,
                                             const std::string& description)
                                             : m_Extension (extension)
                                             , m_BasedataType(basedataType)
                                             , m_Description (description)
                                             , m_Priority (0)
                                             , m_PrototypeFactory(NULL)
{
}

////////////////////// Writing /////////////////////////

void mitk::AbstractFileWriter::Write(const BaseData* data, const std::string& path)
{
  std::ofstream stream;
  stream.open(path.c_str());
  this->Write(data, stream);
}

void mitk::AbstractFileWriter::Write(const mitk::BaseData* data, std::ostream& stream)
{
  // Create a temporary file and write the data to it
  std::ofstream tmpOutputStream;
  std::string tmpFilePath = mitk::IOUtil::CreateTemporaryFile(tmpOutputStream);
  this->Write(data, tmpFilePath);
  tmpOutputStream.close();

  // Now copy the contents
  std::ifstream tmpInputStream(tmpFilePath.c_str(), std::ios_base::binary);
  stream << tmpInputStream.rdbuf();
  tmpInputStream.close();
  std::remove(tmpFilePath.c_str());
}

//////////// µS Registration & Properties //////////////

us::ServiceRegistration<mitk::IFileWriter> mitk::AbstractFileWriter::RegisterService(us::ModuleContext* context)
{
  if (m_PrototypeFactory) return us::ServiceRegistration<mitk::IFileWriter>();

  struct PrototypeFactory : public us::PrototypeServiceFactory
  {
    mitk::AbstractFileWriter* const m_Prototype;

    PrototypeFactory(mitk::AbstractFileWriter* prototype)
      : m_Prototype(prototype)
    {}

    us::InterfaceMap GetService(us::Module* /*module*/, const us::ServiceRegistrationBase& /*registration*/)
    {
      return us::MakeInterfaceMap<mitk::IFileWriter>(m_Prototype->Clone());
    }

    void UngetService(us::Module* /*module*/, const us::ServiceRegistrationBase& /*registration*/,
      const us::InterfaceMap& service)
    {
      delete us::ExtractInterface<mitk::IFileWriter>(service);
    }
  };

  m_PrototypeFactory = new PrototypeFactory(this);
  us::ServiceProperties props = this->GetServiceProperties();
  return context->RegisterService<mitk::IFileWriter>(m_PrototypeFactory, props);
}

us::ServiceProperties mitk::AbstractFileWriter::GetServiceProperties() const
{
  if ( m_Extension.empty() )
    MITK_WARN << "Registered a Writer with no extension defined (m_Extension is empty). Writer will not be found by calls from WriterManager.)";
  if ( m_BasedataType.empty() )
    MITK_WARN << "Registered a Writer with no BasedataType defined (m_BasedataType is empty). Writer will not be found by calls from WriterManager.)";
  if ( m_Description.empty() )
    MITK_WARN << "Registered a Writer with no description defined (m_Description is empty). Writer will have no human readable extension information in FileDialogs.)";

  us::ServiceProperties result;
  result[mitk::IFileWriter::PROP_EXTENSION()]    = m_Extension;
  result[mitk::IFileWriter::PROP_DESCRIPTION()]    = m_Description;
  result[mitk::IFileWriter::PROP_BASEDATA_TYPE()]    = m_BasedataType;
  result[us::ServiceConstants::SERVICE_RANKING()]  = m_Priority;

  for (mitk::IFileWriter::OptionList::const_iterator it = m_Options.begin(); it != m_Options.end(); ++it)
  {
    result[it->first] = std::string("true");
  }
  return result;
}

//////////////////////// Options ///////////////////////

mitk::IFileWriter::OptionList mitk::AbstractFileWriter::GetOptions() const
{
  return m_Options;
}

void mitk::AbstractFileWriter::SetOptions(const OptionList& options)
{
  if (options.size() != m_Options.size()) MITK_WARN << "Number of set Options differs from Number of available Options, which is a sign of false usage. Please consult documentation";
  m_Options = options;
}

////////////////// MISC //////////////////

bool mitk::AbstractFileWriter::CanWrite(const BaseData* data) const
{
  // Default implementation only checks if basedatatype is correct
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
