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

#include <mitkIOUtil.h>
#include <mitkCoreServices.h>
#include <mitkIMimeTypeProvider.h>

#include <usGetModuleContext.h>
#include <usModuleContext.h>
#include <usPrototypeServiceFactory.h>

#include <itksys/SystemTools.hxx>

#include <usLDAPProp.h>

#include <fstream>

namespace mitk {

class AbstractFileReader::Impl
{
public:

  Impl()
    : m_Ranking(0)
    , m_PrototypeFactory(NULL)
  {}

  Impl(const Impl& other)
    : m_MimeType(other.m_MimeType)
    , m_Category(other.m_Category)
    , m_Extensions(other.m_Extensions)
    , m_Description(other.m_Description)
    , m_Ranking(other.m_Ranking)
    , m_Options(other.m_Options)
    , m_PrototypeFactory(NULL)
  {}

  std::string m_MimeType;
  std::string m_Category;
  std::vector<std::string> m_Extensions;
  std::string m_Description;
  int m_Ranking;

  /**
   * \brief Options supported by this reader. Set sensible default values!
   *
   * Can be left emtpy if no special options are required.
   */
  IFileReader::OptionList m_Options;

  us::PrototypeServiceFactory* m_PrototypeFactory;

  Message1<float> m_ProgressMessage;

