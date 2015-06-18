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

#include "mitkOpenIGTLinkTrackingDevice.h"
#include "mitkOpenIGTLinkTrackingTool.h"
#include "mitkIGTConfig.h"
#include "mitkIGTTimeStamp.h"
#include "mitkIGTHardwareException.h"
#include "mitkTrackingTypes.h"
#include <itksys/SystemTools.hxx>
#include <iostream>
#include <itkMutexLockHolder.h>
#include <itkCommand.h>
#include <igtlTrackingDataMessage.h>

//sleep headers
#include <chrono>
#include <thread>

typedef itk::MutexLockHolder<itk::FastMutexLock> MutexLockHolder;


mitk::OpenIGTLinkTrackingDevice::OpenIGTLinkTrackingDevice(): mitk::TrackingDevice()
{
  //set the type of this tracking device
  this->m_Data = mitk::DeviceDataOpenIGTLinkTrackingDeviceConnection;

  m_OpenIGTLinkClient = mitk::IGTLClient::New();
  m_OpenIGTLinkClient->EnableInfiniteBufferingMode(m_OpenIGTLinkClient->GetReceiveQueue(),false);
  m_OpenIGTLinkClient->SetName("OpenIGTLink Tracking Device");

  m_IGTLDeviceSource =  mitk::IGTLDeviceSource::New();
  m_IGTLDeviceSource->SetIGTLDevice(m_OpenIGTLinkClient);
}


mitk::OpenIGTLinkTrackingDevice::~OpenIGTLinkTrackingDevice()
{
}

int mitk::OpenIGTLinkTrackingDevice::GetPortNumber()
{
  return m_OpenIGTLinkClient->GetPortNumber();
}

std::string mitk::OpenIGTLinkTrackingDevice::GetHostname()
{
  return m_OpenIGTLinkClient->GetHostname();
}


void mitk::OpenIGTLinkTrackingDevice::SetPortNumber(int portNumber)
{
  m_OpenIGTLinkClient->SetPortNumber(portNumber);
}

void mitk::OpenIGTLinkTrackingDevice::SetHostname(std::string hostname)
{
  m_OpenIGTLinkClient->SetHostname(hostname);
}

bool mitk::OpenIGTLinkTrackingDevice::IsDeviceInstalled()
{
  return true;
}

mitk::TrackingTool* mitk::OpenIGTLinkTrackingDevice::AddTool( const char* toolName, const char* fileName )
{
  mitk::OpenIGTLinkTrackingTool::Pointer t;// = mitk::OpenIGTLinkTrackingTool::New();
  //TODO: Implement
  if (this->InternalAddTool(t) == false)
    return NULL;
  return t.GetPointer();
}


bool mitk::OpenIGTLinkTrackingDevice::InternalAddTool(OpenIGTLinkTrackingTool::Pointer tool)
{
  m_AllTools.push_back(tool);
  return true;
}


bool mitk::OpenIGTLinkTrackingDevice::DiscoverTools(int WaitingTime)
{
  if (m_OpenIGTLinkClient->GetPortNumber() == -1)
    {
    MITK_WARN << "Connection not initialized, aborting (invalid port number).";
    return false;
    }

  try
    {
    m_IGTLDeviceSource->Connect();
    m_IGTLDeviceSource->StartCommunication();
    }
  catch(std::runtime_error &e)
    {
    MITK_WARN << "Open IGT Link device retruned an error while trying to connect: " << e.what();
    return false;
    }

  //send a message to the server: start tracking stream
  mitk::IGTLMessageFactory::Pointer msgFactory = m_OpenIGTLinkClient->GetMessageFactory();
  std::string message = "STT_TDATA";
  igtl::MessageBase::Pointer sttMsg = msgFactory->CreateInstance(message);
  ((igtl::StartTrackingDataMessage*)sttMsg.GetPointer())->SetResolution(1);
  m_OpenIGTLinkClient->SendMessage(sttMsg);

  std::this_thread::sleep_for(std::chrono::seconds(WaitingTime));
  //Sleep(WaitingTime); //wait for data to arrive

  m_IGTLDeviceSource->Update();

  //check the tracking stream for the number and type of tools
  //igtl::MessageBase::Pointer receivedMessage = m_OpenIGTLinkClient->GetNextMessage();
  mitk::IGTLMessage::Pointer receivedMessage = m_IGTLDeviceSource->GetOutput();
  if (receivedMessage.IsNull())
    {
    MITK_WARN << "No message was received. Is there really a server?";
    return false;
    }
  else if (!receivedMessage->IsDataValid())
    {
    MITK_WARN << "Received invalid message.";
    return false;
    }

  const char* msgType = receivedMessage->GetIGTLMessageType();

  if( !(strcmp(msgType,"TDATA") == 0) )
    {
    MITK_INFO << "Server does not send tracking data. Received data is not of the type TDATA. Received type: " << msgType;
    return true;
    }


  igtl::TrackingDataMessage* tdMsg = (igtl::TrackingDataMessage*)(receivedMessage->GetMessage().GetPointer());

  if(!tdMsg)
    {
     MITK_WARN << "Cannot cast message object as expected, aborting!";
     return false;
    }

  int numberOfTools = tdMsg->GetNumberOfTrackingDataElements();
  MITK_INFO << "Found " << numberOfTools << " tools";
  for(int i=0; i<numberOfTools; i++)
    {
    mitk::OpenIGTLinkTrackingTool::Pointer newTool = mitk::OpenIGTLinkTrackingTool::New();
    igtl::TrackingDataElement::Pointer currentTrackingData;
    tdMsg->GetTrackingDataElement(i,currentTrackingData);
    std::string name = currentTrackingData->GetName();
    if (name == "") //if no name was given create a default name
      {
      std::stringstream defaultName;
      defaultName << "OpenIGTLinkTool#" << i;
      name = defaultName.str();
      }
    MITK_INFO << "Added tool " << name << " to tracking device.";
    newTool->SetToolName(name);
    this->InternalAddTool(newTool);
    }

  m_IGTLDeviceSource->StopCommunication();
  this->SetState(Ready);
  return true;
}

