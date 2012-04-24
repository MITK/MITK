/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$ 
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "mitkConnectomicsSimulatedAnnealingPermutationBase.h"

mitk::ConnectomicsSimulatedAnnealingPermutationBase::ConnectomicsSimulatedAnnealingPermutationBase() 
: m_CostFunction( 0 )
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
