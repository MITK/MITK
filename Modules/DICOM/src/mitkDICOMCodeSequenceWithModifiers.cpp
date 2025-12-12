/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkDICOMCodeSequenceWithModifiers.h"

#include "mitkExceptionMacro.h"

namespace mitk
{
  DICOMCodeSequenceWithModifiers::DICOMCodeSequenceWithModifiers(const std::string& value,
                                                   const std::string& scheme,
                                                   const std::string& meaning)
    : DICOMCodeSequence(value, scheme, meaning)
  {
  }

  DICOMCodeSequenceWithModifiers::DICOMCodeSequenceWithModifiers(const DICOMCodeSequence& code)
    : DICOMCodeSequence(code)
  {
  }

  DICOMCodeSequenceWithModifiers& DICOMCodeSequenceWithModifiers::operator = (const DICOMCodeSequence& code)
  {
    this->m_Value = code.GetValue();
    this->m_Scheme = code.GetScheme();
    this->m_Meaning = code.GetMeaning();
    this->m_Modifiers.clear();
    return *this;
  }

  void DICOMCodeSequenceWithModifiers::AddModifier(const DICOMCodeSequence& modifier)
  {
    m_Modifiers.push_back(modifier);
  }

  const DICOMCodeSequenceWithModifiers::ModifierVector& DICOMCodeSequenceWithModifiers::GetModifiers() const
  {
    return m_Modifiers;
  }

  const DICOMCodeSequence& DICOMCodeSequenceWithModifiers::GetModifier(std::size_t index) const
  {
    if (index >= m_Modifiers.size())
      mitkThrow() << "Called GetModifier() with invalid index. Index: " << index;
    return m_Modifiers[index];
  }

  DICOMCodeSequence& DICOMCodeSequenceWithModifiers::GetModifier(std::size_t index)
  {
    if (index >= m_Modifiers.size())
      mitkThrow() << "Called GetModifier() with invalid index. Index: " << index;
    return m_Modifiers[index];
  }

  void DICOMCodeSequenceWithModifiers::SetModifier(std::size_t index, const DICOMCodeSequence& modifier)
  {
    if (index >= m_Modifiers.size())
      mitkThrow() << "Called SetModifier() with invalid index. Index: " << index;
    m_Modifiers[index] = modifier;
  }

  void DICOMCodeSequenceWithModifiers::SetModifiers(const ModifierVector& modifiers)
  {
    m_Modifiers = modifiers;
  }

  void DICOMCodeSequenceWithModifiers::ClearModifiers()
  {
    m_Modifiers.clear();
  }

  std::size_t DICOMCodeSequenceWithModifiers::GetModifierCount() const
  {
    return m_Modifiers.size();
  }

  bool DICOMCodeSequenceWithModifiers::HasModifiers() const
  {
    return !m_Modifiers.empty();
  }

  bool DICOMCodeSequenceWithModifiers::operator==(const DICOMCodeSequenceWithModifiers& other) const
  {
    return DICOMCodeSequence::operator==(other) && m_Modifiers == other.m_Modifiers;
  }

  bool DICOMCodeSequenceWithModifiers::operator!=(const DICOMCodeSequenceWithModifiers& other) const
  {
    return !(*this == other);
  }

} // namespace mitk
