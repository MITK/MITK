/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkDescriptivePharmacokineticBrixModel.h"

const std::string mitk::DescriptivePharmacokineticBrixModel::MODEL_DISPLAY_NAME =
  "Descriptive Pharmacokinetic Brix Model";

const std::string mitk::DescriptivePharmacokineticBrixModel::MODEL_TYPE =
"Perfusion.MR";

const std::string mitk::DescriptivePharmacokineticBrixModel::NAME_PARAMETER_A = "A";
const std::string mitk::DescriptivePharmacokineticBrixModel::NAME_PARAMETER_kep = "k_{e->p}";
const std::string mitk::DescriptivePharmacokineticBrixModel::NAME_PARAMETER_kel = "k_{el}";
const std::string mitk::DescriptivePharmacokineticBrixModel::NAME_PARAMETER_BAT = "BAT";

const std::string mitk::DescriptivePharmacokineticBrixModel::UNIT_PARAMETER_A = "a.u.";
const std::string mitk::DescriptivePharmacokineticBrixModel::UNIT_PARAMETER_kep = "1/min";
const std::string mitk::DescriptivePharmacokineticBrixModel::UNIT_PARAMETER_kel = "1/min";
const std::string mitk::DescriptivePharmacokineticBrixModel::UNIT_PARAMETER_BAT = "min";

const std::string mitk::DescriptivePharmacokineticBrixModel::NAME_STATIC_PARAMETER_tau = "Injection_time";
const std::string mitk::DescriptivePharmacokineticBrixModel::NAME_STATIC_PARAMETER_s0 = "Pre_contrast_signal";

const std::string mitk::DescriptivePharmacokineticBrixModel::UNIT_STATIC_PARAMETER_tau = "min";
const std::string mitk::DescriptivePharmacokineticBrixModel::UNIT_STATIC_PARAMETER_s0 = "a.u.";

const unsigned int mitk::DescriptivePharmacokineticBrixModel::POSITION_PARAMETER_A = 0;
const unsigned int mitk::DescriptivePharmacokineticBrixModel::POSITION_PARAMETER_kep = 1;
const unsigned int mitk::DescriptivePharmacokineticBrixModel::POSITION_PARAMETER_kel = 2;
const unsigned int mitk::DescriptivePharmacokineticBrixModel::POSITION_PARAMETER_BAT = 3;

const unsigned int mitk::DescriptivePharmacokineticBrixModel::NUMBER_OF_PARAMETERS = 4;
const unsigned int mitk::DescriptivePharmacokineticBrixModel::NUMBER_OF_STATIC_PARAMETERS = 2;

const std::string mitk::DescriptivePharmacokineticBrixModel::X_AXIS_NAME = "Time";
const std::string mitk::DescriptivePharmacokineticBrixModel::X_AXIS_UNIT = "s";
const std::string mitk::DescriptivePharmacokineticBrixModel::Y_AXIS_NAME = "Signal";
const std::string mitk::DescriptivePharmacokineticBrixModel::Y_AXIS_UNIT = "a.u.";


std::string mitk::DescriptivePharmacokineticBrixModel::GetModelDisplayName() const
{
  return MODEL_DISPLAY_NAME;
};

std::string mitk::DescriptivePharmacokineticBrixModel::GetModelType() const
{
  return MODEL_TYPE;
};

std::string mitk::DescriptivePharmacokineticBrixModel::GetXAxisName() const
{
  return X_AXIS_NAME;
};

std::string mitk::DescriptivePharmacokineticBrixModel::GetXAxisUnit() const
{
  return X_AXIS_UNIT;
}

std::string mitk::DescriptivePharmacokineticBrixModel::GetYAxisName() const
{
  return Y_AXIS_NAME;
};

std::string mitk::DescriptivePharmacokineticBrixModel::GetYAxisUnit() const
{
  return Y_AXIS_UNIT;
}

mitk::DescriptivePharmacokineticBrixModel::DescriptivePharmacokineticBrixModel(): m_Tau(0), m_S0(1)
{

}

mitk::DescriptivePharmacokineticBrixModel::~DescriptivePharmacokineticBrixModel()
{

}

mitk::DescriptivePharmacokineticBrixModel::ParameterNamesType
mitk::DescriptivePharmacokineticBrixModel::GetParameterNames() const
{
  ParameterNamesType result;

  result.push_back(NAME_PARAMETER_A);
  result.push_back(NAME_PARAMETER_kep);
  result.push_back(NAME_PARAMETER_kel);
  result.push_back(NAME_PARAMETER_BAT);

  return result;
}

mitk::DescriptivePharmacokineticBrixModel::ParametersSizeType
mitk::DescriptivePharmacokineticBrixModel::GetNumberOfParameters() const
{
  return NUMBER_OF_PARAMETERS;
}

mitk::DescriptivePharmacokineticBrixModel::ParamterUnitMapType
mitk::DescriptivePharmacokineticBrixModel::GetParameterUnits() const
{
  ParamterUnitMapType result;

  result.insert(std::make_pair(NAME_PARAMETER_A, UNIT_PARAMETER_A));
  result.insert(std::make_pair(NAME_PARAMETER_kep, UNIT_PARAMETER_kep));
  result.insert(std::make_pair(NAME_PARAMETER_kel, UNIT_PARAMETER_kel));
  result.insert(std::make_pair(NAME_PARAMETER_BAT, UNIT_PARAMETER_BAT));

  return result;
};

