/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkStandardToftsModel.h"
#include "mitkConvolutionHelper.h"
#include <vnl/algo/vnl_fft_1d.h>
#include <fstream>

const std::string mitk::StandardToftsModel::MODEL_DISPLAY_NAME = "Standard Tofts Model";

const std::string mitk::StandardToftsModel::NAME_PARAMETER_Ktrans = "K^trans";
const std::string mitk::StandardToftsModel::NAME_PARAMETER_ve = "v_e";

const std::string mitk::StandardToftsModel::UNIT_PARAMETER_Ktrans = "ml/min/100ml";
const std::string mitk::StandardToftsModel::UNIT_PARAMETER_ve = "ml/ml";

const unsigned int mitk::StandardToftsModel::POSITION_PARAMETER_Ktrans = 0;
const unsigned int mitk::StandardToftsModel::POSITION_PARAMETER_ve = 1;

const unsigned int mitk::StandardToftsModel::NUMBER_OF_PARAMETERS = 2;

const std::string mitk::StandardToftsModel::NAME_DERIVED_PARAMETER_kep = "k_{e->p}";

const unsigned int mitk::StandardToftsModel::NUMBER_OF_DERIVED_PARAMETERS = 1;

const std::string mitk::StandardToftsModel::UNIT_DERIVED_PARAMETER_kep = "1/min";

const std::string mitk::StandardToftsModel::MODEL_TYPE = "Perfusion.MR";



std::string mitk::StandardToftsModel::GetModelDisplayName() const
{
  return MODEL_DISPLAY_NAME;
};

std::string mitk::StandardToftsModel::GetModelType() const
{
  return MODEL_TYPE;
};

mitk::StandardToftsModel::StandardToftsModel()
{

}

mitk::StandardToftsModel::~StandardToftsModel()
{

}

mitk::StandardToftsModel::ParameterNamesType mitk::StandardToftsModel::GetParameterNames() const
{
  ParameterNamesType result;

  result.push_back(NAME_PARAMETER_Ktrans);
  result.push_back(NAME_PARAMETER_ve);

  return result;
}

mitk::StandardToftsModel::ParametersSizeType  mitk::StandardToftsModel::GetNumberOfParameters()
const
{
  return NUMBER_OF_PARAMETERS;
}

mitk::StandardToftsModel::ParamterUnitMapType mitk::StandardToftsModel::GetParameterUnits() const
{
  ParamterUnitMapType result;

  result.insert(std::make_pair(NAME_PARAMETER_Ktrans, UNIT_PARAMETER_Ktrans));
  result.insert(std::make_pair(NAME_PARAMETER_ve, UNIT_PARAMETER_ve));

  return result;
};

mitk::StandardToftsModel::ParameterNamesType
mitk::StandardToftsModel::GetDerivedParameterNames() const
{
  ParameterNamesType result;
  result.push_back(NAME_DERIVED_PARAMETER_kep);
  return result;
};

mitk::StandardToftsModel::ParametersSizeType
mitk::StandardToftsModel::GetNumberOfDerivedParameters() const
{
  return NUMBER_OF_DERIVED_PARAMETERS;
};

mitk::StandardToftsModel::ParamterUnitMapType mitk::StandardToftsModel::GetDerivedParameterUnits() const
{
  ParamterUnitMapType result;

  result.insert(std::make_pair(NAME_DERIVED_PARAMETER_kep, UNIT_DERIVED_PARAMETER_kep));

  return result;
};

mitk::StandardToftsModel::ModelResultType mitk::StandardToftsModel::ComputeModelfunction(
  const ParametersType& parameters) const
{
  if (this->m_TimeGrid.GetSize() == 0)
  {
    itkExceptionMacro("No Time Grid Set! Cannot Calculate Signal");
  }

  AterialInputFunctionType aterialInputFunction;
  aterialInputFunction = GetAterialInputFunction(this->m_TimeGrid);



  unsigned int timeSteps = this->m_TimeGrid.GetSize();

  //Model Parameters
  double ktrans = parameters[POSITION_PARAMETER_Ktrans] / 6000.0;
  double     ve = parameters[POSITION_PARAMETER_ve];


  double lambda =  ktrans / ve;

  mitk::ModelBase::ModelResultType convolution = mitk::convoluteAIFWithExponential(this->m_TimeGrid,
      aterialInputFunction, lambda);

  //Signal that will be returned by ComputeModelFunction
  mitk::ModelBase::ModelResultType signal(timeSteps);
  signal.fill(0.0);

  mitk::ModelBase::ModelResultType::iterator signalPos = signal.begin();
  mitk::ModelBase::ModelResultType::const_iterator res = convolution.begin();


  for (AterialInputFunctionType::iterator Cp = aterialInputFunction.begin();
       Cp != aterialInputFunction.end(); ++res, ++signalPos, ++Cp)
  {
    *signalPos = ktrans * (*res);
  }

  return signal;

}


mitk::ModelBase::DerivedParameterMapType mitk::StandardToftsModel::ComputeDerivedParameters(
  const mitk::ModelBase::ParametersType& parameters) const
{
  DerivedParameterMapType result;
  double kep = parameters[POSITION_PARAMETER_Ktrans] / parameters[POSITION_PARAMETER_ve];
  result.insert(std::make_pair(NAME_DERIVED_PARAMETER_kep, kep));
  return result;
};

itk::LightObject::Pointer mitk::StandardToftsModel::InternalClone() const
{
  StandardToftsModel::Pointer newClone = StandardToftsModel::New();

  newClone->SetTimeGrid(this->m_TimeGrid);

  return newClone.GetPointer();
};

void mitk::StandardToftsModel::PrintSelf(std::ostream& os, ::itk::Indent indent) const
{
  Superclass::PrintSelf(os, indent);


};

