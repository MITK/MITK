/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkCurveParameterFunctor.h"

mitk::SimpleFunctorBase::OutputPixelVectorType
mitk::CurveParameterFunctor::Compute(const mitk::SimpleFunctorBase::InputPixelVectorType& value)
const
{
  if (this->m_Grid.GetSize() == 0)
  {
    itkExceptionMacro("Error. Cannot compute Curve Description Parameters. No Curve Grid set!");
  }

  if (this->m_DescriptorMap.size() == 0)
  {
    itkGenericExceptionMacro( <<
                              "Error. Cannot compute Curve Description Parameters. No Parameters registered!");
  }

  CurveDescriptionParameterBase::CurveType sample(value.size());

  for (CurveDescriptionParameterBase::CurveType::SizeValueType i = 0; i < sample.Size(); ++i)
  {
    sample[i] = value [i];
  }

  mitk::SimpleFunctorBase::OutputPixelVectorType result;
  result.reserve(m_DescriptorMap.size()); //we will have at least this number of values

  unsigned int i = 0;

  for (DescriptionParameterMapType::const_iterator pos = m_DescriptorMap.begin();
       pos != m_DescriptorMap.end(); ++pos, ++i)
  {
    CurveDescriptionParameterBase* paramFct = const_cast<CurveDescriptionParameterBase*>
        (pos->second.GetPointer());

    CurveDescriptionParameterBase::DescriptionParameterResultsType fctResults =
      paramFct->GetCurveDescriptionParameter(sample, this->m_Grid);
    result.insert(result.end(), fctResults.begin(), fctResults.end());
  }

  if (this->GetDescriptionParameterNames().size() != result.size())
  {
    itkGenericExceptionMacro( <<
                              "Error. Curve parameter functor has an invalid state. Number of parameter names and number of computed values does not match. Names count: "
                              << this->GetDescriptionParameterNames().size() << "; value count: " << result.size());
  }

  return result;
};

unsigned int
mitk::CurveParameterFunctor::GetNumberOfOutputs() const
{
  return GetDescriptionParameterNames().size();
};

mitk::CurveParameterFunctor::GridArrayType
mitk::CurveParameterFunctor::GetGrid() const
{
  return m_Grid;
};

void
mitk::CurveParameterFunctor::ResetDescriptionParameters()
{
  m_DescriptorMap.clear();
};

void
mitk::CurveParameterFunctor::RegisterDescriptionParameter(const std::string& parameterName,
    CurveDescriptionParameterBase* parameterFunction)
{
  CurveDescriptionParameterBase::Pointer paramFunctPtr = parameterFunction;

  m_DescriptorMap.insert(std::make_pair(parameterName, paramFunctPtr));
};

mitk::CurveParameterFunctor::ParameterNamesType
mitk::CurveParameterFunctor::GetDescriptionParameterNames() const
{
  ParameterNamesType result;

  for (const auto &descriptor : m_DescriptorMap)
  {
    CurveDescriptionParameterBase::DescriptionParameterNamesType fctResults =
      descriptor.second->GetDescriptionParameterName();

    if (fctResults.size() > 1)
    {
      for (const auto &fctName : fctResults)
      {
        result.push_back(descriptor.first + "_" + fctName);
      }
    }
    else
    { //if the descriptor has only one parameter just use the registered descriptor name.
      result.push_back(descriptor.first);
    }
  }

  return result;
};

const mitk::CurveDescriptionParameterBase*
mitk::CurveParameterFunctor::GetDescriptionParameterFunction(const std::string& parameterName) const
{
  const CurveDescriptionParameterBase* result = nullptr;

  DescriptionParameterMapType::const_iterator pos = m_DescriptorMap.find(parameterName);

  if (pos != m_DescriptorMap.end())
  {
    result = (pos->second).GetPointer();
  }

  return result;
};

mitk::CurveParameterFunctor::
CurveParameterFunctor()
{};

mitk::CurveParameterFunctor::
~CurveParameterFunctor() {};

