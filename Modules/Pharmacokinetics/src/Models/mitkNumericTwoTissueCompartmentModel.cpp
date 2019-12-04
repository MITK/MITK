/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkNumericTwoTissueCompartmentModel.h"
#include "mitkAIFParametrizerHelper.h"
#include "mitkTimeGridHelper.h"
#include "mitkTwoTissueCompartmentModelDifferentialEquations.h"
#include <vnl/algo/vnl_fft_1d.h>
#include <boost/numeric/odeint.hpp>
#include <fstream>

const std::string mitk::NumericTwoTissueCompartmentModel::MODEL_DISPLAY_NAME =
  "Numeric Two Tissue Compartment Model";

const std::string mitk::NumericTwoTissueCompartmentModel::NAME_PARAMETER_K1 = "K1";
const std::string mitk::NumericTwoTissueCompartmentModel::NAME_PARAMETER_k2 = "k2";
const std::string mitk::NumericTwoTissueCompartmentModel::NAME_PARAMETER_k3 = "k3";
const std::string mitk::NumericTwoTissueCompartmentModel::NAME_PARAMETER_k4 = "k4";
const std::string mitk::NumericTwoTissueCompartmentModel::NAME_PARAMETER_VB = "V_B";

const std::string mitk::NumericTwoTissueCompartmentModel::UNIT_PARAMETER_K1 = "1/min";
const std::string mitk::NumericTwoTissueCompartmentModel::UNIT_PARAMETER_k2 = "1/min";
const std::string mitk::NumericTwoTissueCompartmentModel::UNIT_PARAMETER_k3 = "1/min";
const std::string mitk::NumericTwoTissueCompartmentModel::UNIT_PARAMETER_k4 = "1/min";
const std::string mitk::NumericTwoTissueCompartmentModel::UNIT_PARAMETER_VB = "ml/ml";

const unsigned int mitk::NumericTwoTissueCompartmentModel::POSITION_PARAMETER_K1 = 0;
const unsigned int mitk::NumericTwoTissueCompartmentModel::POSITION_PARAMETER_k2 = 1;
const unsigned int mitk::NumericTwoTissueCompartmentModel::POSITION_PARAMETER_k3 = 2;
const unsigned int mitk::NumericTwoTissueCompartmentModel::POSITION_PARAMETER_k4 = 3;
const unsigned int mitk::NumericTwoTissueCompartmentModel::POSITION_PARAMETER_VB = 4;

const unsigned int mitk::NumericTwoTissueCompartmentModel::NUMBER_OF_PARAMETERS = 5;


std::string mitk::NumericTwoTissueCompartmentModel::GetModelDisplayName() const
{
  return MODEL_DISPLAY_NAME;
};

std::string mitk::NumericTwoTissueCompartmentModel::GetModelType() const
{
  return "Dynamic.PET";
};

mitk::NumericTwoTissueCompartmentModel::NumericTwoTissueCompartmentModel()
{

}

mitk::NumericTwoTissueCompartmentModel::~NumericTwoTissueCompartmentModel()
{

}

mitk::NumericTwoTissueCompartmentModel::ParameterNamesType
mitk::NumericTwoTissueCompartmentModel::GetParameterNames() const
{
  ParameterNamesType result;

  result.push_back(NAME_PARAMETER_K1);
  result.push_back(NAME_PARAMETER_k2);
  result.push_back(NAME_PARAMETER_k3);
  result.push_back(NAME_PARAMETER_k4);
  result.push_back(NAME_PARAMETER_VB);

  return result;
}

mitk::NumericTwoTissueCompartmentModel::ParametersSizeType
mitk::NumericTwoTissueCompartmentModel::GetNumberOfParameters() const
{
  return NUMBER_OF_PARAMETERS;
}

mitk::NumericTwoTissueCompartmentModel::ParamterUnitMapType
mitk::NumericTwoTissueCompartmentModel::GetParameterUnits() const
{
  ParamterUnitMapType result;

  result.insert(std::make_pair(NAME_PARAMETER_K1, UNIT_PARAMETER_K1));
  result.insert(std::make_pair(NAME_PARAMETER_k2, UNIT_PARAMETER_k2));
  result.insert(std::make_pair(NAME_PARAMETER_k3, UNIT_PARAMETER_k3));
  result.insert(std::make_pair(NAME_PARAMETER_k4, UNIT_PARAMETER_k4));
  result.insert(std::make_pair(NAME_PARAMETER_VB, UNIT_PARAMETER_VB));

  return result;
};


