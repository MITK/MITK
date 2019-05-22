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

#include "mitkExtendedToftsModel.h"
#include "mitkConvolutionHelper.h"
#include <vnl/algo/vnl_fft_1d.h>
#include <fstream>

const std::string mitk::ExtendedToftsModel::MODEL_DISPLAY_NAME = "Extended Tofts Model";

const std::string mitk::ExtendedToftsModel::NAME_PARAMETER_Ktrans = "KTrans";
const std::string mitk::ExtendedToftsModel::NAME_PARAMETER_ve = "ve";
const std::string mitk::ExtendedToftsModel::NAME_PARAMETER_vp = "vp";

const std::string mitk::ExtendedToftsModel::UNIT_PARAMETER_Ktrans = "ml/min/100ml";
const std::string mitk::ExtendedToftsModel::UNIT_PARAMETER_ve = "ml/ml";
const std::string mitk::ExtendedToftsModel::UNIT_PARAMETER_vp = "ml/ml";

const unsigned int mitk::ExtendedToftsModel::POSITION_PARAMETER_Ktrans = 0;
const unsigned int mitk::ExtendedToftsModel::POSITION_PARAMETER_ve = 1;
const unsigned int mitk::ExtendedToftsModel::POSITION_PARAMETER_vp = 2;

const unsigned int mitk::ExtendedToftsModel::NUMBER_OF_PARAMETERS = 3;

std::string mitk::ExtendedToftsModel::GetModelDisplayName() const
{
  return MODEL_DISPLAY_NAME;
};

std::string mitk::ExtendedToftsModel::GetModelType() const
{
  return "Perfusion.MR";
};

mitk::ExtendedToftsModel::ExtendedToftsModel()
{

}

mitk::ExtendedToftsModel::~ExtendedToftsModel()
{

}

mitk::ExtendedToftsModel::ParameterNamesType mitk::ExtendedToftsModel::GetParameterNames() const
{
  ParameterNamesType result;

  result.push_back(NAME_PARAMETER_Ktrans);
  result.push_back(NAME_PARAMETER_ve);
  result.push_back(NAME_PARAMETER_vp);

  return result;
}

mitk::ExtendedToftsModel::ParametersSizeType  mitk::ExtendedToftsModel::GetNumberOfParameters()
const
{
  return NUMBER_OF_PARAMETERS;
}

mitk::ExtendedToftsModel::ParamterUnitMapType
mitk::ExtendedToftsModel::GetParameterUnits() const
{
  ParamterUnitMapType result;

  result.insert(std::make_pair(NAME_PARAMETER_Ktrans, UNIT_PARAMETER_Ktrans));
  result.insert(std::make_pair(NAME_PARAMETER_vp, UNIT_PARAMETER_vp));
  result.insert(std::make_pair(NAME_PARAMETER_ve, UNIT_PARAMETER_ve));

  return result;
};



mitk::ExtendedToftsModel::ParameterNamesType
mitk::ExtendedToftsModel::GetDerivedParameterNames() const
{
  ParameterNamesType result;
  result.push_back("kep");
  return result;
};

mitk::ExtendedToftsModel::ParametersSizeType
mitk::ExtendedToftsModel::GetNumberOfDerivedParameters() const
{
  return 1;
};

mitk::ExtendedToftsModel::ParamterUnitMapType mitk::ExtendedToftsModel::GetDerivedParameterUnits() const
{
  ParamterUnitMapType result;

  result.insert(std::make_pair("kep", "1/min"));

  return result;
};

mitk::ExtendedToftsModel::ModelResultType mitk::ExtendedToftsModel::ComputeModelfunction(
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
  double     vp = parameters[POSITION_PARAMETER_vp];


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
    *signalPos = (*Cp) * vp + ktrans * (*res);
  }

  return signal;

}


mitk::ModelBase::DerivedParameterMapType mitk::ExtendedToftsModel::ComputeDerivedParameters(
  const mitk::ModelBase::ParametersType& parameters) const
{
  DerivedParameterMapType result;
  double kep = parameters[POSITION_PARAMETER_Ktrans] / parameters[POSITION_PARAMETER_ve];
  result.insert(std::make_pair("kep", kep));
  return result;
};

itk::LightObject::Pointer mitk::ExtendedToftsModel::InternalClone() const
{
  ExtendedToftsModel::Pointer newClone = ExtendedToftsModel::New();

  newClone->SetTimeGrid(this->m_TimeGrid);

  return newClone.GetPointer();
};

void mitk::ExtendedToftsModel::PrintSelf(std::ostream& os, ::itk::Indent indent) const
{
  Superclass::PrintSelf(os, indent);


};

