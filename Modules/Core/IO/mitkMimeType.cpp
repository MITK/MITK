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

#include "mitkMimeType.h"

#include "mitkCustomMimeType.h"

#include <memory>

namespace mitk {

struct MimeType::Impl : us::SharedData
{
  Impl()
    : m_CustomMimeType(new CustomMimeType())
    , m_Rank(-1)
    , m_Id(-1)
  {}

  Impl(const CustomMimeType& x, int rank, long id)
    : m_CustomMimeType(x.Clone())
    , m_Rank(rank)
    , m_Id(id)
  {}

  std::auto_ptr<CustomMimeType> m_CustomMimeType;

  int m_Rank;
  long m_Id;
};

MimeType::MimeType()
  : m_Data(new Impl)
{
}

MimeType::MimeType(const MimeType& other)
  : m_Data(other.m_Data)
{
}

MimeType::MimeType(const CustomMimeType& x, int rank, long id)
  : m_Data(new Impl(x, rank, id))
{
}

MimeType::~MimeType()
{
}

MimeType& MimeType::operator=(const MimeType& other)
{
  m_Data = other.m_Data;
  return *this;
}

bool MimeType::operator==(const MimeType& other) const
{
  return this->GetName() == other.GetName();
}

bool MimeType::operator<(const MimeType& other) const
{
  if (m_Data->m_Rank != other.m_Data->m_Rank)
  {
    return m_Data->m_Rank < other.m_Data->m_Rank;
  }
  return other.m_Data->m_Id < m_Data->m_Id;
}

std::string MimeType::GetName() const
{
  return m_Data->m_CustomMimeType->GetName();
}

std::string MimeType::GetCategory() const
{
  return m_Data->m_CustomMimeType->GetCategory();
}

std::vector<std::string> MimeType::GetExtensions() const
{
  return m_Data->m_CustomMimeType->GetExtensions();
}

std::string MimeType::GetComment() const
{
  return m_Data->m_CustomMimeType->GetComment();
}

bool MimeType::AppliesTo(const std::string& path) const
{
  return m_Data->m_CustomMimeType->AppliesTo(path);
}

bool MimeType::IsValid() const
{
  return m_Data.Data() != NULL && m_Data->m_CustomMimeType.get() != NULL && !m_Data->m_CustomMimeType->GetName().empty();
}

void MimeType::Swap(MimeType& m)
{
  m_Data.Swap(m.m_Data);
}

void swap(MimeType& m1, MimeType& m2)
{
  m1.Swap(m2);
}

std::ostream& operator<<(std::ostream& os, const MimeType& mimeType)
{
  os << mimeType.GetName() << " (" << mimeType.GetCategory() << ", " << mimeType.GetComment()
        << ") ";
  std::vector<std::string> extensions = mimeType.GetExtensions();
  for (std::vector<std::string>::const_iterator iter = extensions.begin(),
       endIter = extensions.end(); iter != endIter; ++iter)
  {
    if (iter != extensions.begin()) os << ", ";
    os << *iter;
  }
  return os;
}

}
