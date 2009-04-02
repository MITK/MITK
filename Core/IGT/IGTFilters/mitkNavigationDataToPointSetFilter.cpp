/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
Language:  C++
Date:      $Date: 2007-01-08 17:30:05 +0100 (Mo, 08 Jan 2007) $
Version:   $Revision: 8970 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/ for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "mitkNavigationDataToPointSetFilter.h"

#include <mitkPointOperation.h>
#include <mitkInteractionConst.h>

mitk::NavigationDataToPointSetFilter::NavigationDataToPointSetFilter()
{
  OutputType::Pointer output = dynamic_cast<OutputType*> ( this->MakeOutput( 0 ).GetPointer() );
  this->SetNumberOfRequiredInputs(1);
  this->SetNumberOfOutputs( 1 );
  this->SetNthOutput(0, output.GetPointer());

  m_OperationMode = Mode3D;
  m_CurrentTimeStep = 0;
  m_RingBufferSize = 50; //the default ring buffer size
}

mitk::NavigationDataToPointSetFilter::~NavigationDataToPointSetFilter()
{
}

void mitk::NavigationDataToPointSetFilter::GenerateData()
{
  switch (m_OperationMode)
  {
  case Mode3D:
    GenerateDataMode3D();
    break;
  case Mode4D:
    GenerateDataMode4D();
    break;
  default:
    break;
  }
}


void mitk::NavigationDataToPointSetFilter::SetInput(const mitk::NavigationData *input)
{
  // Process object is not const-correct so the const_cast is required here
  this->BaseProcess::SetNthInput(0, 
    const_cast< mitk::NavigationData * >( input ) );

  this->Modified();
}


void mitk::NavigationDataToPointSetFilter::SetInput(const mitk::NavigationData *input, unsigned int index)
{
  // Process object is not const-correct so the const_cast is required here
  this->BaseProcess::SetNthInput(index, 
    const_cast< mitk::NavigationData * >( input ) );

  this->Modified();
}


void mitk::NavigationDataToPointSetFilter::GenerateDataMode3D()
{
  DataObjectPointerArray inputs = this->GetInputs(); //get all inputs

  for (unsigned int index=0; index<inputs.size(); index++)
  {
    mitk::PointSet* outputPointSet = GetOutput();
    NavigationData::PositionType point;

   mitk::NavigationData* nd = dynamic_cast<mitk::NavigationData*>(inputs[index].GetPointer()) ;

    point = nd->GetPosition();  //get the position

    outputPointSet->SetPoint( index, point); //store it in the pointset always at timestep 0
  }
}


void mitk::NavigationDataToPointSetFilter::GenerateDataMode4D()
{
  DataObjectPointerArray inputs = this->GetInputs(); //get all inputs

  for (unsigned int index=0; index<inputs.size(); index++)
  {
    mitk::PointSet* outputPointSet = GetOutput();
    NavigationData::PositionType point;

    mitk::NavigationData* nd = dynamic_cast<mitk::NavigationData*>(inputs[index].GetPointer()) ;

    point = nd->GetPosition();  //get the position

    outputPointSet->SetPoint( index, point, m_CurrentTimeStep); //store it in the pointset always at the current time step
  }  
  
  //in which timestep should be stored next?s
  if (m_CurrentTimeStep == m_RingBufferSize-1)
    m_CurrentTimeStep = 0;
  else
    m_CurrentTimeStep++;
}


void mitk::NavigationDataToPointSetFilter::SetOperationMode( OperationMode mode )
{
  if (mode==Mode3D || mode==Mode4D)
    m_OperationMode = mode;
  else
  {
    m_OperationMode = Mode3D;
    std::cout << "Wrong Mode Setting switched to Mode3D" << std::endl;
  }

  //Initialize 4D Mode
  if (m_OperationMode == Mode4D)
  {
    m_CurrentTimeStep=0;
  }
}