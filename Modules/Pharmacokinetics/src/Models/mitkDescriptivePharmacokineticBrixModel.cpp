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

#include "mitkDescriptivePharmacokineticBrixModel.h"

const std::string mitk::DescriptivePharmacokineticBrixModel::MODEL_DISPLAY_NAME =
  "Descriptive Pharmacokinetic Brix Model";

const std::string mitk::DescriptivePharmacokineticBrixModel::NAME_PARAMETER_A = "A";
const std::string mitk::DescriptivePharmacokineticBrixModel::NAME_PARAMETER_kep = "kep";
const std::string mitk::DescriptivePharmacokineticBrixModel::NAME_PARAMETER_kel = "kel";
//tlag in minutes
const std::string mitk::DescriptivePharmacokineticBrixModel::NAME_PARAMETER_tlag = "tlag";

const std::string mitk::DescriptivePharmacokineticBrixModel::UNIT_PARAMETER_A = "a.u.";
const std::string mitk::DescriptivePharmacokineticBrixModel::UNIT_PARAMETER_kep = "1/min";
const std::string mitk::DescriptivePharmacokineticBrixModel::UNIT_PARAMETER_kel = "1/min";
//tlag in minutes
const std::string mitk::DescriptivePharmacokineticBrixModel::UNIT_PARAMETER_tlag = "min";

const std::string mitk::DescriptivePharmacokineticBrixModel::NAME_STATIC_PARAMETER_Tau = "Tau";
const std::string mitk::DescriptivePharmacokineticBrixModel::NAME_STATIC_PARAMETER_S0 = "S0";

const std::string mitk::DescriptivePharmacokineticBrixModel::UNIT_STATIC_PARAMETER_Tau = "min";
const std::string mitk::DescriptivePharmacokineticBrixModel::UNIT_STATIC_PARAMETER_S0 = "I";

const unsigned int mitk::DescriptivePharmacokineticBrixModel::POSITION_PARAMETER_A = 0;
const unsigned int mitk::DescriptivePharmacokineticBrixModel::POSITION_PARAMETER_kep = 1;
const unsigned int mitk::DescriptivePharmacokineticBrixModel::POSITION_PARAMETER_kel = 2;
const unsigned int mitk::DescriptivePharmacokineticBrixModel::POSITION_PARAMETER_tlag = 3;

const unsigned int mitk::DescriptivePharmacokineticBrixModel::NUMBER_OF_PARAMETERS = 4;

std::string mitk::DescriptivePharmacokineticBrixModel::GetModelDisplayName() const
{
  return MODEL_DISPLAY_NAME;
};

std::string mitk::DescriptivePharmacokineticBrixModel::GetModelType() const
{
  return "Perfusion.MR";
};

std::string mitk::DescriptivePharmacokineticBrixModel::GetXAxisName() const
{
  return "Time";
};

std::string mitk::DescriptivePharmacokineticBrixModel::GetXAxisUnit() const
{
  return "s";
}

std::string mitk::DescriptivePharmacokineticBrixModel::GetYAxisName() const
{
  return "";
};

std::string mitk::DescriptivePharmacokineticBrixModel::GetYAxisUnit() const
{
  return "";
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
  result.push_back(NAME_PARAMETER_tlag);

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
  result.insert(std::make_pair(NAME_PARAMETER_tlag, UNIT_PARAMETER_tlag));

  return result;
};

mitk::DescriptivePharmacokineticBrixModel::ParameterNamesType
mitk::DescriptivePharmacokineticBrixModel::GetStaticParameterNames() const
{
  ParameterNamesType result;

  result.push_back(NAME_STATIC_PARAMETER_Tau);
  result.push_back(NAME_STATIC_PARAMETER_S0);

  return result;
}

mitk::DescriptivePharmacokineticBrixModel::ParamterUnitMapType
mitk::DescriptivePharmacokineticBrixModel::GetStaticParameterUnits() const
{
  ParamterUnitMapType result;

  result.insert(std::make_pair(NAME_STATIC_PARAMETER_Tau, UNIT_STATIC_PARAMETER_Tau));
  result.insert(std::make_pair(NAME_STATIC_PARAMETER_S0, UNIT_STATIC_PARAMETER_S0));

  return result;
};

mitk::DescriptivePharmacokineticBrixModel::ParametersSizeType
mitk::DescriptivePharmacokineticBrixModel::GetNumberOfStaticParameters() const
{
  return 1;
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
  double      tlag = parameters[POSITION_PARAMETER_tlag];

  TimeGridType::const_iterator timeGridEnd = m_TimeGrid.end();
  ModelResultType::iterator signalPos = signal.begin();

  for (TimeGridType::const_iterator gridPos = m_TimeGrid.begin(); gridPos != timeGridEnd;
       ++gridPos, ++signalPos)
  {
    double t = (*gridPos) / 60.0; //convert from [sec] to [min]


    if (t <= tlag)
    {
      tx = 0;
    }
    else if ((t > tlag) && (t < (m_Tau + tlag)))
    {
      tx = t - tlag;
    }
    else if (t >= (m_Tau + tlag))
    {
      tx = m_Tau;
    }

    double kDiff  = kep - kel;
    double tDiff  = t - tlag;

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
  if (name == NAME_STATIC_PARAMETER_Tau)
  {
    SetTau(values[0]);
  }

  if (name == NAME_STATIC_PARAMETER_S0)
  {
    SetS0(values[0]);
  }
};

mitk::DescriptivePharmacokineticBrixModel::StaticParameterValuesType
mitk::DescriptivePharmacokineticBrixModel::GetStaticParameterValue(const ParameterNameType& name)
const
{
  StaticParameterValuesType result;

  if (name == NAME_STATIC_PARAMETER_Tau)
  {
    result.push_back(GetTau());
  }

  if (name == NAME_STATIC_PARAMETER_S0)
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
  os << indent << "S0 (base value): " << m_S0;
};
