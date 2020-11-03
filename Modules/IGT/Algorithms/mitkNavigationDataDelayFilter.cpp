/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkNavigationDataDelayFilter.h"

mitk::NavigationDataDelayFilter::NavigationDataDelayFilter(unsigned int delay) : m_Delay(delay)
{
  m_Tolerance = 0;
}
mitk::NavigationDataDelayFilter::~NavigationDataDelayFilter()
{
}

void mitk::NavigationDataDelayFilter::GenerateData()
{
  // Check if number of outputs has changed since the previous call. If yes, reset buffer.
  // This actually compares the number of Navigation Datas in each step and compares it to the current number of inputs.
  // If these values differ, the number of inputrs have changed.
  if ((!m_Buffer.empty()) && (this->GetNumberOfInputs() != m_Buffer.front().second.size()))
  {
    decltype(m_Buffer) tmp;
    m_Buffer.swap(tmp); // Clear queue with copy-and-swap idiom
  }

  // Put current navigationdatas from input into buffer
  itk::TimeStamp now;
  now.Modified();

  std::vector<mitk::NavigationData::Pointer> ndList;
  for (unsigned int i = 0; i < this->GetNumberOfInputs() ; ++i)
  {
    mitk::NavigationData::Pointer nd = mitk::NavigationData::New();
    nd->Graft(this->GetInput(i));
    ndList.push_back(nd);
  }

  m_Buffer.push( std::make_pair(now.GetMTime(), ndList) );

  // Find most recent member from buffer that is old enough to output, considering the Delay
  // remove all sets that are too old already in the process
  BufferType current;
  bool foundCurrent = false;

  while ( (m_Buffer.size() > 0) && (m_Buffer.front().first + m_Delay <= now.GetMTime() + m_Tolerance ) )
  {
    foundCurrent = true;
    current = m_Buffer.front();
    m_Buffer.pop();
  }

  // update outputs with tracking data from previous step, or none if empty
  if ( !foundCurrent) return;

  for (unsigned int i = 0; i < this->GetNumberOfOutputs() ; ++i)
  {
    mitk::NavigationData* output = this->GetOutput(i);
    assert(output);
    const mitk::NavigationData* input = current.second[i];
    assert(input);

    if (input->IsDataValid() == false)
    {
      output->SetDataValid(false);
      continue;
    }
    output->Graft(input); // First, copy all information from input to output
    output->SetDataValid(true); // operation was successful, therefore data of output is valid.
  }
}
