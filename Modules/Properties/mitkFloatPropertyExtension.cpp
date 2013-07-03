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

#include "mitkFloatPropertyExtension.h"

namespace mitk
{
  struct FloatPropertyExtension::Impl
  {
    Impl();
    Impl(float minimum, float maximum, float singleStep, int decimals);
    ~Impl();

    int Decimals;
    float Maximum;
    float Minimum;
    float SingleStep;
  };

  FloatPropertyExtension::Impl::Impl()
    : Decimals(2),
      Maximum(99.9999999999f),
      Minimum(0.0f),
      SingleStep(1.0f)
  {
  }

  FloatPropertyExtension::Impl::Impl(float minimum, float maximum, float singleStep, int decimals)
    : Decimals(decimals),
      Maximum(maximum),
      Minimum(minimum),
      SingleStep(singleStep)
  {
  }

  FloatPropertyExtension::Impl::~Impl()
  {
  }
}

mitk::FloatPropertyExtension::FloatPropertyExtension()
  : m_Impl(new Impl)
{
}

mitk::FloatPropertyExtension::FloatPropertyExtension(float minimum, float maximum, float singleStep, int decimals)
  : m_Impl(new Impl(minimum, maximum, singleStep, decimals))
{
}

mitk::FloatPropertyExtension::~FloatPropertyExtension()
{
  delete m_Impl;
}

int mitk::FloatPropertyExtension::GetDecimals() const
{
  return m_Impl->Decimals;
}

void mitk::FloatPropertyExtension::SetDecimals(int decimals)
{
  m_Impl->Decimals = decimals;
}

float mitk::FloatPropertyExtension::GetMaximum() const
{
  return m_Impl->Maximum;
}

void mitk::FloatPropertyExtension::SetMaximum(float maximum)
{
  m_Impl->Maximum = maximum;
}

float mitk::FloatPropertyExtension::GetMinimum() const
{
  return m_Impl->Minimum;
}

void mitk::FloatPropertyExtension::SetMinimum(float minimum)
{
  m_Impl->Minimum = minimum;
}

float mitk::FloatPropertyExtension::GetSingleStep() const
{
  return m_Impl->SingleStep;
}

void mitk::FloatPropertyExtension::SetSingleStep(float singleStep)
{
  m_Impl->SingleStep = singleStep;
}
