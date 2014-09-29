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

struct CustomMimeType::Impl
{
  std::string m_Name;
  std::string m_Category;
  std::vector<std::string> m_Extensions;
  std::string m_Comment;
};

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
  : d(new Impl(*other.d.get()))
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
  if (std::find(d->m_Extensions.begin(), d->m_Extensions.end(), extension) ==
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
  Impl* d1 = d.release();
  Impl* d2 = r.d.release();
  d.reset(d2);
  r.d.reset(d1);
}

void swap(CustomMimeType& l, CustomMimeType& r)
{
  l.Swap(r);
}

}
