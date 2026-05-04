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


mitk::SUVFunctorPolicy::SUVFunctorPolicy() = default;

mitk::SUVFunctorPolicy::SUVFunctorPolicy(double injectedActivity, double scaleNumerator, double halfLife)
  : m_InjectedActivity(injectedActivity),
    m_ScaleNumerator(scaleNumerator),
    m_HalfLife(halfLife)
{}

mitk::SUVFunctorPolicy::~SUVFunctorPolicy() = default;

unsigned int mitk::SUVFunctorPolicy::GetNumberOfOutputs() const
{
  return 1;
}

void mitk::SUVFunctorPolicy::SetDecayTimeFunctor(const DecayTimeFunctionType& functor)
{
  m_Functor = functor;
}

void mitk::SUVFunctorPolicy::SetInjectedActivity(double a)
{
  m_InjectedActivity = a;
}

void mitk::SUVFunctorPolicy::SetScaleNumerator(double n)
{
  m_ScaleNumerator = n;
}

void mitk::SUVFunctorPolicy::SetHalfLife(double tau)
{
  m_HalfLife = tau;
}

bool mitk::SUVFunctorPolicy::IsConfigured() const
{
  auto isPositiveFinite = [](double x) { return std::isfinite(x) && x > 0.0; };
  return isPositiveFinite(m_InjectedActivity)
      && isPositiveFinite(m_ScaleNumerator)
      && isPositiveFinite(m_HalfLife)
      && static_cast<bool>(m_Functor);
}

bool mitk::SUVFunctorPolicy::operator!=(const SUVFunctorPolicy& other) const
{
  return !(*this == other);
}

bool mitk::SUVFunctorPolicy::operator==(const SUVFunctorPolicy& other) const
{
  return (this->m_InjectedActivity == other.m_InjectedActivity) &&
    (this->m_ScaleNumerator == other.m_ScaleNumerator) &&
    (this->m_HalfLife == other.m_HalfLife);
}

mitk::SUVFunctorPolicy::SUVPixelType
mitk::SUVFunctorPolicy::operator()(const SUVPixelType& value,
                                   const IndexType& currentIndex) const
{
  return value * computeSUVScaleFactor(m_InjectedActivity, m_ScaleNumerator, m_Functor(currentIndex), m_HalfLife);
}


mitk::SUVbwFunctorPolicy::SUVbwFunctorPolicy(double injectedActivity, double bodyweight, double halfLife)
  : SUVFunctorPolicy(injectedActivity, bodyweight * 1000.0, halfLife)
{}

void mitk::SUVbwFunctorPolicy::SetBodyWeight(double w)
{
  SetScaleNumerator(w * 1000.0);
}
