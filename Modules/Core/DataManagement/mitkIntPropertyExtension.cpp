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

#include "mitkIntPropertyExtension.h"

namespace mitk
{
  struct IntPropertyExtension::Impl
  {
    Impl();
    Impl(int minimum, int maximum, int singleStep);
    ~Impl();

    int Maximum;
    int Minimum;
    int SingleStep;
  };

  IntPropertyExtension::Impl::Impl()
    : Maximum(100),
      Minimum(0),
      SingleStep(1)
  {
  }

  IntPropertyExtension::Impl::Impl(int minimum, int maximum, int singleStep)
    : Maximum(maximum),
      Minimum(minimum),
      SingleStep(singleStep)
  {
  }

  IntPropertyExtension::Impl::~Impl()
  {
  }
}

mitk::IntPropertyExtension::IntPropertyExtension()
  : m_Impl(new Impl)
{
}

mitk::IntPropertyExtension::IntPropertyExtension(int minimum, int maximum, int singleStep)
  : m_Impl(new Impl(minimum, maximum, singleStep))
{
}

mitk::IntPropertyExtension::~IntPropertyExtension()
{
  delete m_Impl;
}

int mitk::IntPropertyExtension::GetMaximum() const
{
  return m_Impl->Maximum;
}

void mitk::IntPropertyExtension::SetMaximum(int maximum)
{
  m_Impl->Maximum = maximum;
}

int mitk::IntPropertyExtension::GetMinimum() const
{
  return m_Impl->Minimum;
}

void mitk::IntPropertyExtension::SetMinimum(int minimum)
{
  m_Impl->Minimum = minimum;
}

int mitk::IntPropertyExtension::GetSingleStep() const
{
  return m_Impl->SingleStep;
}

void mitk::IntPropertyExtension::SetSingleStep(int singleStep)
{
  m_Impl->SingleStep = singleStep;
}
