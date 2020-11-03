/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkModelBase.h"
#include "itkMacro.h"

#include <algorithm>

mitk::ModelBase::ParamterScaleMapType
mitk::ModelBase::GetParameterScales() const
{
  ParamterScaleMapType result;
  ParameterNamesType names = GetParameterNames();

  for (ParameterNamesType::iterator pos = names.begin(); pos != names.end(); ++pos)
  {
    result.insert(std::make_pair(*pos, 1.0));
  }

  return result;
};

mitk::ModelBase::ParamterUnitMapType
mitk::ModelBase::GetParameterUnits() const
{
  ParamterUnitMapType result;
  ParameterNamesType names = GetParameterNames();

  for (ParameterNamesType::iterator pos = names.begin(); pos != names.end(); ++pos)
  {
    result.insert(std::make_pair(*pos, ""));
  }

  return result;
};

mitk::ModelBase::DerivedParamterScaleMapType
mitk::ModelBase::GetDerivedParameterScales() const
{
  DerivedParamterScaleMapType result;
  DerivedParameterNamesType names = this->GetDerivedParameterNames();

  for (DerivedParameterNamesType::iterator pos = names.begin(); pos != names.end(); ++pos)
  {
    result.insert(std::make_pair(*pos, 1.0));
  }

  return result;
};

mitk::ModelBase::DerivedParamterUnitMapType
mitk::ModelBase::GetDerivedParameterUnits() const
{
  DerivedParamterUnitMapType result;
  DerivedParameterNamesType names = this->GetDerivedParameterNames();

  for (DerivedParameterNamesType::iterator pos = names.begin(); pos != names.end(); ++pos)
  {
    result.insert(std::make_pair(*pos, ""));
  }

  return result;
};

std::string
mitk::ModelBase::GetModelDisplayName() const
{
  return this->GetClassID();
};

std::string mitk::ModelBase::GetModelType() const
{
  return "Unkown";
};

mitk::ModelBase::FunctionStringType mitk::ModelBase::GetFunctionString() const
{
  return "";
};

mitk::ModelBase::ModellClassIDType mitk::ModelBase::GetClassID() const
{
  return this->GetNameOfClass();
};

std::string mitk::ModelBase::GetXName() const
{
  return "";
};

std::string mitk::ModelBase::GetXAxisName() const
{
  return "";
};

std::string mitk::ModelBase::GetXAxisUnit() const
{
  return "";
};

std::string mitk::ModelBase::GetYAxisName() const
{
  return "";
};

std::string mitk::ModelBase::GetYAxisUnit() const
{
  return "";
}

mitk::ModelBase::ModelBase()
{
}

mitk::ModelBase::~ModelBase()
{
}

mitk::ModelBase::ModelResultType mitk::ModelBase::GetSignal(const ParametersType& parameters) const
{
  if (parameters.size() != this->GetNumberOfParameters())
  {
    itkExceptionMacro("Passed parameter set has wrong size for model. Cannot evaluate model. Required size: "
                      << this->GetNumberOfParameters() << "; passed parameters: " << parameters);
  }

  std::string error;

  if (!ValidateModel(error))
  {
    itkExceptionMacro("Cannot evaluate model and return signal. Model is in an invalid state. Validation error: "
                      << error);
  }

  ModelResultType signal = ComputeModelfunction(parameters);

  return signal;
}

bool mitk::ModelBase::ValidateModel(std::string& /*error*/) const
{
  return true;
};


void mitk::ModelBase::SetTimeGrid(const TimeGridType& grid)
{
  itkDebugMacro("setting TimeGrid to " << grid);

  if (grid.GetSize() == 0)
  {
    itkExceptionMacro("Time Grid Vector is empty! Set valid time grid");
  }

  if (this->m_TimeGrid != grid)
  {
    this->m_TimeGrid = grid;
    this->Modified();
  }
}

void mitk::ModelBase::PrintSelf(std::ostream& os, ::itk::Indent indent) const
{
  Superclass::PrintSelf(os, indent);

  os << indent << "Time grid: " << m_TimeGrid;
};

void mitk::ModelBase::SetStaticParameters(const StaticParameterMapType& parameters,
    bool allParameters)
{
  ParameterNamesType names = this->GetStaticParameterNames();

  if ((parameters.size() != names.size()) && allParameters)
  {
    itkExceptionMacro("Cannot set static parameter of model. Passed parameters does not define all parameters correctly. Required size:"
                      << names.size() << "; passed size: " << parameters.size());
  }

  for (StaticParameterMapType::const_iterator pos = parameters.begin(); pos != parameters.end();
       ++pos)
  {
    ParameterNamesType::iterator finding = std::find(names.begin(), names.end(), pos->first);

    if (finding == names.end())
    {
      itkExceptionMacro("Cannot set static parameter of model. Passed parameter name is not in the list of valid names. Passed name: "
                        << pos->first);
    }
  }

  //Setting is done in an other loop to ensure that the state of the model only changes if all values are valid.
  for (StaticParameterMapType::const_iterator pos = parameters.begin(); pos != parameters.end();
       ++pos)
  {
    this->SetStaticParameter(pos->first, pos->second);
  }

};

mitk::ModelBase::StaticParameterMapType mitk::ModelBase::GetStaticParameters() const
{
  StaticParameterMapType result;
  ParameterNamesType names = this->GetStaticParameterNames();

  for (ParameterNamesType::const_iterator pos = names.begin(); pos != names.end(); ++pos)
  {
    StaticParameterValuesType values = this->GetStaticParameterValue(*pos);
    result.insert(std::make_pair(*pos, values));
  }

  return result;
};

mitk::ModelBase::DerivedParameterNamesType mitk::ModelBase::GetDerivedParameterNames() const
{
  ParameterNamesType emptyResult;
  return emptyResult;
};

mitk::ModelBase::ParamterUnitMapType
mitk::ModelBase::GetStaticParameterUnits() const
{
  ParamterUnitMapType result;
  ParameterNamesType names = GetStaticParameterNames();

  for (ParameterNamesType::iterator pos = names.begin(); pos != names.end(); ++pos)
  {
    result.insert(std::make_pair(*pos, ""));
  }

  return result;
};

mitk::ModelBase::DerivedParametersSizeType mitk::ModelBase::GetNumberOfDerivedParameters() const
{
  return 0;
};

mitk::ModelBase::DerivedParameterMapType mitk::ModelBase::GetDerivedParameters(
  const mitk::ModelBase::ParametersType& parameters) const
{
  if (parameters.size() != this->GetNumberOfParameters())
  {
    itkExceptionMacro("Cannot compute derived parametes. Passed parameters does not define all parameters correctly. Required size:"
                      << this->GetNumberOfParameters() << "; passed size: " << parameters.size());
  }

  return ComputeDerivedParameters(parameters);
};

mitk::ModelBase::DerivedParameterMapType mitk::ModelBase::ComputeDerivedParameters(
  const mitk::ModelBase::ParametersType& /*parameters*/) const
{
  DerivedParameterMapType emptyResult;
  return emptyResult;
};
