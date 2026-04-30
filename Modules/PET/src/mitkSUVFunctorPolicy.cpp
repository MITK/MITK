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


#include <mitkSUVFunctorPolicy.h>

#include <cmath>

#include <mitkSUVCalculation.h>


mitk::SUVbwFunctorPolicy::SUVbwFunctorPolicy() = default;

mitk::SUVbwFunctorPolicy::SUVbwFunctorPolicy(double injectedActivity, double bodyweight, double halfLife)
  : m_InjectedActivity(injectedActivity),
    m_bodyweight(bodyweight),
    m_halfLife(halfLife)
{}

mitk::SUVbwFunctorPolicy::~SUVbwFunctorPolicy() = default;

unsigned int mitk::SUVbwFunctorPolicy::GetNumberOfOutputs() const
{
  return 1;
}

void mitk::SUVbwFunctorPolicy::SetDecayTimeFunctor(const DecayTimeFunctionType& functor)
{
  m_Functor = functor;
}

void mitk::SUVbwFunctorPolicy::SetInjectedActivity(double a)
{
  m_InjectedActivity = a;
}

void mitk::SUVbwFunctorPolicy::SetBodyWeight(double w)
{
  m_bodyweight = w;
}

void mitk::SUVbwFunctorPolicy::SetHalfLife(double tau)
{
  m_halfLife = tau;
}

bool mitk::SUVbwFunctorPolicy::IsConfigured() const
{
  return std::isfinite(m_InjectedActivity)
      && std::isfinite(m_bodyweight)
      && std::isfinite(m_halfLife)
      && static_cast<bool>(m_Functor);
}

bool mitk::SUVbwFunctorPolicy::operator!=(const SUVbwFunctorPolicy& other) const
{
  return !(*this == other);
}

bool mitk::SUVbwFunctorPolicy::operator==(const SUVbwFunctorPolicy& other) const
{
  return (this->m_InjectedActivity == other.m_InjectedActivity) &&
    (this->m_bodyweight == other.m_bodyweight) &&
    (this->m_halfLife == other.m_halfLife);
}

mitk::SUVbwFunctorPolicy::SUVPixelType
mitk::SUVbwFunctorPolicy::operator()(const SUVPixelType& value,
                                     const IndexType& currentIndex) const
{
  return value * computeSUVbwScaleFactor(m_InjectedActivity, m_bodyweight, m_Functor(currentIndex), m_halfLife);
}
