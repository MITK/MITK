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

#include <usGetModuleContext.h>
#include <usLDAPProp.h>

namespace mitk {

FileReaderWriterBase::FileReaderWriterBase()
 : m_Ranking(0)
{
}

FileReaderWriterBase::FileReaderWriterBase(const FileReaderWriterBase& other)
  : m_MimeType(other.m_MimeType)
  , m_Category(other.m_Category)
  , m_Extensions(other.m_Extensions)
  , m_Description(other.m_Description)
  , m_Ranking(other.m_Ranking)
  , m_Options(other.m_Options)
  , m_DefaultOptions(other.m_DefaultOptions)
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

void FileReaderWriterBase::SetMimeType(const std::string& mimeType)
{
  m_MimeType = mimeType;
}

std::string FileReaderWriterBase::GetMimeType() const
{
  return m_MimeType;
}

void FileReaderWriterBase::AddExtension(const std::string& extension)
{
  m_Extensions.push_back(extension);
}

std::vector<std::string> FileReaderWriterBase::GetExtensions() const
{
  return m_Extensions;
}

bool FileReaderWriterBase::HasExtension(const std::string& extension)
{
  return std::find(m_Extensions.begin(), m_Extensions.end(), extension) != m_Extensions.end();
}

void FileReaderWriterBase::SetDescription(const std::string& description)
{
  m_Description = description;
}

std::string FileReaderWriterBase::GetDescription() const
{
  return m_Description;
}

void FileReaderWriterBase::SetCategory(const std::string& category)
{
  m_Category = category;
}

std::string FileReaderWriterBase::GetCategory() const
{
  return m_Category;
}

void FileReaderWriterBase::AddProgressCallback(const FileReaderWriterBase::ProgressCallback& callback)
{
  m_ProgressMessage += callback;
}

void FileReaderWriterBase::RemoveProgressCallback(const FileReaderWriterBase::ProgressCallback& callback)
{
  m_ProgressMessage -= callback;
}

us::ServiceRegistration<IMimeType> FileReaderWriterBase::RegisterMimeType(us::ModuleContext* context)
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
    return context->RegisterService<IMimeType>(&m_SimpleMimeType, props);
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
      m_MimeType = mimeTypes.front();
    }
  }

  if (m_MimeType.empty())
  {
    // There is no registered mime type for the extension or the extensions
    // list contains more than one entry.
    // Register a new mime type by creating a synthetic mime type id from the
    // first extension in the list
    m_MimeType = "application/vnd.mitk." + primaryExtension;
    props[IMimeType::PROP_ID()] = m_MimeType;
    return context->RegisterService<IMimeType>(&m_SimpleMimeType, props);
  }
  else
  {
    // A mime type for one of the listed extensions was found, do nothing.
    return us::ServiceRegistration<IMimeType>();
  }
}

void FileReaderWriterBase::UnregisterMimeType()
{
  if (m_MimeTypeReg)
  {
    m_MimeTypeReg.Unregister();
  }
}

}
