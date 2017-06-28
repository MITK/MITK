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
  m_OpenIGTLinkClient->EnableInfiniteBufferingMode(false);

  m_IGTLDeviceSource = mitk::IGTLTransformDeviceSource::New();
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
  mitk::IGTLMessage::Pointer receivedMessage = ReceiveMessage(100);

  const char* msgType = receivedMessage->GetIGTLMessageType();

  mitk::OpenIGTLinkTrackingDevice::TrackingMessageType type = GetMessageTypeFromString(msgType);

  switch (type)
  {
  case TDATA:
    returnValue = DiscoverToolsFromTData(dynamic_cast<igtl::TrackingDataMessage*>(receivedMessage->GetMessage().GetPointer()));
    break;
  case QTDATA:
    returnValue = DiscoverToolsFromQTData(dynamic_cast<igtl::QuaternionTrackingDataMessage*>(receivedMessage->GetMessage().GetPointer()));
    break;
  case TRANSFORM:
    returnValue = DiscoverToolsFromTransform();
    break;
  default:
    MITK_INFO << "Server does not send tracking data. Received data is not of a compatible type. Received type: " << msgType;
  }



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

mitk::TrackingTool* mitk::OpenIGTLinkTrackingDevice::AddTool(const char* toolName, const char* fileName)
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

  mitk::NavigationToolStorage::Pointer foundTools;
  switch (type)
  {
  case TDATA:
    foundTools = DiscoverToolsFromTData(dynamic_cast<igtl::TrackingDataMessage*>(receivedMessage->GetMessage().GetPointer()));
    break;
  case QTDATA:
    foundTools = DiscoverToolsFromQTData(dynamic_cast<igtl::QuaternionTrackingDataMessage*>(receivedMessage->GetMessage().GetPointer()));
    break;
  case TRANSFORM:
    foundTools = DiscoverToolsFromTransform();
    break;
  default:
    MITK_INFO << "Server does not send tracking data. Received data is not of a compatible type. Received type: " << msgType;
    return false;
  }
  if (foundTools.IsNull() || (foundTools->GetToolCount() == 0)) { return false; }
  for (int i = 0; i < foundTools->GetToolCount(); i++) { AddNewToolForName(foundTools->GetTool(i)->GetToolName(), i); }
  MITK_INFO << "Found tools: " << foundTools->GetToolCount();
  return true;
}

