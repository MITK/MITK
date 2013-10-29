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

#include "mitkNavigationDataRecorder.h"
#include <mitkIGTTimeStamp.h>

mitk::NavigationDataRecorder::NavigationDataRecorder()
{
  //set default values
  m_NumberOfInputs = 0;
  m_Recording = false;
  m_StandardizedTimeInitialized = false;
  m_RecordCountLimit = -1;
}

mitk::NavigationDataRecorder::~NavigationDataRecorder()
{
  mitk::IGTTimeStamp::GetInstance()->Stop(this);
}

void mitk::NavigationDataRecorder::GenerateData()
{
  // get each input, lookup the associated BaseData and transfer the data
  DataObjectPointerArray inputs = this->GetIndexedInputs(); //get all inputs

  //This vector will hold the NavigationDatas that are copied from the inputs
  std::vector< mitk::NavigationData::Pointer > clonedDatas;

  // For each input
  for (unsigned int index=0; index < inputs.size(); index++)
  {
    // First copy input to output
    this->GetOutput(index)->Graft(this->GetInput(index));

    // if we are not recording, that's all there is to do
    if (! m_Recording) continue;

    // Clone a Navigation Data
    mitk::NavigationData::Pointer clone = mitk::NavigationData::New();
    clone->Graft(this->GetInput(index));
    clonedDatas.push_back(clone);

    if (m_StandardizeTime)
    {
      mitk::NavigationData::TimeStampType igtTimestamp = mitk::IGTTimeStamp::GetInstance()->GetElapsed(this);
      clonedDatas[index]->SetIGTTimeStamp(igtTimestamp);
    }
  }

  // if limitation is set and has been reached, stop recording
  if ((m_RecordCountLimit > 0) && (m_NavigationDataSet->Size() >= m_RecordCountLimit)) m_Recording = false;
  // We can skip the rest of the method, if recording is deactivated
  if  (!m_Recording) return;


  // Add data to set
  m_NavigationDataSet->AddNavigationDatas(clonedDatas);
}

void mitk::NavigationDataRecorder::StartRecording()
{
  if (m_Recording)
  {
    MITK_WARN << "Already recording please stop before start new recording session";
    return;
  }
  m_Recording = true;

  // The first time this StartRecording is called, we initialize the standardized time.
  // Afterwards, it can be reset via ResetNavigationDataSet();
  if (! m_StandardizedTimeInitialized)
    mitk::IGTTimeStamp::GetInstance()->Start(this);

  if (m_NavigationDataSet.IsNull())
    m_NavigationDataSet = mitk::NavigationDataSet::New(GetNumberOfIndexedInputs());
}

void mitk::NavigationDataRecorder::StopRecording()
{
  if (!m_Recording)
  {
    std::cout << "You have to start a recording first" << std::endl;
    return;
  }
  m_Recording = false;
}

void mitk::NavigationDataRecorder::ResetRecording()
{
  m_NavigationDataSet = mitk::NavigationDataSet::New(GetNumberOfIndexedInputs());

  if (m_Recording)
  {
    mitk::IGTTimeStamp::GetInstance()->Stop(this);
    mitk::IGTTimeStamp::GetInstance()->Start(this);
  }
}

int mitk::NavigationDataRecorder::GetNumberOfRecordedSteps()
{
  return m_NavigationDataSet->Size();
}