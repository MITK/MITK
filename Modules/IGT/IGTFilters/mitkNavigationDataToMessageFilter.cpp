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

#include "mitkNavigationDataToMessageFilter.h"
#include "mitkPropertyList.h"
#include "mitkProperties.h"

mitk::NavigationDataToMessageFilter::NavigationDataToMessageFilter()
: mitk::NavigationDataToNavigationDataFilter(),
  m_PositionEpsilon(0.0f), m_OrientationEpsilon(0.0f), m_CovErrorEpsilon(0.0f), m_TimeStampEpsilon(0.0f)
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
    if ((output->GetPosition() - input->GetPosition()).GetNorm() > m_PositionEpsilon)
      m_PositionChangedMessage.Send(input->GetPosition(), i);

    if ((output->GetOrientation() - input->GetOrientation()).magnitude() > m_OrientationEpsilon)
      m_OrientationChangedMessage.Send(input->GetOrientation(), i);

    if ((output->GetCovErrorMatrix().GetVnlMatrix() - input->GetCovErrorMatrix().GetVnlMatrix()).absolute_value_max() > m_CovErrorEpsilon)
      m_ErrorChangedMessage.Send(input->GetCovErrorMatrix(), i);

    if (fabs(output->GetTimeStamp() - input->GetTimeStamp()) > m_TimeStampEpsilon)
      m_TimeStampChangedMessage.Send(input->GetTimeStamp(), i);

    if (input->IsDataValid() != output->IsDataValid())
      m_DataValidChangedMessage.Send(input->IsDataValid(), i);

    output->Graft(input); // Because this is a NavigationDataToNavigationData Filter, we need to copy the input to the output to provide up to date navigation data objects
  }
}
