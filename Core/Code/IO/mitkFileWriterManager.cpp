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


#include "mitkFileWriterManager.h"

// MITK
#include <mitkCoreObjectFactory.h>

// Microservices
#include <usGetModuleContext.h>
#include <mitkModuleContext.h>
#include <usServiceProperties.h>


//////////////////// WRITING DIRECTLY ////////////////////

void mitk::FileWriterManager::Write(mitk::BaseData::Pointer data, const std::string& path, mitk::ModuleContext* context)
{
  // Find extension
  std::string extension = path;
  extension.erase(0, path.find_last_of('.'));

  // Get best Writer
  mitk::IFileWriter* Writer = GetWriter(extension);
  // Throw exception if no compatible Writer was found
  if (Writer == 0) mitkThrow() << "Tried to directly Write a file of type '" + extension + "' via FileWriterManager, but no Writer supporting this filetype was found.";
  Writer->Write(data, path);
}


//////////////////// GETTING WRITERS ////////////////////

mitk::IFileWriter* mitk::FileWriterManager::GetWriter(const std::string& extension, mitk::ModuleContext* context )
{
  std::list <mitk::ServiceReference> results = GetWriterList(extension, context);
  if (results.size() == 0) return 0;
  return context->GetService<mitk::IFileWriter>(results.front());
}

std::list <mitk::IFileWriter*> mitk::FileWriterManager::GetWriters(const std::string& extension, mitk::ModuleContext* context )
{
  std::list <mitk::IFileWriter*> result;
  std::list <mitk::ServiceReference > refs = GetWriterList(extension, context);

  // Translate List of ServiceRefs to List of Pointers
  while ( !refs.empty() )
  {
    result.push_back( context->GetService<mitk::IFileWriter>(refs.front()));
    refs.pop_front();
  }

  return result;
}

mitk::IFileWriter* mitk::FileWriterManager::GetWriter(const std::string& extension, const std::list<std::string>& options, mitk::ModuleContext* context )
{
  std::list <mitk::IFileWriter*> matching = mitk::FileWriterManager::GetWriters(extension, options, context);
  if (matching.size() == 0) return 0;
  return matching.front();
}

std::list <mitk::IFileWriter*> mitk::FileWriterManager::GetWriters(const std::string& extension, const std::list<std::string>& options, mitk::ModuleContext* context )
{
  std::list <mitk::IFileWriter*> allWriters = mitk::FileWriterManager::GetWriters(extension, context);
  std::list <mitk::IFileWriter*> result;
  // the list is alWritey sorted by priority. Now find Writer that supports all options

  while (allWriters.size() > 0)
  {
    mitk::IFileWriter * currentWriter = allWriters.front();
    allWriters.pop_front();
    // Now see if this Writer supports all options. If yes, push to results
    if ( mitk::FileWriterManager::WriterSupportsOptions(currentWriter, options) ) result.push_back(currentWriter);
  }
  return result;
}

//////////////////// GENERIC INFORMATION ////////////////////

std::string mitk::FileWriterManager::GetSupportedExtensions(const std::string& extension)
{
  mitk::ModuleContext* context = mitk::GetModuleContext();
  std::list<mitk::ServiceReference> refs = GetWriterList(extension, context);
  return CreateFileDialogString(refs);
}

std::string mitk::FileWriterManager::GetSupportedWriters(const std::string& basedataType)
{
  mitk::ModuleContext* context = mitk::GetModuleContext();
  std::list<mitk::ServiceReference> refs = GetWriterListByBasedataType(basedataType, context);
  return CreateFileDialogString(refs);
}


//////////////////// INTERNAL CODE ////////////////////

bool mitk::FileWriterManager::WriterSupportsOptions(mitk::IFileWriter* Writer, std::list<std::string> options )
{
  std::list<std::string> WriterOptions = Writer->GetSupportedOptions();
  if (options.size() == 0) return true;         // if no options were requested, return true unconditionally
  if (WriterOptions.size() == 0) return false;  // if options were requested and Writer supports no options, return false

  // For each of the strings in requuested options, check if option is available in Writer
  for(std::list<std::string>::iterator options_i = options.begin(); options_i != options.end(); options_i++)
  {
    {
      bool optionFound = false;
      // Iterate over each available option from Writer to check if one of them matches the current option
      for(std::list<std::string>::iterator options_j = WriterOptions.begin(); options_j != WriterOptions.end(); options_j++)
        if ( *options_i == *options_j ) optionFound = true;
      if (optionFound == false) return false; // If one option was not found, leave method and return false
    }
  }
  return true; // if all options have been found, return true
}

std::string mitk::FileWriterManager::CreateFileDialogString(std::list<mitk::ServiceReference> refs)
{
  std::list<std::string> entries; // Will contain Description + Extension (Human readable)
  std::string knownExtensions; // Will contain plain list of all known extensions (for the QFileDialog entry "All Known Extensions")
  for (std::list<mitk::ServiceReference>::const_iterator iterator = refs.begin(), end = refs.end(); iterator != end; ++iterator)
  {
    // Generate List of Extensions
    if (iterator == refs.begin()) // First entry without semicolon
      knownExtensions += "*" + iterator->GetProperty(mitk::IFileWriter::PROP_EXTENSION).ToString();
    else // Ad semicolon for each following entry
      knownExtensions += "; *" + iterator->GetProperty(mitk::IFileWriter::PROP_EXTENSION).ToString();

    // Generate List of human readable entries composed of Description + Extension
    std::string entry = iterator->GetProperty(mitk::IFileWriter::PROP_DESCRIPTION).ToString() + "(*" + iterator->GetProperty(mitk::IFileWriter::PROP_EXTENSION).ToString() + ");;";
    entries.push_back(entry);
  }
  entries.sort();
  entries.push_front("Known Extensions (" + knownExtensions + ");;");
  entries.push_front("All (*);;");

  std::string result;
  for (std::list<std::string>::const_iterator iterator = entries.begin(), end = entries.end(); iterator != end; ++iterator)
    result = result.append(*iterator);
  return result;
}


//////////////////// uS-INTERACTION ////////////////////

std::list< mitk::ServiceReference > mitk::FileWriterManager::GetWriterList(const std::string& extension, mitk::ModuleContext* context )
{
  // filter for class and extension
  std::string filter;
  if (extension == "")
    filter = "";
  else
    filter = "(&(" + mitk::ServiceConstants::OBJECTCLASS() + "=org.mitk.services.FileWriter)(" + mitk::IFileWriter::PROP_EXTENSION + "=" + extension + "))";
  std::list <mitk::ServiceReference> result = context->GetServiceReferences("org.mitk.services.FileWriter", filter);
  result.sort();
  std::reverse(result.begin(), result.end());
  return result;
}

std::list< mitk::ServiceReference > mitk::FileWriterManager::GetWriterListByBasedataType(const std::string& basedataType, mitk::ModuleContext* context )
{
  // filter for class and extension
  std::string filter = "(&(" + mitk::ServiceConstants::OBJECTCLASS() + "=org.mitk.services.FileWriter)(" + mitk::IFileWriter::PROP_BASEDATA_TYPE + "=" + basedataType + "))";
  std::list <mitk::ServiceReference> result = context->GetServiceReferences("org.mitk.services.FileWriter", filter);
  result.sort();
  std::reverse(result.begin(), result.end());
  return result;
}
