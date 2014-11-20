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

//igt (remove this later)
#include "igtlBindMessage.h"
#include "igtlQuaternionTrackingDataMessage.h"
#include "igtlTrackingDataMessage.h"

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
  //check the type
  std::string reqType(requestType);
  bool isGetMsg = !reqType.find("GET_");
  bool isSTTMsg = !reqType.find("STT_");
  bool isSTPMsg = !reqType.find("STP_");
  bool isRTSMsg = !reqType.find("RTS_");
  //get the type from the request type (remove STT_, STP_, GET_, RTS_)
  std::string type = RemoveRequestPrefixes(requestType);
  //check all microservices if there is a fitting source for the requested type
  mitk::IGTLMessageSource::Pointer source = this->GetFittingSource(type.c_str());
  //if there is no fitting source return an RTS message, if there is a RTS
  //type defined in the message factory
  if ( source.IsNull() )
  {
    //construct the device type for the return message, it starts with RTS_ and
    //continues with the requested type
    std::string returnType("RTS_");
    returnType.append(type);
    //get the message factory. In the final version the factory shall be loaded
    //as microservice
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
    if ( isGetMsg ) //if it is a single value push it into sending queue
    {
      //first it is necessary to update the source. This needs additional time
      //but is necessary. But are we really allowed to call this here? In which
      //thread are we? Is the source thread safe?
      source->Update();
      mitk::IGTLMessage::Pointer sourceOutput = source->GetOutput();
      if (sourceOutput.IsNotNull() && sourceOutput->IsDataValid())
      {
        igtl::MessageBase::Pointer sourceMsg = sourceOutput->GetMessage();
        if ( source.IsNotNull() )
        {
          this->GetIGTLDevice()->SendMessage(sourceMsg);
        }
      }
    }
    else if ( isSTTMsg )
    {
      //if it is a stream start streaming

      //read the requested frames per second
      int fps = 10;

      //this solution is not nice but if you want to change it you have to change
      //OpenIGTLink code. I have to check the type here because the STT_ messages
      //do not have a common base class. In fact they have one MessageBase. But
      //it is necessary to have a STTMessageBase that implements SetFPS and
      //GetFPS in order to set the FPS.
      //On my request the OpenIGTLink developers wrote me that they will
      //implement this
      igtl::MessageBase* curCommandPt = curCommand.GetPointer();
      if ( std::strcmp( curCommand->GetDeviceType(), "STT_BIND" ) == 0 )
      {
        fps = ((igtl::StartBindMessage*)curCommandPt)->GetResolution();
      }
      else if ( std::strcmp( curCommand->GetDeviceType(), "STT_QTDATA" ) == 0 )
      {
        fps = ((igtl::StartQuaternionTrackingDataMessage*)curCommandPt)->GetResolution();
      }
      else if ( std::strcmp( curCommand->GetDeviceType(), "STT_TDATA" ) == 0 )
      {
        fps = ((igtl::StartTrackingDataMessage*)curCommandPt)->GetResolution();
      }

      //this is also a kind of workaround:
      //Problem: This method we are in at the moment is called from another
      //thread (not the main thread). The pipeline normally runs in the main
      //thread and has no thread-safety. Therefore, it would be a bad idea to
      //call the update method from here directly, it should be called from the
      //main thread. To call it from the main thread a timer is needed. That
      //results in two options: 1. boost timer or 2. qt timer
      //I chose the second option. So the way this works in detail is described
      //in QmitkIGTLStreamingConnector.
      //brief: Here, the fps are set. The streaming connector checks this fps
      //in the main thread. Once it is changed it adapts its timer. Everytime the
      //timer is fired the pipeline is updated and the result is added to the
      //send message queue. So the connections between the threads are the queue
      //and the fps variable. They are implemented thread safe.

      //set the fps in the current source object
      source->SetFPS(fps);
    }
    else if ( isSTPMsg )
    {
      //set the fps to 0 to stop streaming
      source->SetFPS(0);
    }
    else
    {
      //do nothing
    }
  }
}

mitk::IGTLMessageSource::Pointer mitk::IGTLMessageProvider::GetFittingSource(const char* requestedType)
{
  //get the context
  us::ModuleContext* context = us::GetModuleContext();
  //define the interface name
  std::string interface = mitk::IGTLMessageSource::US_INTERFACE_NAME;
  //specify a filter that defines the requested type
  std::string filter = "(" + mitk::IGTLMessageSource::US_PROPKEY_DEVICETYPE +
      "=" + requestedType + ")";
  //find the fitting service
  std::vector<us::ServiceReferenceU> serviceReferences =
      context->GetServiceReferences(interface, filter);

  //check if a service reference was found. It is also possible that several
  //services were found. This is not checked here, just the first one is taken.
  if ( serviceReferences.size() )
  {
    mitk::IGTLMessageSource::Pointer curSource =
        context->GetService<mitk::IGTLMessageSource>(serviceReferences.front());

    if ( curSource.IsNotNull() )
      return curSource;
  }
  //no service reference was found or found service reference has no valid source
  return NULL;
}

void mitk::IGTLMessageProvider::Send(mitk::IGTLMessage* msg)
{
  if (msg != NULL)
    this->m_IGTLDevice->SendMessage(msg);
}