mitk::DescriptivePharmacokineticBrixModel::ParameterNamesType
mitk::DescriptivePharmacokineticBrixModel::GetStaticParameterNames() const
{
  ParameterNamesType result;

  result.push_back(NAME_STATIC_PARAMETER_tau);
  result.push_back(NAME_STATIC_PARAMETER_s0);

  return result;
}

mitk::DescriptivePharmacokineticBrixModel::ParamterUnitMapType
mitk::DescriptivePharmacokineticBrixModel::GetStaticParameterUnits() const
{
  ParamterUnitMapType result;

  result.insert(std::make_pair(NAME_STATIC_PARAMETER_tau, UNIT_STATIC_PARAMETER_tau));
  result.insert(std::make_pair(NAME_STATIC_PARAMETER_s0, UNIT_STATIC_PARAMETER_s0));

  return result;
};

mitk::DescriptivePharmacokineticBrixModel::ParametersSizeType
mitk::DescriptivePharmacokineticBrixModel::GetNumberOfStaticParameters() const
{
  return NUMBER_OF_STATIC_PARAMETERS;
}


mitk::DescriptivePharmacokineticBrixModel::ModelResultType
mitk::DescriptivePharmacokineticBrixModel::ComputeModelfunction(const ParametersType& parameters)
const
{
  if (m_TimeGrid.GetSize() == 0)
  {
    itkExceptionMacro("No Time Grid Set! Cannot Calculate Signal");
  }

  if (m_Tau == 0)
  {
    itkExceptionMacro("Injection time is 0! Cannot Calculate Signal");
  }


  ModelResultType signal(m_TimeGrid.GetSize());

  double tx        = 0;
  double amplitude = parameters[POSITION_PARAMETER_A];
  double       kel = parameters[POSITION_PARAMETER_kel];
  double       kep = parameters[POSITION_PARAMETER_kep];
  double       bat = parameters[POSITION_PARAMETER_BAT];

  if (kep == kel)
  {
    itkExceptionMacro("(kep-kel) is 0! Cannot Calculate Signal");
  }

  TimeGridType::const_iterator timeGridEnd = m_TimeGrid.end();
  ModelResultType::iterator signalPos = signal.begin();

  for (TimeGridType::const_iterator gridPos = m_TimeGrid.begin(); gridPos != timeGridEnd;
       ++gridPos, ++signalPos)
  {
    double t = (*gridPos) / 60.0; //convert from [sec] to [min]


    if (t <= bat)
    {
      tx = 0;
    }
    else if ((t > bat) && (t < (m_Tau + bat)))
    {
      tx = t - bat;
    }
    else if (t >= (m_Tau + bat))
    {
      tx = m_Tau;
    }

    double kDiff  = kep - kel;
    double tDiff  = t - bat;

    double expkel   = (kep * exp(-kel * tDiff));
    double expkeltx =        exp(kel * tx);
    double expkep   =        exp(-kep * tDiff);
    double expkeptx =        exp(kep * tx);

    double value =  1 + (amplitude / m_Tau) * (((expkel / (kel * kDiff)) * (expkeltx - 1)) - ((
                      expkep / kDiff)  * (expkeptx - 1)));

    *signalPos = value * m_S0;
  }

  return signal;

}

void mitk::DescriptivePharmacokineticBrixModel::SetStaticParameter(const ParameterNameType& name,
    const StaticParameterValuesType& values)
{
  if (name == NAME_STATIC_PARAMETER_tau)
  {
    SetTau(values[0]);
  }

  if (name == NAME_STATIC_PARAMETER_s0)
  {
    SetS0(values[0]);
  }
};

mitk::DescriptivePharmacokineticBrixModel::StaticParameterValuesType
mitk::DescriptivePharmacokineticBrixModel::GetStaticParameterValue(const ParameterNameType& name)
const
{
  StaticParameterValuesType result;

  if (name == NAME_STATIC_PARAMETER_tau)
  {
    result.push_back(GetTau());
  }

  if (name == NAME_STATIC_PARAMETER_s0)
  {
    result.push_back(GetS0());
  }

  return result;
};

itk::LightObject::Pointer mitk::DescriptivePharmacokineticBrixModel::InternalClone() const
{
  DescriptivePharmacokineticBrixModel::Pointer newClone = DescriptivePharmacokineticBrixModel::New();

  newClone->SetTimeGrid(this->m_TimeGrid);
  newClone->SetTau(this->m_Tau);
  newClone->SetS0(this->m_S0);

  return newClone.GetPointer();
};

void mitk::DescriptivePharmacokineticBrixModel::PrintSelf(std::ostream& os,
    ::itk::Indent indent) const
{
  Superclass::PrintSelf(os, indent);

  os << indent << "Tau (injection time): " << m_Tau;
  os << indent << "S0 (baseline value): " << m_S0;
};
