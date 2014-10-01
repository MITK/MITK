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

std::vector<mitk::FileWriterRegistry::WriterReference> mitk::FileWriterRegistry::GetReferences(const mitk::BaseData* baseData, us::ModuleContext* context)
{
  return GetReferences(baseData, std::string(), context);
}

std::vector<mitk::FileWriterRegistry::WriterReference> mitk::FileWriterRegistry::GetReferences(const mitk::BaseData* baseData, const std::string& mimeType, us::ModuleContext* context)
{
  std::vector<WriterReference> result;

  // loop over the class hierarchy of baseData and get all writers
  // claiming the support the actual baseData class or any of its super classes
  std::vector<std::string> classHierarchy = baseData->GetClassHierarchy();
  for (std::vector<std::string>::const_iterator clIter = classHierarchy.begin(),
       clIterEnd = classHierarchy.end(); clIter != clIterEnd; ++clIter)
  {
    std::string filter = us::LDAPProp(us::ServiceConstants::OBJECTCLASS()) == us_service_interface_iid<IFileWriter>() &&
                         us::LDAPProp(IFileWriter::PROP_BASEDATA_TYPE()) == *clIter &&
                         (mimeType.empty() ? us::LDAPPropExpr(std::string()) : us::LDAPProp(IFileWriter::PROP_MIMETYPE()) == mimeType);
    std::vector<WriterReference> refs = context->GetServiceReferences<IFileWriter>(filter);
    result.insert(result.end(), refs.begin(), refs.end());
  }
  return result;
}

mitk::IFileWriter* mitk::FileWriterRegistry::GetWriter(const mitk::FileWriterRegistry::WriterReference& ref, us::ModuleContext* context)
{
  if (!ref) return NULL;

  us::ServiceObjects<mitk::IFileWriter> serviceObjects = context->GetServiceObjects(ref);
  mitk::IFileWriter* writer = serviceObjects.GetService();
  m_ServiceObjects.insert(std::make_pair(writer, serviceObjects));
  return writer;
}

std::vector<mitk::IFileWriter*> mitk::FileWriterRegistry::GetWriters(const mitk::BaseData* baseData, const std::string& mimeType, us::ModuleContext* context)
{
  std::vector <mitk::IFileWriter*> result;

  std::vector <us::ServiceReference<IFileWriter> > refs;
  if (mimeType.empty())
  {
    refs = GetReferences(baseData, context);
  }
  else
  {
    refs = GetReferences(baseData, mimeType, context);
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