mitk::IGTLMessage::Pointer mitk::OpenIGTLinkTrackingDevice::ReceiveMessage(int waitingTime)
{
  mitk::IGTLMessage::Pointer receivedMessage;
  //send a message to the server: start tracking stream
  mitk::IGTLMessageFactory::Pointer msgFactory = m_OpenIGTLinkClient->GetMessageFactory();
  std::string message = "STT_TDATA";
  igtl::MessageBase::Pointer sttMsg = msgFactory->CreateInstance(message);
  //TODO: Fix this to dynamically get this from GUI
  ((igtl::StartTrackingDataMessage*)sttMsg.GetPointer())->SetResolution(m_UpdateRate);
  m_OpenIGTLinkClient->SendMessage(mitk::IGTLMessage::New(sttMsg, message));



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

mitk::NavigationToolStorage::Pointer mitk::OpenIGTLinkTrackingDevice::DiscoverToolsFromTData(igtl::TrackingDataMessage::Pointer tdMsg)
{
  mitk::NavigationToolStorage::Pointer returnValue = mitk::NavigationToolStorage::New();
  MITK_INFO << "Start discovering tools by TDATA messages";
  if (tdMsg == nullptr)
  {
    MITK_WARN << "Message was not a TrackingDataMessage, aborting!";
    return returnValue;
  }

  int numberOfTools = tdMsg->GetNumberOfTrackingDataElements();
  MITK_INFO << "Found " << numberOfTools << " tools";
  for (int i = 0; i < numberOfTools; i++)
  {
    igtl::TrackingDataElement::Pointer currentTrackingData;
    tdMsg->GetTrackingDataElement(i, currentTrackingData);
    std::string name = currentTrackingData->GetName();

    std::stringstream identifier;
    identifier << "AutoDetectedTool-" << i;
    i++;

    mitk::NavigationTool::Pointer newTool = ConstructDefaultOpenIGTLinkTool(name, identifier.str());

    returnValue->AddTool(newTool);
  }

  m_IGTLDeviceSource->StopCommunication();
  SetState(Ready);
  return returnValue;
}

mitk::NavigationToolStorage::Pointer mitk::OpenIGTLinkTrackingDevice::DiscoverToolsFromQTData(igtl::QuaternionTrackingDataMessage::Pointer msg)
{
  mitk::NavigationToolStorage::Pointer returnValue = mitk::NavigationToolStorage::New();
  MITK_INFO << "Start discovering tools by QTDATA messages";
  if (msg == nullptr)
  {
    MITK_WARN << "Message was not a QuaternionTrackingDataMessage, aborting!";
    return returnValue;
  }
  int numberOfTools = msg->GetNumberOfQuaternionTrackingDataElements();
  MITK_INFO << "Found " << numberOfTools << " tools";
  for (int i = 0; i < numberOfTools; i++)
  {
    igtl::QuaternionTrackingDataElement::Pointer currentTrackingData;
    msg->GetQuaternionTrackingDataElement(i, currentTrackingData);
    std::string name = currentTrackingData->GetName();

    std::stringstream identifier;
    identifier << "AutoDetectedTool-" << i;
    i++;

    mitk::NavigationTool::Pointer newTool = ConstructDefaultOpenIGTLinkTool(name, identifier.str());

    returnValue->AddTool(newTool);
  }
  m_IGTLDeviceSource->StopCommunication();
  SetState(Ready);
  return returnValue;
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

mitk::NavigationToolStorage::Pointer mitk::OpenIGTLinkTrackingDevice::DiscoverToolsFromTransform(int NumberOfMessagesToWait)
{
  MITK_INFO << "Start discovering tools by TRANSFORM messages";
  mitk::NavigationToolStorage::Pointer returnValue = mitk::NavigationToolStorage::New();
  std::map<std::string, int> toolNameMap;

  for (int j=0; j<NumberOfMessagesToWait; j++)
  {
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    m_IGTLDeviceSource->Update();
    igtl::TransformMessage::Pointer msg = dynamic_cast<igtl::TransformMessage*>(m_IGTLDeviceSource->GetOutput()->GetMessage().GetPointer());
    if (msg == nullptr || msg.IsNull())
    {
      MITK_INFO << "Received message could not be casted to TransformMessage. Skipping..";
      continue;
    }

    int count = toolNameMap[msg->GetDeviceName()];
    if (count == 0)
    {
      //MITK_WARN << "ADDED NEW TOOL TO TOOLCHAIN: " << msg->GetDeviceName() << " - 1";
      toolNameMap[msg->GetDeviceName()] = 1;
    }
    else
    {
      toolNameMap[msg->GetDeviceName()]++;
      //MITK_WARN << "INCREMENTED TOOL COUNT IN TOOLCHAIN: " << msg->GetDeviceName() << " - " << toolNameMap[msg->GetDeviceName()];
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

mitk::NavigationTool::Pointer mitk::OpenIGTLinkTrackingDevice::ConstructDefaultOpenIGTLinkTool(std::string name, std::string identifier)
{
  mitk::DataNode::Pointer newNode = mitk::DataNode::New();

  newNode->SetName(name);

  mitk::Surface::Pointer myCone = mitk::Surface::New();
  vtkConeSource *vtkData = vtkConeSource::New();
  vtkData->SetAngle(5.0);
  vtkData->SetResolution(50);
  vtkData->SetHeight(6.0f);
  vtkData->SetRadius(2.0f);
  vtkData->SetCenter(0.0, 0.0, 0.0);
  vtkData->Update();
  myCone->SetVtkPolyData(vtkData->GetOutput());
  vtkData->Delete();
  newNode->SetData(myCone);

  mitk::NavigationTool::Pointer newTool = mitk::NavigationTool::New();
  newTool->SetDataNode(newNode);
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

  mitk::NavigationData::Pointer currentNavData = m_IGTLMsgToNavDataFilter->GetOutput();
  const char* name = currentNavData->GetName();
  //MITK_WARN << name;

  for (int i = 0; i < m_AllTools.size(); i++)
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
