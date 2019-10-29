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

#include "mitkTwoTissueCompartmentModel.h"
#include "mitkConvolutionHelper.h"
#include <fstream>
const std::string mitk::TwoTissueCompartmentModel::MODEL_DISPLAY_NAME = "Two Tissue Compartment Model";

const std::string mitk::TwoTissueCompartmentModel::NAME_PARAMETER_K1 = "K1";
const std::string mitk::TwoTissueCompartmentModel::NAME_PARAMETER_k2 = "k2";
const std::string mitk::TwoTissueCompartmentModel::NAME_PARAMETER_k3 = "k3";
const std::string mitk::TwoTissueCompartmentModel::NAME_PARAMETER_k4 = "k4";
const std::string mitk::TwoTissueCompartmentModel::NAME_PARAMETER_VB = "V_B";

const std::string mitk::TwoTissueCompartmentModel::UNIT_PARAMETER_K1 = "1/min";
const std::string mitk::TwoTissueCompartmentModel::UNIT_PARAMETER_k2 = "1/min";
const std::string mitk::TwoTissueCompartmentModel::UNIT_PARAMETER_k3 = "1/min";
const std::string mitk::TwoTissueCompartmentModel::UNIT_PARAMETER_k4 = "1/min";
const std::string mitk::TwoTissueCompartmentModel::UNIT_PARAMETER_VB = "ml/ml";

const unsigned int mitk::TwoTissueCompartmentModel::POSITION_PARAMETER_K1 = 0;
const unsigned int mitk::TwoTissueCompartmentModel::POSITION_PARAMETER_k2 = 1;
const unsigned int mitk::TwoTissueCompartmentModel::POSITION_PARAMETER_k3 = 2;
const unsigned int mitk::TwoTissueCompartmentModel::POSITION_PARAMETER_k4 = 3;
const unsigned int mitk::TwoTissueCompartmentModel::POSITION_PARAMETER_VB = 4;

const unsigned int mitk::TwoTissueCompartmentModel::NUMBER_OF_PARAMETERS = 5;


inline double square(double a)
{
  return a * a;
}

std::string mitk::TwoTissueCompartmentModel::GetModelDisplayName() const
{
  return MODEL_DISPLAY_NAME;
};

std::string mitk::TwoTissueCompartmentModel::GetModelType() const
{
  return "Dynamic.PET";
};

mitk::TwoTissueCompartmentModel::TwoTissueCompartmentModel()
{

}

mitk::TwoTissueCompartmentModel::~TwoTissueCompartmentModel()
{

}

mitk::TwoTissueCompartmentModel::ParameterNamesType
mitk::TwoTissueCompartmentModel::GetParameterNames() const
{
  ParameterNamesType result;

  result.push_back(NAME_PARAMETER_K1);
  result.push_back(NAME_PARAMETER_k2);
  result.push_back(NAME_PARAMETER_k3);
  result.push_back(NAME_PARAMETER_k4);
  result.push_back(NAME_PARAMETER_VB);

  return result;
}

mitk::TwoTissueCompartmentModel::ParametersSizeType
mitk::TwoTissueCompartmentModel::GetNumberOfParameters() const
{
  return NUMBER_OF_PARAMETERS;
}

mitk::TwoTissueCompartmentModel::ParamterUnitMapType
mitk::TwoTissueCompartmentModel::GetParameterUnits() const
{
  ParamterUnitMapType result;

  result.insert(std::make_pair(NAME_PARAMETER_K1, UNIT_PARAMETER_K1));
  result.insert(std::make_pair(NAME_PARAMETER_k2, UNIT_PARAMETER_k2));
  result.insert(std::make_pair(NAME_PARAMETER_k3, UNIT_PARAMETER_k3));
  result.insert(std::make_pair(NAME_PARAMETER_k4, UNIT_PARAMETER_k4));
  result.insert(std::make_pair(NAME_PARAMETER_VB, UNIT_PARAMETER_VB));

  return result;
};

mitk::TwoTissueCompartmentModel::ModelResultType
mitk::TwoTissueCompartmentModel::ComputeModelfunction(const ParametersType& parameters) const
{
  if (this->m_TimeGrid.GetSize() == 0)
  {
    itkExceptionMacro("No Time Grid Set! Cannot Calculate Signal");
  }

  AterialInputFunctionType aterialInputFunction;
  aterialInputFunction = GetAterialInputFunction(this->m_TimeGrid);


  unsigned int timeSteps = this->m_TimeGrid.GetSize();

  //Model Parameters
  double k1 = (double)parameters[POSITION_PARAMETER_K1] / 60.0;
  double k2 = (double)parameters[POSITION_PARAMETER_k2] / 60.0;
  double k3 = (double)parameters[POSITION_PARAMETER_k3] / 60.0;
  double k4 = (double)parameters[POSITION_PARAMETER_k4] / 60.0;
  double VB = parameters[POSITION_PARAMETER_VB];



  double alpha1 = 0.5 * ((k2 + k3 + k4) - sqrt(square(k2 + k3 + k4) - 4 * k2 * k4));
  double alpha2 = 0.5 * ((k2 + k3 + k4) + sqrt(square(k2 + k3 + k4) - 4 * k2 * k4));

  //double lambda1 = -alpha1;
  //double lambda2 = -alpha2;
  mitk::ModelBase::ModelResultType exp1 = mitk::convoluteAIFWithExponential(this->m_TimeGrid,
                                          aterialInputFunction, alpha1);
  mitk::ModelBase::ModelResultType exp2 = mitk::convoluteAIFWithExponential(this->m_TimeGrid,
                                          aterialInputFunction, alpha2);


  //Signal that will be returned by ComputeModelFunction
  mitk::ModelBase::ModelResultType signal(timeSteps);
  signal.fill(0.0);

  mitk::ModelBase::ModelResultType::const_iterator exp1Pos = exp1.begin();
  mitk::ModelBase::ModelResultType::const_iterator exp2Pos = exp2.begin();
  AterialInputFunctionType::const_iterator aifPos = aterialInputFunction.begin();

  for (mitk::ModelBase::ModelResultType::iterator signalPos = signal.begin();
       signalPos != signal.end(); ++exp1Pos, ++exp2Pos, ++signalPos, ++aifPos)
  {
    double Ci = k1 / (alpha2 - alpha1) * ((k4 - alpha1 + k3) * (*exp1Pos) + (alpha2 - k4 - k3) *
                                          (*exp2Pos));
    *signalPos = VB * (*aifPos) + (1 - VB) * Ci;
  }

  return signal;

}




itk::LightObject::Pointer mitk::TwoTissueCompartmentModel::InternalClone() const
{
  TwoTissueCompartmentModel::Pointer newClone = TwoTissueCompartmentModel::New();

  newClone->SetTimeGrid(this->m_TimeGrid);

  return newClone.GetPointer();
}

void mitk::TwoTissueCompartmentModel::PrintSelf(std::ostream& os, ::itk::Indent indent) const
{
  Superclass::PrintSelf(os, indent);


}



