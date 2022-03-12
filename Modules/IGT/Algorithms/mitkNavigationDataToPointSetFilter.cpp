/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkNavigationDataToPointSetFilter.h"

#include <mitkPointOperation.h>
#include <mitkInteractionConst.h>
#include <itksys/SystemTools.hxx>

mitk::NavigationDataToPointSetFilter::NavigationDataToPointSetFilter()
{
  mitk::PointSet::Pointer output = mitk::PointSet::New();
  this->SetNumberOfRequiredOutputs(1);
  this->SetNthOutput(0, output.GetPointer());

  this->SetNumberOfRequiredInputs(1);

  m_OperationMode = Mode3D;
  m_CurrentTimeStep = 0;
  m_RingBufferSize = 50; //the default ring buffer size
  m_NumberForMean = 100;
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
    return nullptr;
  return static_cast<const NavigationData*>(this->ProcessObject::GetInput(0));
}


const mitk::NavigationData* mitk::NavigationDataToPointSetFilter::GetInput( unsigned int idx )
{
  if (this->GetNumberOfInputs() < 1)
    return nullptr;
  return static_cast<const NavigationData*>(this->ProcessObject::GetInput(idx));
}


void mitk::NavigationDataToPointSetFilter::CreateOutputsForAllInputs()
{
  switch (m_OperationMode)
  {
  case Mode3D:
    this->SetNumberOfIndexedOutputs(this->GetNumberOfIndexedInputs());  // create one pointset output for each navigation data input
    break;
  case Mode3DMean:
    this->SetNumberOfIndexedOutputs(this->GetNumberOfIndexedInputs());  // create one pointset output for each navigation data input
    break;
  case Mode4D:
    this->SetNumberOfIndexedOutputs(1); // create just one output pointset that will contain all input navigation data objects
    break;
  default:
    break;
  }

  for (unsigned int idx = 0; idx < this->GetNumberOfIndexedOutputs(); ++idx)
  {
    if (this->GetOutput(idx) == nullptr)
    {
      DataObjectPointer newOutput = this->MakeOutput(idx);
      this->SetNthOutput(idx, newOutput);
    }
  }

  this->Modified();
}


void mitk::NavigationDataToPointSetFilter::GenerateDataMode3D()
{
  for (unsigned int i = 0; i < this->GetNumberOfIndexedOutputs() ; ++i)  // for each output PointSet
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

  //check for outputs and inputs
  for (unsigned int i = 0; i < this->GetNumberOfIndexedOutputs() ; ++i)  // for each output PointSet; change through pointsets to collect all navigation data in order
  {
    assert(this->GetOutput(i));
    assert(this->GetInput(i));
  }

  //vector of counters for each output
  std::vector<unsigned int> counterVec(this->GetNumberOfIndexedOutputs(),0);

  //vector of old timesteps for each output
  std::vector<mitk::NavigationData::TimeStampType> vectorOldTime(this->GetNumberOfIndexedOutputs());

  //use first Output to get the size of the pointsets. All output pointssets have to have the same size!
  mitk::PointSet::PointIdentifier newPointId = this->GetOutput()->GetSize();

  bool numberForMean_is_reached = false;
  while (!numberForMean_is_reached)
  {
    for (unsigned int i = 0; i < this->GetNumberOfIndexedOutputs() ; ++i)  // for each output PointSet; change through pointsets to collect all navigation data in order
    {
        mitk::PointSet* output = this->GetOutput(i);
        const mitk::NavigationData* input = this->GetInput(i);
        if (input->IsDataValid() == false)  // don't add point if input is invalid
          continue;//do not store
        mitk::PointSet::PointType pos;
        if (counterVec[i] == 0) //first Element must be inserted
        {
          vectorOldTime[i] = input->GetIGTTimeStamp();

          //no need to call an update
          pos = input->GetPosition();  // NavigationData::PositionType must be compatible with PointSet::PointType!
          output->InsertPoint(newPointId, pos);  // add point with current position of input NavigationData to the output PointSet
          counterVec[i]++;
        }
        else
        {
          //manually call an update to track new positions
          this->ProcessObject::GetInput(i)->Update();
          input = this->GetInput(i);
          mitk::NavigationData::TimeStampType newTime = input->GetIGTTimeStamp();
          if (vectorOldTime[i]<newTime)
          {
            pos = input->GetPosition();  // NavigationData::PositionType must be compatible with PointSet::PointType!

            //calculate the summ of the old position and the current coordinate
            mitk::Vector3D vec(0.0);
            vec.SetVnlVector(pos.GetVnlVector().as_ref());
            mitk::PointSet::PointType oPoint = output->GetPoint(newPointId);
            oPoint += vec;//summ up
            output->SetPoint(newPointId, oPoint);

            //store in counterVec to know how many have been added (and not skipped because of invalid data)
            counterVec[i]++;
            vectorOldTime[i] = newTime;
          }
        }
        // \TODO: regard ringbuffersize
    }
    numberForMean_is_reached = true;
    for (unsigned int i = 0; i < this->GetNumberOfIndexedOutputs() ; ++i)
    {
      if (counterVec[i]<m_NumberForMean)
        numberForMean_is_reached = false;
    }

  }

  //divide with counterVec
  for (unsigned int i = 0; i < this->GetNumberOfIndexedOutputs() ; ++i)  // for each output PointSet; change through pointsets to collect all navigation data in order
  {
    mitk::PointSet* output = this->GetOutput(i);
    mitk::PointSet::PointType oPoint = output->GetPoint(newPointId);
    for (unsigned int index = 0; index < oPoint.Size(); index++)
      oPoint[index] = oPoint[index] / counterVec[i];
    output->SetPoint(newPointId, oPoint);
    MBI_INFO << "For output # " << i << ", " << counterVec[i] << " tracked positions used for averaging";
  }
}

void mitk::NavigationDataToPointSetFilter::GenerateDataMode4D()
{
  mitk::PointSet* output = this->GetOutput();
  assert(output);
  for (unsigned int index = 0; index < this->GetNumberOfIndexedInputs(); index++)
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
