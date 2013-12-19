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

#include <usGetModuleContext.h>
#include <usModuleContext.h>
#include <usPrototypeServiceFactory.h>

#include <itksys/SystemTools.hxx>

#include <fstream>

namespace mitk {

class AbstractFileReader::Impl
{
public:

  Impl()
    : m_Priority(0)
    , m_PrototypeFactory(NULL)
  {}

  Impl(const Impl& other)
    : m_MimeType(other.m_MimeType)
    , m_Category(other.m_Category)
    , m_Extensions(other.m_Extensions)
    , m_Description(other.m_Description)
    , m_Priority(other.m_Priority)
    , m_Options(other.m_Options)
    , m_PrototypeFactory(NULL)
  {}

  // Minimal Service Properties: ALWAYS SET THESE IN CONSTRUCTOR OF DERIVED CLASSES!
  std::string m_MimeType;
  std::string m_Category;
  std::vector<std::string> m_Extensions;
  std::string m_Description;
  int m_Priority;

  /**
   * \brief Options supported by this reader. Set sensible default values!
   *
   * Can be left emtpy if no special options are required.
   */
  IFileReader::OptionList m_Options;

  us::PrototypeServiceFactory* m_PrototypeFactory;

  mitk::Message1<float> m_ProgressMessage;

  mitk::SimpleMimeType m_SimpleMimeType;
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

AbstractFileReader::AbstractFileReader(const std::string& mimeType,const std::string& extension,
                                             const std::string& description)
  : d(new Impl)
{
  d->m_MimeType = mimeType;
  d->m_Extensions.push_back(extension);
  d->m_Description = description;
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

  d->m_MimeTypeReg = this->RegisterMimeType(context);

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
  if ( d->m_MimeType.empty() )
    MITK_WARN << "Registered a Reader with no mime type defined (m_MimeType is empty). Reader will not be found by calls from ReaderManager.)";
  if ( d->m_Extensions.empty() )
    MITK_WARN << "Registered a Reader with no extension defined (m_Extension is empty). Reader will not be found by calls from ReaderManager.)";
  if ( d->m_Description.empty() )
    MITK_WARN << "Registered a Reader with no description defined (m_Description is empty). Reader will have no human readable extension information in FileDialogs.)";

  us::ServiceProperties result;

  result[IFileReader::PROP_DESCRIPTION()]    = d->m_Description;
  result[IFileReader::PROP_MIMETYPE()]       = d->m_MimeType;

  result[us::ServiceConstants::SERVICE_RANKING()]  = d->m_Priority;

  for (IFileReader::OptionList::const_iterator it = d->m_Options.begin(); it != d->m_Options.end(); ++it) {
    if (it->second)
      result[it->first] = std::string("true");
    else result[it->first] = std::string("false");
  }
  return result;
}

us::ServiceProperties AbstractFileReader::GetMimeTypeServiceProperties() const
{
  us::ServiceProperties result;

  result[IMimeType::PROP_ID()] = d->m_MimeType;
  result[IMimeType::PROP_CATEGORY()] = d->m_Category;
  result[IMimeType::PROP_EXTENSIONS()] = d->m_Extensions;
  result[IMimeType::PROP_DESCRIPTION()] = d->m_Description;

  result[us::ServiceConstants::SERVICE_RANKING()]  = d->m_Priority;

  return result;
}

us::ServiceRegistration<IMimeType> AbstractFileReader::RegisterMimeType(us::ModuleContext* context)
{
  us::ServiceProperties mimeTypeProps = this->GetMimeTypeServiceProperties();
  return context->RegisterService<IMimeType>(&d->m_SimpleMimeType, mimeTypeProps);
}

void AbstractFileReader::SetMimeType(const std::string& mimeType)
{
  d->m_MimeType = mimeType;
}

void AbstractFileReader::SetCategory(const std::string& category)
{
  d->m_Category = category;
}

void AbstractFileReader::SetExtensions(const std::vector<std::string>& extensions)
{
  d->m_Extensions = extensions;
}

void AbstractFileReader::SetDescription(const std::string& description)
{
  d->m_Description = description;
}

void AbstractFileReader::SetPriority(int priority)
{
  d->m_Priority = priority;
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

int AbstractFileReader::GetPriority() const
{
  return d->m_Priority;
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

}
