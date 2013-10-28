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

#include "mitkDelayFilter.h"

/**mitk::DelayFilter::DataObjectPointer mitk::DelayFilter::MakeOutput ( DataObjectPointerArraySizeType )
{
mitkThrow() << "Unsupported Operation. Please override method in sublclass if you need this functionality.";
}

mitk::DelayFilter::DataObjectPointer mitk::DelayFilter::MakeOutput( const DataObjectIdentifierType& )
{
mitkThrow() << "Unsupported Operation. Please override method in sublclass if you need this functionality.";
}
*/

void mitk::DelayFilter::GenerateData()
{
  // Check if number of outputs has changed since the previous call. If yes, reset buffer.
  // This actually compares the number of Navigation Datas in each step and compares it to the current number of inputs.
  // If these values differ, the number of inputrs have changed.
  if ( (m_Buffer.size() > 0) &&  (this->GetNumberOfInputs() != m_Buffer.front().second.size()) )
    m_Buffer.clear();

  // Put current navigationdatas from input into buffer
  itk::TimeStamp timestamp;
  timestamp.Modified();

  std::vector<mitk::NavigationData::Pointer> ndList;
  for (unsigned int i = 0; i < this->GetNumberOfInputs() ; ++i)
  {
    ndList.push_back( this->GetInput(i)->Clone());
  }

  m_Buffer.push_back( std::make_pair(timestamp, ndList) );

  /* update outputs with tracking data from tools */
  for (unsigned int i = 0; i < this->GetNumberOfOutputs() ; ++i)
  {
    mitk::NavigationData* output = this->GetOutput(i);
    assert(output);
    const mitk::NavigationData* input = this->GetInput(i);
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