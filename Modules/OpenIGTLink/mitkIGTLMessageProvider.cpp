/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkIGTLMessageProvider.h"

#include "mitkIGTLDevice.h"
#include "mitkIGTLMessage.h"
#include "mitkIGTLMessageFactory.h"

#include "mitkCallbackFromGUIThread.h"

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

#ifndef WIN32
#include <unistd.h>
#endif

namespace mitk
{
  itkEventMacroDefinition(StreamingStartRequiredEvent, itk::AnyEvent);
  itkEventMacroDefinition(StreamingStopRequiredEvent, itk::AnyEvent);
}

mitk::IGTLMessageProvider::IGTLMessageProvider()
  : mitk::IGTLDeviceSource()
{
  this->SetName("IGTLMessageProvider");
  m_IsStreaming = false;

  // Create a command object. The function will be called later from the main thread
  this->m_StopStreamingCommand = ProviderCommand::New();
  m_StopStreamingCommand->SetCallbackFunction(this,
    &mitk::IGTLMessageProvider::InvokeStopStreamingEvent);

  this->m_StreamingCommand = ProviderCommand::New();
  m_StreamingCommand->SetCallbackFunction(this,
    &mitk::IGTLMessageProvider::InvokeStartStreamingEvent);
}

mitk::IGTLMessageProvider::~IGTLMessageProvider()
{
  this->InvokeEvent(StreamingStartRequiredEvent());
}

void mitk::IGTLMessageProvider::Update()
{

  Superclass::Update();

  if (this->GetInput() != nullptr)
  {
    igtl::MessageBase::Pointer curMessage = this->GetInput()->GetMessage();
    if (dynamic_cast<igtl::TrackingDataMessage*>(curMessage.GetPointer()) != nullptr)
    {
      igtl::TrackingDataMessage* tdMsg =
        (igtl::TrackingDataMessage*)(curMessage.GetPointer());
      igtl::TrackingDataElement::Pointer trackingData = igtl::TrackingDataElement::New();
      tdMsg->GetTrackingDataElement(0, trackingData);
      float x_pos, y_pos, z_pos;
      trackingData->GetPosition(&x_pos, &y_pos, &z_pos);
    }
  }
}

