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


#include "mitkIndexedValueFunctorPolicy.h"

mitk::IndexedValueFunctorPolicy::IndexedValueFunctorPolicy()
{};

mitk::IndexedValueFunctorPolicy::~IndexedValueFunctorPolicy()
{};

unsigned int mitk::IndexedValueFunctorPolicy::GetNumberOfOutputs() const
{
  unsigned int result = 0;

  if (m_Functor.IsNotNull())
  {
    result = m_Functor->GetNumberOfOutputs();
  }

  return result;
}

void mitk::IndexedValueFunctorPolicy::SetFunctor(const FunctorType *functor)
{
  if (!functor)
  {
    itkGenericExceptionMacro(<< "Error. Functor is Null.");
  }

  m_Functor = functor;
}


bool mitk::IndexedValueFunctorPolicy::operator!=(const IndexedValueFunctorPolicy & other) const
{
  return !(*this == other);
}

bool mitk::IndexedValueFunctorPolicy::operator==(const IndexedValueFunctorPolicy & other) const
{
  return (this->m_Functor == other.m_Functor);
}

mitk::IndexedValueFunctorPolicy::OutputPixelVectorType mitk::IndexedValueFunctorPolicy::operator()(const InputPixelVectorType & value, const IndexType& currentIndex) const
{
  if (!m_Functor)
  {
    itkGenericExceptionMacro(<< "Error. Cannot process operator(). Functor is Null.");
  }

  OutputPixelVectorType result = m_Functor->Compute(value, currentIndex);

  return result;
}
