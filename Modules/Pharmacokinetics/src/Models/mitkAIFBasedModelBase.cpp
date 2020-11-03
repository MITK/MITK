/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkAIFBasedModelBase.h"
#include "mitkTimeGridHelper.h"
#include "mitkAIFParametrizerHelper.h"

#include "itkArray2D.h"


const std::string mitk::AIFBasedModelBase::NAME_STATIC_PARAMETER_AIF = "Aterial Input Function";
const std::string mitk::AIFBasedModelBase::NAME_STATIC_PARAMETER_AIFTimeGrid =
  "Aterial Input Function Timegrid";

//Assumed AIF is always extracted from concentration image
const std::string mitk::AIFBasedModelBase::UNIT_STATIC_PARAMETER_AIF = "C";
const std::string mitk::AIFBasedModelBase::UNIT_STATIC_PARAMETER_AIFTimeGrid =
  "s";

std::string mitk::AIFBasedModelBase::GetXAxisName() const
{
  return "Time";
};

std::string mitk::AIFBasedModelBase::GetXAxisUnit() const
{
  return "s";
}

std::string mitk::AIFBasedModelBase::GetYAxisName() const
{
  return "";
};

std::string mitk::AIFBasedModelBase::GetYAxisUnit() const
{
  return "";
}

mitk::AIFBasedModelBase::AIFBasedModelBase()
{
}

mitk::AIFBasedModelBase::~AIFBasedModelBase()
{
}

const mitk::AIFBasedModelBase::TimeGridType&
mitk::AIFBasedModelBase::GetCurrentAterialInputFunctionTimeGrid() const
{
  if (!m_AterialInputFunctionTimeGrid.empty())
  {
    return m_AterialInputFunctionTimeGrid;
  }
  else
  {
    return m_TimeGrid;
  }
};

const mitk::AIFBasedModelBase::AterialInputFunctionType
mitk::AIFBasedModelBase::GetAterialInputFunction(TimeGridType CurrentTimeGrid) const
{
  if (CurrentTimeGrid.GetSize() == 0)
  {
    return this->m_AterialInputFunctionValues;
  }
  else
  {
    return mitk::InterpolateSignalToNewTimeGrid(m_AterialInputFunctionValues,
           GetCurrentAterialInputFunctionTimeGrid(), CurrentTimeGrid);
  }
}

mitk::AIFBasedModelBase::ParameterNamesType mitk::AIFBasedModelBase::GetStaticParameterNames() const
{
  ParameterNamesType result;

  result.push_back(NAME_STATIC_PARAMETER_AIF);
  result.push_back(NAME_STATIC_PARAMETER_AIFTimeGrid);

  return result;
}

mitk::AIFBasedModelBase::ParametersSizeType  mitk::AIFBasedModelBase::GetNumberOfStaticParameters()
const
{
  return 2;
}

mitk::AIFBasedModelBase::ParamterUnitMapType
mitk::AIFBasedModelBase::GetStaticParameterUnits() const
{
  ParamterUnitMapType result;

  result.insert(std::make_pair(NAME_STATIC_PARAMETER_AIF, UNIT_STATIC_PARAMETER_AIF));
  result.insert(std::make_pair(NAME_STATIC_PARAMETER_AIFTimeGrid, UNIT_STATIC_PARAMETER_AIFTimeGrid));

  return result;
};


void mitk::AIFBasedModelBase::SetStaticParameter(const ParameterNameType& name,
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
};

mitk::AIFBasedModelBase::StaticParameterValuesType mitk::AIFBasedModelBase::GetStaticParameterValue(
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

  return result;
};

bool mitk::AIFBasedModelBase::ValidateModel(std::string& error) const
{
  bool result = Superclass::ValidateModel(error);

  if (result)
  {
    if (m_AterialInputFunctionTimeGrid.empty())
    {
      if (this->m_TimeGrid.GetSize() != m_AterialInputFunctionValues.GetSize())
      {
        result = false;
        error = "Number of elements of Model Time Grid does not match number of elements in Aterial Input Function! Set valid aif or aif time grid.";
      }
    }
    else
    {
      if (m_AterialInputFunctionTimeGrid.GetSize() != m_AterialInputFunctionValues.GetSize())
      {
        result = false;
        error = "Number of elements of Aterial Input Function Time Grid does not match number of elements of Aterial Input Function Values! Set valid curve";
      }
    }
  }

  return result;
};

void mitk::AIFBasedModelBase::PrintSelf(std::ostream& os, ::itk::Indent indent) const
{
  Superclass::PrintSelf(os, indent);

  os << indent << "Aterial Input Function: " << m_AterialInputFunctionValues;
  os << indent << "Aterial Input Function Time Grid: " << m_AterialInputFunctionTimeGrid;
};


