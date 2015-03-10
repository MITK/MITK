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

#include <algorithm>

#include "mitkIsoDoseLevelCollections.h"
#include "mitkExceptionMacro.h"

namespace mitk
{
/** "Private" functor used in std::find_if to find IsoDoseLevels with a given reference value */
class EqualDoseFunctor
{
public:
  typedef IsoDoseLevel::DoseValueType DoseValueType;

  EqualDoseFunctor(const DoseValueType& refValue) : m_refValue(refValue)
  {};

  bool operator () (const IsoDoseLevel* level)
  {
    return level->GetDoseValue() == m_refValue;
  }

protected:
  DoseValueType m_refValue;
};

/** "Private" binary function used in std::sort to check the order of IsoDoseLeveles */
bool lesserIsoDoseLevel(const IsoDoseLevel* first, const IsoDoseLevel* second)
{
  return first->GetDoseValue() < second->GetDoseValue();
}
}

mitk::IsoDoseLevelSet::IsoDoseLevelSet(const IsoDoseLevelSet & other)
{
  if (&other != this)
  {
    this->m_IsoLevels = other.m_IsoLevels;
  }
};

const mitk::IsoDoseLevel& mitk::IsoDoseLevelSet::GetIsoDoseLevel(IsoLevelIndexType index) const
{
  if (index < this->m_IsoLevels.size())
  {
    return *(this->m_IsoLevels[index].GetPointer());
  }
  else
  {
    mitkThrow() << "Try to access non existing dose iso level.";
  }
};

const mitk::IsoDoseLevel& mitk::IsoDoseLevelSet::GetIsoDoseLevel(DoseValueType value) const
{
  InternalVectorType::const_iterator pos = std::find_if(this->m_IsoLevels.begin(), this->m_IsoLevels.end(), EqualDoseFunctor(value));

  if (pos != this->m_IsoLevels.end())
  {
    return *(pos->GetPointer());
  }
  else
  {
    mitkThrow() << "Try to access non existing dose iso level.";
  }
};

void mitk::IsoDoseLevelSet::SetIsoDoseLevel(const IsoDoseLevel* level)
{
  if (!level)
  {
    mitkThrow() << "Cannot set iso level. Passed null pointer.";
  }

  this->DeleteIsoDoseLevel(level->GetDoseValue());

  this->m_IsoLevels.push_back(level->Clone());

  std::sort(this->m_IsoLevels.begin(), this->m_IsoLevels.end(),lesserIsoDoseLevel);
};

bool mitk::IsoDoseLevelSet::DoseLevelExists(IsoLevelIndexType index) const
{
  return index < this->m_IsoLevels.size();
};


bool mitk::IsoDoseLevelSet::DoseLevelExists(DoseValueType value) const
{
  InternalVectorType::const_iterator pos = std::find_if(this->m_IsoLevels.begin(), this->m_IsoLevels.end(), EqualDoseFunctor(value));
  return pos != this->m_IsoLevels.end();
};

void mitk::IsoDoseLevelSet::DeleteIsoDoseLevel(DoseValueType value)
{
  InternalVectorType::iterator pos = std::find_if(this->m_IsoLevels.begin(), this->m_IsoLevels.end(), EqualDoseFunctor(value));

  if (pos != this->m_IsoLevels.end())
  {
    this->m_IsoLevels.erase(pos);
  }
};

void mitk::IsoDoseLevelSet::DeleteIsoDoseLevel(IsoLevelIndexType index)
{
  if (DoseLevelExists(index))
  {
    this->m_IsoLevels.erase(this->m_IsoLevels.begin()+index);
  }
};

mitk::IsoDoseLevelSet::ConstIterator mitk::IsoDoseLevelSet::Begin(void) const
{
  return ConstIterator(this->m_IsoLevels.begin());
};

mitk::IsoDoseLevelSet::ConstIterator mitk::IsoDoseLevelSet::End(void) const
{
  return ConstIterator(this->m_IsoLevels.end());
};

mitk::IsoDoseLevelSet::IsoLevelIndexType mitk::IsoDoseLevelSet::Size(void) const
{
  return this->m_IsoLevels.size();
};

void mitk::IsoDoseLevelSet::Reset(void)
{
  this->m_IsoLevels.clear();
};
