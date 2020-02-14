/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkGenericParamModel.h"
#include "mitkFormulaParser.h"

const std::string mitk::GenericParamModel::NAME_STATIC_PARAMETER_number = "number_of_parameters";

std::string mitk::GenericParamModel::GetModelDisplayName() const
{
  return "Generic Parameter Model";
};

std::string mitk::GenericParamModel::GetModelType() const
{
  return "Generic";
};

mitk::GenericParamModel::FunctionStringType mitk::GenericParamModel::GetFunctionString() const
{
  return m_FunctionString;
};

std::string mitk::GenericParamModel::GetXName() const
{
  return "x";
};

mitk::GenericParamModel::GenericParamModel(): m_FunctionString(""), m_NumberOfParameters(1)
{
};

mitk::GenericParamModel::ParameterNamesType
mitk::GenericParamModel::GetParameterNames() const
{
  ParameterNamesType result;
  result.push_back("a");

  if (m_NumberOfParameters > 1)
  {
    result.push_back("b");
  }
  if (m_NumberOfParameters > 2)
  {
    result.push_back("c");
  }
  if (m_NumberOfParameters > 3)
  {
    result.push_back("d");
  }
  if (m_NumberOfParameters > 4)
  {
    result.push_back("e");
  }
  if (m_NumberOfParameters > 5)
  {
    result.push_back("f");
  }
  if (m_NumberOfParameters > 6)
  {
    result.push_back("g");
  }
  if (m_NumberOfParameters > 7)
  {
    result.push_back("h");
  }
  if (m_NumberOfParameters > 8)
  {
    result.push_back("i");
  }
  if (m_NumberOfParameters > 9)
  {
    result.push_back("j");
  }

  return result;
};

mitk::GenericParamModel::ParametersSizeType
mitk::GenericParamModel::GetNumberOfParameters() const
{
  return m_NumberOfParameters;
};

mitk::GenericParamModel::ModelResultType
mitk::GenericParamModel::ComputeModelfunction(const ParametersType& parameters) const
{
  unsigned int timeSteps = m_TimeGrid.GetSize();
  ModelResultType signal(timeSteps);

  std::map<std::string, double> parameterMap;
  parameterMap.insert(std::make_pair(GetXName(), 0.0));

  auto paramNames = this->GetParameterNames();
  for (ParametersType::size_type i = 0; i < parameters.size(); ++i)
  {
    parameterMap.insert(std::make_pair(paramNames[i], parameters[i]));
  }

  FormulaParser formulaParser(&parameterMap);

  TimeGridType::const_iterator timeGridEnd = m_TimeGrid.end();
  ModelResultType::iterator signalPos = signal.begin();

  for (TimeGridType::const_iterator gridPos = m_TimeGrid.begin(); gridPos != timeGridEnd;
       ++gridPos, ++signalPos)
  {
    parameterMap[GetXName()] = *gridPos;
    *signalPos = formulaParser.parse(m_FunctionString);
  }

  return signal;
};

mitk::GenericParamModel::ParameterNamesType mitk::GenericParamModel::GetStaticParameterNames()
const
{
  ParameterNamesType result;
  result.push_back(NAME_STATIC_PARAMETER_number);
  return result;
}

mitk::GenericParamModel::ParametersSizeType
mitk::GenericParamModel::GetNumberOfStaticParameters() const
{
  return 1;
}

void mitk::GenericParamModel::SetStaticParameter(const ParameterNameType& name,
    const StaticParameterValuesType& values)
{
  if (name == NAME_STATIC_PARAMETER_number)
  {
    SetNumberOfParameters(values[0]);
  }
};

mitk::GenericParamModel::StaticParameterValuesType
mitk::GenericParamModel::GetStaticParameterValue(
  const ParameterNameType& name) const
{
  StaticParameterValuesType result;

  if (name == NAME_STATIC_PARAMETER_number)
  {
    result.push_back(m_NumberOfParameters);
  }

  return result;
};

itk::LightObject::Pointer mitk::GenericParamModel::InternalClone() const
{
  GenericParamModel::Pointer newClone = GenericParamModel::New();

  newClone->SetTimeGrid(this->m_TimeGrid);
  newClone->SetNumberOfParameters(this->m_NumberOfParameters);

  return newClone.GetPointer();
};
