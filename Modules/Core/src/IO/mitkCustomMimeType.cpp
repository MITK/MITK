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

#include "mitkCustomMimeType.h"

#include "mitkMimeType.h"

#include <algorithm>

namespace  mitk {

class FindCaseInsensitive
{
public:
  FindCaseInsensitive(std::string value)
  {
    lcValue.resize(value.size());
    std::transform(value.begin(), value.end(), lcValue.begin(), ::tolower);
  }

  bool operator()(std::string elem)
  {
    std::transform(elem.begin(), elem.end(), elem.begin(), ::tolower);
    return elem == lcValue;
  }

private:
  std::string lcValue;
};

struct CustomMimeType::Impl
{
  std::string m_Name;
  std::string m_Category;
  std::vector<std::string> m_Extensions;
  std::string m_Comment;
};

CustomMimeType::~CustomMimeType()
{
  delete d;
}

CustomMimeType::CustomMimeType()
  : d(new Impl)
{
}

CustomMimeType::CustomMimeType(const std::string& name)
  : d(new Impl)
{
  d->m_Name = name;
}

CustomMimeType::CustomMimeType(const CustomMimeType& other)
  : d(new Impl(*other.d))
{
}

CustomMimeType::CustomMimeType(const MimeType& other)
  : d(new Impl)
{
  d->m_Name = other.GetName();
  d->m_Category = other.GetCategory();
  d->m_Extensions = other.GetExtensions();
  d->m_Comment = other.GetComment();
}

CustomMimeType& CustomMimeType::operator=(const CustomMimeType& other)
{
  CustomMimeType tmp(other);
  Swap(tmp);
  return *this;
}

CustomMimeType&CustomMimeType::operator=(const MimeType& other)
{
  CustomMimeType tmp(other);
  Swap(tmp);
  return *this;
}

std::string CustomMimeType::GetName() const
{
  return d->m_Name;
}

std::string CustomMimeType::GetCategory() const
{
  return d->m_Category;
}

std::vector<std::string> CustomMimeType::GetExtensions() const
{
  return d->m_Extensions;
}

std::string CustomMimeType::GetComment() const
{
  if (!d->m_Comment.empty()) return d->m_Comment;
  if (!d->m_Extensions.empty())
  {
    return d->m_Extensions.front() + " File";
  }
  return "Unknown";
}

bool CustomMimeType::AppliesTo(const std::string& path) const
{
  for (std::vector<std::string>::const_iterator iter = d->m_Extensions.begin(),
       iterEnd = d->m_Extensions.end(); iter != iterEnd; ++iter)
  {
    if (!iter->empty() && path.size() >= iter->size())
    {
      FindCaseInsensitive cmp(*iter);
      if (cmp(path.substr(path.size() - iter->size())))
      {
        return true;
      }
    }
  }
  return false;
}

void CustomMimeType::SetName(const std::string& name)
{
  d->m_Name = name;
}

void CustomMimeType::SetCategory(const std::string& category)
{
  d->m_Category = category;
}

void CustomMimeType::SetExtension(const std::string& extension)
{
  d->m_Extensions.clear();
  d->m_Extensions.push_back(extension);
}

void CustomMimeType::AddExtension(const std::string& extension)
{
  if (std::find_if(d->m_Extensions.begin(), d->m_Extensions.end(), FindCaseInsensitive(extension)) ==
      d->m_Extensions.end())
  {
    d->m_Extensions.push_back(extension);
  }
}

void CustomMimeType::SetComment(const std::string& comment)
{
  d->m_Comment = comment;
}

void CustomMimeType::Swap(CustomMimeType& r)
{
  Impl* d1 = d;
  d = r.d;
  r.d = d1;
}

CustomMimeType* CustomMimeType::Clone() const
{
  return new CustomMimeType(*this);
}

void swap(CustomMimeType& l, CustomMimeType& r)
{
  l.Swap(r);
}

}
