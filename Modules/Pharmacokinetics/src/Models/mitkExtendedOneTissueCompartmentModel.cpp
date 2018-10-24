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

#include "mitkExtendedOneTissueCompartmentModel.h"
#include "mitkConvolutionHelper.h"
#include <vnl/algo/vnl_fft_1d.h>
#include <fstream>

const std::string mitk::ExtendedOneTissueCompartmentModel::MODEL_DISPLAY_NAME = "Extended One Tissue Compartment Model (with blood volume)";

const std::string mitk::ExtendedOneTissueCompartmentModel::NAME_PARAMETER_k1 = "K1";
const std::string mitk::ExtendedOneTissueCompartmentModel::NAME_PARAMETER_k2 = "k2";
const std::string mitk::ExtendedOneTissueCompartmentModel::NAME_PARAMETER_VB = "VB";

const std::string mitk::ExtendedOneTissueCompartmentModel::UNIT_PARAMETER_k1 = "1/min";
const std::string mitk::ExtendedOneTissueCompartmentModel::UNIT_PARAMETER_k2 = "1/min";
const std::string mitk::ExtendedOneTissueCompartmentModel::UNIT_PARAMETER_VB = "ml/ml";

const unsigned int mitk::ExtendedOneTissueCompartmentModel::POSITION_PARAMETER_k1 = 0;
const unsigned int mitk::ExtendedOneTissueCompartmentModel::POSITION_PARAMETER_k2 = 1;
const unsigned int mitk::ExtendedOneTissueCompartmentModel::POSITION_PARAMETER_VB = 2;

const unsigned int mitk::ExtendedOneTissueCompartmentModel::NUMBER_OF_PARAMETERS = 3;

std::string mitk::ExtendedOneTissueCompartmentModel::GetModelDisplayName() const
{
  return MODEL_DISPLAY_NAME;
};

std::string mitk::ExtendedOneTissueCompartmentModel::GetModelType() const
{
  return "Dynamic.PET";
};

mitk::ExtendedOneTissueCompartmentModel::ExtendedOneTissueCompartmentModel()
{

}

mitk::ExtendedOneTissueCompartmentModel::~ExtendedOneTissueCompartmentModel()
{

}

mitk::ExtendedOneTissueCompartmentModel::ParameterNamesType mitk::ExtendedOneTissueCompartmentModel::GetParameterNames() const
{
  ParameterNamesType result;

  result.push_back(NAME_PARAMETER_k1);
  result.push_back(NAME_PARAMETER_k2);
  result.push_back(NAME_PARAMETER_VB);

  return result;
}

mitk::ExtendedOneTissueCompartmentModel::ParametersSizeType  mitk::ExtendedOneTissueCompartmentModel::GetNumberOfParameters()
const
{
  return NUMBER_OF_PARAMETERS;
}

mitk::ExtendedOneTissueCompartmentModel::ParamterUnitMapType
mitk::ExtendedOneTissueCompartmentModel::GetParameterUnits() const
{
  ParamterUnitMapType result;

  result.insert(std::make_pair(NAME_PARAMETER_k1, UNIT_PARAMETER_k1));
  result.insert(std::make_pair(NAME_PARAMETER_k2, UNIT_PARAMETER_k2));
  result.insert(std::make_pair(NAME_PARAMETER_VB, UNIT_PARAMETER_VB));

  return result;
};

mitk::ExtendedOneTissueCompartmentModel::ModelResultType mitk::ExtendedOneTissueCompartmentModel::ComputeModelfunction(
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
  double     K1 = (double) parameters[POSITION_PARAMETER_k1] / 60.0;
  double     k2 = (double) parameters[POSITION_PARAMETER_k2] / 60.0;
  double     VB = parameters[POSITION_PARAMETER_VB];



  mitk::ModelBase::ModelResultType convolution = mitk::convoluteAIFWithExponential(this->m_TimeGrid,
      aterialInputFunction, k2);

  //Signal that will be returned by ComputeModelFunction
  mitk::ModelBase::ModelResultType signal(timeSteps);
  signal.fill(0.0);

  mitk::ModelBase::ModelResultType::iterator signalPos = signal.begin();

  AterialInputFunctionType::const_iterator aifPos = aterialInputFunction.begin();


  for (mitk::ModelBase::ModelResultType::const_iterator res = convolution.begin(); res != convolution.end(); ++res, ++signalPos, ++aifPos)
  {
    *signalPos = VB * (*aifPos) + (1 - VB) * K1 * (*res);
  }

  return signal;

}




itk::LightObject::Pointer mitk::ExtendedOneTissueCompartmentModel::InternalClone() const
{
  ExtendedOneTissueCompartmentModel::Pointer newClone = ExtendedOneTissueCompartmentModel::New();

  newClone->SetTimeGrid(this->m_TimeGrid);

  return newClone.GetPointer();
};

void mitk::ExtendedOneTissueCompartmentModel::PrintSelf(std::ostream& os, ::itk::Indent indent) const
{
  Superclass::PrintSelf(os, indent);


};

