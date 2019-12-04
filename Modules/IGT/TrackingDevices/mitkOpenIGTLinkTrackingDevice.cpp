/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

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
#include <mitkOpenIGTLinkTypeInformation.h>
#include <vtkConeSource.h>

//sleep headers
#include <chrono>
#include <thread>

typedef itk::MutexLockHolder<itk::FastMutexLock> MutexLockHolder;

mitk::OpenIGTLinkTrackingDevice::OpenIGTLinkTrackingDevice() : mitk::TrackingDevice(), m_UpdateRate(60)
{
  //set the type of this tracking device
  this->m_Data = mitk::OpenIGTLinkTypeInformation::GetDeviceDataOpenIGTLinkTrackingDeviceConnection();

  m_OpenIGTLinkClient = mitk::IGTLClient::New(true);
  m_OpenIGTLinkClient->SetName("OpenIGTLink Tracking Device");
  m_OpenIGTLinkClient->EnableNoBufferingMode(false);

  m_IGTLDeviceSource = mitk::IGTLTrackingDataDeviceSource::New();
  m_IGTLDeviceSource->SetIGTLDevice(m_OpenIGTLinkClient);
}

mitk::OpenIGTLinkTrackingDevice::~OpenIGTLinkTrackingDevice()
{
}

int mitk::OpenIGTLinkTrackingDevice::GetPortNumber()
{
  return m_OpenIGTLinkClient->GetPortNumber();
}

bool mitk::OpenIGTLinkTrackingDevice::AutoDetectToolsAvailable()
{
  return true;
}

mitk::NavigationToolStorage::Pointer mitk::OpenIGTLinkTrackingDevice::AutoDetectTools()
{
  mitk::NavigationToolStorage::Pointer returnValue = mitk::NavigationToolStorage::New();

  if (m_OpenIGTLinkClient->GetPortNumber() == -1)
  {
    MITK_WARN << "Connection not initialized, aborting (invalid port number).";
    return mitk::NavigationToolStorage::New();
  }

  //open connection
  try
  {
    m_IGTLDeviceSource->Connect();
    m_IGTLDeviceSource->StartCommunication();
  }
  catch (std::runtime_error &e)
  {
    MITK_WARN << "AutoDetection: Open IGT Link device retruned an error while trying to connect: " << e.what();
    return mitk::NavigationToolStorage::New();
  }

  //get a message to find out type
  m_IGTLDeviceSource->SettrackingDataType(mitk::IGTLTrackingDataDeviceSource::UNKNOWN);
  mitk::IGTLMessage::Pointer receivedMessage = ReceiveMessage(100);

  const char* msgType = receivedMessage->GetIGTLMessageType();

  if (std::string(msgType).empty())
  {
    MITK_INFO << "Did not receive a message. Do you have to start the stream manually at the server?";
    MITK_INFO << "Waiting for 10 seconds ...";
    receivedMessage = ReceiveMessage(10000);
    msgType = receivedMessage->GetIGTLMessageType();
  }
  MITK_INFO << "################# got message type: " << msgType;
  mitk::OpenIGTLinkTrackingDevice::TrackingMessageType type = GetMessageTypeFromString(msgType);
  switch (type)
  {
  case UNKNOWN:
    m_IGTLDeviceSource->SettrackingDataType(mitk::IGTLTrackingDataDeviceSource::UNKNOWN);
    break;
  case TDATA:
    m_IGTLDeviceSource->SettrackingDataType(mitk::IGTLTrackingDataDeviceSource::TDATA);
    break;
  case QTDATA:
    m_IGTLDeviceSource->SettrackingDataType(mitk::IGTLTrackingDataDeviceSource::QTDATA);
    break;
  case TRANSFORM:
    m_IGTLDeviceSource->SettrackingDataType(mitk::IGTLTrackingDataDeviceSource::TRANSFORM);
    break;
  }
  returnValue = DiscoverToolsAndConvertToNavigationTools(type);

  //close connection
  try
  {
    m_IGTLDeviceSource->StopCommunication();
    m_IGTLDeviceSource->Disconnect();
  }
  catch (std::runtime_error &e)
  {
    MITK_WARN << "AutoDetection: Open IGT Link device retruned an error while trying to disconnect: " << e.what();
    return mitk::NavigationToolStorage::New();
  }


  return returnValue;
}

