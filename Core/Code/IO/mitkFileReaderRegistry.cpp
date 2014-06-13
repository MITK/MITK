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

#include "mitkFileReaderRegistry.h"

#include "mitkIMimeTypeProvider.h"
#include "mitkCoreServices.h"

// Microservices
#include <usGetModuleContext.h>
#include <usModuleContext.h>
#include <usServiceProperties.h>
#include <usLDAPProp.h>

#include "itksys/SystemTools.hxx"

mitk::FileReaderRegistry::FileReaderRegistry()
{
}

mitk::FileReaderRegistry::~FileReaderRegistry()
{
  for (std::map<mitk::IFileReader*, us::ServiceObjects<mitk::IFileReader> >::iterator iter = m_ServiceObjects.begin(),
    end = m_ServiceObjects.end(); iter != end; ++iter)
  {
    iter->second.UngetService(iter->first);
  }
}

//////////////////// READING DIRECTLY ////////////////////

std::vector< mitk::BaseData::Pointer > mitk::FileReaderRegistry::Read(const std::string& path, us::ModuleContext* context)
{
  // Find extension
  std::string extension = itksys::SystemTools::GetFilenameExtension(path);
  if (!extension.empty())
  {
    extension = extension.substr(1, extension.size()-1);
  }

  // Get best Reader
  FileReaderRegistry readerRegistry;
  mitk::IFileReader* reader = readerRegistry.GetReader(extension, context);
  // Throw exception if no compatible reader was found
  if (reader == NULL) mitkThrow() << "Tried to directly read a file with extension '" + extension + "' via FileReaderRegistry, but no reader supporting this file type was found.";
  return reader->Read(path);
}

std::vector< mitk::BaseData::Pointer > mitk::FileReaderRegistry::ReadAll(
  const std::vector<std::string>& paths, std::vector<std::string>* unreadableFiles,
    us::ModuleContext* context)
{
  FileReaderRegistry readerRegistry;
  std::vector< mitk::BaseData::Pointer > result;
  for (std::vector<std::string>::const_iterator iterator = paths.begin(), end = paths.end(); iterator != end; ++iterator)
  {
    try
    {
      std::string extension = itksys::SystemTools::GetFilenameExtension(*iterator);
      if (!extension.empty())
      {
        extension = extension.substr(1, extension.size()-1);
      }
      mitk::IFileReader* reader = readerRegistry.GetReader(extension, context);
      // Throw exception if no compatible reader was found
      if (reader == NULL) throw;

      std::vector<mitk::BaseData::Pointer> baseDataList = reader->Read( *iterator );
      result.insert(result.end(), baseDataList.begin(), baseDataList.end());
    }
    catch (...)
    {
      if (unreadableFiles) unreadableFiles->push_back( *iterator );
    }
  }
  return result;
}


//////////////////// GETTING READERS ////////////////////

mitk::FileReaderRegistry::ReaderReference mitk::FileReaderRegistry::GetReaderReference(const std::string& mimeType, us::ModuleContext* context)
{
  std::vector<ReaderReference> refs = GetReaderReferences(mimeType, context);
  if (refs.empty()) return ReaderReference();
  std::sort(refs.begin(), refs.end());
  return refs.back();
}

std::vector<mitk::FileReaderRegistry::ReaderReference> mitk::FileReaderRegistry::GetReaderReferences(const std::string& mimeType, us::ModuleContext* context)
{
  std::string filter = us::LDAPProp(us::ServiceConstants::OBJECTCLASS()) == us_service_interface_iid<IFileReader>() &&
                       us::LDAPProp(IFileReader::PROP_MIMETYPE()) == mimeType;
  return context->GetServiceReferences<IFileReader>(filter);
}

mitk::IFileReader* mitk::FileReaderRegistry::GetReader(const mitk::FileReaderRegistry::ReaderReference& ref, us::ModuleContext* context)
{
  us::ServiceObjects<mitk::IFileReader> serviceObjects = context->GetServiceObjects(ref);
  mitk::IFileReader* reader = serviceObjects.GetService();
  m_ServiceObjects.insert(std::make_pair(reader, serviceObjects));
  return reader;
}

mitk::IFileReader* mitk::FileReaderRegistry::GetReader(const std::string& extension, us::ModuleContext* context )
{
  std::vector<mitk::IFileReader*> results = GetReaders(extension, context);
  if (results.empty()) return NULL;
  return results.front();
}

