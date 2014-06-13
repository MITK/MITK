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
#include <mitkSimpleMimeType.h>

#include <usGetModuleContext.h>
#include <usModuleContext.h>
#include <usPrototypeServiceFactory.h>

#include <itksys/SystemTools.hxx>

#include <fstream>

namespace mitk {

class AbstractFileWriter::Impl
{
public:

  Impl()
    : m_Ranking(0)
    , m_PrototypeFactory(NULL)
  {}

  Impl(const Impl& other)
    : m_MimeType(other.m_MimeType)
    , m_Category(other.m_Category)
    , m_BaseDataType(other.m_BaseDataType)
    , m_Extensions(other.m_Extensions)
    , m_Description(other.m_Description)
    , m_Ranking(other.m_Ranking)
    , m_Options(other.m_Options)
    , m_PrototypeFactory(NULL)
  {}

  std::string m_MimeType;
  std::string m_Category;
  std::string m_BaseDataType;
  std::vector<std::string> m_Extensions;
  std::string m_Description;
  int m_Ranking;

  /**
   * \brief Options supported by this writer. Set sensible default values!
   *
   * Can be left emtpy if no special options are required.
   */
  IFileWriter::OptionList m_Options;


  us::PrototypeServiceFactory* m_PrototypeFactory;

  Message1<float> m_ProgressMessage;

  SimpleMimeType m_SimpleMimeType;
  us::ServiceRegistration<IMimeType> m_MimeTypeReg;
};


AbstractFileWriter::AbstractFileWriter()
  : d(new Impl)
{
}

AbstractFileWriter::~AbstractFileWriter()
{
  delete d->m_PrototypeFactory;

  if (d->m_MimeTypeReg)
  {
    d->m_MimeTypeReg.Unregister();
  }
}

AbstractFileWriter::AbstractFileWriter(const AbstractFileWriter& other)
  : d(new Impl(*other.d.get()))
{
}

AbstractFileWriter::AbstractFileWriter(const std::string& baseDataType, const std::string& extension,
                                       const std::string& description)
  : d(new Impl)
{
  d->m_BaseDataType = baseDataType;
  d->m_Description = description;
  d->m_Extensions.push_back(extension);
}

////////////////////// Writing /////////////////////////

void AbstractFileWriter::Write(const BaseData* data, const std::string& path)
{
  std::ofstream stream;
  stream.open(path.c_str());
  this->Write(data, stream);
}

void AbstractFileWriter::Write(const BaseData* data, std::ostream& stream)
{
  // Create a temporary file and write the data to it
  std::ofstream tmpOutputStream;
  std::string tmpFilePath = IOUtil::CreateTemporaryFile(tmpOutputStream);
  this->Write(data, tmpFilePath);
  tmpOutputStream.close();

  // Now copy the contents
  std::ifstream tmpInputStream(tmpFilePath.c_str(), std::ios_base::binary);
  stream << tmpInputStream.rdbuf();
  tmpInputStream.close();
  std::remove(tmpFilePath.c_str());
}

//////////// µS Registration & Properties //////////////

us::ServiceRegistration<IFileWriter> AbstractFileWriter::RegisterService(us::ModuleContext* context)
{
  if (d->m_PrototypeFactory) return us::ServiceRegistration<IFileWriter>();

  struct PrototypeFactory : public us::PrototypeServiceFactory
  {
    AbstractFileWriter* const m_Prototype;

    PrototypeFactory(AbstractFileWriter* prototype)
      : m_Prototype(prototype)
    {}

    us::InterfaceMap GetService(us::Module* /*module*/, const us::ServiceRegistrationBase& /*registration*/)
    {
      return us::MakeInterfaceMap<IFileWriter>(m_Prototype->Clone());
    }

    void UngetService(us::Module* /*module*/, const us::ServiceRegistrationBase& /*registration*/,
      const us::InterfaceMap& service)
    {
      delete us::ExtractInterface<IFileWriter>(service);
    }
  };

  d->m_PrototypeFactory = new PrototypeFactory(this);
  us::ServiceProperties props = this->GetServiceProperties();
  return context->RegisterService<IFileWriter>(d->m_PrototypeFactory, props);
}

us::ServiceProperties AbstractFileWriter::GetServiceProperties() const
{
  if ( d->m_Extensions.empty() )
    MITK_WARN << "Registered a Writer with no extension defined (m_Extension is empty). Writer will not be found by calls from WriterManager.)";
  if ( d->m_BaseDataType.empty() )
    MITK_WARN << "Registered a Writer with no BasedataType defined (m_BasedataType is empty). Writer will not be found by calls from WriterManager.)";
  if ( d->m_Description.empty() )
    MITK_WARN << "Registered a Writer with no description defined (m_Description is empty). Writer will have no human readable extension information in FileDialogs.)";

  us::ServiceProperties result;
  //result[IFileWriter::PROP_EXTENSION()] = d->m_Extensions;
  result[IFileWriter::PROP_DESCRIPTION()] = d->m_Description;
  result[IFileWriter::PROP_BASEDATA_TYPE()] = d->m_BaseDataType;
  result[us::ServiceConstants::SERVICE_RANKING()] = d->m_Ranking;

  for (IFileWriter::OptionList::const_iterator it = d->m_Options.begin(); it != d->m_Options.end(); ++it)
  {
    result[it->first] = std::string("true");
  }
  return result;
}

void AbstractFileWriter::SetRanking(int ranking)
{
  d->m_Ranking = ranking;
}

//////////////////////// Options ///////////////////////

IFileWriter::OptionList AbstractFileWriter::GetOptions() const
{
  return d->m_Options;
}

void AbstractFileWriter::SetOptions(const OptionList& options)
{
  if (options.size() != d->m_Options.size()) MITK_WARN << "Number of set Options differs from Number of available Options, which is a sign of false usage. Please consult documentation";
  d->m_Options = options;
}

////////////////// MISC //////////////////

bool AbstractFileWriter::CanWrite(const BaseData* data) const
{
  // Default implementation only checks if basedatatype is correct
  std::string externalDataType = data->GetNameOfClass();
  return (externalDataType == d->m_BaseDataType);
}

void AbstractFileWriter::AddProgressCallback(const ProgressCallback& callback)
{
  d->m_ProgressMessage += callback;
}

void AbstractFileWriter::RemoveProgressCallback(const ProgressCallback& callback)
{
  d->m_ProgressMessage -= callback;
}

////////////////// µS related Getters //////////////////

int AbstractFileWriter::GetRanking() const
{
  return d->m_Ranking;
}

std::vector<std::string> AbstractFileWriter::GetExtensions() const
{
  return d->m_Extensions;
}

void AbstractFileWriter::AddExtension(const std::string& extension)
{
  d->m_Extensions.push_back(extension);
}

void AbstractFileWriter::SetCategory(const std::string& category)
{
  d->m_Category = category;
}

std::string AbstractFileWriter::GetCategory() const
{
  return d->m_Category;
}

void AbstractFileWriter::SetBaseDataType(const std::string& baseDataType)
{
  d->m_BaseDataType = baseDataType;
}

std::string AbstractFileWriter::GetDescription() const
{
  return d->m_Description;
}

std::string AbstractFileWriter::GetBaseDataType() const
{
  return d->m_BaseDataType;
}

void AbstractFileWriter::SetDescription(const std::string& description)
{
  d->m_Description = description;
}

}
