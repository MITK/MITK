/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkNumericTwoCompartmentExchangeModel.h"
#include "mitkAIFParametrizerHelper.h"
#include "mitkTimeGridHelper.h"
#include "mitkTwoCompartmentExchangeModelDifferentialEquations.h"
#include <vnl/algo/vnl_fft_1d.h>
#include <boost/numeric/odeint.hpp>
#include <fstream>

const std::string mitk::NumericTwoCompartmentExchangeModel::MODEL_DISPLAY_NAME =
  "Numeric Two Compartment Exchange Model";

const std::string mitk::NumericTwoCompartmentExchangeModel::NAME_PARAMETER_F = "F";
const std::string mitk::NumericTwoCompartmentExchangeModel::NAME_PARAMETER_PS = "PS";
const std::string mitk::NumericTwoCompartmentExchangeModel::NAME_PARAMETER_ve = "ve";
const std::string mitk::NumericTwoCompartmentExchangeModel::NAME_PARAMETER_vp = "vp";

const std::string mitk::NumericTwoCompartmentExchangeModel::UNIT_PARAMETER_F = "ml/min/100ml";
const std::string mitk::NumericTwoCompartmentExchangeModel::UNIT_PARAMETER_PS = "ml/min/100ml";
const std::string mitk::NumericTwoCompartmentExchangeModel::UNIT_PARAMETER_ve = "ml/ml";
const std::string mitk::NumericTwoCompartmentExchangeModel::UNIT_PARAMETER_vp = "ml/ml";

const unsigned int mitk::NumericTwoCompartmentExchangeModel::POSITION_PARAMETER_F = 0;
const unsigned int mitk::NumericTwoCompartmentExchangeModel::POSITION_PARAMETER_PS = 1;
const unsigned int mitk::NumericTwoCompartmentExchangeModel::POSITION_PARAMETER_ve = 2;
const unsigned int mitk::NumericTwoCompartmentExchangeModel::POSITION_PARAMETER_vp = 3;

const unsigned int mitk::NumericTwoCompartmentExchangeModel::NUMBER_OF_PARAMETERS = 4;

const std::string mitk::NumericTwoCompartmentExchangeModel::NAME_STATIC_PARAMETER_ODEINTStepSize = "ODEIntStepSize";


std::string mitk::NumericTwoCompartmentExchangeModel::GetModelDisplayName() const
{
  return MODEL_DISPLAY_NAME;
};

std::string mitk::NumericTwoCompartmentExchangeModel::GetModelType() const
{
  return "Perfusion.MR";
};


mitk::NumericTwoCompartmentExchangeModel::NumericTwoCompartmentExchangeModel()
{

}

mitk::NumericTwoCompartmentExchangeModel::~NumericTwoCompartmentExchangeModel()
{

}

mitk::NumericTwoCompartmentExchangeModel::ParameterNamesType mitk::NumericTwoCompartmentExchangeModel::GetStaticParameterNames() const
{
  ParameterNamesType result;

  result.push_back(NAME_STATIC_PARAMETER_AIF);
  result.push_back(NAME_STATIC_PARAMETER_AIFTimeGrid);
  result.push_back(NAME_STATIC_PARAMETER_ODEINTStepSize);

  return result;
}

mitk::NumericTwoCompartmentExchangeModel::ParametersSizeType  mitk::NumericTwoCompartmentExchangeModel::GetNumberOfStaticParameters()
const
{
  return 3;
}


void mitk::NumericTwoCompartmentExchangeModel::SetStaticParameter(const ParameterNameType& name,
    const StaticParameterValuesType& values)
{
  if (name == NAME_STATIC_PARAMETER_AIF)
  {
    AterialInputFunctionType aif = mitk::convertParameterToArray(values);

    SetAterialInputFunctionValues(aif);
  }

  if (name == NAME_STATIC_PARAMETER_AIFTimeGrid)
  {
    TimeGridType timegrid = mitk::convertParameterToArray(values);

    SetAterialInputFunctionTimeGrid(timegrid);
  }

  if (name == NAME_STATIC_PARAMETER_ODEINTStepSize)
  {
      SetODEINTStepSize(values[0]);
  }
};

mitk::NumericTwoCompartmentExchangeModel::StaticParameterValuesType mitk::NumericTwoCompartmentExchangeModel::GetStaticParameterValue(
  const ParameterNameType& name) const
{
  StaticParameterValuesType result;

  if (name == NAME_STATIC_PARAMETER_AIF)
  {
    result = mitk::convertArrayToParameter(this->m_AterialInputFunctionValues);
  }

  if (name == NAME_STATIC_PARAMETER_AIFTimeGrid)
  {
    result = mitk::convertArrayToParameter(this->m_AterialInputFunctionTimeGrid);
  }
  if (name == NAME_STATIC_PARAMETER_ODEINTStepSize)
  {
    result.push_back(GetODEINTStepSize());
  }

  return result;
};


mitk::NumericTwoCompartmentExchangeModel::ParameterNamesType
mitk::NumericTwoCompartmentExchangeModel::GetParameterNames() const
{
  ParameterNamesType result;

  result.push_back(NAME_PARAMETER_F);
  result.push_back(NAME_PARAMETER_PS);
  result.push_back(NAME_PARAMETER_ve);
  result.push_back(NAME_PARAMETER_vp);

  return result;
}

mitk::NumericTwoCompartmentExchangeModel::ParametersSizeType
mitk::NumericTwoCompartmentExchangeModel::GetNumberOfParameters() const
{
  return NUMBER_OF_PARAMETERS;
}


