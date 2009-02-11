/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
Language:  C++
Date:      $Date$
Version:   $Revision: $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

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
  }

  void mitk::NavigationDataVisualizationFilter::SetInput( unsigned int idx, const NavigationData* nd )
  {
    // Process object is not const-correct so the const_cast is required here
    this->ProcessObject::SetNthInput(idx, const_cast<NavigationData*>(nd));  
  }

  const NavigationData* mitk::NavigationDataVisualizationFilter::GetInput( void )
  {
    if (this->GetNumberOfInputs() < 1)
      return NULL;

    return static_cast<const NavigationData*>(this->ProcessObject::GetInput(0));
  }

  const NavigationData* mitk::NavigationDataVisualizationFilter::GetInput( unsigned int idx )
  {
    if (this->GetNumberOfInputs() < 1)
      return NULL;

    return static_cast<const NavigationData*>(this->ProcessObject::GetInput(idx));
  }

}