  SimpleMimeType m_SimpleMimeType;
  us::ServiceRegistration<IMimeType> m_MimeTypeReg;
};


AbstractFileReader::AbstractFileReader()
  : d(new Impl)
{
}

AbstractFileReader::~AbstractFileReader()
{
  delete d->m_PrototypeFactory;

  if (d->m_MimeTypeReg)
  {
    d->m_MimeTypeReg.Unregister();
  }
}

AbstractFileReader::AbstractFileReader(const AbstractFileReader& other)
  : d(new Impl(*other.d.get()))
{
}

AbstractFileReader::AbstractFileReader(const MimeType& mimeType, const std::string& description)
  : d(new Impl)
{
  d->m_MimeType = mimeType;
  d->m_Description = description;
}

AbstractFileReader::AbstractFileReader(const std::string& extension, const std::string& description)
  : d(new Impl)
{
  d->m_Description = description;

  d->m_Extensions.push_back(extension);
}

////////////////////// Reading /////////////////////////

std::vector<itk::SmartPointer<BaseData> > AbstractFileReader::Read(const std::string& path)
{
  if (!itksys::SystemTools::FileExists(path.c_str()))
    mitkThrow() << "File '" + path + "' not found.";
  std::ifstream stream;
  stream.open(path.c_str());
  return this->Read(stream);
}

std::vector<itk::SmartPointer<BaseData> > AbstractFileReader::Read(std::istream& stream)
{
  // Create a temporary file and copy the data to it
  std::ofstream tmpOutputStream;
  std::string tmpFilePath = IOUtil::CreateTemporaryFile(tmpOutputStream);
  tmpOutputStream << stream.rdbuf();
  tmpOutputStream.close();

  // Now read from the temporary file
  std::vector<itk::SmartPointer<BaseData> > result = this->Read(tmpFilePath);
  std::remove(tmpFilePath.c_str());
  return result;
}

std::vector<std::pair<itk::SmartPointer<BaseData>,bool> > AbstractFileReader::Read(const std::string& path, DataStorage& /*ds*/)
{
  std::vector<std::pair<BaseData::Pointer,bool> > result;
  std::vector<BaseData::Pointer> data = this->Read(path);
  for (std::vector<BaseData::Pointer>::iterator iter = data.begin();
       iter != data.end(); ++iter)
  {
    result.push_back(std::make_pair(*iter, false));
  }
  return result;
}

std::vector<std::pair<itk::SmartPointer<BaseData>,bool> > AbstractFileReader::Read(std::istream& stream, DataStorage& /*ds*/)
{
  std::vector<std::pair<BaseData::Pointer,bool> > result;
  std::vector<BaseData::Pointer> data = this->Read(stream);
  for (std::vector<BaseData::Pointer>::iterator iter = data.begin();
       iter != data.end(); ++iter)
  {
    result.push_back(std::make_pair(*iter, false));
  }
  return result;
}

//////////// µS Registration & Properties //////////////

us::ServiceRegistration<IFileReader> AbstractFileReader::RegisterService(us::ModuleContext* context)
{
  if (d->m_PrototypeFactory) return us::ServiceRegistration<IFileReader>();

  if(context == NULL)
  {
    context = us::GetModuleContext();
  }

  d->m_MimeTypeReg = this->RegisterMimeType(context);

  if (this->GetMimeType().empty())
  {
    MITK_WARN << "Not registering reader " << typeid(this).name() << " due to empty MIME type.";
    return us::ServiceRegistration<IFileReader>();
  }

  struct PrototypeFactory : public us::PrototypeServiceFactory
  {
    AbstractFileReader* const m_Prototype;

    PrototypeFactory(AbstractFileReader* prototype)
      : m_Prototype(prototype)
    {}

    us::InterfaceMap GetService(us::Module* /*module*/, const us::ServiceRegistrationBase& /*registration*/)
    {
      return us::MakeInterfaceMap<IFileReader>(m_Prototype->Clone());
    }

    void UngetService(us::Module* /*module*/, const us::ServiceRegistrationBase& /*registration*/,
      const us::InterfaceMap& service)
    {
      delete us::ExtractInterface<IFileReader>(service);
    }
  };

  d->m_PrototypeFactory = new PrototypeFactory(this);
  us::ServiceProperties props = this->GetServiceProperties();
  return context->RegisterService<IFileReader>(d->m_PrototypeFactory, props);
}

us::ServiceProperties AbstractFileReader::GetServiceProperties() const
{
  if ( d->m_Description.empty() )
    MITK_WARN << "Registered a Reader with no description defined. Reader will have no human readable extension information.)";

  us::ServiceProperties result;

  result[IFileReader::PROP_DESCRIPTION()] = this->GetDescription();
  result[IFileReader::PROP_MIMETYPE()] = this->GetMimeType();
  result[us::ServiceConstants::SERVICE_RANKING()]  = this->GetRanking();

  for (IFileReader::OptionList::const_iterator it = d->m_Options.begin(); it != d->m_Options.end(); ++it) {
    if (it->second)
      result[it->first] = std::string("true");
    else result[it->first] = std::string("false");
  }
  return result;
}

us::ServiceRegistration<IMimeType> AbstractFileReader::RegisterMimeType(us::ModuleContext* context)
{
  if (context == NULL) throw std::invalid_argument("The context argument must not be NULL.");

  const std::string mimeType = this->GetMimeType();
  std::vector<std::string> extensions = this->GetExtensions();
  const std::string primaryExtension = extensions.empty() ? "" : extensions.front();
  std::sort(extensions.begin(), extensions.end());
  extensions.erase(std::unique(extensions.begin(), extensions.end()), extensions.end());

  us::ServiceProperties props;

  props[IMimeType::PROP_ID()] = mimeType;
  props[IMimeType::PROP_CATEGORY()] = this->GetCategory();
  props[IMimeType::PROP_EXTENSIONS()] = extensions;
  props[IMimeType::PROP_DESCRIPTION()] = std::string("Generated MIME type from mitk::AbstractFileReader");
  props[us::ServiceConstants::SERVICE_RANKING()]  = this->GetRanking();

  // If the mime type is set and the list of extensions is not empty,
  // register a new IMimeType service
  if (!mimeType.empty() && !extensions.empty())
  {
    return context->RegisterService<IMimeType>(&d->m_SimpleMimeType, props);
  }

  // If the mime type is set and the list of extensions is empty,
  // look up the mime type in the registry and print a warning if
  // there is none
  if (!mimeType.empty() && extensions.empty())
  {
    if(us::GetModuleContext()->GetServiceReferences<IMimeType>(us::LDAPProp(IMimeType::PROP_ID()) == mimeType).empty())
    {
      MITK_WARN << "Registering a MITK reader with an unknown MIME type " << mimeType;
    }
    return us::ServiceRegistration<IMimeType>();
  }

  // If the mime type is empty, get a mime type using the extensions list
  assert(mimeType.empty());
  mitk::CoreServicePointer<IMimeTypeProvider> mimeTypeProvider(mitk::CoreServices::GetMimeTypeProvider());

  if(extensions.empty())
  {
    MITK_WARN << "Trying to register a MITK reader with an empty mime type and empty extension list.";
    return us::ServiceRegistration<IMimeType>();
  }
  else if(extensions.size() == 1)
  {
    // If there is only one extension, try to look-up an existing mime tpye
    std::vector<std::string> mimeTypes = mimeTypeProvider->GetMimeTypesForExtension(extensions.front());
    if (!mimeTypes.empty())
    {
      d->m_MimeType = mimeTypes.front();
    }
  }

  if (d->m_MimeType.empty())
  {
    // There is no registered mime type for the extension or the extensions
    // list contains more than one entry.
    // Register a new mime type by creating a synthetic mime type id from the
    // first extension in the list
    d->m_MimeType = "application/vnd.mitk." + primaryExtension;
    props[IMimeType::PROP_ID()] = d->m_MimeType;
    return context->RegisterService<IMimeType>(&d->m_SimpleMimeType, props);
  }
  else
  {
    // A mime type for one of the listed extensions was found, do nothing.
    return us::ServiceRegistration<IMimeType>();
  }
}

void AbstractFileReader::SetMimeType(const std::string& mimeType)
{
  d->m_MimeType = mimeType;
}

void AbstractFileReader::SetCategory(const std::string& category)
{
  d->m_Category = category;
}

void AbstractFileReader::AddExtension(const std::string& extension)
{
  d->m_Extensions.push_back(extension);
}

void AbstractFileReader::SetDescription(const std::string& description)
{
  d->m_Description = description;
}

void AbstractFileReader::SetRanking(int ranking)
{
  d->m_Ranking = ranking;
}

int AbstractFileReader::GetRanking() const
{
  return d->m_Ranking;
}

//////////////////////// Options ///////////////////////

IFileReader::OptionList AbstractFileReader::GetOptions() const
{
  return d->m_Options;
}

void AbstractFileReader::SetOptions(const OptionList& options)
{
  if (options.size() != d->m_Options.size()) MITK_WARN << "Number of set Options differs from Number of available Options, which is a sign of false usage. Please consult documentation";
  d->m_Options = options;
}

////////////////// MISC //////////////////

bool AbstractFileReader::CanRead(const std::string& path) const
{
  if (!itksys::SystemTools::FileExists(path.c_str(), true))
  {
    return false;
  }

  // Default implementation only checks if extension is correct
  std::string extension = itksys::SystemTools::GetFilenameExtension(path);
  extension = extension.substr(1, extension.size()-1);
  if (std::find(d->m_Extensions.begin(), d->m_Extensions.end(), extension) == d->m_Extensions.end())
  {
    return false;
  }

  std::ifstream stream(path.c_str());
  return this->CanRead(stream);
}

bool AbstractFileReader::CanRead(std::istream& stream) const
{
  return stream.good();
}

void AbstractFileReader::AddProgressCallback(const ProgressCallback& callback)
{
  d->m_ProgressMessage += callback;
}

void AbstractFileReader::RemoveProgressCallback(const ProgressCallback& callback)
{
  d->m_ProgressMessage -= callback;
}

////////////////// µS related Getters //////////////////

std::string AbstractFileReader::GetCategory() const
{
  return d->m_Category;
}

std::string AbstractFileReader::GetMimeType() const
{
  return d->m_MimeType;
}

std::vector<std::string> AbstractFileReader::GetExtensions() const
{
  return d->m_Extensions;
}

std::string AbstractFileReader::GetDescription() const
{
  return d->m_Description;
}

AbstractFileReader::MimeType::MimeType(const std::string& mimeType)
  : std::string(mimeType)
{
  if (this->empty())
  {
    throw std::invalid_argument("MIME type must not be empty.");
  }
}

AbstractFileReader::MimeType::MimeType()
{
}

}