void mitk::IGTLMessageProvider::GenerateData()
{
  if (this->m_IGTLDevice.IsNull())
    return;

  for (unsigned int index = 0; index < this->GetNumberOfIndexedInputs(); index++)
  {
    mitk::IGTLMessage::Pointer msg = const_cast<mitk::IGTLMessage*>(this->GetInput(index));
    if (msg == nullptr)
    {
       continue;
    }

    if ( !msg->IsDataValid() )
    {
      continue;
    }

    this->m_IGTLDevice->SendMessage(msg);
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
  if (this->GetOutput(0) == nullptr)
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

void mitk::IGTLMessageProvider::OnLostConnection()
{
  //in case the provider is streaming at the moment we have to stop it
  if (m_IsStreaming)
  {
    MITK_DEBUG("IGTLMessageProvider") << "lost connection, stop streaming";
    this->StopStreamingOfAllSources();
  }
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

  //get the type from the request type (remove STT_, STP_, GET_, RTS_)
  std::string type = RemoveRequestPrefixes(requestType);
  //check all microservices if there is a fitting source for the requested type
  mitk::IGTLMessageSource::Pointer source = this->GetFittingSource(type.c_str());
  //if there is no fitting source return a RTS message, if there is a RTS
  //type defined in the message factory send it
  if ( source.IsNull() )
  {
    if ( !this->GetIGTLDevice()->SendRTSMessage(type.c_str()) )
    {
      //sending RTS message failed, probably because the type is not in the
      //message factory
      MITK_WARN("IGTLMessageProvider") << "Tried to send a RTS message but did "
                                          "not succeed. Check if this type ( "
                                       << type << " ) was added to the message "
                                          "factory. ";
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
        if ( source.IsNotNull() )
        {
          this->GetIGTLDevice()->SendMessage(sourceOutput);
        }
      }
    }
    else if ( isSTTMsg )
    {
      //read the requested frames per second
      int fps = 10;

      //read the fps from the command
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

      this->StartStreamingOfSource(source, fps);
    }
    else if ( isSTPMsg )
    {
      this->StopStreamingOfSource(source);
    }
    else
    {
      //do nothing
    }
  }
}

bool mitk::IGTLMessageProvider::IsStreaming()
{
  return m_IsStreaming;
}

void mitk::IGTLMessageProvider::StartStreamingOfSource(IGTLMessageSource* src,
                                                       unsigned int fps)
{
  if ( src == nullptr )
    return;

  //so far the provider allows the streaming of a single source only
  //if the streaming thread is already running return a RTS message
  if ( !m_IsStreaming )
  {
    //if it is a stream establish a connection between the provider and the
    //source
    this->ConnectTo(src);

    // calculate the streaming time
    this->m_StreamingTimeMutex.lock();
    this->m_StreamingTime = 1.0 / (double) fps * 1000.0;
    this->m_StreamingTimeMutex.unlock();

    //// For streaming we need a continues time signal, since there is no timer
    //// available we start a thread that generates a timing signal
    //// This signal is invoked from the other thread the update of the pipeline
    //// has to be executed from the main thread. Thus, we use the
    //// callbackfromGUIThread class to pass the execution to the main thread
    //this->m_ThreadId = m_MultiThreader->SpawnThread(this->TimerThread, this);

    mitk::CallbackFromGUIThread::GetInstance()->CallThisFromGUIThread(
          this->m_StreamingCommand);

    this->m_IsStreaming = true;
  }
  else
  {
    MITK_WARN("IGTLMessageProvider") << "This provider just supports the "
                                        "streaming of one source.";
  }
}

void mitk::IGTLMessageProvider::InvokeStartStreamingEvent()
{
  this->InvokeEvent(StreamingStartRequiredEvent());
}

void mitk::IGTLMessageProvider::InvokeStopStreamingEvent()
{
  this->InvokeEvent(StreamingStopRequiredEvent());
}

void mitk::IGTLMessageProvider::StopStreamingOfSource(IGTLMessageSource* src)
{
  //this is something bad!!! The streaming thread has to be stopped before the
  //source is disconnected otherwise it can cause a crash. This has to be added!!
  this->DisconnectFrom(src);

  mitk::CallbackFromGUIThread::GetInstance()->CallThisFromGUIThread(
    this->m_StopStreamingCommand);

  //does this flag needs a mutex???
  this->m_IsStreaming = false;
}

void mitk::IGTLMessageProvider::StopStreamingOfAllSources()
{
  // \todo remove all inputs

  mitk::CallbackFromGUIThread::GetInstance()->CallThisFromGUIThread(
    this->m_StopStreamingCommand);

  //does this flag needs a mutex???
  this->m_IsStreaming = false;
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
  return nullptr;
}

void mitk::IGTLMessageProvider::Send(mitk::IGTLMessage::Pointer msg)
{
  if (msg != nullptr)
  {
    MITK_INFO << "Sending OpenIGTLink Message: " << msg->ToString();
    this->m_IGTLDevice->SendMessage(msg);
  }
}

void
mitk::IGTLMessageProvider::ConnectTo( mitk::IGTLMessageSource* UpstreamFilter )
{
  for (DataObjectPointerArraySizeType i = 0;
       i < UpstreamFilter->GetNumberOfOutputs(); i++)
  {
    this->SetInput(i, UpstreamFilter->GetOutput(i));
  }
}

void
mitk::IGTLMessageProvider::DisconnectFrom( mitk::IGTLMessageSource* UpstreamFilter )
{
  if (UpstreamFilter == nullptr)
    return;

  for (DataObjectPointerArraySizeType i = 0; i < UpstreamFilter->GetNumberOfOutputs(); ++i)
  {
    auto input = UpstreamFilter->GetOutput(i);

    if (input == nullptr)
      continue;

    auto nb = this->GetNumberOfIndexedInputs();

    for (DataObjectPointerArraySizeType i = 0; i < nb; ++i)
    {
      if (this->GetInput(i) == input)
      {
        this->RemoveInput(i);
        break;
      }
    }
  }
}
