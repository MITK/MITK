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

void MimeTypeProvider::Start()
{
  if (m_Tracker == NULL)
  {
    m_Tracker = new us::ServiceTracker<IMimeType, MimeTypeTrackerTypeTraits>(us::GetModuleContext(), this);
  }
  m_Tracker->Open();
}

void MimeTypeProvider::Stop()
{
  m_Tracker->Close();
}

std::vector<std::string> MimeTypeProvider::GetMimeTypes() const
{
  std::vector<std::string> result;
  for (MapType::const_iterator iter = m_MimeTypeToRefs.begin(),
       end = m_MimeTypeToRefs.end(); iter != end; ++iter)
  {
    result.push_back(iter->first);
  }
  return result;
}

std::vector<std::string> MimeTypeProvider::GetMimeTypesForFile(const std::string& filePath) const
{
  // For now, just use the file extension to look-up the registered mime-types.
  std::string extension = itksys::SystemTools::GetFilenameExtension(filePath);
  extension = extension.substr(1, extension.size()-1);
  return this->GetMimeTypesForExtension(extension);
}

std::vector<std::string> MimeTypeProvider::GetMimeTypesForExtension(const std::string& extension) const
{
  std::vector<std::string> result;
  std::vector<us::ServiceReference<IMimeType> > mimeTypeRefs;
  for (MapType::const_iterator iter = m_MimeTypeToRefs.begin(),
       end = m_MimeTypeToRefs.end(); iter != end; ++iter)
  {
    us::Any any = iter->second.rbegin()->GetProperty(IMimeType::PROP_EXTENSIONS());
    if (!any.Empty() && any.Type() == typeid(std::vector<std::string>))
    {
      const std::vector<std::string>& extensions = us::ref_any_cast<std::vector<std::string> >(any);
      if (std::find(extensions.begin(), extensions.end(), extension) != extensions.end())
      {
        mimeTypeRefs.push_back(*(iter->second.rbegin()));
      }
    }
  }
  std::sort(mimeTypeRefs.begin(), mimeTypeRefs.end());
  for (std::vector<us::ServiceReference<IMimeType> >::reverse_iterator iter = mimeTypeRefs.rbegin();
       iter != mimeTypeRefs.rend(); ++iter)
  {
    result.push_back(us::ref_any_cast<std::string>(iter->GetProperty(IMimeType::PROP_ID())));
  }
  return result;
}

std::vector<std::string> MimeTypeProvider::GetMimeTypesForCategory(const std::string& category) const
{
  std::vector<std::string> result;
  for (MapType::const_iterator iter = m_MimeTypeToRefs.begin(),
       end = m_MimeTypeToRefs.end(); iter != end; ++iter)
  {
    us::Any cat = iter->second.rbegin()->GetProperty(IMimeType::PROP_CATEGORY());
    if (!cat.Empty() && cat.Type() == typeid(std::string) &&
        us::ref_any_cast<std::string>(cat) == category)
    {
      result.push_back(iter->first);
    }
  }
  return result;
}

std::string MimeTypeProvider::GetDescription(const std::string& mimeType) const
{
  MapType::const_iterator iter = m_MimeTypeToRefs.find(mimeType);
  if (iter == m_MimeTypeToRefs.end()) return std::string();

  us::Any description = iter->second.rbegin()->GetProperty(IMimeType::PROP_DESCRIPTION());
  if (!description.Empty() && description.Type() == typeid(std::string))
  {
    return us::ref_any_cast<std::string>(description);
  }
  return std::string();
}

std::vector<std::string> MimeTypeProvider::GetExtensions(const std::string& mimeType) const
{
  MapType::const_iterator iter = m_MimeTypeToRefs.find(mimeType);
  if (iter == m_MimeTypeToRefs.end()) return std::vector<std::string>();

  us::Any extensions = iter->second.rbegin()->GetProperty(IMimeType::PROP_EXTENSIONS());
  if (!extensions.Empty() && extensions.Type() == typeid(std::vector<std::string>))
  {
    return us::ref_any_cast<std::vector<std::string> >(extensions);
  }
  return std::vector<std::string>();
}

std::string MimeTypeProvider::GetCategory(const std::string& mimeType) const
{
  MapType::const_iterator iter = m_MimeTypeToRefs.find(mimeType);
  if (iter == m_MimeTypeToRefs.end()) return std::string();

  us::Any category = iter->second.rbegin()->GetProperty(IMimeType::PROP_CATEGORY());
  if (!category.Empty() && category.Type() == typeid(std::string))
  {
    return us::ref_any_cast<std::string>(category);
  }
  return std::string();
}

std::vector<std::string> MimeTypeProvider::GetCategories() const
{
  std::vector<std::string> result;
  for (MapType::const_iterator iter = m_MimeTypeToRefs.begin(),
       end = m_MimeTypeToRefs.end(); iter != end; ++iter)
  {
    us::Any category = iter->second.rbegin()->GetProperty(IMimeType::PROP_CATEGORY());
    if (!category.Empty() && category.Type() == typeid(std::string))
    {
      std::string s = us::ref_any_cast<std::string>(category);
      if (!s.empty())
      {
        result.push_back(s);
      }
    }
  }
  result.erase(std::unique(result.begin(), result.end()), result.end());
  return result;
}

us::ServiceReference<IMimeType> MimeTypeProvider::AddingService(const ServiceReferenceType& reference)
{
  us::Any id = reference.GetProperty(IMimeType::PROP_ID());
  if (!id.Empty() && id.Type() == typeid(std::string))
  {
    m_MimeTypeToRefs[us::ref_any_cast<std::string>(id)].insert(reference);
    return reference;
  }
  return ServiceReferenceType();
}

void MimeTypeProvider::ModifiedService(const ServiceReferenceType& /*reference*/, ServiceReferenceType /*service*/)
{
}

void MimeTypeProvider::RemovedService(const ServiceReferenceType& /*reference*/, ServiceReferenceType service)
{
  std::string id = us::ref_any_cast<std::string>(service.GetProperty(IMimeType::PROP_ID()));
  std::set<us::ServiceReferenceU>& refs = m_MimeTypeToRefs[id];
  refs.erase(service);
  if (refs.empty())
  {
    m_MimeTypeToRefs.erase(id);
  }
}

}