mitk::NavigationToolStorage::Pointer mitk::OpenIGTLinkTrackingDevice::DiscoverToolsAndConvertToNavigationTools(mitk::OpenIGTLinkTrackingDevice::TrackingMessageType type, int NumberOfMessagesToWait)
{
  MITK_INFO << "Start discovering tools by " << type << " messages";
  mitk::NavigationToolStorage::Pointer returnValue = mitk::NavigationToolStorage::New();
  std::map<std::string, int> toolNameMap;

  for (int j = 0; j<NumberOfMessagesToWait; j++)
  {
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    m_IGTLDeviceSource->Update();
    switch (type)
    {
    case TRANSFORM:
      {
      igtl::TransformMessage::Pointer msg = dynamic_cast<igtl::TransformMessage*>(m_IGTLDeviceSource->GetOutput()->GetMessage().GetPointer());
      if (msg == nullptr || msg.IsNull()) { MITK_INFO << "Received message is invalid / null. Skipping.."; continue; }
      int count = toolNameMap[msg->GetDeviceName()];
      if (count == 0) { toolNameMap[msg->GetDeviceName()] = 1; }
      else { toolNameMap[msg->GetDeviceName()]++; }
      }
      break;
    case TDATA:
      {
      igtl::TrackingDataMessage::Pointer msg = dynamic_cast<igtl::TrackingDataMessage*>(m_IGTLDeviceSource->GetOutput()->GetMessage().GetPointer());
      if (msg == nullptr || msg.IsNull()) { MITK_INFO << "Received message is invalid / null. Skipping.."; continue; }
      for (int k = 0; k < msg->GetNumberOfTrackingDataElements(); k++)
        {
        igtl::TrackingDataElement::Pointer tde;
        msg->GetTrackingDataElement(k, tde);
        if (tde.IsNotNull())
          {
          int count = toolNameMap[tde->GetName()];
          if (count == 0) { toolNameMap[tde->GetName()] = 1; }
          else { toolNameMap[tde->GetName()]++; }
          }
        }
      }
      break;
    default:
      MITK_WARN << "Only TRANSFORM and TDATA is currently supported, skipping!";
      break;
    }
  }

  int i = 0;
  for (std::map<std::string, int>::iterator it = toolNameMap.begin(); it != toolNameMap.end(); ++it)
  {
    MITK_INFO << "Found tool: " << it->first;

    std::stringstream name;
    name << it->first;

    std::stringstream identifier;
    identifier << "AutoDetectedTool-" << i;
    i++;

    mitk::NavigationTool::Pointer newTool = ConstructDefaultOpenIGTLinkTool(name.str(), identifier.str());

    returnValue->AddTool(newTool);
  }

  return returnValue;
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

mitk::TrackingTool* mitk::OpenIGTLinkTrackingDevice::AddTool(const char*, const char*)
{
  mitk::OpenIGTLinkTrackingTool::Pointer t;// = mitk::OpenIGTLinkTrackingTool::New();
  //TODO: Implement
  if (this->InternalAddTool(t) == false)
    return nullptr;
  return t.GetPointer();
}

bool mitk::OpenIGTLinkTrackingDevice::InternalAddTool(OpenIGTLinkTrackingTool::Pointer tool)
{
  m_AllTools.push_back(tool);
  return true;
}

bool mitk::OpenIGTLinkTrackingDevice::DiscoverTools(int waitingTime)
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
  catch (std::runtime_error &e)
  {
    MITK_WARN << "Open IGT Link device retruned an error while trying to connect: " << e.what();
    return false;
  }

  mitk::IGTLMessage::Pointer receivedMessage = ReceiveMessage(waitingTime);

  //check the tracking stream for the number and type of tools
  //igtl::MessageBase::Pointer receivedMessage = m_OpenIGTLinkClient->GetNextMessage();
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

  mitk::OpenIGTLinkTrackingDevice::TrackingMessageType type = GetMessageTypeFromString(msgType);

  mitk::NavigationToolStorage::Pointer foundTools = this->DiscoverToolsAndConvertToNavigationTools(type);
  if (foundTools.IsNull() || (foundTools->GetToolCount() == 0)) { return false; }
  for (unsigned int i = 0; i < foundTools->GetToolCount(); i++) { AddNewToolForName(foundTools->GetTool(i)->GetToolName(), i); }
  MITK_INFO << "Found tools: " << foundTools->GetToolCount();
  return true;
}

mitk::IGTLMessage::Pointer mitk::OpenIGTLinkTrackingDevice::ReceiveMessage(int waitingTime)
{
  mitk::IGTLMessage::Pointer receivedMessage;
  //send a message to the server: start tracking stream
  mitk::IGTLMessageFactory::Pointer msgFactory = m_OpenIGTLinkClient->GetMessageFactory();
  std::string message[2] = {"STT_QTDATA","STT_TDATA"};

  for (int i = 0; i < 2; i++)
  {
    igtl::MessageBase::Pointer sttMsg = msgFactory->CreateInstance(message[i]);
    //TODO: Fix this to dynamically get this from GUI
    ((igtl::StartTrackingDataMessage*)sttMsg.GetPointer())->SetResolution(m_UpdateRate);
    m_OpenIGTLinkClient->SendMessage(mitk::IGTLMessage::New(sttMsg));
  }

  std::chrono::high_resolution_clock::time_point time = std::chrono::high_resolution_clock::now();
  std::chrono::milliseconds d = std::chrono::milliseconds(waitingTime);

  while (!(receivedMessage.IsNotNull() && receivedMessage->IsDataValid()))
  {
    m_IGTLDeviceSource->Update();
    receivedMessage = m_IGTLDeviceSource->GetOutput();

    if ((time + d) < std::chrono::high_resolution_clock::now())
      break;

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }
  return receivedMessage;
}

