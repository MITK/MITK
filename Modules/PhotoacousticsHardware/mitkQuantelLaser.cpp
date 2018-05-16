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

#include "mitkQuantelLaser.h"

#include <chrono>
#include <thread>
#include <mutex>

#include <tinyxml.h>

#include <usModuleContext.h>
#include <usGetModuleContext.h>

const unsigned char CR = 0xD; // == '\r' - carriage return
const unsigned char LF = 0xA; // == '\n' - line feed

mitk::QuantelLaser::QuantelLaser() :
  m_State(mitk::QuantelLaser::UNCONNECTED),
  m_FlashlampRunning(false),
  m_ShutterOpen(false),
  m_LaserEmission(false),
  m_DeviceName(),
  m_PortNumber(mitk::SerialCommunication::COM6), 
  m_BaudRate(mitk::SerialCommunication::BaudRate115200),
  m_DataBits(mitk::SerialCommunication::DataBits8), 
  m_Parity(mitk::SerialCommunication::None), 
  m_StopBits(mitk::SerialCommunication::StopBits1),
  m_HardwareHandshake(mitk::SerialCommunication::HardwareHandshakeOff),
  m_SerialCommunication(nullptr),
  m_StayAliveMessageThread(),
  m_SerialCommunicationMutex()
{
}

mitk::QuantelLaser::~QuantelLaser()
{
  /* stop tracking and disconnect from tracking device */
  if ((GetState() == STATE3) || (GetState() == STATE4) || (GetState() == STATE5) || (GetState() == STATE6))
  {
    this->StopQswitching();
    this->StopFlashing();
  }
  if (GetState() == STATE2)
  {
    this->CloseConnection();
  }
  /* cleanup stay alive thread */
  if (m_StayAliveMessageThread.joinable()) 
    m_StayAliveMessageThread.join();

  /* free serial communication interface */
  if (m_SerialCommunication.IsNotNull())
  {
    m_SerialCommunication->ClearReceiveBuffer();
    m_SerialCommunication->ClearSendBuffer();
    m_SerialCommunication->CloseConnection();
    m_SerialCommunication = nullptr;
  }
}

std::string mitk::QuantelLaser::SendAndReceiveLine(const std::string* input, std::string* answer)
{
  MITK_INFO << "[Quantel Laser Debug] sending: " << input->c_str();
  if (input == nullptr)
    return "SERIALSENDERROR";
  
  std::string message;

  message = *input + '\n';

  // Clear send buffer
  m_SerialCommunication->ClearSendBuffer();
  m_SerialCommunication->Send(message);
  std::this_thread::sleep_for(std::chrono::milliseconds(50));
  std::string m;
  do
  {
    long returnvalue = m_SerialCommunication->Receive(m, 1);
    if ((returnvalue == 0) || (m.size() != 1))
      return "SERIALRECEIVEERROR";
    *answer += m;
  } while (m.at(0) != LF);
  m_SerialCommunication->ClearReceiveBuffer();
  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  m_SerialCommunication->ClearReceiveBuffer();
  MITK_INFO << "[Quantel Laser Debug] received: " << answer->c_str();
  return "OK";
}

void mitk::QuantelLaser::LoadResorceFile(std::string filename, std::string* lines)
{
  us::ModuleResource resorceFile = us::GetModuleContext()->GetModule()->GetResource(filename);
  std::string line;
  if (resorceFile.IsValid() && resorceFile.IsFile())
  {
    us::ModuleResourceStream stream(resorceFile);
    while (std::getline(stream, line))
    {
      *lines = *lines + line + "\n";
    }
  }
  else
  {
    MITK_ERROR << "[Quantel Laser Debug] Resource file was not valid";
  }
}

