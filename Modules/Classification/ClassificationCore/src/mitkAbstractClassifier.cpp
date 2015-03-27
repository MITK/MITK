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

#include <mitkAbstractClassifier.h>

//void mitk::AbstractClassifier::SetConfiguration(const ConfigurationHolder& value)
//{
//  m_Config = value;
//}

//mitk::ConfigurationHolder& mitk::AbstractClassifier::Configuration()
//{
//  return m_Config;
//}

//mitk::ConfigurationHolder mitk::AbstractClassifier::GetConfigurationCopy()
//{
//  return m_Config;
//}


void mitk::AbstractClassifier::SetPointWiseWeight(const VectorType& w)
{
  m_PointWiseWeight = w;
  m_UsePointWiseWeight = true;
}

mitk::AbstractClassifier::VectorType mitk::AbstractClassifier::GetPointWiseWeightCopy()
{
  return m_PointWiseWeight;
}

void mitk::AbstractClassifier::UsePointWiseWeight(bool value)
{
  m_UsePointWiseWeight = value;
}

bool mitk::AbstractClassifier::IsUsingPointWiseWeight()
{
  return m_UsePointWiseWeight;
}

bool mitk::AbstractClassifier::SupportsPointWiseWeight()
{
  return false;
}

bool mitk::AbstractClassifier::SupportsPointWiseProba()
{
  return false;
}

mitk::AbstractClassifier::VectorType& mitk::AbstractClassifier::PointWeight()
{
  return m_PointWiseWeight;
}
