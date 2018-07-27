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
