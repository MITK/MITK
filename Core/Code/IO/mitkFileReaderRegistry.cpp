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

// Microservices
#include <usGetModuleContext.h>
#include <usModuleContext.h>
#include <usServiceProperties.h>
#include <usLDAPProp.h>

// Legacy Support
#include <mitkCoreObjectFactory.h>

//const std::string mitk::IFileWriter::PROP_EXTENSION = "org.mitk.services.FileWriter.Extension";

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

std::list< mitk::BaseData::Pointer > mitk::FileReaderRegistry::Read(const std::string& path, us::ModuleContext* context)
{
  // Find extension
  std::string extension = path;
  extension.erase(0, path.find_last_of('.'));

  // Get best Reader
  mitk::IFileReader* reader = GetReader(extension, context);
  // Throw exception if no compatible reader was found
  if (reader == NULL) mitkThrow() << "Tried to directly read a file of type '" + extension + "' via FileReaderRegistry, but no reader supporting this filetype was found.";
  return reader->Read(path);
}

std::list< mitk::BaseData::Pointer > mitk::FileReaderRegistry::ReadAll(const std::list<std::string> paths, std::list<std::string>* unreadableFiles,
                                                                      us::ModuleContext* context)
{
  std::list< mitk::BaseData::Pointer > result;
  for (std::list<std::string>::const_iterator iterator = paths.begin(), end = paths.end(); iterator != end; ++iterator)
  {
    try
    {
      std::list<mitk::BaseData::Pointer> baseDataList = Read( *iterator, context );
      result.merge(baseDataList);
    }
    catch (...)
    {
      if (unreadableFiles) unreadableFiles->push_back( *iterator );
    }
  }
  return result;
}

//////////////////// GETTING READERS ////////////////////

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

mitk::IFileReader* mitk::FileReaderRegistry::GetReader(const std::string& extension, const std::list<std::string>& options, us::ModuleContext* context )
{
  std::vector <mitk::IFileReader*> matching = mitk::FileReaderRegistry::GetReaders(extension, options, context);
  if (matching.empty()) return NULL;
  return matching.front();
}

std::vector <mitk::IFileReader*> mitk::FileReaderRegistry::GetReaders(const std::string& extension, const std::list<std::string>& options, us::ModuleContext* context )
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

//////////////////// GENERIC INFORMATION ////////////////////

std::string mitk::FileReaderRegistry::GetSupportedExtensions(const std::string& extension)
{
  us::ModuleContext* context = us::GetModuleContext();
  const std::vector<us::ServiceReference<IFileReader> > refs = GetReaderList(extension, context);
  std::vector<std::string> entries; // Will contain Description + Extension (Human readable)
  entries.reserve(refs.size());
  std::string knownExtensions; // Will contain plain list of all known extensions (for the QFileDialog entry "All Known Extensions")
  for (std::vector<us::ServiceReference<IFileReader> >::const_iterator iterator = refs.begin(), end = refs.end(); iterator != end; ++iterator)
  {
    // Generate List of Extensions
    if (iterator == refs.begin()) // First entry without semicolon
      knownExtensions += "*" + iterator->GetProperty(mitk::IFileReader::PROP_EXTENSION).ToString();
    else // Ad semicolon for each following entry
      knownExtensions += "; *" + iterator->GetProperty(mitk::IFileReader::PROP_EXTENSION).ToString();

    // Generate List of human readable entries composed of Description + Extension
    std::string entry = iterator->GetProperty(mitk::IFileReader::PROP_DESCRIPTION).ToString() + "(*" + iterator->GetProperty(mitk::IFileReader::PROP_EXTENSION).ToString() + ");;";
    entries.push_back(entry);
  }
  std::sort(entries.begin(), entries.end());

  std::string result = "Known Extensions (" + knownExtensions + ");;All (*)";
  for (std::vector<std::string>::const_iterator iterator = entries.begin(), end = entries.end(); iterator != end; ++iterator)
  {
    result += ";;" + *iterator;
  }
  return result;
}

//////////////////// INTERNAL CODE ////////////////////

bool mitk::FileReaderRegistry::ReaderSupportsOptions(mitk::IFileReader* reader, const std::list<std::string>& options )
{
  const std::list< mitk::IFileReader::FileServiceOption > readerOptions = reader->GetOptions();
  if (options.empty()) return true;         // if no options were requested, return true unconditionally
  if (readerOptions.empty()) return false;  // if options were requested and reader supports no options, return false

  // For each of the strings in requested options, check if option is available in reader
  for(std::list< std::string >::const_iterator options_i = options.begin(), i_end = options.end(); options_i != i_end; ++options_i)
  {
    {
      bool optionFound = false;
      // Iterate over each available option from reader to check if one of them matches the current option
      for(std::list<mitk::IFileReader::FileServiceOption>::const_iterator options_j = readerOptions.begin(), j_end = readerOptions.end();
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
  // filter for class and extension
  std::string filter;
  if (!extension.empty())
  {
    filter = us::LDAPProp(mitk::IFileReader::PROP_EXTENSION) == extension;
  }
  std::vector<us::ServiceReference<IFileReader> > result = context->GetServiceReferences<IFileReader>(filter);
  std::sort(result.begin(), result.end());
  std::reverse(result.begin(), result.end());
  return result;
}
