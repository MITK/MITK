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

#include "mitkNavigationDataVisualizationFilter.h"

namespace mitk{

  NavigationDataVisualizationFilter::NavigationDataVisualizationFilter()
  {
  }

  NavigationDataVisualizationFilter::~NavigationDataVisualizationFilter()
  {
  }

  void mitk::NavigationDataVisualizationFilter::SetInput( const NavigationData* nd )
  {
    // Process object is not const-correct so the const_cast is required here
    this->ProcessObject::SetNthInput(0, const_cast<NavigationData*>(nd));
    this->CreateOutputsForAllInputs();
  }

  void mitk::NavigationDataVisualizationFilter::SetInput( unsigned int idx, const NavigationData* nd )
  {
    // Process object is not const-correct so the const_cast is required here
    this->ProcessObject::SetNthInput(idx, const_cast<NavigationData*>(nd));
    this->CreateOutputsForAllInputs();
  }

  const NavigationData* mitk::NavigationDataVisualizationFilter::GetInput( void )
  {
    if (this->GetNumberOfInputs() < 1)
      return nullptr;

    return static_cast<const NavigationData*>(this->ProcessObject::GetInput(0));
  }

  const NavigationData* mitk::NavigationDataVisualizationFilter::GetInput( unsigned int idx )
  {
    if (this->GetNumberOfInputs() < 1)
      return nullptr;

    return static_cast<const NavigationData*>(this->ProcessObject::GetInput(idx));
  }

} //namespace mitk