mitk::NumericTwoCompartmentExchangeModel::ParamterUnitMapType
mitk::NumericTwoCompartmentExchangeModel::GetParameterUnits() const
{
  ParamterUnitMapType result;

  result.insert(std::make_pair(NAME_PARAMETER_F, UNIT_PARAMETER_F));
  result.insert(std::make_pair(NAME_PARAMETER_PS, UNIT_PARAMETER_PS));
  result.insert(std::make_pair(NAME_PARAMETER_vp, UNIT_PARAMETER_vp));
  result.insert(std::make_pair(NAME_PARAMETER_ve, UNIT_PARAMETER_ve));

  return result;
};

mitk::NumericTwoCompartmentExchangeModel::ModelResultType
mitk::NumericTwoCompartmentExchangeModel::ComputeModelfunction(const ParametersType& parameters)
const
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
  mitk::TwoCompartmentExchangeModelDifferentialEquations::AIFType aif = mitk::convertArrayToParameter(
        aterialInputFunction);
  mitk::TwoCompartmentExchangeModelDifferentialEquations::AIFType grid =
    mitk::convertArrayToParameter(m_TimeGrid);

  mitk::TwoCompartmentExchangeModelDifferentialEquations::AIFType aifODE = aif;
  aifODE.push_back(aif[timeSteps - 1]);
  mitk::TwoCompartmentExchangeModelDifferentialEquations::AIFType gridODE = grid;
  gridODE.push_back(grid[timeSteps - 1] + (grid[timeSteps - 1] - grid[timeSteps - 2]));



  //Model Parameters
  double F = (double) parameters[POSITION_PARAMETER_F] / 6000.0;
  double PS  = (double) parameters[POSITION_PARAMETER_PS] / 6000.0;
  double ve = (double) parameters[POSITION_PARAMETER_ve];
  double vp = (double) parameters[POSITION_PARAMETER_vp];


  /** @brief Initialize class TwoCompartmentExchangeModelDifferentialEquations defining the differential equations. AIF and Grid must be set so that at step t the aterial Concentration Ca(t) can be interpolated from AIF*/
  mitk::TwoCompartmentExchangeModelDifferentialEquations ode;
  ode.initialize(F, PS, ve, vp);
  ode.setAIF(aifODE);
  ode.setAIFTimeGrid(gridODE);

  state_type x(2);
  x[0] = 0.0;
  x[1] = 0.0;
  typedef boost::numeric::odeint::runge_kutta_cash_karp54<state_type> error_stepper_type;
  //typedef boost::numeric::odeint::runge_kutta4< state_type > stepper_type;

  /** @brief Results of odeeint x[0] and x[1]*/
  ConcentrationVectorType Cp;
  ConcentrationVectorType Ce;
  ConcentrationVectorType odeTimeGrid;

  error_stepper_type stepper;


  /** @brief Stepsize. Should be adapted by stepper (runge_kutta_cash_karp54) */
//  const double dt = 0.05;
   const double dt = this->m_ODEINTStepSize;


  /** @brief perform Step t -> t+dt to calculate approximate value x(t+dt)*/
  for (double t = 0.0; t < this->m_TimeGrid(this->m_TimeGrid.GetSize() - 1) - 2*dt; t += dt)
  {
    stepper.do_step(ode, x, t, dt);
    Cp.push_back(x[0]);
    Ce.push_back(x[1]);
    odeTimeGrid.push_back(t);
  }

  /** @brief transfom result of Differential equations back to itk::Array and interpolate to m_TimeGrid (they are calculated on a different grid defined by stepsize of odeint)*/
  ConcentrationCurveType plasmaConcentration = mitk::convertParameterToArray(Cp);
  ConcentrationCurveType EESConcentration = mitk::convertParameterToArray(Ce);
  ConcentrationCurveType rungeKuttaTimeGrid = mitk::convertParameterToArray(odeTimeGrid);

  mitk::ModelBase::ModelResultType C_Plasma = mitk::InterpolateSignalToNewTimeGrid(plasmaConcentration, rungeKuttaTimeGrid, m_TimeGrid);
  mitk::ModelBase::ModelResultType C_EES = mitk::InterpolateSignalToNewTimeGrid(EESConcentration, rungeKuttaTimeGrid, m_TimeGrid);


  //Signal that will be returned by ComputeModelFunction
  mitk::ModelBase::ModelResultType signal(timeSteps);
  signal.fill(0.0);

  mitk::ModelBase::ModelResultType::iterator signalPos = signal.begin();
  mitk::ModelBase::ModelResultType::const_iterator CePos = C_EES.begin();

  mitk::ModelBase::ModelResultType::const_iterator t = this->m_TimeGrid.begin();
  mitk::ModelBase::ModelResultType::const_iterator Cin = aterialInputFunction.begin();



  for (mitk::ModelBase::ModelResultType::const_iterator CpPos = C_Plasma.begin();
       CpPos != C_Plasma.end(); ++CpPos, ++CePos, ++signalPos, ++t, ++Cin)
  {
    *signalPos = vp * (*CpPos) + ve * (*CePos);

  }

  return signal;

}




itk::LightObject::Pointer mitk::NumericTwoCompartmentExchangeModel::InternalClone() const
{
  NumericTwoCompartmentExchangeModel::Pointer newClone = NumericTwoCompartmentExchangeModel::New();

  newClone->SetTimeGrid(this->m_TimeGrid);

  return newClone.GetPointer();
}

void mitk::NumericTwoCompartmentExchangeModel::PrintSelf(std::ostream& os,
    ::itk::Indent indent) const
{
  Superclass::PrintSelf(os, indent);


}


