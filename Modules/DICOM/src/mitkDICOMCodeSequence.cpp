/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkDICOMCodeSequence.h"

namespace mitk
{
  DICOMCodeSequence::DICOMCodeSequence(const std::string& value, const std::string& scheme, const std::string& meaning)
    : m_Value(value), m_Scheme(scheme), m_Meaning(meaning)
  {
  }

  const std::string& DICOMCodeSequence::GetValue() const
  {
    return m_Value;
  }

  void DICOMCodeSequence::SetValue(const std::string& value)
  {
    m_Value = value;
  }

  const std::string& DICOMCodeSequence::GetScheme() const
  {
    return m_Scheme;
  }

  void DICOMCodeSequence::SetScheme(const std::string& scheme)
  {
    m_Scheme = scheme;
  }

  const std::string& DICOMCodeSequence::GetMeaning() const
  {
    return m_Meaning;
  }

  void DICOMCodeSequence::SetMeaning(const std::string& meaning)
  {
    m_Meaning = meaning;
  }

  bool DICOMCodeSequence::IsEmpty() const
  {
    return m_Value.empty() && m_Scheme.empty() && m_Meaning.empty();
  }

  bool DICOMCodeSequence::operator==(const DICOMCodeSequence& other) const
  {
    return m_Value == other.m_Value &&
           m_Scheme == other.m_Scheme &&
           m_Meaning == other.m_Meaning;
  }

  bool DICOMCodeSequence::operator!=(const DICOMCodeSequence& other) const
  {
    return !(*this == other);
  }

} // namespace mitk

