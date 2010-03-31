/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include <iostream>
#include "mitkSerialCommunication.h"

/**Documentation
* \brief SerialCommunicationTest - send and receive bytes over a com port
*
* This is a simple test program for the mitk::SerialCommunication class
* It opens a com port, sends two strings and receives two messages with
* different timeout modes - first it waits indefinitely until all characters
* are received, next it waits not more than 6 seconds.
* you can use the windows program HpyerTerminal to receive text from this 
* program and send text back to it.
*/

int main()
{
  std::cout << "Test Program for mitkSerialCommunication.cpp\n";
  std::cout << "Using com port COM7\n"; // current test uses hard coded COM7
//  std::cout << "Using com port /dev/ttyUSB1\n"; 

  mitk::SerialCommunication::Pointer serial = mitk::SerialCommunication::New();
  serial->SetBaudRate(mitk::SerialCommunication::BaudRate115200);
  serial->SetPortNumber(mitk::SerialCommunication::COM7);
  //serial->SetDeviceName("/dev/ttyUSB1");
  serial->SetDataBits(mitk::SerialCommunication::DataBits8);
  serial->SetParity(mitk::SerialCommunication::Even);
  serial->SetStopBits(mitk::SerialCommunication::StopBits1);
  serial->SetHardwareHandshake(mitk::SerialCommunication::HardwareHandshakeOff);
  serial->SetSendTimeout(2000);
  serial->SetReceiveTimeout(2000);
  if (serial->OpenConnection() == false)
  {
    std::cout << "Error Opening connection to com port #" << serial->GetPortNumber() << std::endl;
    return -1;
  }
  std::string message = "Hello World\r\n";
  if (serial->Send(message) == false)
  {
    std::cout << "Error sending string '" << message << "'" << std::endl;
    serial->CloseConnection();
    return -1;
  }
  message = "";
  serial->Send(std::string("Oh, Hello again, dort am Fluss wo die Baeume steh'n, will ich Dir in die Augen seh'n, ob ich da bleiben kann.\r\n"));

  std::cout << "Waiting to receive 4 characters. [readtimeout = 0! --> blocking --> waiting until 4 characters are send in windows implementation!]\n";
  if (serial->Receive(message, 4) == false)// receive 4 bytes
    std::cout << "Error receiving message. Only " << message.size() << " characters received: '" << message << "'.\n";
  else
    std::cout << "Received message: '" << message << "'.\n";

  std::cout << "Setting Receive timeout to 6 seconds, waiting to receive 10 more characters.\n";
  serial->CloseConnection();
  serial->SetReceiveTimeout(6000);
  serial->OpenConnection();
  if (serial->Receive(message, 10) == false)// receive 10 bytes
    std::cout << "Error receiving message. Received " << message.size() << " characters: '" << message << "'.\n";
  else
    std::cout << "Received message: '" << message << "' without errors.\n";  
  
  std::cout << "sending a new message.\n";
  serial->Send(std::string("all your base are belong to us\r\n"));
  std::cout << "Waiting to receive 20 characters.\n";
  serial->Receive(message, 20);
  std::cout << "Received " << message.size() << " characters. Message: '" << message << "'.\n";
  serial->CloseConnection();
  std::cout << "closing connection.\n";
  std::cout << "good bye.\n";
  return 0;
}
