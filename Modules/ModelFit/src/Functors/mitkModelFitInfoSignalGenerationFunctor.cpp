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

#include "mitkModelFitInfoSignalGenerationFunctor.h"


mitk::IndexedValueFunctorBase::OutputPixelVectorType
mitk::ModelFitInfoSignalGenerationFunctor::Compute(const InputPixelVectorType &, const IndexType& currentIndex) const
{
  return this->Compute(currentIndex);
};

mitk::IndexedValueFunctorBase::OutputPixelVectorType
mitk::ModelFitInfoSignalGenerationFunctor::Compute(const IndexType& currentIndex) const
    {
      if (this->m_ModelParameterizer->GetDefaultTimeGrid().GetSize() == 0)
      {
        itkExceptionMacro("Error. Cannot compute SignalCurve. No time grid is set in parameterizer!");
      }

      ModelBase::Pointer model = this->m_ModelParameterizer->GenerateParameterizedModel(currentIndex);

      auto params = this->CompileModelParameters(currentIndex, model);

      SignalType signal = model->GetSignal(params);

      OutputPixelVectorType result;

      for (SignalType::const_iterator pos = signal.begin(); pos!= signal.end(); ++pos)
      {
        result.push_back(*pos);
      }

      return result;
    };

mitk::ModelBase::ParametersType
mitk::ModelFitInfoSignalGenerationFunctor::CompileModelParameters(const IndexType& currentIndex, const ModelBase * model) const
{
  auto paramMap = mitk::ExtractParameterValueMapFromModelFit(m_FitInfo, currentIndex);
  return mitk::ConvertParameterMapToParameterVector(paramMap, model);
};

unsigned int
mitk::ModelFitInfoSignalGenerationFunctor::GetNumberOfOutputs() const
    {
      if (m_ModelParameterizer.IsNotNull())
      {
        return m_ModelParameterizer->GetDefaultTimeGrid().GetSize();
      }

      return 0;
    };

mitk::IndexedValueFunctorBase::GridArrayType
mitk::ModelFitInfoSignalGenerationFunctor::GetGrid() const
    {
      if (m_ModelParameterizer.IsNotNull())
      {
        return m_ModelParameterizer->GetDefaultTimeGrid();
      }

      return GridArrayType();
    };

mitk::ModelFitInfoSignalGenerationFunctor::ModelFitInfoSignalGenerationFunctor()
{};
mitk::ModelFitInfoSignalGenerationFunctor::~ModelFitInfoSignalGenerationFunctor()
{};
