/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkModelDataGenerationFunctor.h"

mitk::SimpleFunctorBase::OutputPixelVectorType
mitk::ModelDataGenerationFunctor::Compute(const InputPixelVectorType &value) const {
  if (this->m_ModelParameterizer->GetDefaultTimeGrid().GetSize() == 0) {
    itkExceptionMacro("Error. Cannot compute SignalCurve. No time grid is set "
                      "in parameterizer!");
  }

  ModelParametersType parameters(value.size());

  for (ModelParametersType::SizeValueType i = 0; i < parameters.Size(); ++i) {
    parameters[i] = value[i];
  }

  ModelBase::Pointer model =
      this->m_ModelParameterizer->GenerateParameterizedModel();
  SignalType signal = model->GetSignal(parameters);

  OutputPixelVectorType result;

  for (SignalType::const_iterator pos = signal.begin(); pos != signal.end();
       ++pos) {
    result.push_back(*pos);
  }

  return result;
};

unsigned int mitk::ModelDataGenerationFunctor::GetNumberOfOutputs() const {
  if (m_ModelParameterizer.IsNotNull()) {
    return m_ModelParameterizer->GetDefaultTimeGrid().GetSize();
  }

  return 0;
};

mitk::ModelDataGenerationFunctor::GridArrayType
mitk::ModelDataGenerationFunctor::GetGrid() const {
  if (m_ModelParameterizer.IsNotNull()) {
    return m_ModelParameterizer->GetDefaultTimeGrid();
  }

  return GridArrayType();
};

mitk::ModelDataGenerationFunctor::ModelDataGenerationFunctor() = default;
mitk::ModelDataGenerationFunctor::~ModelDataGenerationFunctor() = default;
