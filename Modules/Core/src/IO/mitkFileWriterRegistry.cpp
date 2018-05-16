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
#include "mitkBaseData.h"
#include "mitkCoreServices.h"
#include "mitkIMimeTypeProvider.h"

// Microservices
#include <usGetModuleContext.h>
#include <usLDAPProp.h>
#include <usModuleContext.h>
#include <usServiceProperties.h>

mitk::FileWriterRegistry::FileWriterRegistry()
{
}

mitk::FileWriterRegistry::~FileWriterRegistry()
{
  for (auto &elem : m_ServiceObjects)
  {
    elem.second.UngetService(elem.first);
  }
}

std::vector<mitk::FileWriterRegistry::WriterReference> mitk::FileWriterRegistry::GetReferences(
  const mitk::BaseData *baseData, us::ModuleContext *context)
{
  return GetReferences(baseData, std::string(), context);
}

std::vector<mitk::FileWriterRegistry::WriterReference> mitk::FileWriterRegistry::GetReferences(
  const mitk::BaseData *baseData, const std::string &mimeType, us::ModuleContext *context)
{
  if (baseData == nullptr)
  {
    mitkThrow() << "FileWriterRegistry::GetReferences was called with null basedata.";
    std::vector<mitk::FileWriterRegistry::WriterReference> emptyResult;
    return emptyResult;
  }

  if (context == nullptr)
    context = us::GetModuleContext();

  std::vector<WriterReference> result;

  // loop over the class hierarchy of baseData and get all writers
  // claiming to support the actual baseData class or any of its super classes
  std::vector<std::string> classHierarchy = baseData->GetClassHierarchy();
  for (std::vector<std::string>::const_iterator clIter = classHierarchy.begin(), clIterEnd = classHierarchy.end();
       clIter != clIterEnd;
       ++clIter)
  {
    std::string filter =
      us::LDAPProp(us::ServiceConstants::OBJECTCLASS()) == us_service_interface_iid<IFileWriter>() &&
      us::LDAPProp(IFileWriter::PROP_BASEDATA_TYPE()) == *clIter &&
      (mimeType.empty() ? us::LDAPPropExpr(std::string()) : us::LDAPProp(IFileWriter::PROP_MIMETYPE()) == mimeType);
    std::vector<WriterReference> refs = context->GetServiceReferences<IFileWriter>(filter);
    result.insert(result.end(), refs.begin(), refs.end());
  }
  return result;
}

mitk::IFileWriter *mitk::FileWriterRegistry::GetWriter(const mitk::FileWriterRegistry::WriterReference &ref,
                                                       us::ModuleContext *context)
{
  if (!ref)
    return nullptr;

  if (context == nullptr)
    context = us::GetModuleContext();

  us::ServiceObjects<mitk::IFileWriter> serviceObjects = context->GetServiceObjects(ref);
  mitk::IFileWriter *writer = serviceObjects.GetService();
  m_ServiceObjects.insert(std::make_pair(writer, serviceObjects));
  return writer;
}

std::vector<mitk::IFileWriter *> mitk::FileWriterRegistry::GetWriters(const mitk::BaseData *baseData,
                                                                      const std::string &mimeType,
                                                                      us::ModuleContext *context)
{
  if (baseData == nullptr)
  {
    mitkThrow() << "FileWriterRegistry::GetReferences was called with null basedata.";
    std::vector<mitk::IFileWriter *> emptyResult;
    return emptyResult;
  }

  if (context == nullptr)
    context = us::GetModuleContext();

  std::vector<mitk::IFileWriter *> result;

  std::vector<us::ServiceReference<IFileWriter>> refs;
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
  for (std::vector<us::ServiceReference<IFileWriter>>::const_reverse_iterator iter = refs.rbegin(), end = refs.rend();
       iter != end;
       ++iter)
  {
    us::ServiceObjects<mitk::IFileWriter> serviceObjects = context->GetServiceObjects(*iter);
    mitk::IFileWriter *writer = serviceObjects.GetService();
    m_ServiceObjects.insert(std::make_pair(writer, serviceObjects));
    result.push_back(writer);
  }

  return result;
}

void mitk::FileWriterRegistry::UngetWriter(mitk::IFileWriter *writer)
{
  auto writerIter =
    m_ServiceObjects.find(writer);
  if (writerIter != m_ServiceObjects.end())
  {
    writerIter->second.UngetService(writer);
    m_ServiceObjects.erase(writerIter);
  }
}

void mitk::FileWriterRegistry::UngetWriters(const std::vector<mitk::IFileWriter *> &writers)
{
  for (const auto &writer : writers)
  {
    this->UngetWriter(writer);
  }
}
