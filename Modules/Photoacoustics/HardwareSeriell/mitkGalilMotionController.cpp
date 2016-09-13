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

#include "mitkGalilMotionController.h"

#include <chrono>
#include <thread>
#include <mutex>

#include <usModuleContext.h>
#include <usGetModuleContext.h>

const unsigned char CR = 0xD; // == '\r' - carriage return
const unsigned char LF = 0xA; // == '\n' - line feed

mitk::GalilMotionController::GalilMotionController() :
  m_CurrentPosition(-1),
  m_DeviceName(),
  m_PortNumber(mitk::SerialCommunication::COM7), 
  m_BaudRate(mitk::SerialCommunication::BaudRate115200),
  m_DataBits(mitk::SerialCommunication::DataBits8), 
  m_Parity(mitk::SerialCommunication::None), 
  m_StopBits(mitk::SerialCommunication::StopBits1),
  m_HardwareHandshake(mitk::SerialCommunication::HardwareHandshakeOff),
  m_SerialCommunication(nullptr),
  m_SerialCommunicationMutex()

{
}

mitk::GalilMotionController::~GalilMotionController()
{
  this->CloseConnection();
}

std::string mitk::GalilMotionController::Send(const std::string* input)
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

std::string mitk::GalilMotionController::ReceiveLine(std::string* answer)
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


void mitk::GalilMotionController::ClearSendBuffer()
{
  std::lock_guard<std::mutex> lock(m_SerialCommunicationMutex); // lock the mutex until the end of the scope
  m_SerialCommunication->ClearSendBuffer();
}


void mitk::GalilMotionController::ClearReceiveBuffer()
{
  std::lock_guard<std::mutex> lock(m_SerialCommunicationMutex); // lock the mutex until the end of the scope
  m_SerialCommunication->ClearReceiveBuffer();
}


bool mitk::GalilMotionController::OpenConnection()
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
  m_SerialCommunication->SetSendTimeout(150);
  m_SerialCommunication->SetReceiveTimeout(150);
  if (m_SerialCommunication->OpenConnection() == 0) // 0 == ERROR_VALUE
  {
    m_SerialCommunication->CloseConnection();
    m_SerialCommunication = nullptr;
    MITK_ERROR << "[OPO Motor] " << "Can not open serial port";
    return false;
  }
  else
    return true;
}

bool mitk::GalilMotionController::CloseConnection()
{
  if (m_SerialCommunication.IsNotNull())
  {
    m_SerialCommunication->ClearReceiveBuffer();
    m_SerialCommunication->ClearSendBuffer();
    m_SerialCommunication->CloseConnection();
    m_SerialCommunication = nullptr;
  }
  return true;
}


bool mitk::GalilMotionController::Test()
{
  std::string *command = new std::string;
  std::string answer("");
  command->assign("TPA");

  this->Send(command);
  this->ReceiveLine(&answer);
  this->ClearReceiveBuffer();
  
  MITK_INFO << "[OPO Motor] TPA said: "<< answer;
  
  return true;
}

bool mitk::GalilMotionController::Home()
{
  std::string *command = new std::string;
  std::string answer("");
  command->assign("#HOME\nOE 0\nJG -10000;BG\n#JOG;JP#JOG,@ABS[_TEX]<1000\nST;AM\nMG \"HIT END\"\nOE 1\nWT100\nEN\n#RESETPOS\nDP *= 0\nEN");
  return true;
}

bool mitk::GalilMotionController::Tune()
{
  return true;
}