void mitk::OpenIGTLinkTrackingDevice::AddNewToolForName(std::string name, int i)
{
  mitk::OpenIGTLinkTrackingTool::Pointer newTool = mitk::OpenIGTLinkTrackingTool::New();
  if (name == "") //if no name was given create a default name
  {
    std::stringstream defaultName;
    defaultName << "OpenIGTLinkTool#" << i;
    name = defaultName.str();
  }
  MITK_INFO << "Added tool " << name << " to tracking device.";
  newTool->SetToolName(name);
  InternalAddTool(newTool);
}

mitk::NavigationTool::Pointer mitk::OpenIGTLinkTrackingDevice::ConstructDefaultOpenIGTLinkTool(std::string name, std::string identifier)
{
  mitk::NavigationTool::Pointer newTool = mitk::NavigationTool::New();
  newTool->GetDataNode()->SetName(name);
  newTool->SetIdentifier(identifier);

  newTool->SetTrackingDeviceType(mitk::OpenIGTLinkTypeInformation::GetDeviceDataOpenIGTLinkTrackingDeviceConnection().Line);

  return newTool;

}

void mitk::OpenIGTLinkTrackingDevice::UpdateTools()
{
  if (this->GetState() != Tracking)
  {
    MITK_ERROR << "Method was called in the wrong state, something went wrong!";
    return;
  }

  m_IGTLMsgToNavDataFilter->Update();

  for (std::size_t j = 0; j < m_IGTLMsgToNavDataFilter->GetNumberOfIndexedOutputs(); ++j)
  {
    mitk::NavigationData::Pointer currentNavData = m_IGTLMsgToNavDataFilter->GetOutput(j);
    const char* name = currentNavData->GetName();
    for (std::size_t i = 0; i < m_AllTools.size(); i++)
    {
      if (strcmp(m_AllTools.at(i)->GetToolName(), name) == 0)
      {
        m_AllTools.at(i)->SetDataValid(currentNavData->IsDataValid());
        m_AllTools.at(i)->SetPosition(currentNavData->GetPosition());
        m_AllTools.at(i)->SetOrientation(currentNavData->GetOrientation());
        m_AllTools.at(i)->SetIGTTimeStamp(currentNavData->GetIGTTimeStamp());
      }
    }
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
  catch (std::runtime_error &e)
  {
    MITK_WARN << "Open IGT Link device retruned an error while starting communication: " << e.what();
    return false;
  }

  //create internal igtl pipeline
  m_IGTLMsgToNavDataFilter = mitk::IGTLMessageToNavigationDataFilter::New();
  m_IGTLMsgToNavDataFilter->SetNumberOfExpectedOutputs(this->GetToolCount());
  m_IGTLMsgToNavDataFilter->ConnectTo(m_IGTLDeviceSource);

  //connect itk events
  typedef itk::SimpleMemberCommand< mitk::OpenIGTLinkTrackingDevice > CurCommandType;
  CurCommandType::Pointer messageReceivedCommand = CurCommandType::New();
  messageReceivedCommand->SetCallbackFunction(this, &mitk::OpenIGTLinkTrackingDevice::UpdateTools);
  m_MessageReceivedObserverTag = m_OpenIGTLinkClient->AddObserver(mitk::MessageReceivedEvent(), messageReceivedCommand);

  m_OpenIGTLinkClient->EnableNoBufferingMode(true);
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
  catch (std::runtime_error &e)
  {
    MITK_WARN << "Open IGT Link device retruned an error while stopping communication: " << e.what();
    return false;
  }
  m_OpenIGTLinkClient->EnableNoBufferingMode(false);
  this->SetState(Ready);
  return true;
}

unsigned int mitk::OpenIGTLinkTrackingDevice::GetToolCount() const
{
  return (unsigned int)this->m_AllTools.size();
}

mitk::TrackingTool* mitk::OpenIGTLinkTrackingDevice::GetTool(unsigned int toolNumber) const
{
  if (toolNumber >= this->GetToolCount())
    return nullptr;
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
  catch (std::runtime_error &e)
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
  catch (std::runtime_error &e)
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

mitk::OpenIGTLinkTrackingDevice::TrackingMessageType mitk::OpenIGTLinkTrackingDevice::GetMessageTypeFromString(const char* messageTypeString)
{
  if (strcmp(messageTypeString, "TDATA") == 0)
  {
    return mitk::OpenIGTLinkTrackingDevice::TrackingMessageType::TDATA;
  }
  else if (strcmp(messageTypeString, "QTDATA") == 0)
  {
    return mitk::OpenIGTLinkTrackingDevice::TrackingMessageType::QTDATA;
  }
  else if (strcmp(messageTypeString, "TRANSFORM") == 0)
  {
    return mitk::OpenIGTLinkTrackingDevice::TrackingMessageType::TRANSFORM;
  }
  else
  {
    return mitk::OpenIGTLinkTrackingDevice::TrackingMessageType::UNKNOWN;
  }
}