std::vector <mitk::IFileReader*> mitk::FileReaderRegistry::GetReaders(const std::string& extension, us::ModuleContext* context )
{
  std::vector <mitk::IFileReader*> result;
  const std::vector <us::ServiceReference<IFileReader> > refs = GetReaderList(extension, context);
  result.reserve(refs.size());

  // Translate List of ServiceRefs to List of Pointers
  for (std::vector <us::ServiceReference<IFileReader> >::const_iterator iter = refs.begin(), end = refs.end();
    iter != end; ++iter)
  {
    us::ServiceObjects<mitk::IFileReader> serviceObjects = context->GetServiceObjects(*iter);
    mitk::IFileReader* reader = serviceObjects.GetService();
    m_ServiceObjects.insert(std::make_pair(reader, serviceObjects));
    result.push_back(reader);
  }

  return result;
}

mitk::IFileReader* mitk::FileReaderRegistry::GetReader(
    const std::string& extension, const mitk::IFileReader::OptionNames& options,
    us::ModuleContext* context )
{
  std::vector <mitk::IFileReader*> matching = mitk::FileReaderRegistry::GetReaders(extension, options, context);
  if (matching.empty()) return NULL;
  return matching.front();
}

std::vector <mitk::IFileReader*> mitk::FileReaderRegistry::GetReaders(
    const std::string& extension, const mitk::IFileReader::OptionNames& options,
    us::ModuleContext* context )
{
  const std::vector <mitk::IFileReader*> allReaders = mitk::FileReaderRegistry::GetReaders(extension, context);
  std::vector <mitk::IFileReader*> result;
  result.reserve(allReaders.size());
  // the list is already sorted by priority. Now find reader that supports all options

  for (std::vector <mitk::IFileReader*>::const_iterator iter = allReaders.begin(), end = allReaders.end();
    iter != end; ++iter)
  {
    mitk::IFileReader * currentReader = *iter;
    // Now see if this reader supports all options. If yes, push to results
    if ( mitk::FileReaderRegistry::ReaderSupportsOptions(currentReader, options) )
    {
      result.push_back(currentReader);
    }
  }
  return result;
}

void mitk::FileReaderRegistry::UngetReader(mitk::IFileReader* reader)
{
  std::map<mitk::IFileReader*, us::ServiceObjects<mitk::IFileReader> >::iterator readerIter =
    m_ServiceObjects.find(reader);
  if (readerIter != m_ServiceObjects.end())
  {
    readerIter->second.UngetService(reader);
    m_ServiceObjects.erase(readerIter);
  }
}

void mitk::FileReaderRegistry::UngetReaders(const std::vector<mitk::IFileReader*>& readers)
{
  for (std::vector<mitk::IFileReader*>::const_iterator iter = readers.begin(), end = readers.end();
    iter != end; ++iter)
  {
    this->UngetReader(*iter);
  }
}

//////////////////// INTERNAL CODE ////////////////////

bool mitk::FileReaderRegistry::ReaderSupportsOptions(mitk::IFileReader* reader,
                                                     const mitk::IFileReader::OptionNames& options )
{
  const mitk::IFileReader::OptionList readerOptions = reader->GetOptions();
  if (options.empty()) return true;         // if no options were requested, return true unconditionally
  if (readerOptions.empty()) return false;  // if options were requested and reader supports no options, return false

  // For each of the strings in requested options, check if option is available in reader
  for(mitk::IFileReader::OptionNames::const_iterator options_i = options.begin(), i_end = options.end(); options_i != i_end; ++options_i)
  {
    {
      bool optionFound = false;
      // Iterate over each available option from reader to check if one of them matches the current option
      for(mitk::IFileReader::OptionList::const_iterator options_j = readerOptions.begin(), j_end = readerOptions.end();
        options_j != j_end; ++options_j)
      {
        if ( *options_i == options_j->first ) optionFound = true;
      }
      if (optionFound == false) return false; // If one option was not found, leave method and return false
    }
  }
  return true; // if all options have been found, return true
}

//////////////////// uS-INTERACTION ////////////////////

std::vector< us::ServiceReference<mitk::IFileReader> > mitk::FileReaderRegistry::GetReaderList(const std::string& extension, us::ModuleContext* context )
{
  std::vector<us::ServiceReference<IFileReader> > result;

  // filter for mime type
  mitk::IMimeTypeProvider* mimeTypeProvider = mitk::CoreServices::GetMimeTypeProvider(context);
  std::vector<std::string> mimeTypes = mimeTypeProvider->GetMimeTypesForExtension(extension);
  if (mimeTypes.empty())
  {
    MITK_WARN << "No mime-type information for extension " << extension << " available.";
    return result;
  }
  std::string filter = us::LDAPProp(mitk::IFileReader::PROP_MIMETYPE()) == mimeTypes.front();
  result = context->GetServiceReferences<IFileReader>(filter);
  std::sort(result.begin(), result.end());
  std::reverse(result.begin(), result.end());
  return result;
}
