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

#include "mitkMimeTypeProvider.h"

#include "mitkLogMacros.h"

#include <usGetModuleContext.h>
#include <usModuleContext.h>

#include <itksys/SystemTools.hxx>

#ifdef _MSC_VER
#pragma warning(disable:4503) // decorated name length exceeded, name was truncated
#pragma warning(disable:4355)
#endif

namespace mitk {

MimeTypeProvider::MimeTypeProvider()
  : m_Tracker(NULL)
{
}

MimeTypeProvider::~MimeTypeProvider()
{
  delete m_Tracker;
}

void MimeTypeProvider::Start()
{
  if (m_Tracker == NULL)
  {
    m_Tracker = new us::ServiceTracker<CustomMimeType, MimeTypeTrackerTypeTraits>(us::GetModuleContext(), this);
  }
  m_Tracker->Open();
}

void MimeTypeProvider::Stop()
{
  m_Tracker->Close();
}

std::vector<MimeType> MimeTypeProvider::GetMimeTypes() const
{
  std::vector<MimeType> result;
  for (std::map<std::string, MimeType>::const_iterator iter = m_NameToMimeType.begin(),
       end = m_NameToMimeType.end(); iter != end; ++iter)
  {
    result.push_back(iter->second);
  }
  return result;
}

std::vector<MimeType> MimeTypeProvider::GetMimeTypesForFile(const std::string& filePath) const
{
  std::vector<MimeType> result;
  for (std::map<std::string, MimeType>::const_iterator iter = m_NameToMimeType.begin(),
       iterEnd = m_NameToMimeType.end(); iter != iterEnd; ++iter)
  {
    if (iter->second.AppliesTo(filePath))
    {
      result.push_back(iter->second);
    }
  }
  std::sort(result.begin(), result.end());
  std::reverse(result.begin(), result.end());
  return result;
}

std::vector<MimeType> MimeTypeProvider::GetMimeTypesForCategory(const std::string& category) const
{
  std::vector<MimeType> result;
  for (std::map<std::string, MimeType>::const_iterator iter = m_NameToMimeType.begin(),
       end = m_NameToMimeType.end(); iter != end; ++iter)
  {
    if (iter->second.GetCategory() == category)
    {
      result.push_back(iter->second);
    }
  }
  return result;
}

MimeType MimeTypeProvider::GetMimeTypeForName(const std::string& name) const
{
  std::map<std::string, MimeType>::const_iterator iter = m_NameToMimeType.find(name);
  if (iter != m_NameToMimeType.end()) return iter->second;
  return MimeType();
}

std::vector<std::string> MimeTypeProvider::GetCategories() const
{
  std::vector<std::string> result;
  for (std::map<std::string, MimeType>::const_iterator iter = m_NameToMimeType.begin(),
       end = m_NameToMimeType.end(); iter != end; ++iter)
  {
    std::string category = iter->second.GetCategory();
    if (!category.empty())
    {
      result.push_back(category);
    }
  }
  std::sort(result.begin(), result.end());
  result.erase(std::unique(result.begin(), result.end()), result.end());
  return result;
}

MimeTypeProvider::TrackedType MimeTypeProvider::AddingService(const ServiceReferenceType& reference)
{
  MimeType result = this->GetMimeType(reference);
  if (result.IsValid())
  {
    std::string name = result.GetName();
    m_NameToMimeTypes[name].insert(result);

    // get the highest ranked mime-type
    m_NameToMimeType[name] = *(m_NameToMimeTypes[name].rbegin());
  }
  return result;
}

void MimeTypeProvider::ModifiedService(const ServiceReferenceType& /*reference*/, TrackedType /*mimetype*/)
{
  // should we track changes in the ranking property?
}

void MimeTypeProvider::RemovedService(const ServiceReferenceType& /*reference*/, TrackedType mimeType)
{
  std::string name = mimeType.GetName();
  std::set<MimeType>& mimeTypes = m_NameToMimeTypes[name];
  mimeTypes.erase(mimeType);
  if (mimeTypes.empty())
  {
    m_NameToMimeTypes.erase(name);
    m_NameToMimeType.erase(name);
  }
  else
  {
    // get the highest ranked mime-type
    m_NameToMimeType[name] = *(mimeTypes.rbegin());
  }
}

MimeType MimeTypeProvider::GetMimeType(const ServiceReferenceType& reference) const
{
  MimeType result;
  if (!reference) return result;

  CustomMimeType* mimeType = us::GetModuleContext()->GetService(reference);
  if (mimeType != NULL)
  {
    try
    {
      int rank = 0;
      us::Any rankProp = reference.GetProperty(us::ServiceConstants::SERVICE_RANKING());
      if (!rankProp.Empty())
      {
        rank = us::any_cast<int>(rankProp);
      }
      long id = us::any_cast<long>(reference.GetProperty(us::ServiceConstants::SERVICE_ID()));
      result = MimeType(*mimeType, rank, id);
    }
    catch (const us::BadAnyCastException& e)
    {
      MITK_WARN << "Unexpected exception: " << e.what();
    }
    us::GetModuleContext()->UngetService(reference);
  }
  return result;
}

}
