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
#include "mitkIGTLMessageFactory.h"

//#include "mitkIGTTimeStamp.h"
//#include "mitkIGTException.h"

//Microservices
#include "usServiceReference.h"
#include "usModuleContext.h"
#include "usServiceEvent.h"
#include "mitkServiceInterface.h"
#include "usGetModuleContext.h"

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
  igtl::MessageBase::Pointer msgIn = m_IGTLDevice->GetNextMessage();
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

}

std::string RemoveRequestPrefixes(std::string requestType)
{
  return requestType.substr(4);
}

void mitk::IGTLMessageProvider::OnIncomingCommand()
{
  //get the next command
  igtl::MessageBase::Pointer curCommand = this->m_IGTLDevice->GetNextCommand();
  //extract the type
  const char * requestType = curCommand->GetDeviceType();
  //get the type from the request type (remove STT_, STP_ or GET_)
  std::string type = RemoveRequestPrefixes(requestType);
  //check all microservices if there is a fitting source for the requested type
  mitk::IGTLMessageSource::Pointer source = this->GetFittingSource(type.c_str());
  //if there is no fitting source return an RTS message
  if ( source.IsNull() )
  {
    std::string returnType("RTS_");
    returnType.append(type);
    mitk::IGTLMessageFactory::Pointer msgFactory =
        this->GetIGTLDevice()->GetMessageFactory();
    mitk::IGTLMessageFactory::PointerToMessageBaseNew retMsgNew =
        msgFactory->GetMessageTypeNewPointer(returnType);
    //if retMsgNew is NULL there is no return message defined and thus it is not
    //necessary to send one back
    if ( retMsgNew != NULL )
    {
      igtl::MessageBase::Pointer rtsMsg = retMsgNew();
      this->GetIGTLDevice()->SendMessage(rtsMsg);
    }
  }
  else
  {
    //if it is a single value return the value
    source->GetOutput();
    //if it is a stream start streaming
  }
}

mitk::IGTLMessageSource::Pointer mitk::IGTLMessageProvider::GetFittingSource(const char* requestedType)
{
  us::ModuleContext* context = us::GetModuleContext();
  std::string interface = mitk::IGTLMessageSource::US_INTERFACE_NAME;
  std::string filter = "(" + us::ServiceConstants::OBJECTCLASS() + "=" + interface + ")";
  std::vector<us::ServiceReferenceU> serviceReferences =
      context->GetServiceReferences(interface, "");
  std::vector<us::ServiceReferenceU>::iterator it = serviceReferences.begin();
  std::vector<us::ServiceReferenceU>::iterator end = serviceReferences.end();
  for ( ; it != end; it++ )
  {
    mitk::IGTLMessageSource::Pointer curSource =
        context->GetService<mitk::IGTLMessageSource>(*it);
    std::string type = curSource->GetType();
    if ( std::strcmp(type.c_str(), requestedType) == 0 )
    {
      return curSource;
    }
  }
  return NULL;
}
