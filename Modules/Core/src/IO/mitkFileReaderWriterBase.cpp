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

#include "mitkFileReaderWriterBase.h"

#include "mitkLogMacros.h"
#include "mitkCoreServices.h"
#include "mitkIMimeTypeProvider.h"
#include "mitkIOMimeTypes.h"

#include <usGetModuleContext.h>
#include <usLDAPProp.h>

namespace mitk {

FileReaderWriterBase::FileReaderWriterBase()
 : m_Ranking(0)
 , m_MimeTypePrefix(IOMimeTypes::DEFAULT_BASE_NAME() + ".")
{
}

FileReaderWriterBase::~FileReaderWriterBase()
{
  this->UnregisterMimeType();
}

FileReaderWriterBase::FileReaderWriterBase(const FileReaderWriterBase& other)
  : m_Description(other.m_Description)
  , m_Ranking(other.m_Ranking)
  , m_MimeTypePrefix(other.m_MimeTypePrefix)
  , m_Options(other.m_Options)
  , m_DefaultOptions(other.m_DefaultOptions)
  , m_CustomMimeType(other.m_CustomMimeType->Clone())
{

}


FileReaderWriterBase::Options FileReaderWriterBase::GetOptions() const
{
  Options options = m_Options;
  options.insert(m_DefaultOptions.begin(), m_DefaultOptions.end());
  return options;
}

us::Any FileReaderWriterBase::GetOption(const std::string& name) const
{
  Options::const_iterator iter = m_Options.find(name);
  if (iter != m_Options.end())
  {
    return iter->second;
  }
  iter = m_DefaultOptions.find(name);
  if (iter != m_DefaultOptions.end())
  {
    return iter->second;
  }
  return us::Any();
}

void FileReaderWriterBase::SetOptions(const FileReaderWriterBase::Options& options)
{
  for(Options::const_iterator iter = options.begin(), iterEnd = options.end();
      iter != iterEnd; ++iter)
  {
    this->SetOption(iter->first, iter->second);
  }
}

void FileReaderWriterBase::SetOption(const std::string& name, const us::Any& value)
{
  if (m_DefaultOptions.find(name) == m_DefaultOptions.end())
  {
    MITK_WARN << "Ignoring unknown IFileReader option '" << name << "'";
  }
  else
  {
    if (value.Empty())
    {
      // an empty Any signals 'reset to default value'
      m_Options.erase(name);
    }
    else
    {
      m_Options[name] = value;
    }
  }
}

void FileReaderWriterBase::SetDefaultOptions(const FileReaderWriterBase::Options& defaultOptions)
{
  m_DefaultOptions = defaultOptions;
}

FileReaderWriterBase::Options FileReaderWriterBase::GetDefaultOptions() const
{
  return m_DefaultOptions;
}

void FileReaderWriterBase::SetRanking(int ranking)
{
  m_Ranking = ranking;
}

int FileReaderWriterBase::GetRanking() const
{
  return m_Ranking;
}

void FileReaderWriterBase::SetMimeType(const CustomMimeType& mimeType)
{
  m_CustomMimeType.reset(mimeType.Clone());
}

const CustomMimeType* FileReaderWriterBase::GetMimeType() const
{
  return m_CustomMimeType.get();
}

CustomMimeType* FileReaderWriterBase::GetMimeType()
{
  return m_CustomMimeType.get();
}

MimeType FileReaderWriterBase::GetRegisteredMimeType() const
{
  MimeType result;
  if (!m_MimeTypeReg)
  {
    if (!m_CustomMimeType->GetName().empty())
    {
      CoreServicePointer<IMimeTypeProvider> mimeTypeProvider(
            CoreServices::GetMimeTypeProvider(us::GetModuleContext()));
      return mimeTypeProvider->GetMimeTypeForName(m_CustomMimeType->GetName());
    }
    return result;
  }

  us::ServiceReferenceU reference = m_MimeTypeReg.GetReference();
  try
  {
    int rank = 0;
    us::Any rankProp = reference.GetProperty(us::ServiceConstants::SERVICE_RANKING());
    if (!rankProp.Empty())
    {
      rank = us::any_cast<int>(rankProp);
    }
    long id = us::any_cast<long>(reference.GetProperty(us::ServiceConstants::SERVICE_ID()));
    result = MimeType(*m_CustomMimeType, rank, id);
  }
  catch (const us::BadAnyCastException& e)
  {
    MITK_WARN << "Unexpected exception: " << e.what();
  }
  return result;
}

void FileReaderWriterBase::SetMimeTypePrefix(const std::string& prefix)
{
  m_MimeTypePrefix = prefix;
}

std::string FileReaderWriterBase::GetMimeTypePrefix() const
{
  return m_MimeTypePrefix;
}

void FileReaderWriterBase::SetDescription(const std::string& description)
{
  m_Description = description;
}

std::string FileReaderWriterBase::GetDescription() const
{
  return m_Description;
}

void FileReaderWriterBase::AddProgressCallback(const FileReaderWriterBase::ProgressCallback& callback)
{
  m_ProgressMessage += callback;
}

void FileReaderWriterBase::RemoveProgressCallback(const FileReaderWriterBase::ProgressCallback& callback)
{
  m_ProgressMessage -= callback;
}

us::ServiceRegistration<CustomMimeType> FileReaderWriterBase::RegisterMimeType(us::ModuleContext* context)
{
  if (context == NULL) throw std::invalid_argument("The context argument must not be NULL.");

  CoreServicePointer<IMimeTypeProvider> mimeTypeProvider(CoreServices::GetMimeTypeProvider(context));

  const std::vector<std::string> extensions = m_CustomMimeType->GetExtensions();

  // If the mime type name is set and the list of extensions is empty,
  // look up the mime type in the registry and print a warning if
  // there is none
  if (!m_CustomMimeType->GetName().empty() && extensions.empty())
  {
    if(!mimeTypeProvider->GetMimeTypeForName(m_CustomMimeType->GetName()).IsValid())
    {
      MITK_WARN << "Registering a MITK reader or writer with an unknown MIME type " << m_CustomMimeType->GetName();
    }
    return m_MimeTypeReg;
  }

  // If the mime type name and extensions list is empty, print a warning
  if(m_CustomMimeType->GetName().empty() && extensions.empty())
  {
    MITK_WARN << "Trying to register a MITK reader or writer with an empty mime type name and empty extension list.";
    return m_MimeTypeReg;
  }

  // extensions is not empty

  if(m_CustomMimeType->GetName().empty())
  {
    // Create a synthetic mime type name from the
    // first extension in the list
    m_CustomMimeType->SetName(m_MimeTypePrefix + extensions.front());
  }

  // Register a new mime type
  //us::ServiceProperties props;
  //props["name"] = m_CustomMimeType.GetName();
  //props["extensions"] = m_CustomMimeType.GetExtensions();
  m_MimeTypeReg = context->RegisterService<CustomMimeType>(m_CustomMimeType.get());

  return m_MimeTypeReg;
}

void FileReaderWriterBase::UnregisterMimeType()
{
  if (m_MimeTypeReg)
  {
    try
    {
      m_MimeTypeReg.Unregister();
    }
    catch (const std::logic_error&)
    {
      // service already unregistered
    }
  }
}

}
