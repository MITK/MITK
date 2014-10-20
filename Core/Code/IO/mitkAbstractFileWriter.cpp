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
#include <mitkExceptionMacro.h>

#include <Internal/mitkFileReaderWriterBase.h>

#include <usGetModuleContext.h>
#include <usModuleContext.h>
#include <usPrototypeServiceFactory.h>

#include <itksys/SystemTools.hxx>

#include <fstream>

namespace mitk {

struct AbstractFileWriter::LocalFile::Impl
{
  Impl(const std::string& location, std::ostream* os)
    : m_Location(location)
    , m_Stream(os)
  {}

  std::string m_Location;
  std::string m_TmpFileName;
  std::ostream* m_Stream;
};

AbstractFileWriter::LocalFile::LocalFile(IFileWriter *writer)
  : d(new Impl(writer->GetOutputLocation(), writer->GetOutputStream()))
{
}

AbstractFileWriter::LocalFile::~LocalFile()
{
  if (d->m_Stream && !d->m_TmpFileName.empty())
  {
    std::ifstream ifs(d->m_TmpFileName.c_str(), std::ios_base::binary);
    *d->m_Stream << ifs.rdbuf();
    d->m_Stream->flush();
    ifs.close();
    std::remove(d->m_TmpFileName.c_str());
  }
}

std::string AbstractFileWriter::LocalFile::GetFileName()
{
  if (d->m_Stream == NULL)
  {
    return d->m_Location;
  }
  else if (d->m_TmpFileName.empty())
  {
    std::string ext = itksys::SystemTools::GetFilenameExtension(d->m_Location);
    d->m_TmpFileName = IOUtil::CreateTemporaryFile("XXXXXX" + ext);
  }
  return d->m_TmpFileName;
}

AbstractFileWriter::OutputStream::OutputStream(IFileWriter* writer, std::ios_base::openmode mode)
  : std::ostream(NULL)
  , m_Stream(NULL)
{
  std::ostream* stream = writer->GetOutputStream();
  if (stream)
  {
    this->init(stream->rdbuf());
  }
  else
  {
    m_Stream = new std::ofstream(writer->GetOutputLocation().c_str(), mode);
    this->init(m_Stream->rdbuf());
  }
}

AbstractFileWriter::OutputStream::~OutputStream()
{
  delete m_Stream;
}


class AbstractFileWriter::Impl : public FileReaderWriterBase
{
public:

  Impl()
    : FileReaderWriterBase()
    , m_BaseData(NULL)
    , m_Stream(NULL)
    , m_PrototypeFactory(NULL)
  {}

  Impl(const Impl& other)
    : FileReaderWriterBase(other)
    , m_BaseDataType(other.m_BaseDataType)
    , m_BaseData(NULL)
    , m_Stream(NULL)
    , m_PrototypeFactory(NULL)
  {}

  std::string m_BaseDataType;
  const BaseData* m_BaseData;
  std::string m_Location;
  std::ostream* m_Stream;

  us::PrototypeServiceFactory* m_PrototypeFactory;
  us::ServiceRegistration<IFileWriter> m_Reg;

};

void AbstractFileWriter::SetInput(const BaseData* data)
{
  d->m_BaseData = data;
}

const BaseData* AbstractFileWriter::GetInput() const
{
  return d->m_BaseData;
}

void AbstractFileWriter::SetOutputLocation(const std::string& location)
{
  d->m_Location = location;
  d->m_Stream = NULL;
}

std::string AbstractFileWriter::GetOutputLocation() const
{
  return d->m_Location;
}

void AbstractFileWriter::SetOutputStream(const std::string& location, std::ostream* os)
{
  d->m_Location = location;
  d->m_Stream = os;
}

std::ostream* AbstractFileWriter::GetOutputStream() const
{
  return d->m_Stream;
}

AbstractFileWriter::~AbstractFileWriter()
{
  UnregisterService();

  delete d->m_PrototypeFactory;
}

AbstractFileWriter::AbstractFileWriter(const AbstractFileWriter& other)
  : IFileWriter(),
    d(new Impl(*other.d.get()))
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

////////////////////// Writing /////////////////////////

IFileWriter::ConfidenceLevel AbstractFileWriter::GetConfidenceLevel() const
{
  if (d->m_BaseData == NULL) return Unsupported;

  std::vector<std::string> classHierarchy = d->m_BaseData->GetClassHierarchy();
  if (std::find(classHierarchy.begin(), classHierarchy.end(), d->m_BaseDataType) ==
      classHierarchy.end())
  {
    return Unsupported;
  }
  return Supported;
}

MimeType AbstractFileWriter::GetRegisteredMimeType() const
{
  return d->GetRegisteredMimeType();
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

  if (this->GetMimeType()->GetName().empty())
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
  result[IFileWriter::PROP_MIMETYPE()] = this->GetMimeType()->GetName();
  result[IFileWriter::PROP_BASEDATA_TYPE()] = d->m_BaseDataType;
  result[us::ServiceConstants::SERVICE_RANKING()] = this->GetRanking();

//  for (IFileWriter::OptionList::const_iterator it = d->m_Options.begin(); it != d->m_Options.end(); ++it)
//  {
//    result[it->first] = std::string("true");
//  }
  return result;
}

const CustomMimeType* AbstractFileWriter::GetMimeType() const
{
  return d->GetMimeType();
}

void AbstractFileWriter::SetMimeTypePrefix(const std::string& prefix)
{
  d->SetMimeTypePrefix(prefix);
}

std::string AbstractFileWriter::GetMimeTypePrefix() const
{
  return d->GetMimeTypePrefix();
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

void AbstractFileWriter::ValidateOutputLocation() const
{
  if (this->GetOutputStream() == NULL)
  {
    // check if a file name is set and if we can write to it
    const std::string fileName = this->GetOutputLocation();
    if (fileName.empty())
    {
      mitkThrow() << "No output location or stream specified";
    }
  }
}

void AbstractFileWriter::SetDescription(const std::string& description)
{
  d->SetDescription(description);
}

}
