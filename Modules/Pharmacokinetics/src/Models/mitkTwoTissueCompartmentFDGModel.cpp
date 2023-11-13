/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkTwoTissueCompartmentFDGModel.h"
#include "mitkConvolutionHelper.h"
#include <fstream>
const std::string mitk::TwoTissueCompartmentFDGModel::MODEL_DISPLAY_NAME = "Two Tissue Compartment Model for FDG (Sokoloff Model)";

const std::string mitk::TwoTissueCompartmentFDGModel::NAME_PARAMETER_K1 = "K_1";
const std::string mitk::TwoTissueCompartmentFDGModel::NAME_PARAMETER_k2 = "k_2";
const std::string mitk::TwoTissueCompartmentFDGModel::NAME_PARAMETER_k3 = "k_3";
const std::string mitk::TwoTissueCompartmentFDGModel::NAME_PARAMETER_vb = "v_b";

const std::string mitk::TwoTissueCompartmentFDGModel::UNIT_PARAMETER_K1 = "1/min";
const std::string mitk::TwoTissueCompartmentFDGModel::UNIT_PARAMETER_k2 = "1/min";
const std::string mitk::TwoTissueCompartmentFDGModel::UNIT_PARAMETER_k3 = "1/min";
const std::string mitk::TwoTissueCompartmentFDGModel::UNIT_PARAMETER_vb = "ml/ml";

const unsigned int mitk::TwoTissueCompartmentFDGModel::POSITION_PARAMETER_K1 = 0;
const unsigned int mitk::TwoTissueCompartmentFDGModel::POSITION_PARAMETER_k2 = 1;
const unsigned int mitk::TwoTissueCompartmentFDGModel::POSITION_PARAMETER_k3 = 2;
const unsigned int mitk::TwoTissueCompartmentFDGModel::POSITION_PARAMETER_vb = 3;

const unsigned int mitk::TwoTissueCompartmentFDGModel::NUMBER_OF_PARAMETERS = 4;

const std::string mitk::TwoTissueCompartmentFDGModel::MODEL_TYPE = "Dynamic.PET";

std::string mitk::TwoTissueCompartmentFDGModel::GetModelDisplayName() const
{
  return MODEL_DISPLAY_NAME;
};

std::string mitk::TwoTissueCompartmentFDGModel::GetModelType() const
{
  return MODEL_TYPE;
};

mitk::TwoTissueCompartmentFDGModel::TwoTissueCompartmentFDGModel()
{

}

mitk::TwoTissueCompartmentFDGModel::~TwoTissueCompartmentFDGModel()
{

}

mitk::TwoTissueCompartmentFDGModel::ParameterNamesType
mitk::TwoTissueCompartmentFDGModel::GetParameterNames() const
{
  ParameterNamesType result;

  result.push_back(NAME_PARAMETER_K1);
  result.push_back(NAME_PARAMETER_k2);
  result.push_back(NAME_PARAMETER_k3);
  result.push_back(NAME_PARAMETER_vb);

  return result;
}

mitk::TwoTissueCompartmentFDGModel::ParametersSizeType
mitk::TwoTissueCompartmentFDGModel::GetNumberOfParameters() const
{
  return NUMBER_OF_PARAMETERS;
}


mitk::TwoTissueCompartmentFDGModel::ParamterUnitMapType
mitk::TwoTissueCompartmentFDGModel::GetParameterUnits() const
{
  ParamterUnitMapType result;

  result.insert(std::make_pair(NAME_PARAMETER_K1, UNIT_PARAMETER_K1));
  result.insert(std::make_pair(NAME_PARAMETER_k2, UNIT_PARAMETER_k2));
  result.insert(std::make_pair(NAME_PARAMETER_k3, UNIT_PARAMETER_k3));
  result.insert(std::make_pair(NAME_PARAMETER_vb, UNIT_PARAMETER_vb));

  return result;
};

mitk::TwoTissueCompartmentFDGModel::ModelResultType
mitk::TwoTissueCompartmentFDGModel::ComputeModelfunction(const ParametersType& parameters) const
{
  if (this->m_TimeGrid.GetSize() == 0)
  {
    itkExceptionMacro("No Time Grid Set! Cannot Calculate Signal");
  }

  AterialInputFunctionType aterialInputFunction;
  aterialInputFunction = GetAterialInputFunction(this->m_TimeGrid);


  unsigned int timeSteps = this->m_TimeGrid.GetSize();

  //Model Parameters
  double K1 = (double)parameters[POSITION_PARAMETER_K1] / 60.0;
  double k2 = (double)parameters[POSITION_PARAMETER_k2] / 60.0;
  double k3 = (double)parameters[POSITION_PARAMETER_k3] / 60.0;
  double vb = parameters[POSITION_PARAMETER_vb];




  double lambda = k2+k3;

  mitk::ModelBase::ModelResultType exp = mitk::convoluteAIFWithExponential(this->m_TimeGrid,
                                          aterialInputFunction, lambda);
  mitk::ModelBase::ModelResultType CA = mitk::convoluteAIFWithConstant(this->m_TimeGrid, aterialInputFunction, k3);

  //Signal that will be returned by ComputeModelFunction
  mitk::ModelBase::ModelResultType signal(timeSteps);
  signal.fill(0.0);

  mitk::ModelBase::ModelResultType::const_iterator expPos = exp.begin();
  mitk::ModelBase::ModelResultType::const_iterator CAPos = CA.begin();
  AterialInputFunctionType::const_iterator aifPos = aterialInputFunction.begin();

  for (mitk::ModelBase::ModelResultType::iterator signalPos = signal.begin();
       signalPos != signal.end(); ++expPos, ++signalPos, ++aifPos)
  {
      double Ci = K1 * k2 /lambda *(*expPos) + K1*k3/lambda*(*CAPos);
      *signalPos = vb * (*aifPos) + (1 - vb) * Ci;
  }

  return signal;

}




itk::LightObject::Pointer mitk::TwoTissueCompartmentFDGModel::InternalClone() const
{
  TwoTissueCompartmentFDGModel::Pointer newClone = TwoTissueCompartmentFDGModel::New();

  newClone->SetTimeGrid(this->m_TimeGrid);

  return newClone.GetPointer();
}

void mitk::TwoTissueCompartmentFDGModel::PrintSelf(std::ostream& os, ::itk::Indent indent) const
{
  Superclass::PrintSelf(os, indent);


}



