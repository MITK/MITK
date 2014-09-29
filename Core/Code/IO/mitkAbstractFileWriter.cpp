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
#include <mitkCustomMimeType.h>

#include <Internal/mitkFileReaderWriterBase.h>

#include <usGetModuleContext.h>
#include <usModuleContext.h>
#include <usPrototypeServiceFactory.h>

#include <itksys/SystemTools.hxx>

#include <fstream>

namespace mitk {

class AbstractFileWriter::Impl : public FileReaderWriterBase
{
public:

  Impl()
    : FileReaderWriterBase()
    , m_PrototypeFactory(NULL)
  {}

  Impl(const Impl& other)
    : FileReaderWriterBase(other)
    , m_BaseDataType(other.m_BaseDataType)
    , m_PrototypeFactory(NULL)
  {}

  std::string m_BaseDataType;

  us::PrototypeServiceFactory* m_PrototypeFactory;
  us::ServiceRegistration<IFileWriter> m_Reg;

};


AbstractFileWriter::~AbstractFileWriter()
{
  UnregisterService();

  delete d->m_PrototypeFactory;
}

AbstractFileWriter::AbstractFileWriter(const AbstractFileWriter& other)
  : d(new Impl(*other.d.get()))
{
}

AbstractFileWriter::AbstractFileWriter(const std::string& baseDataType)
  : d(new Impl)
{
  d->m_BaseDataType = baseDataType;
}

AbstractFileWriter::AbstractFileWriter(const std::string& baseDataType, const CustomMimeType& mimeType,
                                       const std::string& description)
  : d(new Impl)
{
  d->m_BaseDataType = baseDataType;
  d->SetMimeType(mimeType);
  d->SetDescription(description);
}

AbstractFileWriter::AbstractFileWriter(const std::string& baseDataType, const std::string& extension,
                                       const std::string& description)
  : d(new Impl)
{
  d->m_BaseDataType = baseDataType;
  d->SetDescription(description);
  CustomMimeType customMimeType;
  customMimeType.AddExtension(extension);
  d->SetMimeType(customMimeType);
}

////////////////////// Writing /////////////////////////

void AbstractFileWriter::Write(const BaseData* data, const std::string& path)
{
  std::ofstream stream;
  stream.open(path.c_str());
  try
  {
    this->Write(data, stream);
  }
  catch(mitk::Exception& e)
  {
    mitkReThrow(e) << "Error writing file '" << path << "'";
  }
  catch(const std::exception& e)
  {
    mitkThrow() << "Error writing file '" << path << "': " << e.what();
  }
}

IFileWriter::ConfidenceLevel AbstractFileWriter::GetConfidenceLevel(const BaseData* data) const
{
  if (data == NULL) return Unsupported;

  if (data->GetNameOfClass() == d->m_BaseDataType)
  {
    return Supported;
  }
  return Unsupported;
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

  if(context == NULL)
  {
    context = us::GetModuleContext();
  }

  d->RegisterMimeType(context);

  if (this->GetMimeType().GetName().empty())
  {
    MITK_WARN << "Not registering writer due to empty MIME type.";
    return us::ServiceRegistration<IFileWriter>();
  }

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
  d->m_Reg = context->RegisterService<IFileWriter>(d->m_PrototypeFactory, props);
  return d->m_Reg;
}

void AbstractFileWriter::UnregisterService()
{
  try
  {
    d->m_Reg.Unregister();
  }
  catch (const std::exception&)
  {}
}

us::ServiceProperties AbstractFileWriter::GetServiceProperties() const
{
  us::ServiceProperties result;
  result[IFileWriter::PROP_DESCRIPTION()] = this->GetDescription();
  result[IFileWriter::PROP_MIMETYPE()] = this->GetMimeType().GetName();
  result[IFileWriter::PROP_BASEDATA_TYPE()] = d->m_BaseDataType;
  result[us::ServiceConstants::SERVICE_RANKING()] = this->GetRanking();

//  for (IFileWriter::OptionList::const_iterator it = d->m_Options.begin(); it != d->m_Options.end(); ++it)
//  {
//    result[it->first] = std::string("true");
//  }
  return result;
}

CustomMimeType AbstractFileWriter::GetMimeType() const
{
  return d->GetMimeType();
}

us::ServiceRegistration<CustomMimeType> AbstractFileWriter::RegisterMimeType(us::ModuleContext* context)
{
  return d->RegisterMimeType(context);
}

void AbstractFileWriter::SetMimeType(const CustomMimeType& mimeType)
{
  d->SetMimeType(mimeType);
}

void AbstractFileWriter::SetRanking(int ranking)
{
  d->SetRanking(ranking);
}

//////////////////////// Options ///////////////////////

void AbstractFileWriter::SetDefaultOptions(const IFileWriter::Options& defaultOptions)
{
  d->SetDefaultOptions(defaultOptions);
}

IFileWriter::Options AbstractFileWriter::GetDefaultOptions() const
{
  return d->GetDefaultOptions();
}

IFileWriter::Options AbstractFileWriter::GetOptions() const
{
  return d->GetOptions();
}

us::Any AbstractFileWriter::GetOption(const std::string& name) const
{
  return d->GetOption(name);
}

void AbstractFileWriter::SetOption(const std::string& name, const us::Any& value)
{
  d->SetOption(name, value);
}

void AbstractFileWriter::SetOptions(const Options& options)
{
  d->SetOptions(options);
}

////////////////// MISC //////////////////


void AbstractFileWriter::AddProgressCallback(const ProgressCallback& callback)
{
  d->AddProgressCallback(callback);
}

void AbstractFileWriter::RemoveProgressCallback(const ProgressCallback& callback)
{
  d->RemoveProgressCallback(callback);
}

////////////////// µS related Getters //////////////////

int AbstractFileWriter::GetRanking() const
{
  return d->GetRanking();
}

void AbstractFileWriter::SetBaseDataType(const std::string& baseDataType)
{
  d->m_BaseDataType = baseDataType;
}

std::string AbstractFileWriter::GetDescription() const
{
  return d->GetDescription();
}

std::string AbstractFileWriter::GetBaseDataType() const
{
  return d->m_BaseDataType;
}

void AbstractFileWriter::SetDescription(const std::string& description)
{
  d->SetDescription(description);
}

}