bool mitk::QuantelLaser::OpenConnection(std::string configurationFile)
{
  LoadResorceFile(configurationFile + ".xml", &m_XmlPumpLaserConfiguration);
  TiXmlDocument xmlDoc;

  if (xmlDoc.Parse(m_XmlPumpLaserConfiguration.c_str(), 0, TIXML_ENCODING_UTF8))
  {
    TiXmlElement* root = xmlDoc.FirstChildElement("PumpLaser");
    if (root)
    {
      TiXmlElement* elementNode = root->FirstChildElement("Seriell");
      TiXmlElement* element = elementNode->FirstChildElement("PortNumber");
      m_PortNumber = mitk::SerialCommunication::PortNumber(std::stoi(element->GetText()));
      element = elementNode->FirstChildElement("Baud");
      m_BaudRate = mitk::SerialCommunication::BaudRate(std::stoi(element->GetText()));

      MITK_INFO << m_PortNumber << m_BaudRate;
    }
  }
  else
  {
    MITK_ERROR << "[Quantel Laser Debug] Could not load configuration xml ";
    return false;
  }

  m_SerialCommunication = mitk::SerialCommunication::New();

  if (m_DeviceName.empty())
    m_SerialCommunication->SetPortNumber(m_PortNumber);
  else
    m_SerialCommunication->SetDeviceName(m_DeviceName);

  m_SerialCommunication->SetBaudRate(m_BaudRate);
  m_SerialCommunication->SetDataBits(m_DataBits);
  m_SerialCommunication->SetParity(m_Parity);
  m_SerialCommunication->SetStopBits(m_StopBits);
  m_SerialCommunication->SetSendTimeout(1000);
  m_SerialCommunication->SetReceiveTimeout(1000);
  if (m_SerialCommunication->OpenConnection() == 0) // 0 == ERROR_VALUE
  {
    m_SerialCommunication->CloseConnection();
    m_SerialCommunication = nullptr;
    MITK_ERROR << "[Quantel Laser Debug] " << "Can not open serial port";
    return false;
  }

  if (this->GetState() != UNCONNECTED)
  {
    std::string *command = new std::string;
    std::string answer("");
    command->assign("STOP");
    this->SendAndReceiveLine(command, &answer);
    return true;
  }
  else
    return false;
}

bool mitk::QuantelLaser::CloseConnection()
{
  if (this->GetState() != UNCONNECTED)
  {
    this->StopQswitching();
    this->StopFlashing();

    // close the serial connection
    m_SerialCommunication->CloseConnection();
    m_SerialCommunication = nullptr;
    this->GetState();
  }
  return true;
}

mitk::QuantelLaser::LaserState mitk::QuantelLaser::GetState()
{
  if (m_SerialCommunication == nullptr)
  {
    m_State = UNCONNECTED;
    return m_State;
  }
  
  if (!m_SerialCommunication->IsConnected())
  {
    m_State = UNCONNECTED;
    return m_State;
  }

  std::string *command = new std::string;
  std::string answer("");
  command->assign("STATE");
  this->SendAndReceiveLine(command, &answer);
  MITK_INFO << "[Quantel Laser Debug] get state:" << answer;
  if (answer == "STATE = 0\n")
    m_State = STATE0;
  else if(answer == "STATE = 1\n")
  {
    m_State = STATE1;
    m_FlashlampRunning = false;
    m_ShutterOpen = false;
    m_LaserEmission = false;
  }
  else if(answer == "STATE = 2\n") // laser ready for RUN
  {
    m_State = STATE2;
    m_FlashlampRunning = false;
    m_ShutterOpen = false;
    m_LaserEmission = false;
  }
  else if(answer == "STATE = 3\n")
  {
    m_State = STATE3;
    m_FlashlampRunning = true;
    m_ShutterOpen = false;
    m_LaserEmission = false;
  }
  else if(answer == "STATE = 4\n")
  {
    m_State = STATE4;
    m_FlashlampRunning = true;
    m_ShutterOpen = false;
    m_LaserEmission = false;
  }
  else if (answer == "STATE = 5\n")
  {
    m_State = STATE5;
    m_FlashlampRunning = true;
    m_ShutterOpen = true;
    m_LaserEmission = false;
  }
  else if (answer == "STATE = 6\n")
  {
    m_State = STATE6;
    m_FlashlampRunning = true;
    m_ShutterOpen = true;
    m_LaserEmission = true;
  }
  else if (answer == "STATE = 7\n")
  {
    m_State = STATE7;
    m_FlashlampRunning = true;
    m_ShutterOpen = true;
    m_LaserEmission = true;
  }
  else
  { 
    m_State = UNCONNECTED;
    m_FlashlampRunning = false;
    m_ShutterOpen = false;
    m_LaserEmission = false;
  }

  return m_State;
}


