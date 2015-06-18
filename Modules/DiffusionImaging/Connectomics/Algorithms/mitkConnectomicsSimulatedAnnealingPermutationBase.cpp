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

#include "mitkConnectomicsSimulatedAnnealingPermutationBase.h"

mitk::ConnectomicsSimulatedAnnealingPermutationBase::ConnectomicsSimulatedAnnealingPermutationBase()
: m_CostFunction( nullptr )
{
}

mitk::ConnectomicsSimulatedAnnealingPermutationBase::~ConnectomicsSimulatedAnnealingPermutationBase()
{
}

void mitk::ConnectomicsSimulatedAnnealingPermutationBase::SetCostFunction(
  mitk::ConnectomicsSimulatedAnnealingCostFunctionBase::Pointer costFunction )
{
  m_CostFunction = costFunction;
}

bool mitk::ConnectomicsSimulatedAnnealingPermutationBase::HasCostFunction( )
{
  bool hasCostFunction( false );

  if( m_CostFunction.IsNotNull() )
  {
    hasCostFunction = true;
  }

  return hasCostFunction;
}
