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

#include "mitkParameterFitImageGeneratorBase.h"

bool
  mitk::ParameterFitImageGeneratorBase::HasOutdatedResult() const
{
  bool result = this->GetMTime() > this->m_GenerationTimeStamp;

  return result;
};

void
  mitk::ParameterFitImageGeneratorBase::CheckValidInputs() const
{
};


void
  mitk::ParameterFitImageGeneratorBase::Generate()
{
  CheckValidInputs();

  if (!this->HasOutdatedResult())
  { //still up to date -> nothing to do
    return;
  }

  ParameterImageMapType paramImages;
  ParameterImageMapType derivedImages;
  ParameterImageMapType criterionImages;
  ParameterImageMapType evaluationImages;

  DoFitAndGetResults(paramImages, derivedImages, criterionImages, evaluationImages);

  m_ParameterImageMap = paramImages;
  m_DerivedParameterImageMap = derivedImages;
  m_CriterionImageMap = criterionImages;
  m_EvaluationParameterImageMap = evaluationImages;

  this->m_GenerationTimeStamp.Modified();
};

mitk::ParameterFitImageGeneratorBase::ParameterImageMapType
  mitk::ParameterFitImageGeneratorBase::GetParameterImages()
{
  if (this->HasOutdatedResult())
  {
    Generate();
  }

  return m_ParameterImageMap;
};

mitk::ParameterFitImageGeneratorBase::ParameterImageMapType
  mitk::ParameterFitImageGeneratorBase::GetDerivedParameterImages()
{
  if (this->HasOutdatedResult())
  {
    Generate();
  }

  return m_DerivedParameterImageMap;
};

mitk::ParameterFitImageGeneratorBase::ParameterImageMapType
  mitk::ParameterFitImageGeneratorBase::GetCriterionImages()
{
  if (this->HasOutdatedResult())
  {
    Generate();
  }

  return m_CriterionImageMap;
};

mitk::ParameterFitImageGeneratorBase::ParameterImageMapType
  mitk::ParameterFitImageGeneratorBase::GetEvaluationParameterImages()
{
  if (this->HasOutdatedResult())
  {
    Generate();
  }

  return m_EvaluationParameterImageMap;
};