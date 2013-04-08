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


#include "mitkFileReaderManager.h"

// Microservices
#include <usGetModuleContext.h>
#include <mitkModuleContext.h>
#include <usServiceProperties.h>


//////////////////// READING DIRECTLY ////////////////////

mitk::BaseData::Pointer mitk::FileReaderManager::Read(std::string path)
{
  mitk::ModuleContext * context = mitk::GetModuleContext();
  // Find extension
  std::string extension = path;
  extension.erase(0, path.find_last_of('.'));

  // Get best Reader
  mitk::FileReaderInterface::Pointer reader = GetReader(extension);

  //Read
  //return reader->Read(path);
  return 0;
}


//////////////////// GETTING READERS ////////////////////

mitk::FileReaderInterface* mitk::FileReaderManager::GetReader(std::string extension)
{
  mitk::ModuleContext * context = mitk::GetModuleContext();
  return context->GetService<mitk::FileReaderInterface>(GetReaderList(extension).front());
}

std::list <mitk::FileReaderInterface*> mitk::FileReaderManager::GetReaders(std::string extension)
{
  mitk::ModuleContext * context = mitk::GetModuleContext();
  std::list <mitk::FileReaderInterface*> result;
  std::list <mitk::ServiceReference > refs = GetReaderList(extension);

  // Translate List of ServiceRefs to List of Pointers
  while ( !refs.empty() )
  {
    result.push_back( context->GetService<mitk::FileReaderInterface>(refs.front()));
    refs.pop_front();
  }

  return result;
}



//////////////////// uS-INTERACTION ////////////////////

std::list< mitk::ServiceReference > mitk::FileReaderManager::GetReaderList(std::string extension)
{
  mitk::ModuleContext * context = mitk::GetModuleContext();
  // filter for class and extension
  std::string filter = "(&(" + mitk::ServiceConstants::OBJECTCLASS() + "=org.mitk.services.FileReader)(" + mitk::FileReaderInterface::US_EXTENSION + "=" + extension + "))";
  std::list <mitk::ServiceReference> result = context->GetServiceReferences("org.mitk.services.FileReader", filter);
  // the comparator sorts by priority
  result.sort(mitk::FileReaderManager::CompareServiceRef);
  return result;
}

bool mitk::FileReaderManager::CompareServiceRef(mitk::ServiceReference first, mitk::ServiceReference second)
{
  return (any_cast<int>(first.GetProperty(mitk::FileReaderInterface::US_PRIORITY)) > any_cast<int> (second.GetProperty(mitk::FileReaderInterface::US_PRIORITY)));
}
