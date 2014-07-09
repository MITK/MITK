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
#include "mitkIMimeTypeProvider.h"
#include "mitkCoreServices.h"
#include "mitkBaseData.h"

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

std::pair<std::string, std::string> mitk::FileWriterRegistry::GetDefaultExtension(const mitk::FileWriterRegistry::WriterReference& ref, us::ModuleContext* context)
{
  if (!ref) return std::make_pair(std::string(), std::string());

  std::string mimeType = ref.GetProperty(IFileWriter::PROP_MIMETYPE()).ToString();

  CoreServicePointer<IMimeTypeProvider> mimeTypeProvider(CoreServices::GetMimeTypeProvider(context));
  std::vector<std::string> extensions = mimeTypeProvider->GetExtensions(mimeType);
  if (extensions.empty()) return std::make_pair(std::string(), std::string());
  return std::make_pair(extensions.front(), mimeType);
}

std::pair<std::string, std::string> mitk::FileWriterRegistry::GetDefaultExtension(const mitk::BaseData* data, us::ModuleContext* context)
{
  return GetDefaultExtension(GetReference(data, context));
}

mitk::FileWriterRegistry::WriterReference mitk::FileWriterRegistry::GetReference(const mitk::BaseData* baseData, us::ModuleContext* context)
{
  return GetReference(baseData->GetNameOfClass(), context);
}

mitk::FileWriterRegistry::WriterReference mitk::FileWriterRegistry::GetReference(const mitk::BaseData* baseData, const std::string& mimeType, us::ModuleContext* context)
{
  return GetReference(baseData->GetNameOfClass(), mimeType, context);
}

mitk::FileWriterRegistry::WriterReference mitk::FileWriterRegistry::GetReference(const std::string& baseDataType, us::ModuleContext* context)
{
  std::vector<WriterReference> refs = GetReferences(baseDataType, context);
  if (refs.empty()) return WriterReference();
  std::sort(refs.begin(), refs.end());
  return refs.back();
}

mitk::FileWriterRegistry::WriterReference mitk::FileWriterRegistry::GetReference(const std::string& baseDataType, const std::string& mimeType, us::ModuleContext* context)
{
  std::vector<WriterReference> refs = GetReferences(baseDataType, mimeType, context);
  if (refs.empty()) return WriterReference();
  std::sort(refs.begin(), refs.end());
  return refs.back();
}

std::vector<mitk::FileWriterRegistry::WriterReference> mitk::FileWriterRegistry::GetReferences(const mitk::BaseData* baseData, us::ModuleContext* context)
{
  return GetReferences(baseData->GetNameOfClass(), context);
}

std::vector<mitk::FileWriterRegistry::WriterReference> mitk::FileWriterRegistry::GetReferences(const mitk::BaseData* baseData, const std::string& mimeType, us::ModuleContext* context)
{
  return GetReferences(baseData->GetNameOfClass(), mimeType, context);
}

std::vector<mitk::FileWriterRegistry::WriterReference> mitk::FileWriterRegistry::GetReferences(const std::string& baseDataType, us::ModuleContext* context)
{
  return GetReferences(baseDataType, std::string(), context);
}

std::vector<mitk::FileWriterRegistry::WriterReference> mitk::FileWriterRegistry::GetReferences(const std::string& baseDataType, const std::string& mimeType, us::ModuleContext* context)
{
  std::string filter = us::LDAPProp(us::ServiceConstants::OBJECTCLASS()) == us_service_interface_iid<IFileWriter>() &&
                       us::LDAPProp(IFileWriter::PROP_BASEDATA_TYPE()) == baseDataType &&
                       (mimeType.empty() ? us::LDAPPropExpr(std::string()) : us::LDAPProp(IFileWriter::PROP_MIMETYPE()) == mimeType);
  return context->GetServiceReferences<IFileWriter>(filter);
}

