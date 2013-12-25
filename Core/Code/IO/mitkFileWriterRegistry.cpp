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


#include "mitkFileWriterRegistry.h"

// MITK
#include <mitkCoreObjectFactory.h>

// ITK
#include <itksys/SystemTools.hxx>

// Microservices
#include <usGetModuleContext.h>
#include <usModuleContext.h>
#include <usServiceProperties.h>
#include <usLDAPProp.h>

mitk::FileWriterRegistry::FileWriterRegistry()
{
}

mitk::FileWriterRegistry::~FileWriterRegistry()
{
  for (std::map<mitk::IFileWriter*, us::ServiceObjects<mitk::IFileWriter> >::iterator iter = m_ServiceObjects.begin(),
       end = m_ServiceObjects.end(); iter != end; ++iter)
  {
    iter->second.UngetService(iter->first);
  }
}

//////////////////// WRITING DIRECTLY ////////////////////

void mitk::FileWriterRegistry::Write(const mitk::BaseData* data, const std::string& path, us::ModuleContext* context)
{
  // Find extension
  std::string extension = itksys::SystemTools::GetFilenameExtension(path);
  if (!extension.empty())
  {
    extension = extension.substr(1, extension.size()-1);
  }

  // Get best Writer
  FileWriterRegistry writerRegistry;
  mitk::IFileWriter* writer = writerRegistry.GetWriter(data, extension, mitk::IFileWriter::OptionNames(), context);
  // Throw exception if no compatible Writer was found
  if (writer == NULL)
  {
    mitkThrow() << "Tried to directly write a file of type '" << data->GetNameOfClass()
                << "' and extension '" << extension
                << "' via FileWriterRegistry, but no writer supporting this file type was found.";
  }
  writer->Write(data, path);
}

void mitk::FileWriterRegistry::Write(const mitk::BaseData* data, std::ostream& os, us::ModuleContext* context)
{
  // Get best Writer
  FileWriterRegistry writerRegistry;
  mitk::IFileWriter* writer = writerRegistry.GetWriter(data, std::string(), mitk::IFileWriter::OptionNames(), context);
  // Throw exception if no compatible Writer was found
  if (writer == NULL)
  {
    mitkThrow() << "Tried to directly write a file of type '" << data->GetNameOfClass()
                << "' to a std::ostream via FileWriterRegistry, but no writer supporting this BaseData type was found.";
  }
  writer->Write(data, os);
}


//////////////////// GETTING WRITERS ////////////////////

mitk::IFileWriter* mitk::FileWriterRegistry::GetWriter(
    const mitk::BaseData* baseData, const std::string& extension,
    const mitk::IFileWriter::OptionNames& options, us::ModuleContext* context)
{
  return GetWriter(baseData->GetNameOfClass(), extension, options, context);
}

mitk::IFileWriter* mitk::FileWriterRegistry::GetWriter(
    const std::string& baseDataType, const std::string& extension,
    const mitk::IFileWriter::OptionNames& options, us::ModuleContext* context)
{
  std::vector<mitk::IFileWriter*> results = GetWriters(baseDataType, extension, options, context);
  if (results.empty()) return NULL;
  return results.front();
}

std::vector<mitk::IFileWriter*> mitk::FileWriterRegistry::GetWriters(
    const mitk::BaseData* baseData, const std::string& extension,
    const mitk::IFileWriter::OptionNames& options, us::ModuleContext* context)
{
  return GetWriters(baseData->GetNameOfClass(), extension, options, context);
}

std::vector<mitk::IFileWriter*> mitk::FileWriterRegistry::GetWriters(
    const std::string& baseDataType, const std::string& extension,
    const mitk::IFileWriter::OptionNames& options, us::ModuleContext* context)
{
  // filter for class and extension
  std::string filter = us::LDAPProp(mitk::IFileWriter::PROP_BASEDATA_TYPE()) == baseDataType &&
                       us::LDAPProp(mitk::IFileWriter::PROP_EXTENSION()) == extension;

  std::vector <us::ServiceReference<IFileWriter> > refs = context->GetServiceReferences<IFileWriter>(filter);
  std::sort(refs.begin(), refs.end());
  std::reverse(refs.begin(), refs.end());

  std::vector<mitk::IFileWriter*> result;
  result.reserve(refs.size());

  // Translate List of ServiceRefs to List of Pointers
  for (std::vector <us::ServiceReference<IFileWriter> >::const_iterator iter = refs.begin(), end = refs.end();
       iter != end; ++iter)
  {
    us::ServiceObjects<mitk::IFileWriter> serviceObjects = context->GetServiceObjects(*iter);
    mitk::IFileWriter* writer = serviceObjects.GetService();

    // Now see if this Writer supports all options. If yes, push to results
    if (mitk::FileWriterRegistry::WriterSupportsOptions(writer, options))
    {
      m_ServiceObjects.insert(std::make_pair(writer, serviceObjects));
      result.push_back(writer);
    }
  }

  return result;
}


