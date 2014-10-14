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

mitk::MimeType mitk::FileReaderRegistry::GetMimeTypeForFile(const std::string& path, us::ModuleContext* context)
{
  mitk::CoreServicePointer<mitk::IMimeTypeProvider> mimeTypeProvider(mitk::CoreServices::GetMimeTypeProvider(context));
  std::vector<MimeType> mimeTypes = mimeTypeProvider->GetMimeTypesForFile(path);
  if (mimeTypes.empty())
  {
    return MimeType();
  }
  else
  {
    return mimeTypes.front();
  }
}

std::vector<mitk::FileReaderRegistry::ReaderReference> mitk::FileReaderRegistry::GetReferences(const MimeType& mimeType, us::ModuleContext* context)
{
  std::string filter = us::LDAPProp(us::ServiceConstants::OBJECTCLASS()) == us_service_interface_iid<IFileReader>() &&
                       us::LDAPProp(IFileReader::PROP_MIMETYPE()) == mimeType.GetName();
  return context->GetServiceReferences<IFileReader>(filter);
}

mitk::IFileReader* mitk::FileReaderRegistry::GetReader(const mitk::FileReaderRegistry::ReaderReference& ref, us::ModuleContext* context)
{
  us::ServiceObjects<mitk::IFileReader> serviceObjects = context->GetServiceObjects(ref);
  mitk::IFileReader* reader = serviceObjects.GetService();
  m_ServiceObjects.insert(std::make_pair(reader, serviceObjects));
  return reader;
}

std::vector <mitk::IFileReader*> mitk::FileReaderRegistry::GetReaders(const MimeType& mimeType, us::ModuleContext* context )
{
  std::vector <mitk::IFileReader*> result;

  if (!mimeType.IsValid()) return result;

  std::vector <us::ServiceReference<IFileReader> > refs = GetReferences(mimeType, context);
  std::sort(refs.begin(), refs.end());

  result.reserve(refs.size());

  // Translate List of ServiceRefs to List of Pointers
  for (std::vector<us::ServiceReference<IFileReader> >::const_reverse_iterator iter = refs.rbegin(), end = refs.rend();
       iter != end; ++iter)
  {
    us::ServiceObjects<mitk::IFileReader> serviceObjects = context->GetServiceObjects(*iter);
    mitk::IFileReader* reader = serviceObjects.GetService();
    m_ServiceObjects.insert(std::make_pair(reader, serviceObjects));
    result.push_back(reader);
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
