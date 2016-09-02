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

#include "mitkLaserDevice.h"

#include <chrono>
#include <thread>
#include <mutex>

#include <usModuleContext.h>
#include <usGetModuleContext.h>

const unsigned char CR = 0xD; // == '\r' - carriage return
const unsigned char LF = 0xA; // == '\n' - line feed

mitk::LaserDevice::LaserDevice() :
  m_State(mitk::LaserDevice::UNCONNECTED),
  m_FlashlampRunning(false),
  m_ShutterOpen(false),
  m_LaserEmission(false),
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

mitk::LaserDevice::~LaserDevice()
{
  /* stop tracking and disconnect from tracking device */
  if ((GetState() == STATE3) || (GetState() == STATE4) || (GetState() == STATE5))
  {
    this->StopQswitch();
    this->StopFlashlamps();
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

std::string mitk::LaserDevice::Send(const std::string* input)
{
  if (input == nullptr)
    return "SERIALSENDERROR";

  std::string message;

  //message = *input + std::string(1, CR);
  message = *input + '\n';

  // Clear send buffer
  this->ClearSendBuffer();
  // Send the date to the device

  std::lock_guard<std::mutex> lock(m_SerialCommunicationMutex); // lock the mutex until the end of the scope

  long returnvalue = m_SerialCommunication->Send(message);

  if (returnvalue == 0)
    return "SERIALSENDERROR";
  else
    return "OK";
}

std::string mitk::LaserDevice::ReceiveLine(std::string* answer)
{
  if (answer == nullptr)
    return "SERIALRECEIVEERROR";

  std::string m;
  std::lock_guard<std::mutex> lock(m_SerialCommunicationMutex); // lock the mutex until the end of the scope
  do
  {
    long returnvalue = m_SerialCommunication->Receive(m, 1);
    if ((returnvalue == 0) || (m.size() != 1))
      return "SERIALRECEIVEERROR";
    *answer += m;
  } while (m.at(0) != LF);
  return "OK";
}


void mitk::LaserDevice::ClearSendBuffer()
{
  std::lock_guard<std::mutex> lock(m_SerialCommunicationMutex); // lock the mutex until the end of the scope
  m_SerialCommunication->ClearSendBuffer();
}


void mitk::LaserDevice::ClearReceiveBuffer()
{
  std::lock_guard<std::mutex> lock(m_SerialCommunicationMutex); // lock the mutex until the end of the scope
  m_SerialCommunication->ClearReceiveBuffer();
}


bool mitk::LaserDevice::OpenConnection()
{
  m_SerialCommunication = mitk::SerialCommunication::New();

  if (m_DeviceName.empty())
    m_SerialCommunication->SetPortNumber(m_PortNumber);
  else
    m_SerialCommunication->SetDeviceName(m_DeviceName);

  m_SerialCommunication->SetBaudRate(m_BaudRate);
  m_SerialCommunication->SetDataBits(m_DataBits);
  m_SerialCommunication->SetParity(m_Parity);
  m_SerialCommunication->SetStopBits(m_StopBits);
  m_SerialCommunication->SetSendTimeout(300);
  m_SerialCommunication->SetReceiveTimeout(300);
  if (m_SerialCommunication->OpenConnection() == 0) // 0 == ERROR_VALUE
  {
    m_SerialCommunication->CloseConnection();
    m_SerialCommunication = nullptr;
    MITK_ERROR << "[Pump Laser] " << "Can not open serial port";
    return 0;
  }

  if (this->GetState() != UNCONNECTED)
    return true;
  else
    return false;
}

bool mitk::LaserDevice::CloseConnection()
{
  if (this->GetState() != UNCONNECTED)
  {
    this->StopQswitch();
    this->StopFlashlamps();

    // close the serial connection
    m_SerialCommunication->CloseConnection();
    m_SerialCommunication = nullptr;
    this->GetState();
  }
  return true;
}

mitk::LaserDevice::LaserDeviceState mitk::LaserDevice::GetState()
{
  
  std::string *command = new std::string;
  std::string answer("");
  command->assign("STATE");

  this->Send(command);
  this->ReceiveLine(&answer);
  this->ClearReceiveBuffer();

  if (answer == "STATE 0")
    m_State = STATE0;
  else if(answer == "STATE 1")
  {
    m_State = STATE1;
    m_FlashlampRunning = false;
    m_ShutterOpen = false;
    m_LaserEmission = false;
  }
  else if(answer == "STATE 2") // laser ready for RUN
  {
    m_State = STATE2;
    m_FlashlampRunning = false;
    m_ShutterOpen = false;
    m_LaserEmission = false;
  }
  else if(answer == "STATE 3")
  {
    m_State = STATE3;
    m_FlashlampRunning = true;
    m_ShutterOpen = false;
    m_LaserEmission = false;
  }
  else if(answer == "STATE 4")
  {
    m_State = STATE4;
    m_FlashlampRunning = true;
    m_ShutterOpen = false;
    m_LaserEmission = true;
  }
  else if (answer == "STATE 5")
  {
    m_State = STATE5;
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


void mitk::LaserDevice::StayAlive()
{
  do{
    std::this_thread::sleep_for(std::chrono::seconds(1));
    std::string *command = new std::string;
    std::string answer("");
    command->assign("STATE");

    this->Send(command);
    this->ReceiveLine(&answer);
    this->ClearReceiveBuffer();
  } while (m_KeepAlive);
}

bool mitk::LaserDevice::StartFlashlamps()
{
  this->GetState();
  if (!m_FlashlampRunning)
  {
    if (m_LaserEmission)
      this->StopQswitch();

    std::string *command = new std::string;
    std::string answer("");
    command->assign("RUN");

    this->Send(command);
    this->ReceiveLine(&answer);
    this->ClearReceiveBuffer();

    if (answer == "OK")
    {
      m_FlashlampRunning = true;
      m_ShutterOpen = false;
      m_KeepAlive = true;
      m_StayAliveMessageThread = std::thread(&mitk::LaserDevice::StayAlive, this);
    }
    else
    {
      MITK_ERROR << "[Pump Laser] " << "Cannot start flashlamps." << " Laser is telling me: " << answer;
      return false;
    }
  }
  MITK_INFO << "[Pump Laser] " << "Flashlamps are already running";
  
  return true;
}

bool mitk::LaserDevice::StopFlashlamps ()
{
  this->GetState();
  if (m_FlashlampRunning)
  {
    if (m_LaserEmission)
      this->StopQswitch();

    std::string *command = new std::string;
    std::string answer("");
    command->assign("STOP");

    this->Send(command);
    this->ReceiveLine(&answer);
    this->ClearReceiveBuffer();

    if (answer == "OK")
    {
      m_FlashlampRunning = false;
      m_ShutterOpen = false;
      m_KeepAlive = false;
    }
    else
    {  
      MITK_ERROR << "[Pump Laser] " << "Cannot Stop flashlamps." << " Laser is telling me: " << answer;
      return false;
    }

  }
  MITK_INFO << "[Pump Laser] " << "Flashlamps are not running";
  return true;
}

bool mitk::LaserDevice::StopQswitch()
{
  this->GetState();
  if (m_FlashlampRunning && m_LaserEmission)
  {
    std::string *command = new std::string;
    std::string answer("");
    command->assign("QSW 2");

    this->Send(command);
    this->ReceiveLine(&answer);
    this->ClearReceiveBuffer();

    if (answer == "OK")
    {
      m_LaserEmission = false;
    }
    else
      MITK_ERROR << "[Pump Laser] " << "Cannot stop Flashlamps.";

  }
  return true;
}