//////////////////// GENERIC INFORMATION ////////////////////

std::string mitk::FileWriterRegistry::GetSupportedExtensions(const std::string& extension, us::ModuleContext* context)
{
  std::string filter = us::LDAPProp(mitk::IFileWriter::PROP_EXTENSION()) == extension;
  const std::vector <us::ServiceReference<IFileWriter> > refs = context->GetServiceReferences<IFileWriter>(filter);
  return CreateFileDialogString(refs);
}

std::string mitk::FileWriterRegistry::GetSupportedWriters(const std::string& baseDataType, us::ModuleContext* context)
{
  std::string filter = us::LDAPProp(mitk::IFileWriter::PROP_BASEDATA_TYPE()) == baseDataType;
  const std::vector <us::ServiceReference<IFileWriter> > refs = context->GetServiceReferences<IFileWriter>(filter);
  return CreateFileDialogString(refs);
}


//////////////////// INTERNAL CODE ////////////////////

bool mitk::FileWriterRegistry::WriterSupportsOptions(mitk::IFileWriter* writer, const mitk::IFileWriter::OptionNames& options )
{
  const mitk::IFileWriter::OptionList writerOptions = writer->GetOptions();
  if (options.empty()) return true;         // if no options were requested, return true unconditionally
  if (writerOptions.empty()) return false;  // if options were requested and reader supports no options, return false

  // For each of the strings in requested options, check if option is available in reader
  for(mitk::IFileWriter::OptionNames::const_iterator options_i = options.begin(), i_end = options.end(); options_i != i_end; ++options_i)
  {
    {
      bool optionFound = false;
      // Iterate over each available option from reader to check if one of them matches the current option
      for(mitk::IFileWriter::OptionList::const_iterator options_j = writerOptions.begin(), j_end = writerOptions.end();
          options_j != j_end; ++options_j)
      {
        if ( *options_i == options_j->first ) optionFound = true;
      }
      if (optionFound == false) return false; // If one option was not found, leave method and return false
    }
  }
  return true; // if all options have been found, return true
}

std::string mitk::FileWriterRegistry::CreateFileDialogString(const std::vector<us::ServiceReference<IFileWriter> >& refs)
{
  std::vector<std::string> entries; // Will contain Description + Extension (Human readable)
  entries.reserve(refs.size());
  std::string knownExtensions; // Will contain plain list of all known extensions (for the QFileDialog entry "All Known Extensions")
  for (std::vector<us::ServiceReference<IFileWriter> >::const_iterator iterator = refs.begin(), end = refs.end();
       iterator != end; ++iterator)
  {
    // Generate List of Extensions
    if (iterator == refs.begin()) // First entry without semicolon
      knownExtensions += "*" + iterator->GetProperty(mitk::IFileWriter::PROP_EXTENSION()).ToString();
    else // Ad semicolon for each following entry
      knownExtensions += "; *" + iterator->GetProperty(mitk::IFileWriter::PROP_EXTENSION()).ToString();

    // Generate List of human readable entries composed of Description + Extension
    std::string entry = iterator->GetProperty(mitk::IFileWriter::PROP_DESCRIPTION()).ToString() + "(*" + iterator->GetProperty(mitk::IFileWriter::PROP_EXTENSION()).ToString() + ");;";
    entries.push_back(entry);
  }
  std::sort(entries.begin(), entries.end());

  std::string result = "Known Extensions (" + knownExtensions + ");;All (*)";
  for (std::vector<std::string>::const_iterator iterator = entries.begin(), end = entries.end();
       iterator != end; ++iterator)
  {
    result += ";;" + *iterator;
  }
  return result;
}
