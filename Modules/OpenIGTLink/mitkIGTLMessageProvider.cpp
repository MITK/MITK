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

#include "mitkIGTLMessageProvider.h"

#include "mitkIGTLDevice.h"
#include "mitkIGTLMessage.h"

//#include "mitkIGTTimeStamp.h"
//#include "mitkIGTException.h"

mitk::IGTLMessageProvider::IGTLMessageProvider()
  : mitk::IGTLDeviceSource()
{
  this->SetName("IGTLMessageProvider");
}

mitk::IGTLMessageProvider::~IGTLMessageProvider()
{

}

void mitk::IGTLMessageProvider::GenerateData()
{
  if (m_IGTLDevice.IsNull())
    return;

  /* update output with message from the device */
  IGTLMessage* msgOut = this->GetOutput();
  assert(msgOut);
  igtl::MessageBase::Pointer msgIn = m_IGTLDevice->GetLatestMessage();
  if ( msgIn.IsNotNull() )
  {
    assert(msgIn);

    msgOut->SetMessage(msgIn);
    msgOut->SetName(msgIn->GetDeviceName());
  }
}

void mitk::IGTLMessageProvider::CreateOutputs()
{
  //if outputs are set then delete them
  if (this->GetNumberOfOutputs() > 0)
  {
    for (int numOP = this->GetNumberOfOutputs() - 1; numOP >= 0; numOP--)
      this->RemoveOutput(numOP);
    this->Modified();
  }

  //fill the outputs if a valid OpenIGTLink device is set
  if (m_IGTLDevice.IsNull())
    return;

  this->SetNumberOfIndexedOutputs(1);
  if (this->GetOutput(0) == NULL)
  {
    DataObjectPointer newOutput = this->MakeOutput(0);
    this->SetNthOutput(0, newOutput);
    this->Modified();
  }
}

//void mitk::IGTLMessageProvider::UpdateOutputInformation()
//{
//  this->Modified();  // make sure that we need to be updated
//  Superclass::UpdateOutputInformation();
//}


void mitk::IGTLMessageProvider::OnIncomingMessage()
{
  //check type of incoming message, it must be a request type (GET_ or STT_)
  //otherwise ignore it
  //check all microservices if there is a fitting source for the requested type
  //if it is a single value return the value
  //if it is a stream start streaming
}
