/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkSimpleFunctorPolicy.h"

mitk::SimpleFunctorPolicy::SimpleFunctorPolicy() = default;
mitk::SimpleFunctorPolicy::~SimpleFunctorPolicy() = default;

unsigned int
  mitk::SimpleFunctorPolicy::GetNumberOfOutputs() const
{
  unsigned int result = 0;

  if (m_Functor.IsNotNull())
  {
    result =  m_Functor->GetNumberOfOutputs();
  }

  return result;
}

void
  mitk::SimpleFunctorPolicy::SetFunctor (const mitk::SimpleFunctorBase *functor)
{
  if (!functor)
  {
    itkGenericExceptionMacro(<< "Error. Functor is Null.");
  }

  m_Functor = functor;
}


bool
  mitk::SimpleFunctorPolicy::operator!=( const SimpleFunctorPolicy & other) const
{
  return !(*this == other);
}

bool
  mitk::SimpleFunctorPolicy::operator==( const SimpleFunctorPolicy & other ) const
{
  return (this->m_Functor == other.m_Functor);
}
