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

mitk::IGTLMessageProvider::IGTLMessageProvider()
  : mitk::IGTLDeviceSource()
{
  this->SetName("IGTLMessageProvider");
  m_MultiThreader = itk::MultiThreader::New();
  m_StreamingTimeMutex = itk::FastMutexLock::New();
  m_StopStreamingThreadMutex = itk::FastMutexLock::New();
  m_ThreadId = 0;
  m_IsStreaming = false;
}

mitk::IGTLMessageProvider::~IGTLMessageProvider()
{
  // terminate worker thread on destruction
  this->m_StopStreamingThreadMutex->Lock();
  this->m_StopStreamingThread = true;
  this->m_StopStreamingThreadMutex->Unlock();
  if ( m_ThreadId >= 0)
  {
    this->m_MultiThreader->TerminateThread(m_ThreadId);
  }
}

void mitk::IGTLMessageProvider::GenerateData()
{
  if (this->m_IGTLDevice.IsNull())
    return;

  for (unsigned int index = 0; index < this->GetNumberOfIndexedInputs(); index++)
  {
    const IGTLMessage* msg = this->GetInput(index);
    assert(msg);

    if ( !msg->IsDataValid() )
    {
      continue;
    }

    igtl::MessageBase::Pointer igtlMsg = msg->GetMessage();

    if ( igtlMsg.IsNotNull() )
    {
      //send the message
      this->m_IGTLDevice->SendMessage(igtlMsg);
    }
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
        igtl::MessageBase::Pointer sourceMsg = sourceOutput->GetMessage();
        if ( source.IsNotNull() )
        {
          this->GetIGTLDevice()->SendMessage(sourceMsg);
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
  if ( src == NULL )
    return;

  //so far the provider allows the streaming of a single source only
  //if the streaming thread is already running return a RTS message
  if ( !m_IsStreaming )
  {
    //if it is a stream establish a connection between the provider and the
    //source
    this->ConnectTo(src);

    // calculate the streaming time
    this->m_StreamingTimeMutex->Lock();
    this->m_StreamingTime = 1.0 / (double) fps * 1000.0;
    this->m_StreamingTimeMutex->Unlock();

    // Create a command object. The function will be called later from the
    // main thread
    this->m_StreamingCommand = ProviderCommand::New();
    m_StreamingCommand->SetCallbackFunction(this,
        &mitk::IGTLMessageProvider::Update);

    // For streaming we need a continues time signal, since there is no timer
    // available we start a thread that generates a timing signal
    // This signal is invoked from the other thread the update of the pipeline
    // has to be executed from the main thread. Thus, we use the
    // callbackfromGUIThread class to pass the execution to the main thread
    this->m_ThreadId = m_MultiThreader->SpawnThread(this->TimerThread, this);
    this->m_IsStreaming = true;
  }
  else
  {
    MITK_WARN("IGTLMessageProvider") << "This provider just supports the "
                                        "streaming of one source.";
  }
}

void mitk::IGTLMessageProvider::StopStreamingOfSource(IGTLMessageSource* src)
{
  //this is something bad!!! The streaming thread has to be stopped before the
  //source is disconnected otherwise it can cause a crash. This has to be added!!
  this->DisconnectFrom(src);

  this->m_StopStreamingThreadMutex->Lock();
  this->m_StopStreamingThread = true;
  this->m_StopStreamingThreadMutex->Unlock();

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
  return NULL;
}

void mitk::IGTLMessageProvider::Send(const mitk::IGTLMessage* msg)
{
  if (msg != NULL)
    this->m_IGTLDevice->SendMessage(msg);
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
  for (DataObjectPointerArraySizeType i = 0;
       i < UpstreamFilter->GetNumberOfOutputs(); i++)
  {
    this->RemoveInput(UpstreamFilter->GetOutput(i));
  }
}

ITK_THREAD_RETURN_TYPE mitk::IGTLMessageProvider::TimerThread(void* pInfoStruct)
{
  // extract this pointer from thread info structure
  struct itk::MultiThreader::ThreadInfoStruct * pInfo =
      (struct itk::MultiThreader::ThreadInfoStruct*)pInfoStruct;
  mitk::IGTLMessageProvider* thisObject =
      static_cast<mitk::IGTLMessageProvider*>(pInfo->UserData);

  itk::SimpleMutexLock mutex;
  mutex.Lock();

  thisObject->m_StopStreamingThreadMutex->Lock();
  thisObject->m_StopStreamingThread = false;
  thisObject->m_StopStreamingThreadMutex->Unlock();

  thisObject->m_StreamingTimeMutex->Lock();
  unsigned int waitingTime = thisObject->m_StreamingTime;
  thisObject->m_StreamingTimeMutex->Unlock();

  while (true)
  {
    thisObject->m_StopStreamingThreadMutex->Lock();
    bool stopThread = thisObject->m_StopStreamingThread;
    thisObject->m_StopStreamingThreadMutex->Unlock();

    if (stopThread)
    {
      break;
    }

    //wait for the time given
    //I know it is not the nicest solution but we just need an approximate time
    //sleeps for 20 ms
    #if defined (WIN32) || defined (_WIN32)
    Sleep(waitingTime);
    #else
    usleep(waitingTime * 1000);
    #endif

    // Ask to execute that command from the GUI thread
    mitk::CallbackFromGUIThread::GetInstance()->CallThisFromGUIThread(
          thisObject->m_StreamingCommand);
  }

  thisObject->m_ThreadId = 0;

  mutex.Unlock();

  return ITK_THREAD_RETURN_VALUE;
}
