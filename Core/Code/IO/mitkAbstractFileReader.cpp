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

#include <Internal/mitkFileReaderWriterBase.h>

#include <usGetModuleContext.h>
#include <usModuleContext.h>
#include <usPrototypeServiceFactory.h>

#include <itksys/SystemTools.hxx>

#include <fstream>

namespace mitk {

class AbstractFileReader::Impl : public FileReaderWriterBase
{
public:

  Impl()
    : FileReaderWriterBase()
    , m_PrototypeFactory(NULL)
  {}

  Impl(const Impl& other)
    : FileReaderWriterBase(other)
    , m_PrototypeFactory(NULL)
  {}

  us::PrototypeServiceFactory* m_PrototypeFactory;

};


AbstractFileReader::AbstractFileReader()
  : d(new Impl)
{
}

AbstractFileReader::~AbstractFileReader()
{
  delete d->m_PrototypeFactory;

  d->UnregisterMimeType();
}

AbstractFileReader::AbstractFileReader(const AbstractFileReader& other)
  : d(new Impl(*other.d.get()))
{
}

AbstractFileReader::AbstractFileReader(const MimeType& mimeType, const std::string& description)
  : d(new Impl)
{
  d->SetMimeType(mimeType);
  d->SetDescription(description);
}

AbstractFileReader::AbstractFileReader(const std::string& extension, const std::string& description)
  : d(new Impl)
{
  d->SetDescription(description);
  d->AddExtension(extension);
}

////////////////////// Reading /////////////////////////

std::vector<itk::SmartPointer<BaseData> > AbstractFileReader::Read(const std::string& path)
{
  if (!itksys::SystemTools::FileExists(path.c_str()))
  {
    mitkThrow() << "File '" + path + "' not found.";
  }
  std::ifstream stream;
  stream.open(path.c_str(), std::ios_base::in | std::ios_base::binary);
  try
  {
    return this->Read(stream);
  }
  catch (mitk::Exception& e)
  {
    mitkReThrow(e) << "Error reading file '" << path << "'";
  }
  catch (const std::exception& e)
  {
    mitkThrow() << "Error reading file '" << path << "': " << e.what();
  }
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

DataStorage::SetOfObjects::Pointer AbstractFileReader::Read(const std::string& path, DataStorage& ds)
{
  DataStorage::SetOfObjects::Pointer result = DataStorage::SetOfObjects::New();
  std::vector<BaseData::Pointer> data = this->Read(path);
  for (std::vector<BaseData::Pointer>::iterator iter = data.begin();
       iter != data.end(); ++iter)
  {
    mitk::DataNode::Pointer node = mitk::DataNode::New();
    node->SetData(*iter);
    this->SetDefaultDataNodeProperties(node, path);
    ds.Add(node);
    result->InsertElement(result->Size(), node);
  }
  return result;
}

DataStorage::SetOfObjects::Pointer AbstractFileReader::Read(std::istream& stream, DataStorage& ds)
{
  DataStorage::SetOfObjects::Pointer result = DataStorage::SetOfObjects::New();
  std::vector<BaseData::Pointer> data = this->Read(stream);
  for (std::vector<BaseData::Pointer>::iterator iter = data.begin();
       iter != data.end(); ++iter)
  {
    mitk::DataNode::Pointer node = mitk::DataNode::New();
    node->SetData(*iter);
    this->SetDefaultDataNodeProperties(node, std::string());
    ds.Add(node);
    result->InsertElement(result->Size(), node);
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

  d->RegisterMimeType(context);

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
  us::ServiceProperties result;

  result[IFileReader::PROP_DESCRIPTION()] = this->GetDescription();
  result[IFileReader::PROP_MIMETYPE()] = this->GetMimeType();
  result[us::ServiceConstants::SERVICE_RANKING()]  = this->GetRanking();
  return result;
}

us::ServiceRegistration<IMimeType> AbstractFileReader::RegisterMimeType(us::ModuleContext* context)
{
  return d->RegisterMimeType(context);
}

void AbstractFileReader::SetMimeType(const std::string& mimeType)
{
  d->SetMimeType(mimeType);
}

void AbstractFileReader::SetCategory(const std::string& category)
{
  d->SetCategory(category);
}

void AbstractFileReader::AddExtension(const std::string& extension)
{
  d->AddExtension(extension);
}

void AbstractFileReader::SetDescription(const std::string& description)
{
  d->SetDescription(description);
}

void AbstractFileReader::SetRanking(int ranking)
{
  d->SetRanking(ranking);
}

int AbstractFileReader::GetRanking() const
{
  return d->GetRanking();
}

//////////////////////// Options ///////////////////////

void AbstractFileReader::SetDefaultOptions(const IFileReader::Options& defaultOptions)
{
  d->SetDefaultOptions(defaultOptions);
}

IFileReader::Options AbstractFileReader::GetDefaultOptions() const
{
  return d->GetDefaultOptions();
}

IFileReader::Options AbstractFileReader::GetOptions() const
{
  return d->GetOptions();
}

us::Any AbstractFileReader::GetOption(const std::string& name) const
{
  return d->GetOption(name);
}

void AbstractFileReader::SetOptions(const Options& options)
{
  d->SetOptions(options);
}

void AbstractFileReader::SetOption(const std::string& name, const us::Any& value)
{
  d->SetOption(name, value);
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
  if (!d->HasExtension(extension))
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
  d->AddProgressCallback(callback);
}

void AbstractFileReader::RemoveProgressCallback(const ProgressCallback& callback)
{
  d->RemoveProgressCallback(callback);
}

////////////////// µS related Getters //////////////////

std::string AbstractFileReader::GetCategory() const
{
  return d->GetCategory();
}

std::string AbstractFileReader::GetMimeType() const
{
  return d->GetMimeType();
}

std::vector<std::string> AbstractFileReader::GetExtensions() const
{
  return d->GetExtensions();
}

std::string AbstractFileReader::GetDescription() const
{
  return d->GetDescription();
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

void AbstractFileReader::SetDefaultDataNodeProperties(DataNode* node, const std::string& filePath)
{
  // path
  if (!filePath.empty())
  {
    mitk::StringProperty::Pointer pathProp = mitk::StringProperty::New( itksys::SystemTools::GetFilenamePath(filePath) );
    node->SetProperty(StringProperty::PATH, pathProp);
  }

  // name already defined?
  mitk::StringProperty::Pointer nameProp = dynamic_cast<mitk::StringProperty*>(node->GetProperty("name"));
  if(nameProp.IsNull() || (strcmp(nameProp->GetValue(),"No Name!")==0))
  {
    // name already defined in BaseData
    mitk::StringProperty::Pointer baseDataNameProp = dynamic_cast<mitk::StringProperty*>(node->GetData()->GetProperty("name").GetPointer() );
    if(baseDataNameProp.IsNull() || (strcmp(baseDataNameProp->GetValue(),"No Name!")==0))
    {
      // name neither defined in node, nor in BaseData -> name = filebasename;
      nameProp = mitk::StringProperty::New(itksys::SystemTools::GetFilenameWithoutExtension(itksys::SystemTools::GetFilenameName(filePath)));
      node->SetProperty("name", nameProp);
    }
    else
    {
      // name defined in BaseData!
      nameProp = mitk::StringProperty::New(baseDataNameProp->GetValue());
      node->SetProperty("name", nameProp);
    }
  }

  // visibility
  if(!node->GetProperty("visible"))
  {
    node->SetVisibility(true);
  }
}

}
