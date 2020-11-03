/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkDescriptionParameterImageGeneratorBase.h"

mitk::DescriptionParameterImageGeneratorBase::DescriptionParameterImageGeneratorBase() = default;
mitk::DescriptionParameterImageGeneratorBase::~DescriptionParameterImageGeneratorBase() = default;

bool
  mitk::DescriptionParameterImageGeneratorBase::HasOutdatedResult() const
{
  bool result = this->GetMTime() > this->m_GenerationTimeStamp;

  return result;
};

void
  mitk::DescriptionParameterImageGeneratorBase::CheckValidInputs() const
{
};


void
  mitk::DescriptionParameterImageGeneratorBase::Generate()
{
  CheckValidInputs();

  if (!this->HasOutdatedResult())
  { //still up to date -> nothing to do
    return;
  }

  ParameterImageMapType paramImages;

  DoParameterCalculationAndGetResults(paramImages);

  m_ParameterImageMap = paramImages;

  this->m_GenerationTimeStamp.Modified();
};

mitk::DescriptionParameterImageGeneratorBase::ParameterImageMapType
  mitk::DescriptionParameterImageGeneratorBase::GetParameterImages()
{
  if (this->HasOutdatedResult())
  {
    Generate();
  }


  return m_ParameterImageMap;
};