mitk::IFileWriter*mitk::FileWriterRegistry::GetWriter(const mitk::FileWriterRegistry::WriterReference& ref, us::ModuleContext* context)
{
  us::ServiceObjects<mitk::IFileWriter> serviceObjects = context->GetServiceObjects(ref);
  mitk::IFileWriter* writer = serviceObjects.GetService();
  m_ServiceObjects.insert(std::make_pair(writer, serviceObjects));
  return writer;
}

mitk::IFileWriter*mitk::FileWriterRegistry::GetWriter(const std::string& baseDataType, us::ModuleContext* context)
{
  std::vector<IFileWriter*> result = GetWriters(baseDataType, context);
  if (result.empty())
  {
    return NULL;
  }
  return result.front();
}

mitk::IFileWriter*mitk::FileWriterRegistry::GetWriter(const std::string& baseDataType, const std::string& mimeType, us::ModuleContext* context)
{
  std::vector<IFileWriter*> result = GetWriters(baseDataType, mimeType, context);
  if (result.empty())
  {
    return NULL;
  }
  return result.front();
}

mitk::IFileWriter*mitk::FileWriterRegistry::GetWriter(const mitk::BaseData* baseData, us::ModuleContext* context)
{
  return GetWriter(baseData->GetNameOfClass(), context);
}

mitk::IFileWriter*mitk::FileWriterRegistry::GetWriter(const mitk::BaseData* baseData, const std::string& mimeType, us::ModuleContext* context)
{
  return GetWriter(baseData->GetNameOfClass(), mimeType, context);
}

std::vector<mitk::IFileWriter*> mitk::FileWriterRegistry::GetWriters(const std::string& baseDataType, us::ModuleContext* context)
{
  return GetWriters(baseDataType, std::string(), context);
}

std::vector<mitk::IFileWriter*> mitk::FileWriterRegistry::GetWriters(const std::string& baseDataType, const std::string& mimeType, us::ModuleContext* context)
{
  std::vector <mitk::IFileWriter*> result;

  std::vector <us::ServiceReference<IFileWriter> > refs;
  if (mimeType.empty())
  {
    refs = GetReferences(baseDataType, context);
  }
  else
  {
    refs = GetReferences(baseDataType, mimeType, context);
  }
  std::sort(refs.begin(), refs.end());

  result.reserve(refs.size());

  // Translate List of ServiceRefs to List of Pointers
  for (std::vector<us::ServiceReference<IFileWriter> >::const_reverse_iterator iter = refs.rbegin(), end = refs.rend();
       iter != end; ++iter)
  {
    us::ServiceObjects<mitk::IFileWriter> serviceObjects = context->GetServiceObjects(*iter);
    mitk::IFileWriter* writer = serviceObjects.GetService();
    m_ServiceObjects.insert(std::make_pair(writer, serviceObjects));
    result.push_back(writer);
  }

  return result;
}

std::vector<mitk::IFileWriter*> mitk::FileWriterRegistry::GetWriters(const mitk::BaseData* baseData, us::ModuleContext* context)
{
  return GetWriters(baseData->GetNameOfClass(), context);
}

std::vector<mitk::IFileWriter*> mitk::FileWriterRegistry::GetWriters(const mitk::BaseData* baseData, const std::string& mimeType, us::ModuleContext* context)
{
  return GetWriters(baseData->GetNameOfClass(), mimeType, context);
}

void mitk::FileWriterRegistry::UngetWriter(mitk::IFileWriter* writer)
{
  std::map<mitk::IFileWriter*, us::ServiceObjects<mitk::IFileWriter> >::iterator writerIter =
      m_ServiceObjects.find(writer);
  if (writerIter != m_ServiceObjects.end())
  {
    writerIter->second.UngetService(writer);
    m_ServiceObjects.erase(writerIter);
  }
}

void mitk::FileWriterRegistry::UngetWriters(const std::vector<mitk::IFileWriter*>& writers)
{
  for (std::vector<mitk::IFileWriter*>::const_iterator iter = writers.begin(), end = writers.end();
    iter != end; ++iter)
  {
    this->UngetWriter(*iter);
  }
}