void mitk::OpenIGTLinkTrackingDevice::UpdateTools()
{
  if (this->GetState() != Tracking)
    {
    MITK_ERROR << "Method was called in the wrong state, something went wrong!";
    return;
    }

  m_IGTLMsgToNavDataFilter->Update();
  for (int i=0; i<this->GetToolCount(); i++)
  {
    mitk::NavigationData::Pointer currentNavData = m_IGTLMsgToNavDataFilter->GetOutput(i);
    m_AllTools.at(i)->SetDataValid(currentNavData->IsDataValid());
    m_AllTools.at(i)->SetPosition(currentNavData->GetPosition());
    m_AllTools.at(i)->SetOrientation(currentNavData->GetOrientation());
    m_AllTools.at(i)->SetIGTTimeStamp(currentNavData->GetIGTTimeStamp());


    mitk::Point3D pos;
    m_AllTools.at(i)->GetPosition(pos);

    //MITK_INFO << "Updated Tool " << i << " Pos: " << pos;
  }

}


bool mitk::OpenIGTLinkTrackingDevice::StartTracking()
{
   //check tracking state
   if (this->GetState() != Ready)
    {
    MITK_WARN << "Cannot start tracking, device is not ready!";
    return false;
    }

   try
    {
    m_IGTLDeviceSource->StartCommunication();

    //send a message to the server: start tracking stream
    mitk::IGTLMessageFactory::Pointer msgFactory = m_OpenIGTLinkClient->GetMessageFactory();
    std::string message = "STT_TDATA";
    //m_OpenIGTLinkClient->SendMessage(msgFactory->CreateInstance(message));
    }
  catch(std::runtime_error &e)
    {
    MITK_WARN << "Open IGT Link device retruned an error while starting communication: " << e.what();
    return false;
    }

  //create internal igtl pipeline
  m_IGTLMsgToNavDataFilter =  mitk::IGTLMessageToNavigationDataFilter::New();
  m_IGTLMsgToNavDataFilter->SetNumberOfExpectedOutputs(this->GetToolCount());
  m_IGTLMsgToNavDataFilter->ConnectTo(m_IGTLDeviceSource);


  //connect itk events
  typedef itk::SimpleMemberCommand< mitk::OpenIGTLinkTrackingDevice > CurCommandType;
  CurCommandType::Pointer messageReceivedCommand = CurCommandType::New();
  messageReceivedCommand->SetCallbackFunction(this, &mitk::OpenIGTLinkTrackingDevice::UpdateTools);
  m_MessageReceivedObserverTag = m_OpenIGTLinkClient->AddObserver(mitk::MessageReceivedEvent(),messageReceivedCommand);

  this->SetState(Tracking);
  return true;
}


bool mitk::OpenIGTLinkTrackingDevice::StopTracking()
{
  //check tracking state
  if (this->GetState() != Tracking)
    {
    MITK_WARN << "Cannot open connection, device is already connected!";
    return false;
    }

  m_OpenIGTLinkClient->RemoveObserver(m_MessageReceivedObserverTag); //disconnect itk events

  try
    {
    m_IGTLDeviceSource->StopCommunication();
    }
  catch(std::runtime_error &e)
    {
    MITK_WARN << "Open IGT Link device retruned an error while stopping communication: " << e.what();
    return false;
    }
  this->SetState(Ready);
  return true;
}


unsigned int mitk::OpenIGTLinkTrackingDevice::GetToolCount() const
{
  return (unsigned int)this->m_AllTools.size();
}


mitk::TrackingTool* mitk::OpenIGTLinkTrackingDevice::GetTool(unsigned int toolNumber) const
{
  if ( toolNumber >= this->GetToolCount())
    return NULL;
  else
    return this->m_AllTools[toolNumber];
}


bool mitk::OpenIGTLinkTrackingDevice::OpenConnection()
{
  //check tracking state
  if (this->GetState() != Setup)
    {
    MITK_WARN << "Cannot open connection, device is already connected!";
    return false;
    }

  try
    {
    m_IGTLDeviceSource->Connect();
    }
  catch(std::runtime_error &e)
    {
    MITK_WARN << "Open IGT Link device retruned an error while trying to connect: " << e.what();
    return false;
    }
  this->SetState(Ready);
  return true;
}


bool mitk::OpenIGTLinkTrackingDevice::CloseConnection()
{
  //check tracking state
  if (this->GetState() != Ready)
    {
    MITK_WARN << "Cannot close connection, device is in the wrong state!";
    return false;
    }

  try
    {
    m_IGTLDeviceSource->Disconnect();
    }
  catch(std::runtime_error &e)
    {
    MITK_WARN << "Open IGT Link device retruned an error while trying to disconnect: " << e.what();
    return false;
    }

  this->SetState(Setup);

  return true;
}

std::vector<mitk::OpenIGTLinkTrackingTool::Pointer> mitk::OpenIGTLinkTrackingDevice::GetAllTools()
{
  return this->m_AllTools;
}