void mitk::QuantelLaser::StayAlive()
{
  do{
    std::this_thread::sleep_for(std::chrono::seconds(2));
    std::string *command = new std::string;
    std::string answer("");
    command->assign("STATE");
    this->SendAndReceiveLine(command, &answer);
  } while (m_KeepAlive);
}

bool mitk::QuantelLaser::StartFlashing()
{
  this->GetState();
  if (!m_FlashlampRunning)
  {
    if (m_LaserEmission)
      this->StopQswitching();

    std::string *command = new std::string;
    std::string answer("");
    command->assign("RUN");
    this->SendAndReceiveLine(command, &answer);
    MITK_INFO << answer;
    if (answer.at(0) == 'O' && answer.at(1) == 'K')
    {
      m_FlashlampRunning = true;
      m_ShutterOpen = false;
      m_KeepAlive = true;
      //m_StayAliveMessageThread = std::thread(&mitk::QuantelLaser::StayAlive, this);
    }
    else
    {
      MITK_ERROR << "[Quantel Laser Debug] " << "Cannot start flashlamps." << " Laser is telling me: " << answer;
      return false;
    }
  }
  else
    MITK_INFO << "[Quantel Laser Debug] " << "Flashlamps are already running";
  
  return true;
}

bool mitk::QuantelLaser::StopFlashing()
{
  this->GetState();
  if (m_FlashlampRunning)
  {
    if (m_LaserEmission)
      this->StopQswitching();

    std::string *command = new std::string;
    std::string answer("");
    command->assign("STOP");
    this->SendAndReceiveLine(command, &answer);
    MITK_INFO << answer;
    if (answer.at(0) == 'O' && answer.at(1) == 'K')
    {
      m_FlashlampRunning = false;
      m_ShutterOpen = false;
      m_KeepAlive = false;
    }
    else
    {  
      MITK_ERROR << "[Quantel Laser Debug] " << "Cannot Stop flashlamps." << " Laser is telling me: " << answer;
      return false;
    }

  }
  else
    MITK_INFO << "[Quantel Laser Debug] " << "Flashlamps are not running";

  return true;
}

bool mitk::QuantelLaser::StartQswitching()
{
  this->GetState();
  if (!m_LaserEmission)
  {
    std::string *command = new std::string;
    std::string answer("");
    command->assign("QSW 1");
    this->SendAndReceiveLine(command, &answer);
    MITK_INFO << answer;
    if (answer.at(0) == 'O' && answer.at(1) == 'K')
    {
      m_FlashlampRunning = true;
      m_ShutterOpen = true;
      return true;
    }
    else
    {
      MITK_ERROR << "[Quantel Laser Debug] " << "Cannot start Qswitch." << " Laser is telling me: " << answer;
      return false;
    }
  }
  else
  {
    MITK_INFO << "[Quantel Laser Debug] " << "Laser is already emitting";
    return true;
  }
}

bool mitk::QuantelLaser::StopQswitching()
{
  this->GetState();
  if (m_FlashlampRunning && m_LaserEmission)
  {
    std::string *command = new std::string;
    std::string answer("");
    command->assign("QSW 0");
    this->SendAndReceiveLine(command, &answer);
    MITK_INFO << answer;
    if (answer.at(0) == 'O' && answer.at(1) == 'K')
    {
      m_LaserEmission = false;
    }
    else
      MITK_ERROR << "[Quantel Laser Debug] " << "Cannot stop Q-switch.";

  }
  return true;
}

bool mitk::QuantelLaser::IsEmitting()
{
  return m_LaserEmission;
}
bool mitk::QuantelLaser::IsFlashing()
{
  return m_FlashlampRunning;
}