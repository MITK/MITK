/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
Language:  C++
Date:      $Date: 2009-04-02 15:46:56 +0200 (Do, 02 Apr 2009) $
Version:   $Revision: 16783 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "mitkNavigationDataToMessageFilter.h"
#include "mitkPropertyList.h"
#include "mitkProperties.h"

mitk::NavigationDataToMessageFilter::NavigationDataToMessageFilter() 
: mitk::NavigationDataToNavigationDataFilter()
{
  this->SetNumberOfRequiredInputs(1);
}


mitk::NavigationDataToMessageFilter::~NavigationDataToMessageFilter()
{
}


//void mitk::NavigationDataToMessageFilter::SetInput( unsigned int idx, const NavigationData* nd )
//{
//  if (idx > 0)
//    throw std::invalid_argument("mitk::NavigationDataToMessageFilter: only one input is supported");
//  // Process object is not const-correct so the const_cast is required here
//  this->ProcessObject::SetNthInput(idx, const_cast<NavigationData*>(nd));
//  this->CreateOutputsForAllInputs();
//}


//void mitk::NavigationDataToMessageFilter::SetInput( const NavigationData* nd )
//{
//  // Process object is not const-correct so the const_cast is required here
//  this->ProcessObject::SetNthInput(0, const_cast<NavigationData*>(nd));
//  this->CreateOutputsForAllInputs();
//}


void mitk::NavigationDataToMessageFilter::GenerateData()
{
  /* update outputs with tracking data from tools */
  for (unsigned int i = 0; i < this->GetNumberOfOutputs() ; ++i)
  {
    mitk::NavigationData* output = this->GetOutput(i);
    assert(output);
    const mitk::NavigationData* input = this->GetInput(i);
    assert(input);

    /* check for differences, then send message. */
    if (input->GetPosition() != output->GetPosition())
      m_PositionChangedMessage.Send(input->GetPosition(), i);
    if (input->GetOrientation() != output->GetOrientation())
      m_OrientationChangedMessage.Send(input->GetOrientation(), i);
    if (input->GetCovErrorMatrix() != output->GetCovErrorMatrix())
      m_ErrorChangedMessage.Send(input->GetCovErrorMatrix(), i);
    if (input->GetTimeStamp() != output->GetTimeStamp())
      m_TimeStampChangedMessage.Send(input->GetTimeStamp(), i);
    if (input->IsDataValid() != output->IsDataValid())
      m_DataValidChangedMessage.Send(input->IsDataValid(), i);

    output->Graft(input); // Because this is a NavigationDataToNavigationData Filter, we need to copy the input to the output to provide up to date navigation data objects
  }
}
