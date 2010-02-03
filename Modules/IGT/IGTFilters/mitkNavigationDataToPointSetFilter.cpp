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

  this->SetNumberOfRequiredInputs(1);

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
  case Mode3DMean:
    GenerateDataMode3DMean();
    break;
  case Mode4D:
    GenerateDataMode4D();
    break;
  default:
    break;
  }
}


void mitk::NavigationDataToPointSetFilter::SetInput(const NavigationData* nd)
{
  // Process object is not const-correct so the const_cast is required here
  this->ProcessObject::SetNthInput(0, const_cast<NavigationData*>(nd));
  this->CreateOutputsForAllInputs();
}


void mitk::NavigationDataToPointSetFilter::SetInput(unsigned int idx, const NavigationData* nd)
{
  // Process object is not const-correct so the const_cast is required here
  this->ProcessObject::SetNthInput(idx, const_cast<NavigationData*>(nd));
  this->CreateOutputsForAllInputs();
}


const mitk::NavigationData* mitk::NavigationDataToPointSetFilter::GetInput( void )
{
  if (this->GetNumberOfInputs() < 1)
    return NULL;
  return static_cast<const NavigationData*>(this->ProcessObject::GetInput(0));
}


const mitk::NavigationData* mitk::NavigationDataToPointSetFilter::GetInput( unsigned int idx )
{
  if (this->GetNumberOfInputs() < 1)
    return NULL;
  return static_cast<const NavigationData*>(this->ProcessObject::GetInput(idx));
}


void mitk::NavigationDataToPointSetFilter::CreateOutputsForAllInputs()
{
  switch (m_OperationMode)
  {
  case Mode3D:
    this->SetNumberOfOutputs(this->GetNumberOfInputs());  // create one pointset output for each navigation data input
    break;
  case Mode4D:
    this->SetNumberOfOutputs(1); // create just one output pointset that will contain all input navigation data objects
    break;
  default:
    break;
  }
  
  for (unsigned int idx = 0; idx < this->GetNumberOfOutputs(); ++idx)
    if (this->GetOutput(idx) == NULL)
    {
      DataObjectPointer newOutput = this->MakeOutput(idx);
      this->SetNthOutput(idx, newOutput);
    }
  this->Modified();
}


void mitk::NavigationDataToPointSetFilter::GenerateDataMode3D()
{
  for (unsigned int i = 0; i < this->GetNumberOfOutputs() ; ++i)  // for each output PointSet
  {
    mitk::PointSet* output = this->GetOutput(i);
    assert(output);
    const mitk::NavigationData* input = this->GetInput(i);
    assert(input);
    if (input->IsDataValid() == false)  // don't add point if input is invalid
      continue;
    mitk::PointSet::PointType pos = input->GetPosition();  // NavigationData::PositionType must be compatible with PointSet::PointType!
    output->InsertPoint(output->GetSize(), pos);  // add point with current position of input NavigationData to the output PointSet
    // \TODO: regard ringbuffersize
  }
}

/**
* @brief read n times all connected inputs and sum them into outputs. Finish with dividing each output by n.
**/
void mitk::NavigationDataToPointSetFilter::GenerateDataMode3DMean()
{
  //make it editable through a Set method if needed
  const unsigned int numberforMean = 100;
  
  //check for outputs and inputs
  for (unsigned int i = 0; i < this->GetNumberOfOutputs() ; ++i)  // for each output PointSet; change through pointsets to collect all navigation data in order
  {
    mitk::PointSet* output = this->GetOutput(i);
    assert(output);
    const mitk::NavigationData* input = this->GetInput(i);
    assert(input);
  }
  
  //vector of counters for each output
  std::vector<unsigned int> counterVec(this->GetNumberOfOutputs());
  
  //use first Output to get the size of the pointsets. All output pointssets have to have the same size!
  mitk::PointSet::PointIdentifier newPointId = this->GetOutput(0)->GetSize(); 

  for (unsigned int counter = 0; counter < numberforMean; counter++)
  {
    for (unsigned int i = 0; i < this->GetNumberOfOutputs() ; ++i)  // for each output PointSet; change through pointsets to collect all navigation data in order
    {
      mitk::PointSet* output = this->GetOutput(i);
      const mitk::NavigationData* input = this->GetInput(i);
      if (input->IsDataValid() == false)  // don't add point if input is invalid
        continue;//do not store
      mitk::PointSet::PointType pos = input->GetPosition();  // NavigationData::PositionType must be compatible with PointSet::PointType!
      if (counter == 0) //first Element must be inserted
      {
        output->InsertPoint(newPointId, pos);  // add point with current position of input NavigationData to the output PointSet
        counterVec[i]++;
      }
      else
      {
        //manually call an update to track new positions
        this->ProcessObject::GetInput(i)->Update();

        //calculate the summ of the old position and the current coordinate
        mitk::Vector3D vec;
        vec.SetVnlVector(pos.GetVnlVector());
        mitk::PointSet::PointType oPoint = output->GetPoint(newPointId);
        oPoint += vec;//summ up 
        output->SetPoint(newPointId, oPoint); 
        
        //store in counterVec to know how many have been added (and not skipped because of invalid data)
        counterVec[i]++;
      }
      // \TODO: regard ringbuffersize
    }
  }

  //divide with counterVec
  for (unsigned int i = 0; i < this->GetNumberOfOutputs() ; ++i)  // for each output PointSet; change through pointsets to collect all navigation data in order
  {
    mitk::PointSet* output = this->GetOutput(i);
    mitk::PointSet::PointType oPoint = output->GetPoint(newPointId);
    for (unsigned int index = 0; index < oPoint.Size(); index++)
      oPoint[index] = oPoint[index] / counterVec[i];
    output->SetPoint(newPointId, oPoint); 
  }

}

void mitk::NavigationDataToPointSetFilter::GenerateDataMode4D()
{
  mitk::PointSet* output = GetOutput();
  assert(output);
  for (unsigned int index = 0; index < this->GetNumberOfInputs(); index++)
  {
    const mitk::NavigationData* nd = GetInput(index);
    assert(nd);
    mitk::NavigationData::PositionType point = nd->GetPosition();  //get the position
    output->SetPoint( index, point, m_CurrentTimeStep); //store it in the pointset always at the current time step
  }   
  if (m_CurrentTimeStep == m_RingBufferSize - 1) // update ring buffer index
    m_CurrentTimeStep = 0;
  else
    m_CurrentTimeStep++;
}


void mitk::NavigationDataToPointSetFilter::SetOperationMode( OperationMode mode )
{
  m_OperationMode = mode;
  //Initialize 4D Mode
  if (m_OperationMode == Mode4D)
    m_CurrentTimeStep = 0;
  this->Modified();
  this->CreateOutputsForAllInputs();
}