mitk::NumericTwoTissueCompartmentModel::ModelResultType
mitk::NumericTwoTissueCompartmentModel::ComputeModelfunction(const ParametersType& parameters) const
{
  typedef itk::Array<double> ConcentrationCurveType;
  typedef std::vector<double> ConcentrationVectorType;

  if (this->m_TimeGrid.GetSize() == 0)
  {
    itkExceptionMacro("No Time Grid Set! Cannot Calculate Signal");
  }

  AterialInputFunctionType aterialInputFunction;
  aterialInputFunction = GetAterialInputFunction(this->m_TimeGrid);

  unsigned int timeSteps = this->m_TimeGrid.GetSize();

  /** @brief Boost::numeric::odeint works with type std::vector<double> thus, aif and grid are converted to ModelParameters( of type std::vector)
   */
  mitk::TwoTissueCompartmentModelDifferentialEquations::AIFType aif = mitk::convertArrayToParameter(
        aterialInputFunction);
  mitk::TwoTissueCompartmentModelDifferentialEquations::AIFType grid = mitk::convertArrayToParameter(
        m_TimeGrid);

  mitk::TwoTissueCompartmentModelDifferentialEquations::AIFType aifODE = aif;
  aifODE.push_back(aif[timeSteps - 1]);
  mitk::TwoTissueCompartmentModelDifferentialEquations::AIFType gridODE = grid;
  gridODE.push_back(grid[timeSteps - 1] + (grid[timeSteps - 1] - grid[timeSteps - 2]));

  //Model Parameters
  double K1 = (double)parameters[POSITION_PARAMETER_K1] / 60.0;
  double k2 = (double)parameters[POSITION_PARAMETER_k2] / 60.0;
  double k3 = (double)parameters[POSITION_PARAMETER_k3] / 60.0;
  double k4 = (double)parameters[POSITION_PARAMETER_k4] / 60.0;
  double VB = parameters[POSITION_PARAMETER_VB];


  /** @brief Initialize class TwpTissueCompartmentModelDifferentialEquations defining the differential equations. AIF and Grid must be set so that at step t the aterial Concentration Ca(t) can be interpolated from AIF*/
  mitk::TwoTissueCompartmentModelDifferentialEquations ode;
  ode.initialize(K1, k2, k3, k4);
  ode.setAIF(aifODE);
  ode.setAIFTimeGrid(gridODE);

  state_type x(2);
  x[0] = 0.0;
  x[1] = 0.0;
  typedef boost::numeric::odeint::runge_kutta_cash_karp54<state_type> error_stepper_type;
  //typedef boost::numeric::odeint::runge_kutta4< state_type > stepper_type;

  /** @brief Results of odeeint x[0] and x[1]*/
  ConcentrationVectorType C1;
  ConcentrationVectorType C2;
  ConcentrationVectorType odeTimeGrid;

  error_stepper_type stepper;
  /** @brief Stepsize. Should be adapted by stepper (runge_kutta_cash_karp54) */
  const double dt = 0.1;
  /** @brief perform Step t -> t+dt to calculate approximate value x(t+dt)*/

  double T = this->m_TimeGrid(timeSteps - 1) + (grid[timeSteps - 1] - grid[timeSteps - 2]);

  for (double t = 0.0; t < T; t += dt)
  {
    stepper.do_step(ode, x, t, dt);
    C1.push_back(x[0]);
    C2.push_back(x[1]);
    odeTimeGrid.push_back(t);

  }

  /** @brief transfom result of Differential equations back to itk::Array and interpolate to m_TimeGrid (they are calculated on a different grid defined by stepsize of odeint)*/
  ConcentrationCurveType ConcentrationCompartment1 = mitk::convertParameterToArray(C1);
  ConcentrationCurveType ConcentrationCompartment2 = mitk::convertParameterToArray(C2);
  ConcentrationCurveType rungeKuttaTimeGrid = mitk::convertParameterToArray(odeTimeGrid);

  mitk::ModelBase::ModelResultType C_1 = mitk::InterpolateSignalToNewTimeGrid(
      ConcentrationCompartment1, rungeKuttaTimeGrid, m_TimeGrid);
  mitk::ModelBase::ModelResultType C_2 = mitk::InterpolateSignalToNewTimeGrid(
      ConcentrationCompartment2, rungeKuttaTimeGrid, m_TimeGrid);


  //Signal that will be returned by ComputeModelFunction
  mitk::ModelBase::ModelResultType signal(timeSteps);
  signal.fill(0.0);
  mitk::ModelBase::ModelResultType::iterator signalPos = signal.begin();
  mitk::ModelBase::ModelResultType::const_iterator C1Pos = C_1.begin();
  mitk::ModelBase::ModelResultType::const_iterator C2Pos = C_2.begin();


  for (AterialInputFunctionType::const_iterator aifpos = aterialInputFunction.begin();
       aifpos != aterialInputFunction.end(); ++aifpos, ++C1Pos, ++C2Pos, ++signalPos)
  {
    *signalPos = VB * (*aifpos) + (1 - VB) * (*C1Pos + *C2Pos);
  }

  return signal;

}

itk::LightObject::Pointer mitk::NumericTwoTissueCompartmentModel::InternalClone() const
{
  NumericTwoTissueCompartmentModel::Pointer newClone = NumericTwoTissueCompartmentModel::New();

  newClone->SetTimeGrid(this->m_TimeGrid);

  return newClone.GetPointer();
}

void mitk::NumericTwoTissueCompartmentModel::PrintSelf(std::ostream& os, ::itk::Indent indent) const
{
  Superclass::PrintSelf(os, indent);


}




