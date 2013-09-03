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
}

mitk::NavigationDataRecorder::~NavigationDataRecorder()
{
  mitk::IGTTimeStamp::GetInstance()->Stop(this);
}

void mitk::NavigationDataRecorder::GenerateData()
{
  // get each input, lookup the associated BaseData and transfer the data
  DataObjectPointerArray inputs = this->GetInputs(); //get all inputs

  //This vector will hold the NavigationDatas that are copied from the inputs
  //it is initialized with the size of inputs (one for each) and a new constructor, so we can graft later
  std::vector< mitk::NavigationData::Pointer > clonedDatas = std::vector< mitk::NavigationData::Pointer > (inputs.size(), mitk::NavigationData::New());
  for (unsigned int index=0; index < inputs.size(); index++)
  {
    //get the necessary variables
    clonedDatas[index]->Graft(this->GetInput(index));

    mitk::NavigationData* output = this->GetOutput(index);

    if (m_StandardizeTime)
    {
      mitk::NavigationData::TimeStampType igtTimestamp = mitk::IGTTimeStamp::GetInstance()->GetElapsed(this);
      clonedDatas[index]->SetIGTTimeStamp(igtTimestamp);
    }
  }
  // if limitation is set and has been reached, stop recording
  if ((m_RecordCountLimit <= 0) && (m_RecordCountLimit >=  m_NavigationDataSet->Size())) m_Recording = false;

  // Do the actual recording
  if (m_Recording)
  {
    m_NavigationDataSet->AddNavigationDatas(clonedDatas);
  }
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
