/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
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

#include "mitkNDIProtocol.h"
#include "mitkNDITrackingDevice.h"
#include <string>
#include <algorithm>
#include <sstream>
#include <itksys/SystemTools.hxx>
#include <stdio.h>


mitk::NDIProtocol::NDIProtocol() 
: itk::Object(), m_TrackingDevice(NULL), m_UseCRC(true)
{
}


mitk::NDIProtocol::~NDIProtocol()
{
}


mitk::NDIErrorCode mitk::NDIProtocol::COMM(mitk::SerialCommunication::BaudRate baudRate , mitk::SerialCommunication::DataBits dataBits, mitk::SerialCommunication::Parity parity, mitk::SerialCommunication::StopBits stopBits, mitk::SerialCommunication::HardwareHandshake hardwareHandshake)
{
  /* Build parameter string */
  std::string param;
  switch (baudRate) 
  {
  case mitk::SerialCommunication::BaudRate14400:
    param += "1";
    break;
  case mitk::SerialCommunication::BaudRate19200:
    param += "2";
    break;
  case mitk::SerialCommunication::BaudRate38400:
    param += "3";
    break;
  case mitk::SerialCommunication::BaudRate57600:
    param += "4";
    break;
  case mitk::SerialCommunication::BaudRate115200:
    param += "5";
    break;
  case mitk::SerialCommunication::BaudRate9600:
  default:            // assume 9600 Baud as default
    param += "0"; 
    break;
  }
  switch (dataBits)
  {
  case mitk::SerialCommunication::DataBits7:
    param += "1";
    break;
  case mitk::SerialCommunication::DataBits8:
  default:            // set 8 data bits as default
    param += "0";
    break;
  }
  switch (parity)
  {
  case mitk::SerialCommunication::Odd:
    param += "1";
    break;
  case mitk::SerialCommunication::Even:
    param += "2";
    break;
  case mitk::SerialCommunication::None:
  default:            // set no parity as default
    param += "0";
    break;
  }
  switch (stopBits)
  {
  case mitk::SerialCommunication::StopBits2:
    param += "1";
    break;
  case mitk::SerialCommunication::StopBits1:
  default:            // set 1 stop bit as default
    param += "0";
    break;
  }
  switch (hardwareHandshake)
  {
  case mitk::SerialCommunication::HardwareHandshakeOn:
    param += "1";
    break;
  case mitk::SerialCommunication::HardwareHandshakeOff:
  default:            // set no hardware handshake as default
    param += "0";
    break;
  }
  return GenericCommand("COMM", &param);
}


mitk::NDIErrorCode mitk::NDIProtocol::INIT()
{
  return GenericCommand("INIT");
}


mitk::NDIErrorCode mitk::NDIProtocol::DSTART()
{
  return GenericCommand("DSTART");
}


mitk::NDIErrorCode mitk::NDIProtocol::DSTOP()
{
  return GenericCommand("DSTOP");
}


mitk::NDIErrorCode mitk::NDIProtocol::IRINIT()
{
  return GenericCommand("IRINIT");
}


mitk::NDIErrorCode mitk::NDIProtocol::IRCHK(bool* IRdetected)
{
  NDIErrorCode returnValue = NDIUNKNOWNERROR; // return code for this function. Will be set according to reply from trackingsystem

  if (m_TrackingDevice == NULL)
    return TRACKINGDEVICENOTSET;

  /* send command */
  std::string fullcommand;
  if (m_UseCRC == true)
    fullcommand = "IRCHK:0001";     // command string format 1: with crc
  else
    fullcommand = "IRCHK 0001";     // command string format 2: without crc

  returnValue = m_TrackingDevice->Send(&fullcommand, m_UseCRC);

  if (returnValue != NDIOKAY)       // check for send error
  {
    m_TrackingDevice->ClearReceiveBuffer();   // flush the buffer to remove any reply
    return returnValue;
  }
  /* wait for the trackingsystem to process the command */
  itksys::SystemTools::Delay(100);
  /* read and parse the reply from tracking device */
  // the reply for IRCHK can be either Infrared Source Information or ERROR##    
  // because we use the simple reply format, the answer will be only one char:
  // "0" - no IR detected
  // "1" - IR detected
  std::string reply;
  char b;
  m_TrackingDevice->ReceiveByte(&b);// read the first byte
  reply = b;
  if ((b == '0') || (b == '1'))     // normal answer
  {
    /* perform CRC checking */
    std::string expectedCRC = m_TrackingDevice->CalcCRC(&reply);  // calculate crc for received reply string
    std::string readCRC;                                          // read attached crc value
    m_TrackingDevice->Receive(&readCRC, 4);                       // CRC16 is 2 bytes long, which is transmitted as 4 hexadecimal digits
    if (expectedCRC == readCRC)                                   // if the read CRC is correct, return normal error code 
    {
      if ( b == '0')
        *IRdetected = false;
      else
        *IRdetected = true;
      returnValue = NDIOKAY;
    } 
    else                            // return error in CRC 
    {
      returnValue = NDICRCERROR;
      *IRdetected = false;          // IRdetected is only valid if return code of this function is NDIOKAY
    }
  } 
  else if (b =='E')                 // expect ERROR##
  {
    std::string errorstring;
    m_TrackingDevice->Receive(&errorstring, 4);   // read the remaining 4 characters of ERROR
    reply += errorstring;
    static const std::string error("ERROR"); 
    if (error.compare(0, 5, reply) == 0)          // check for "ERROR"
    {
      std::string errorcode;
      m_TrackingDevice->Receive(&errorcode, 2);   // now read 2 bytes error code
      /* perform CRC checking */
      reply += errorcode;                         // build complete reply string
      std::string expectedCRC = m_TrackingDevice->CalcCRC(&reply);    // calculate crc for received reply string
      std::string readCRC;                        // read attached crc value
      m_TrackingDevice->Receive(&readCRC, 4);     // CRC16 is 2 bytes long, which is transmitted as 4 hexadecimal digits
      if (expectedCRC == readCRC)                 // if the read CRC is correct, return normal error code 
        returnValue = this->GetErrorCode(&errorcode);
      else
        returnValue = NDICRCERROR;                // return error in CRC 
    }
  } else  // something else, that we do not expect
    returnValue = NDIUNEXPECTEDREPLY;

  /* cleanup and return */
  m_TrackingDevice->ClearReceiveBuffer();        // flush the buffer to remove the remaining carriage return or unknown/unexpected reply
  return returnValue;
}


mitk::NDIErrorCode mitk::NDIProtocol::PHSR(PHSRQueryType queryType, std::string* portHandles)
{
  NDIErrorCode returnValue = NDIUNKNOWNERROR; 

  if (m_TrackingDevice == NULL)
    return TRACKINGDEVICENOTSET;

  /* send command */
  std::string command;
  char stringQueryType[3];
  sprintf(stringQueryType, "%02X", queryType);// convert numerical queryType to string in hexadecimal format

  if (m_UseCRC == true)
    command = std::string("PHSR:")  + std::string(stringQueryType); 
  else //if (m_UseCRC != true)
    command = std::string("PHSR ")  + std::string(stringQueryType); // command string format 2: without crc

  returnValue = m_TrackingDevice->Send(&command, m_UseCRC);

  if (returnValue != NDIOKAY)                 // check for send error
  {
    m_TrackingDevice->ClearReceiveBuffer();   // flush the buffer to remove any reply
    return returnValue;
  }

  itksys::SystemTools::Delay(100);

  std::string reply;
  m_TrackingDevice->Receive(&reply, 2);       // read first 2 characters of reply ("Number of Handles" as a 2 digit hexadecimal number)
  static const std::string error("ERROR"); 
  if (error.compare(0, 2, reply) == 0)        // check for "ERROR" (compare for "ER" because we can not be sure that the reply is more than 2 characters (in case of 0 port handles returned)
  {
    std::string ror;
    m_TrackingDevice->Receive(&ror, 3);       // read rest of ERROR (=> "ROR")
    reply += ror;                             // build complete reply string
    std::string errorcode;
    m_TrackingDevice->Receive(&errorcode, 2); // now read 2 bytes error code
    reply += errorcode;                       // build complete reply string
    /* perform CRC checking */
    std::string expectedCRC = m_TrackingDevice->CalcCRC(&reply);    // calculate crc for received reply string
    std::string readCRC;                      // read attached crc value
    m_TrackingDevice->Receive(&readCRC, 4);   // CRC16 is 2 bytes long, which is transmitted as 4 hexadecimal digits
    if (expectedCRC == readCRC)               // if the read CRC is correct, return normal error code 
      returnValue = this->GetErrorCode(&errorcode);
    else                                      // return error in CRC 
      returnValue = NDICRCERROR;
  }
  else  // No error, expect number of handles as a 2 character hexadecimal value
  {
    unsigned int numberOfHandles = 0;
    // convert the hexadecimal string representation to a numerical using a stringstream
    std::stringstream s;
    s << reply;
    s >> numberOfHandles;
    if (numberOfHandles > 16)  // there can not be more than 16 handles ToDo: exact maximum number depend on tracking device and firmware revision. these data could be read with VER and used here
    {
      returnValue = NDIUNKNOWNERROR;
    }
    else
    {
      std::string handleInformation;
      portHandles->clear();
      for (unsigned int i = 0; i < numberOfHandles; i++)  // read 5 characters for each handle and extract port handle 
      {
        m_TrackingDevice->Receive(&handleInformation, 5);
        *portHandles += handleInformation.substr(0, 2);   // Append the port handle to the portHandles string
        reply += handleInformation;                       // build complete reply string for crc checking
      }
      /* perform CRC checking */
      std::string expectedCRC = m_TrackingDevice->CalcCRC(&reply);    // calculate crc for received reply string
      std::string readCRC;                                // read attached crc value
      m_TrackingDevice->Receive(&readCRC, 4);             // CRC16 is 2 bytes long, which is transmitted as 4 hexadecimal digits
      if (expectedCRC == readCRC)                         // if the read CRC is correct, return okay
        returnValue = NDIOKAY;
      else                                                // return error in CRC
        returnValue = NDICRCERROR;
    }
  }
  /* cleanup and return */
  m_TrackingDevice->ClearReceiveBuffer();                 // flush the buffer to remove the remaining carriage return or unknown/unexpected reply
  return returnValue;
}


mitk::NDIErrorCode mitk::NDIProtocol::PHRQ(std::string* portHandle)
{
  NDIErrorCode returnValue = NDIUNKNOWNERROR; // return code for this function. Will be set according to reply from tracking device

  if (m_TrackingDevice == NULL)
    return TRACKINGDEVICENOTSET;

  /* send command */
  std::string command;
  if (m_UseCRC == true)
    command = "PHRQ:*********1****";          // command string format 1: with crc
  else
    command = "PHRQ *********1****";          // command string format 2: without crc

  returnValue = m_TrackingDevice->Send(&command, m_UseCRC);

  if (returnValue != NDIOKAY)                 // check for send error
  {
    m_TrackingDevice->ClearReceiveBuffer();   // flush the buffer to remove any reply
    return returnValue;
  }

  itksys::SystemTools::Delay(100);            // give the tracking device some time to process the command

  std::string reply;
  m_TrackingDevice->Receive(&reply, 2);       // read first 2 characters of reply ("Number of Handles" as a 2 digit hexadecimal number)
  static const std::string error("ERROR"); 
  if (error.compare(0, 2, reply) == 0)        // check for "ERROR" (compare for "ER" because we can not be sure that the reply is more than 2 characters (in case of 0 port handles returned)
  {
    std::string ror;
    m_TrackingDevice->Receive(&ror, 3);       // read rest of ERROR (=> "ROR")
    reply += ror;                             // build complete reply string
    std::string errorcode;
    m_TrackingDevice->Receive(&errorcode, 2); // now read 2 bytes error code
    reply += errorcode;                       // build complete reply string
    /* perform CRC checking */
    std::string expectedCRC = m_TrackingDevice->CalcCRC(&reply);    // calculate crc for received reply string
    std::string readCRC;                      // read attached crc value
    m_TrackingDevice->Receive(&readCRC, 4);   // CRC16 is 2 bytes long, which is transmitted as 4 hexadecimal digits
    if (expectedCRC == readCRC)               // if the read CRC is correct, return normal error code 
      returnValue = this->GetErrorCode(&errorcode);
    else                                      // return error in CRC 
      returnValue = NDICRCERROR;
  }
  else  // No error, expect port handle as a 2 character hexadecimal value
  {
    *portHandle = reply;                      // assign the port handle to the return string
    /* perform CRC checking */
    std::string expectedCRC = m_TrackingDevice->CalcCRC(&reply);    // calculate crc for received reply string
    std::string readCRC;                      // read attached crc value
    m_TrackingDevice->Receive(&readCRC, 4);   // CRC16 is 2 bytes long, which is transmitted as 4 hexadecimal digits
    if (expectedCRC == readCRC)               // if the read CRC is correct, return okay
      returnValue = NDIOKAY;
    else                                      // else return error in CRC
      returnValue = NDICRCERROR;
  }
  /* cleanup and return */
  m_TrackingDevice->ClearReceiveBuffer();     // flush the buffer to remove the remaining carriage return or unknown/unexpected reply
  return returnValue;
}


mitk::NDIErrorCode mitk::NDIProtocol::PVWR(std::string* portHandle, const unsigned char* sromData, unsigned int sromDataLength)
{
  NDIErrorCode returnValue = NDIUNKNOWNERROR; // return code for this function. Will be set according to reply from trackingsystem

  if (m_TrackingDevice == NULL)
    return TRACKINGDEVICENOTSET;

  if (sromDataLength > 1024)
    return SROMFILETOOLARGE;

  if (sromDataLength == 0)
    return SROMFILETOOSMALL;

  /* build send commands */
  std::string basecommand;
  if (m_UseCRC == true)
    basecommand = "PVWR:";                        // command string format 1: with crc
  else
    basecommand = "PVWR ";                        // command string format 2: without crc

  std::string hexSROMData;
  hexSROMData.reserve(2 * sromDataLength);
  char hexcharacter[20];  // 7 bytes should be enough (in send loop)
  for (unsigned int i = 0; i < sromDataLength; i++)
  {
    sprintf(hexcharacter, "%02X", sromData[i]);   // convert srom byte to string in hexadecimal format
    //hexSROMData += "12";
    hexSROMData += hexcharacter;                  // append hex string to srom data in hex format 
  }
  /* data must be written in chunks of 64 byte (128 hex characters). To ensure 64 byte chunks the last chunk must be padded with 00 */
  unsigned int zerosToPad = 128 - (hexSROMData.size() % 128);  // hexSROMData must be a multiple of 128
  if (zerosToPad > 0)
    hexSROMData.append(zerosToPad, '0');
  /* now we have all the data, send it in 128 character chunks */
  std::string fullcommand;
  for (unsigned int j = 0; j < hexSROMData.size(); j += 128)
  {
    sprintf(hexcharacter, "%s%04X", portHandle->c_str(), j/2);              // build the first two parameters: PortHandle and SROM device adress (not in hex characters, but in bytes)
    fullcommand = basecommand + hexcharacter + hexSROMData.substr(j, 128);  // build complete command string
    returnValue = m_TrackingDevice->Send(&fullcommand, m_UseCRC);           // send command
    itksys::SystemTools::Delay(50);               // Wait for trackingsystem to process the data
    if (returnValue != NDIOKAY)                   // check for send error
      break;
    returnValue = this->ParseOkayError();         // parse answer
    if (returnValue != NDIOKAY)                   // check for error returned from tracking device
      break;
  }
  /* cleanup and return */
  m_TrackingDevice->ClearReceiveBuffer();         // flush the buffer to remove the remaining carriage return or unknown/unexpected reply
  return returnValue;
}


mitk::NDIErrorCode mitk::NDIProtocol::PINIT(std::string* portHandle)
{
  NDIErrorCode returnValue = NDIUNKNOWNERROR; // return code for this function. Will be set according to reply from trackingsystem

  if (m_TrackingDevice == NULL)
    return TRACKINGDEVICENOTSET;

  /* send command */
  std::string fullcommand;
  if (m_UseCRC == true)
    fullcommand = std::string("PINIT:") + *portHandle;  // command string format 1: with crc
  else
    fullcommand = std::string("PINIT ") + *portHandle;  // command string format 2: without crc

  returnValue = m_TrackingDevice->Send(&fullcommand, m_UseCRC);

  if (returnValue != NDIOKAY)                           // check for send error
  {
    m_TrackingDevice->ClearReceiveBuffer();             // flush the buffer to remove any reply
    return returnValue;
  }

  /* wait for the trackingsystem to process the command */
  itksys::SystemTools::Delay(100);
  std::string reply;
  m_TrackingDevice->Receive(&reply, 4);                 // read first 4 characters of reply

  /* Parse reply from tracking device */
  static const std::string okay("OKAYA896");            // OKAY is static, so we can perform a static crc check
  static const std::string error("ERROR"); 
  static const std::string warning("WARNING7423");      // WARNING has a static crc too

  if (okay.compare(0, 4, reply) == 0)                   // check for "OKAY": compare first 4 characters from okay with reply
  {
    // OKAY was found, now check the CRC16 too
    m_TrackingDevice->Receive(&reply, 4);               // read 4 hexadecimal characters for CRC16
    if (okay.compare(4, 4, reply, 0, 4) == 0)           // first 4 from new reply should match last 4 from okay
      returnValue = NDIOKAY;
    else
      returnValue = NDICRCERROR;
  } 
  else if (warning.compare(0, 4, reply) == 0)           // check for "WARNING"
  {
    // WARN was found, now check remaining characters and CRC16
    m_TrackingDevice->Receive(&reply, 4);               // read 4 hexadecimal characters for CRC16
    if (warning.compare(4, 7, reply, 0, 7) == 0)        // first 7 from new reply should match last 7 from okay
      returnValue = NDIWARNING;
    else
      returnValue = NDICRCERROR;
  } 
  else if (error.compare(0, 4, reply) == 0)             // check for "ERRO"
  {
    char b;                                             // The ERROR reply is not static, so we can not use a static crc check.
    m_TrackingDevice->ReceiveByte(&b);                  // read next character ("R" from ERROR)
    reply += b;                                         // to build complete reply string
    std::string errorcode;
    m_TrackingDevice->Receive(&errorcode, 2);           // now read 2 bytes error code
    reply += errorcode;                                 // build complete reply string
    /* perform CRC checking */
    std::string expectedCRC = m_TrackingDevice->CalcCRC(&reply);    // calculate crc for received reply string
    std::string readCRC;                                // read attached crc value
    m_TrackingDevice->Receive(&readCRC, 4);             // CRC16 is 2 bytes long, which is transmitted as 4 hexadecimal digits
    if (expectedCRC == readCRC)                         // if the read CRC is correct, return normal error code 
      returnValue = this->GetErrorCode(&errorcode);
    else                                                // return error in CRC 
      returnValue = NDICRCERROR;
  } else                                                // else it is something else, that we do not expect
    returnValue = NDIUNEXPECTEDREPLY;


  //read cr carriage return
  char b;
  m_TrackingDevice->ReceiveByte(&b);

  /* cleanup and return */
  m_TrackingDevice->ClearReceiveBuffer();               // flush the buffer to remove the remaining carriage return or unknown/unexpected reply
  return returnValue;
}


mitk::NDIErrorCode mitk::NDIProtocol::PENA(std::string* portHandle, TrackingPriority prio)
{
  std::string param;
  if (portHandle != NULL)
    param = *portHandle + (char) prio;
  else 
    param = "";
  return this->GenericCommand("PENA", &param);
}


mitk::NDIErrorCode mitk::NDIProtocol::PHINF(std::string portHandle, std::string* portInfo)
{
  std::string command;
  if (m_UseCRC) command = "PHINF:" + portHandle;
  else command = "PHINF " + portHandle;
  mitk::NDIErrorCode returnValue = m_TrackingDevice->Send(&command, m_UseCRC); 
  if (returnValue==NDIOKAY)
    {
    m_TrackingDevice->Receive(portInfo, 33);
    }
  return returnValue;
}


mitk::NDIErrorCode mitk::NDIProtocol::PDIS(std::string* portHandle)
{
  return this->GenericCommand("PDIS", portHandle);
}


mitk::NDIErrorCode mitk::NDIProtocol::PHF(std::string* portHandle)
{
  return this->GenericCommand("PHF", portHandle);
}


mitk::NDIErrorCode mitk::NDIProtocol::IRATE(IlluminationActivationRate rate)
{
  std::string param;
  switch (rate)
  {
  case Hz60:
    param = "2";
    break;
  case Hz30:
    param = "1";
    break;
  case Hz20:
  default:
    param = "0";
    break;
  }
  return this->GenericCommand("IRATE", &param);
}


mitk::NDIErrorCode mitk::NDIProtocol::BEEP(unsigned char count)
{
  NDIErrorCode returnValue = NDIUNKNOWNERROR; // return code for this function. Will be set according to reply from trackingsystem

  if (m_TrackingDevice == NULL)
    return TRACKINGDEVICENOTSET;

  std::string p;
  if ((count >= 1) && (count <= 9))
    p = (count + '0');                        // convert the number count to a character representation
  else
    return NDICOMMANDPARAMETEROUTOFRANGE;

  /* send command */
  std::string fullcommand;
  if (m_UseCRC == true)
    fullcommand = "BEEP:" + p;                // command string format 1: with crc
  else
    fullcommand = "BEEP " + p;                // command string format 2: without crc

  returnValue = m_TrackingDevice->Send(&fullcommand, m_UseCRC);

  if (returnValue != NDIOKAY)                 // check for send error
  {
    m_TrackingDevice->ClearReceiveBuffer();   // flush the buffer to remove any reply
    return returnValue;
  }

  /* wait for the trackingsystem to process the command */
  itksys::SystemTools::Delay(100);

  std::string reply;
  char b;
  m_TrackingDevice->ReceiveByte(&b);          // read the first byte
  reply = b;
  if ((b == '0') || (b == '1'))               // normal answer
  {
    /* perform CRC checking */
    std::string expectedCRC = m_TrackingDevice->CalcCRC(&reply);  // calculate crc for received reply string
    std::string readCRC;                                          // read attached crc value
    m_TrackingDevice->Receive(&readCRC, 4);                       // CRC16 is 2 bytes long, which is transmitted as 4 hexadecimal digits
    if (expectedCRC == readCRC)                                   // if the read CRC is correct, return normal error code 
      returnValue = NDIOKAY;
    else                                      // return error in CRC 
      returnValue = NDICRCERROR;
  } 
  else if (b =='E')                           // expect ERROR##
  {
    std::string errorstring;
    m_TrackingDevice->Receive(&errorstring, 4);   // read the remaining 4 characters of ERROR
    reply += errorstring;
    static const std::string error("ERROR"); 
    if (error.compare(0, 5, reply) == 0)          // check for "ERROR"
    {
      std::string errorcode;
      m_TrackingDevice->Receive(&errorcode, 2);   // now read 2 bytes error code
      /* perform CRC checking */
      reply += errorcode;                         // build complete reply string
      std::string expectedCRC = m_TrackingDevice->CalcCRC(&reply);    // calculate crc for received reply string
      std::string readCRC;                        // read attached crc value
      m_TrackingDevice->Receive(&readCRC, 4);     // CRC16 is 2 bytes long, which is transmitted as 4 hexadecimal digits
      if (expectedCRC == readCRC)                 // if the read CRC is correct, return normal error code 
        returnValue = this->GetErrorCode(&errorcode);
      else
        returnValue = NDICRCERROR;                // return error in CRC 
    }
  } else  // something else, that we do not expect
    returnValue = NDIUNEXPECTEDREPLY;

  /* cleanup and return */
  m_TrackingDevice->ClearReceiveBuffer();         // flush the buffer to remove the remaining carriage return or unknown/unexpected reply
  return returnValue;

}


mitk::NDIErrorCode mitk::NDIProtocol::TSTART(bool resetFrameCounter)
{
  std::string param = "80";
  if (resetFrameCounter == true)
    return this->GenericCommand("TSTART", &param);
  else
    return this->GenericCommand("TSTART");
}


mitk::NDIErrorCode mitk::NDIProtocol::TSTOP()
{
  return this->GenericCommand("TSTOP");
}


mitk::NDIErrorCode mitk::NDIProtocol::PSOUT(std::string portHandle, std::string state)
{
  std::string param = portHandle + state;
  return this->GenericCommand("PSOUT", &param);
}


mitk::NDIErrorCode mitk::NDIProtocol::TX(bool trackIndividualMarkers, MarkerPointContainerType* markerPositions)
{
  NDIErrorCode returnValue = NDIUNKNOWNERROR; // return code for this function. Will be set according to reply from tracking device

  if(trackIndividualMarkers)
    markerPositions->clear();

  if (m_TrackingDevice == NULL)
    return TRACKINGDEVICENOTSET;

  /* send command */
  std::string fullcommand;
  if (trackIndividualMarkers)
  {
    if (m_UseCRC == true)
      fullcommand = "TX:1001";          // command string format 1: with crc
    else
      fullcommand = "TX 1001";          // command string format 2: without crc
  } else 
  {
    if (m_UseCRC == true)
      fullcommand = "TX:";          // command string format 1: with crc
    else
      fullcommand = "TX ";          // command string format 2: without crc
  }

  returnValue = m_TrackingDevice->Send(&fullcommand, m_UseCRC);
  if (returnValue != NDIOKAY)
  {
    /* cleanup and return */   
    m_TrackingDevice->ClearReceiveBuffer();   // flush the buffer to remove the remaining carriage return or unknown/unexpected reply
    return returnValue;
  }
  /* read number of handles returned */
  std::string reply;
  std::string s;
  m_TrackingDevice->Receive(&reply, 2);       // read first 2 characters of reply (error or number of handles returned)
  //printf("%d",reply);
  static const std::string error("ERROR");
  if (error.compare(0, 2, reply) == 0)
  {
    m_TrackingDevice->Receive(&s, 3);         // read next characters ("ROR" from ERROR)
    reply += s;                               // to build complete reply string
    std::string errorcode;
    m_TrackingDevice->Receive(&errorcode, 2); // now read 2 bytes error code
    reply += errorcode;                       // build complete reply string
    /* perform CRC checking */
    std::string expectedCRC = m_TrackingDevice->CalcCRC(&reply);    // calculate crc for received reply string
    std::string readCRC;                      // read attached crc value
    m_TrackingDevice->Receive(&readCRC, 4);   // CRC16 is 2 bytes long, which is transmitted as 4 hexadecimal digits
    if (expectedCRC == readCRC)               // if the read CRC is correct, return normal error code 
    {
      returnValue = this->GetErrorCode(&errorcode);
    }
    else                                      // return error in CRC 
    {
      returnValue = NDICRCERROR;
    }
  }
  else                                        // transformation data is returned
  {
    /* parse number of handles from first 2 characters */
    std::stringstream converter;
    unsigned int numberOfHandles = 0;
    converter << std::hex << reply;                       // insert reply into stringstream
    converter >> numberOfHandles;             // extract number of handles as unsigned byte
    converter.clear();                        // converter must be cleared to be reused
    converter.str("");

    /* read and parse transformation data for each handle */
    for (unsigned int i = 0; i < numberOfHandles; i++)    // for each handle
    {
      /* Read port handle */
      m_TrackingDevice->Receive(&s, 2);        // read port handle
      reply += s;                              // build complete command string
      NDIPassiveTool::Pointer tool = m_TrackingDevice->GetInternalTool(s);   // get tool object for that handle
      if (tool.IsNull())
      {
        returnValue = UNKNOWNHANDLERETURNED;
        break;  // if we do not know the handle, we can not assume anything about the remaining data, so we better abort (we could read up to the next LF)
      }    
      /* Parse reply from tracking device */
      static const std::string missing("MISSING"); 
      static const std::string disabled("DISABLED"); 
      static const std::string unoccupied("UNOCCUPIED");       

      m_TrackingDevice->Receive(&s, 6);        // read next 6 characters: either an error message or part of the transformation data
      reply += s;                              // build complete command string
      if (missing.compare(0, 6, s) == 0)
      {
        tool->SetErrorMessage("Tool is reported as 'missing'.");
        tool->SetDataValid(false);
        m_TrackingDevice->Receive(&s, 18);     // after 'missin', 1 character for 'g', 8 characters for port status, 8 characters for frame number  and one for line feed are send
        reply += s;                            // build complete command string
      }
      else if (disabled.compare(0, 6, s) == 0)
      {
        tool->SetErrorMessage("Tool is reported as 'disabled'.");
        tool->SetDataValid(false);
        m_TrackingDevice->Receive(&s, 3);     // read last characters of disabled plus 8 characters for port status, 8 characters for frame number  and one for line feed
        reply += s;                            // build complete command string
      }
      else if (unoccupied.compare(0, 6, s) == 0)
      {
        tool->SetErrorMessage("Tool is reported as 'unoccupied'.");
        tool->SetDataValid(false);
        m_TrackingDevice->Receive(&s, 21);     // read remaining characters of UNOCCUPIED
        reply += s;                            // build complete command string
      }
      else  // transformation data
      {
        /* define local copies */
        signed int number = 0;
        float localPos[3] = {0.0, 0.0, 0.0};
        float localQuat[4] = {0.0, 0.0, 0.0, 0.0};
        float localError = 0.0;
        unsigned long localPortStatus = 0;
        unsigned int localFrameNumber = 0;
        /* read and parse the four 6 character quaternion values */
        //std::cout << "s =      " << s << std::endl;
        converter << std::dec << s;         // insert string with first number into stringstream
        converter >> number;                // extract first number as integer
        converter.clear();                  // converter must be cleared to be reused
        converter.str("");
        //std::cout << "number = " << number << std::endl;
        localQuat[0] = number / 10000.0;    // the value is send with an implied decimal point with 4 digits to the right
        for (unsigned int i = 1; i < 4; i++)// read the next 3 numbers
        {
          m_TrackingDevice->Receive(&s, 6); // read the next number
          reply += s;                       // build complete command string
          converter << std::dec << s;                   // insert string with first number into stringstream
          converter >> number;              // extract first number as integer
          converter.clear();                // converter must be cleared to be reused
          converter.str("");
          localQuat[i] = number / 10000.0;  // the value is send with an implied decimal point with 4 digits to the right
        }
        /* read and parse the three 7 character translation values */
        for (unsigned int i = 0; i < 3; i++)
        {
          m_TrackingDevice->Receive(&s, 7); // read the next position vector number
          reply += s;                       // build complete command string
          converter << std::dec << s;                   // insert string with number into stringstream
          converter >> number;              // extract first number as integer
          converter.clear();                // converter must be cleared to be reused
          converter.str("");
          localPos[i] = number / 100.0;     // the value is send with an implied decimal point with 2 digits to the right
        }
        /* read and parse 6 character error value */
        m_TrackingDevice->Receive(&s, 6);   // read the error value
        reply += s;                         // build complete command string
        converter << std::dec << s;                     // insert string with number into stringstream
        converter >> number;                // extract the number as integer
        converter.clear();                  // converter must be cleared to be reused
        converter.str("");
        localError = number / 10000.0;      // the error value is send with an implied decimal point with 4 digits to the right
        /* read and parse 8 character port status */
        m_TrackingDevice->Receive(&s, 8);   // read the port status value
        reply += s;                         // build complete command string
        converter << std::hex << s;         // insert string into stringstream
        converter >> localPortStatus;       // extract the number as unsigned long
        converter.clear();                  // converter must be cleared to be reused
        converter.str("");
        /* read and parse 8 character frame number as hexadecimal */
        m_TrackingDevice->Receive(&s, 8);   // read the frame number value
        reply += s;                         // build complete command string
        converter << std::hex << s;         // insert string with hex value encoded number into stringstream
        converter >> localFrameNumber;      // extract the number as unsigned long
        converter.clear();                  // converter must be cleared to be reused
        converter.str("");

        /* copy local values to the tool */
        mitk::Quaternion orientation(localQuat[1], localQuat[2], localQuat[3], localQuat[0]);
        tool->SetOrientation(orientation);
        mitk::Point3D position;
        position[0] = localPos[0];
        position[1] = localPos[1];
        position[2] = localPos[2];
        tool->SetPosition(position);
        tool->SetTrackingError(localError);
        tool->SetErrorMessage("");
        tool->SetDataValid(true);
        m_TrackingDevice->Receive(&s, 1);   // read the line feed character, that terminates each handle data
        reply += s;                         // build complete command string
      }
    } // for


    //Read Reply Option 1000 data
    if(trackIndividualMarkers)
    {

      /* parse number of markers from first 2 characters */
      m_TrackingDevice->Receive(&s, 2);
      reply += s;
      unsigned int numberOfMarkers = 0;
      converter << std::hex << s;           // insert reply into stringstream
      converter >> numberOfMarkers;             // extract number of markers as unsigned byte
      converter.clear();                        // converter must be cleared to be reused
      converter.str(""); 

      unsigned int oovReplySize = (unsigned int)ceil((double)numberOfMarkers/4.0);
      unsigned int nbMarkersInVolume = 0;
      char c;
      // parse oov data to find out how many marker positions were recorded
      for (unsigned int i = 0; i < oovReplySize; i++)
      {
        m_TrackingDevice->ReceiveByte(&c); 
        reply += c;
        nbMarkersInVolume += ByteToNbBitsOn(c);
      }   

      nbMarkersInVolume = numberOfMarkers-nbMarkersInVolume;

      /* read and parse position data for each marker */
      for (unsigned int i = 0; i < nbMarkersInVolume; i++)
      {    
        /* define local copies */   
        signed int number = 0;
        MarkerPointType markerPosition;
        /* read and parse the three 7 character translation values */
        for (unsigned int i = 0; i < 3; i++)
        {
          m_TrackingDevice->Receive(&s, 7); // read the next position vector number
          reply += s;                       // build complete command string
          converter << std::dec << s;       // insert string with number into stringstream
          converter >> number;              // extract first number as integer
          converter.clear();                // converter must be cleared to be reused
          converter.str("");
          markerPosition[i] = number / 100.0;     // the value is send with an implied decimal point with 2 digits to the right
        }
        markerPositions->push_back(markerPosition);

      } // end for all markers
    } 
    //END read Reply Option 1000 data


    /* Read System Status */
    m_TrackingDevice->Receive(&s, 4);       // read system status
    reply += s;                             // build complete command string
    /* now the reply string is complete, perform crc checking */
    std::string expectedCRC = m_TrackingDevice->CalcCRC(&reply);    // calculate crc for received reply string
    std::string readCRC;                    // read attached crc value
    m_TrackingDevice->Receive(&readCRC, 4); // CRC16 is 2 bytes long, which is transmitted as 4 hexadecimal digits
    if (expectedCRC == readCRC)             // if the read CRC is correct, return okay
    {
      returnValue = NDIOKAY;
    }
    else                                    // return error in CRC 
    {
      returnValue = NDICRCERROR;
      /* Invalidate all tools because the received data contained an error */
      m_TrackingDevice->InvalidateAll();
      if(trackIndividualMarkers)
        markerPositions->clear();
    }
  } // else

  /* cleanup and return */
  m_TrackingDevice->Receive(&s, 1);         // read the last linde feed (because the tracking system device is sometimes to slow to send it before we clear the buffer. In this case, the LF would remain in the receive buffer and be read as the first character of the next command
  m_TrackingDevice->ClearReceiveBuffer();   // flush the buffer to remove the remaining carriage return or unknown/unexpected reply
  return returnValue;
}


mitk::NDIErrorCode mitk::NDIProtocol::TX1000(MarkerPointContainerType* markerPositions)
{

  NDIErrorCode returnValue = NDIUNKNOWNERROR; // return code for this function. Will be set according to reply from trackingsystem

  markerPositions->clear();

  if (m_TrackingDevice == NULL)
    return TRACKINGDEVICENOTSET;

  /* send command */
  std::string fullcommand;
  if (m_UseCRC == true)
    fullcommand = "TX:1001";          // command string format 1: with crc
  else
    fullcommand = "TX 1001";          // command string format 2: without crc

  returnValue = m_TrackingDevice->Send(&fullcommand, m_UseCRC);

  /* read number of handles returned */
  std::string reply;
  std::string s;
  m_TrackingDevice->Receive(&reply, 2);       // read first 2 characters of reply (error or number of handles returned)

  static const std::string error("ERROR");
  if (error.compare(0, 2, reply) == 0)
  {
    m_TrackingDevice->Receive(&s, 3);         // read next characters ("ROR" from ERROR)
    reply += s;                               // to build complete reply string
    std::string errorcode;
    m_TrackingDevice->Receive(&errorcode, 2); // now read 2 bytes error code
    reply += errorcode;                       // build complete reply string
    /* perform CRC checking */
    std::string expectedCRC = m_TrackingDevice->CalcCRC(&reply);    // calculate crc for received reply string
    std::string readCRC;                      // read attached crc value
    m_TrackingDevice->Receive(&readCRC, 4);   // CRC16 is 2 bytes long, which is transmitted as 4 hexadecimal digits
    if (expectedCRC == readCRC)               // if the read CRC is correct, return normal error code 
    {
      returnValue = this->GetErrorCode(&errorcode);
    }
    else                                      // return error in CRC 
    {
      returnValue = NDICRCERROR;
    }
  }
  else                                        // transformation data is returned
  {
    /* parse number of handles from first 2 characters */
    std::stringstream converter;
    unsigned int numberOfHandles = 0;
    converter << std::hex << reply;                       // insert reply into stringstream
    converter >> numberOfHandles;             // extract number of handles as unsigned byte
    converter.clear();                        // converter must be cleared to be reused
    converter.str("");

    /* read and parse transformation data for each handle */
    for (unsigned int i = 0; i < numberOfHandles; i++)    // for each handle
    {
      /* Read port handle */
      m_TrackingDevice->Receive(&s, 2);        // read port handle
      reply += s;                              // build complete command string
      NDIPassiveTool::Pointer tool = m_TrackingDevice->GetInternalTool(s);   // get tool object for that handle
      if (tool.IsNull())
      {
        returnValue = UNKNOWNHANDLERETURNED;
        break;  // if we do not know the handle, we can not assume anything about the remaining data, so we better abort (we could read up to the next LF)
      }    
      /* Parse reply from tracking device */
      static const std::string missing("MISSING"); 
      static const std::string disabled("DISABLED"); 
      static const std::string unoccupied("UNOCCUPIED");       

      m_TrackingDevice->Receive(&s, 6);        // read next 6 characters: either an error message or part of the transformation data
      reply += s;                              // build complete command string
      if (missing.compare(0, 6, s) == 0)
      {
        tool->SetErrorMessage("Tool is reported as 'missing'.");
        tool->SetDataValid(false);
        m_TrackingDevice->Receive(&s, 18);     // after 'missin', 1 character for 'g', 8 characters for port status, 8 characters for frame number  and one for line feed are send
        reply += s;                            // build complete command string
      }
      else if (disabled.compare(0, 6, s) == 0)
      {
        tool->SetErrorMessage("Tool is reported as 'disabled'.");
        tool->SetDataValid(false);
        m_TrackingDevice->Receive(&s, 19);     // read last characters of disabled plus 8 characters for port status, 8 characters for frame number  and one for line feed
        reply += s;                            // build complete command string
      }
      else if (unoccupied.compare(0, 6, s) == 0)
      {
        tool->SetErrorMessage("Tool is reported as 'unoccupied'.");
        tool->SetDataValid(false);
        m_TrackingDevice->Receive(&s, 21);     // read remaining characters of UNOCCUPIED
        reply += s;                            // build complete command string
      }
      else  // transformation data
      {
        /* define local copies */
        signed int number = 0;
        float localPos[3] = {0.0, 0.0, 0.0};
        float localQuat[4] = {0.0, 0.0, 0.0, 0.0};
        float localError = 0.0;
        unsigned long localPortStatus = 0;
        unsigned int localFrameNumber = 0;
        /* read and parse the four 6 character quaternion values */
        //std::cout << "s =      " << s << std::endl;
        converter << std::dec << s;         // insert string with first number into stringstream
        converter >> number;                // extract first number as integer
        converter.clear();                  // converter must be cleared to be reused
        converter.str("");
        //std::cout << "number = " << number << std::endl;
        localQuat[0] = number / 10000.0;    // the value is send with an implied decimal point with 4 digits to the right
        for (unsigned int i = 1; i < 4; i++)// read the next 3 numbers
        {
          m_TrackingDevice->Receive(&s, 6); // read the next number
          reply += s;                       // build complete command string
          converter << std::dec << s;                   // insert string with first number into stringstream
          converter >> number;              // extract first number as integer
          converter.clear();                // converter must be cleared to be reused
          converter.str("");
          localQuat[i] = number / 10000.0;  // the value is send with an implied decimal point with 4 digits to the right
        }
        /* read and parse the three 7 character translation values */
        for (unsigned int i = 0; i < 3; i++)
        {
          m_TrackingDevice->Receive(&s, 7); // read the next position vector number
          reply += s;                       // build complete command string
          converter << std::dec << s;                   // insert string with number into stringstream
          converter >> number;              // extract first number as integer
          converter.clear();                // converter must be cleared to be reused
          converter.str("");
          localPos[i] = number / 100.0;     // the value is send with an implied decimal point with 2 digits to the right
        }
        /* read and parse 6 character error value */
        m_TrackingDevice->Receive(&s, 6);   // read the error value
        reply += s;                         // build complete command string
        converter << std::dec << s;                     // insert string with number into stringstream
        converter >> number;                // extract the number as integer
        converter.clear();                  // converter must be cleared to be reused
        converter.str("");
        localError = number / 10000.0;      // the error value is send with an implied decimal point with 4 digits to the right
        /* read and parse 8 character port status */
        m_TrackingDevice->Receive(&s, 8);   // read the port status value
        reply += s;                         // build complete command string
        converter << std::hex << s;         // insert string into stringstream
        converter >> localPortStatus;       // extract the number as unsigned long
        converter.clear();                  // converter must be cleared to be reused
        converter.str("");
        /* read and parse 8 character frame number as hexadecimal */
        m_TrackingDevice->Receive(&s, 8);   // read the frame number value
        reply += s;                         // build complete command string
        converter << std::hex << s;         // insert string with hex value encoded number into stringstream
        converter >> localFrameNumber;      // extract the number as unsigned long
        converter.clear();                  // converter must be cleared to be reused
        converter.str("");

        /* copy local values to the tool */
        mitk::Quaternion orientation(localQuat[1], localQuat[2], localQuat[3], localQuat[0]);
        tool->SetOrientation(orientation);
        mitk::Point3D position;
        position[0] = localPos[0];
        position[1] = localPos[1];
        position[2] = localPos[2];
        tool->SetPosition(position);
        tool->SetTrackingError(localError);
        tool->SetErrorMessage("");
        tool->SetDataValid(true);
        m_TrackingDevice->Receive(&s, 1);   // read the line feed character, that terminates each handle data
        reply += s;                         // build complete command string
      }
    }
    //Read Reply Option 1000 data

    /* parse number of markers from first 2 characters */
    m_TrackingDevice->Receive(&s, 2);
    reply += s;
    unsigned int numberOfMarkers = 0;
    converter << std::hex << s;           // insert reply into stringstream
    converter >> numberOfMarkers;             // extract number of markers as unsigned byte
    converter.clear();                        // converter must be cleared to be reused
    converter.str(""); 

    unsigned int oovReplySize = (unsigned int)ceil((double)numberOfMarkers/4.0);
    unsigned int nbMarkersInVolume = 0;
    char c;
    // parse oov data to find out how many marker positions were recorded
    for (unsigned int i = 0; i < oovReplySize; i++)
    {
      m_TrackingDevice->ReceiveByte(&c); 
      reply += c;
      nbMarkersInVolume += ByteToNbBitsOn(c);
    }   

    nbMarkersInVolume = numberOfMarkers-nbMarkersInVolume;

    /* read and parse position data for each marker */
    for (unsigned int i = 0; i < nbMarkersInVolume; i++)
    {    
      /* define local copies */   
      signed int number = 0;
      MarkerPointType markerPosition;
      /* read and parse the three 7 character translation values */
      for (unsigned int i = 0; i < 3; i++)
      {
        m_TrackingDevice->Receive(&s, 7); // read the next position vector number
        reply += s;                       // build complete command string
        converter << std::dec << s;       // insert string with number into stringstream
        converter >> number;              // extract first number as integer
        converter.clear();                // converter must be cleared to be reused
        converter.str("");
        markerPosition[i] = number / 100.0;     // the value is send with an implied decimal point with 2 digits to the right
      }
      markerPositions->push_back(markerPosition);

    } // end for all markers
    //m_TrackingDevice->Receive(&s, 1);   // read the line feed character, that terminates each handle data
    //reply += s;                         // build complete command string
    //  

    //END read Reply Option 1000 data


    m_TrackingDevice->Receive(&s, 4);       // read system status
    reply += s;                             // build complete command string
    /* now the reply string is complete, perform crc checking */
    std::string expectedCRC = m_TrackingDevice->CalcCRC(&reply);    // calculate crc for received reply string
    std::string readCRC;                    // read attached crc value
    m_TrackingDevice->Receive(&readCRC, 4); // CRC16 is 2 bytes long, which is transmitted as 4 hexadecimal digits
    if (expectedCRC == readCRC)             // if the read CRC is correct, return okay
    {
      returnValue = NDIOKAY;
    }
    else                                    // return error in CRC 
    {
      returnValue = NDICRCERROR;
      /* Invalidate all tools because the received data contained an error */
      markerPositions->clear();
      m_TrackingDevice->InvalidateAll();
    }
  } // else

  /* cleanup and return */
  m_TrackingDevice->Receive(&s, 1);         // read the last linde feed (because the tracking system device is sometimes to slow to send it before we clear the buffer. In this case, the LF would remain in the receive buffer and be read as the first character of the next command
  m_TrackingDevice->ClearReceiveBuffer();   // flush the buffer to remove the remaining carriage return or unknown/unexpected reply
  return returnValue;
}


mitk::NDIErrorCode mitk::NDIProtocol::BX()
{
  std::cout << "BX() not implemented yet, using TX() instead." << std::endl;
  return this->TX();
}


mitk::NDIErrorCode mitk::NDIProtocol::VER(mitk::TrackingDeviceType& t)
{
  if (m_TrackingDevice == NULL)
    return TRACKINGDEVICENOTSET;

  NDIErrorCode returnValue = NDIUNKNOWNERROR; // return code for this function. Will be set according to reply from tracking system
  /* send command */
  std::string fullcommand;
  if (m_UseCRC == true)
    fullcommand = "VER:4";          // command string format 1: with crc
  else
    fullcommand = "VER 4";          // command string format 2: without crc

  returnValue = m_TrackingDevice->Send(&fullcommand, m_UseCRC);
  if (returnValue != NDIOKAY)
  {
    /* cleanup and return */   
    m_TrackingDevice->ClearReceiveBuffer();   // flush the buffer to remove the remaining carriage return or unknown/unexpected reply
    return returnValue;
  }
  /* read number of handles returned */
  std::string reply;
  m_TrackingDevice->Receive(&reply, 5);       // read first 5 characters of reply (error beginning of version information)  
  static const std::string error("ERROR");
  if (error.compare(0, 6, reply) == 0) // ERROR case
  { 
    std::string errorcode;
    m_TrackingDevice->Receive(&errorcode, 2); // now read 2 bytes error code
    reply += errorcode;                       // build complete reply string
    /* perform CRC checking */
    std::string expectedCRC = m_TrackingDevice->CalcCRC(&reply);    // calculate crc for received reply string
    std::string readCRC;                      // read attached crc value
    m_TrackingDevice->Receive(&readCRC, 4);   // CRC16 is 2 bytes long, which is transmitted as 4 hexadecimal digits
    if (expectedCRC == readCRC)               // if the read CRC is correct, return normal error code 
      returnValue = this->GetErrorCode(&errorcode);
    else                                      // return error in CRC 
      returnValue = NDICRCERROR;
  }
  else // no error, valid reply
  {
    std::string s;
    m_TrackingDevice->ReceiveLine(&s);       // read until first LF character
    reply += s;
    std::string upperCaseReply;
    upperCaseReply.resize(reply.size());
    std::transform (reply.begin(), reply.end(), upperCaseReply.begin(), toupper);  // convert reply to uppercase to ease finding 
    if (upperCaseReply.find("POLARIS") != std::string::npos)
      t = mitk::NDIPolaris;
    else if (upperCaseReply.find("AURORA") != std::string::npos)
      t = mitk::NDIAurora;
    else
      t = mitk::TrackingSystemNotSpecified;
    // check for "VICRA", "SPECTRA", "ACCEDO"
    /* do not check for remaining reply, do not check for CRC, just delete remaining reply */
    itksys::SystemTools::Delay(500); // wait until reply should be finished
    m_TrackingDevice->ClearReceiveBuffer();
    returnValue = NDIOKAY;
  }
  return returnValue;
}


mitk::NDIErrorCode mitk::NDIProtocol::POS3D(MarkerPointContainerType* markerPositions)
{
  NDIErrorCode returnValue = NDIUNKNOWNERROR; // return code for this function. Will be set according to reply from trackingsystem

  if (m_TrackingDevice == NULL)
  {
    return TRACKINGDEVICENOTSET;
  }
  if (markerPositions == NULL)
  {
    std::cout << "ERROR: markerPositions==NULL" << std::endl;
    return NDIUNKNOWNERROR;
  }

  markerPositions->clear();   // empty point container
  /* try to obtain a porthandle */
  if (m_TrackingDevice->GetToolCount() == 0)
  {
    std::cout << "ERROR: no tools present" << std::endl;
    return NDIUNKNOWNERROR;
  }
  const TrackingTool* t = m_TrackingDevice->GetTool(static_cast<unsigned int>(0));
  const NDIPassiveTool* t2 = dynamic_cast<const NDIPassiveTool*>(t);
  if (t2 == NULL)
  {
    std::cout << "ERROR: no tool present" << std::endl;
    return NDIUNKNOWNERROR;
  }
  std::string portHandle = t2->GetPortHandle();
  if (portHandle.size() == 0)
  {
    std::cout << "ERROR: no port handle" << std::endl;
    return NDIUNKNOWNERROR;
  }

  /* send command */
  std::string fullcommand;
  if (m_UseCRC == true)
    fullcommand = "3D:" + portHandle + "5";          // command string format 1: with crc
  else
    fullcommand = "3D " + portHandle + "5";          // command string format 2: without crc

  m_TrackingDevice->ClearReceiveBuffer();
  returnValue = m_TrackingDevice->Send(&fullcommand, m_UseCRC);

  /* read number of markers returned */
  std::string reply;
  std::string s;

  mitk::NDIErrorCode receivevalue = m_TrackingDevice->Receive(&reply, 3);       // read first 3 characters of reply (error or number of markers returned)
  if(receivevalue != NDIOKAY)
  {
    std::cout << "ERROR: receive_value != NDIOKAY" << std::endl;
    return receivevalue;
  }

  static const std::string error("ERROR");
  if (error.compare(0, 3, reply) == 0)
  {
    m_TrackingDevice->Receive(&s, 2);         // read next characters ("OR" from ERROR)
    reply += s;                               // to build complete reply string
    std::string errorcode;
    m_TrackingDevice->Receive(&errorcode, 2); // now read 2 bytes error code
    reply += errorcode;                       // build complete reply string
    /* perform CRC checking */
    std::string expectedCRC = m_TrackingDevice->CalcCRC(&reply);    // calculate crc for received reply string
    std::string readCRC;                      // read attached crc value
    m_TrackingDevice->Receive(&readCRC, 4);   // CRC16 is 2 bytes long, which is transmitted as 4 hexadecimal digits
    if (expectedCRC == readCRC)               // if the read CRC is correct, return normal error code 
    {
      returnValue = this->GetErrorCode(&errorcode);
    }
    else                                      // return error in CRC 
    {
      returnValue = NDICRCERROR;
    }
  }
  else                                        // transformation data is returned
  {
    signed int number = 0;
    //float localPos[3] = {0.0, 0.0, 0.0};
    MarkerPointType p;
    float lineSeparation = 0.0;

    /* parse number of markers from first 3 characters */
    std::stringstream converter;
    unsigned int numberOfMarkers = 0;
    converter << std::dec << reply;           // insert reply into stringstream
    converter >> numberOfMarkers;             // extract number of handles as unsigned byte
    converter.clear();                        // converter must be cleared to be reused
    converter.str("");   
    /* read and parse 3D data for each marker */
    for (unsigned int markerID = 0; markerID < numberOfMarkers; markerID++)    // for each marker
    {
      m_TrackingDevice->Receive(&s, 1);       // read line feed
      reply += s;                             // build complete command string
      /* read and parse the three 9 character translation values */
      for (unsigned int i = 0; i < 3; i++)
      {
        receivevalue = m_TrackingDevice->Receive(&s, 9); // read the next position vector number
        if(receivevalue != NDIOKAY) 
        {
          markerPositions->clear();
          std::cout << "ERROR: receive_value != NDIOKAY" << std::endl;
          return receivevalue;
        }
        reply += s;                       // build complete command string
        converter << std::dec << s;       // insert string with number into stringstream
        converter >> number;              // extract the number as integer
        converter.clear();                // converter must be cleared to be reused
        converter.str("");
        p[i] = number / 10000.0;          // the value is send with an implied decimal point with 4 digits to the right
      }
      /* read and parse 4 character line separation value */
      receivevalue = m_TrackingDevice->Receive(&s, 4);   // read the line separation value
      if(receivevalue != NDIOKAY) 
      {
        markerPositions->clear();
        std::cout << "ERROR: receive_value != NDIOKAY" << std::endl;
        return receivevalue;
      }
      reply += s;                         // build complete command string
      converter << std::dec << s;         // insert string with number into stringstream
      converter >> number;                // extract the number as integer
      converter.clear();                  // converter must be cleared to be reused
      converter.str("");
      lineSeparation = number / 100.0;    // the line separation value is send with an implied decimal point with 2 digits to the right
      /* read and parse 1 character out of volume value */
      receivevalue = m_TrackingDevice->Receive(&s, 1);   // read the port status value
      if(receivevalue != NDIOKAY) 
      {
        markerPositions->clear();
        std::cout << std::endl << std::endl << std::endl << "ERROR: POS3D != NDIOKAY" << std::endl;
        return receivevalue;
      }
      reply += s;                         // build complete command string
      /* store the marker positions in the point container */
      markerPositions->push_back(p);
    }    
    //std::cout << "INFO: Found " << markerPositions->size() << " markers." << std::endl;

    /* now the reply string is complete, perform crc checking */
    std::string expectedCRC = m_TrackingDevice->CalcCRC(&reply);    // calculate crc for received reply string
    std::string readCRC;                    // read attached crc value
    m_TrackingDevice->Receive(&readCRC, 4); // CRC16 is 2 bytes long, which is transmitted as 4 hexadecimal digits
    if (expectedCRC == readCRC)             // if the read CRC is correct, return okay
    {
      returnValue = NDIOKAY;
    }
    else                                    // return error in CRC 
    {
      returnValue = NDICRCERROR;
      std::cout << "ERROR: receive_value != NDIOKAY" << std::endl;
      /* delete all marker positions because the received data contained an error */
      markerPositions->clear();
    }
  } // else

  /* cleanup and return */
  m_TrackingDevice->Receive(&s, 1);         // read the last linde feed (because the tracking system device is sometimes to slow to send it before we clear the buffer. In this case, the LF would remain in the receive buffer and be read as the first character of the next command
  m_TrackingDevice->ClearReceiveBuffer();   // flush the buffer to remove the remaining carriage return or unknown/unexpected reply

  return returnValue;
}


mitk::NDIErrorCode mitk::NDIProtocol::GenericCommand(const std::string command, const std::string* parameter)
{
  NDIErrorCode returnValue = NDIUNKNOWNERROR; // return code for this function. Will be set according to reply from trackingsystem

  if (m_TrackingDevice == NULL)
    return TRACKINGDEVICENOTSET;

  std::string p;
  if (parameter != NULL)
    p = *parameter;
  else
    p = "";

  /* send command */
  std::string fullcommand;
  if (m_UseCRC == true)
    fullcommand = command + ":" + p;          // command string format 1: with crc
  else
    fullcommand = command + " " + p;          // command string format 2: without crc

  returnValue = m_TrackingDevice->Send(&fullcommand, m_UseCRC);

  if (returnValue != NDIOKAY)                 // check for send error
  {
    m_TrackingDevice->ClearReceiveBuffer();   // flush the buffer to remove any reply
    return returnValue;
  }
  /* wait for the trackingsystem to process the command */
  itksys::SystemTools::Delay(100);

  /* read and parse the reply from tracking device */
  // the reply for a generic command can be OKAY or ERROR##    
  // so we can use the generic parse method for these replies
  this->ParseOkayError();
  return returnValue;
}


unsigned int mitk::NDIProtocol::ByteToNbBitsOn(char& c) const
{
  if(c == '0')
    return 0;
  else if (c == '1' || c == '2' || c == '4' || c == '8')
    return 1;
  else if (c == '3' || c == '5' || c == '9' || c == '6' || c == 'A' || c == 'C')
    return 2;
  else if (c == '7' || c == 'B' || c == 'D' || c == 'E')
    return 3;
  else if (c == 'F')
    return 4;
  else
    return 0;
}


mitk::NDIErrorCode mitk::NDIProtocol::ParseOkayError()
{
  NDIErrorCode returnValue  = NDIUNKNOWNERROR;
  /* read reply from tracking device */
  // the reply is expected to be OKAY or ERROR##    
  // define reply strings
  std::string reply;
  m_TrackingDevice->Receive(&reply, 4);       // read first 4 characters of reply

  /* Parse reply from tracking device */
  static const std::string okay("OKAYA896");  // OKAY is static, so we can perform a static crc check
  static const std::string error("ERROR"); 

  if (okay.compare(0, 4, reply) == 0)         // check for "OKAY": compare first 4 characters from okay with reply
  {
    // OKAY was found, now check the CRC16 too
    m_TrackingDevice->Receive(&reply, 4);     // read 4 hexadecimal characters for CRC16
    if (okay.compare(4, 4, reply, 0, 4) == 0) // first 4 from new reply should match last 4 from okay
      returnValue = NDIOKAY;
    else
      returnValue = NDICRCERROR;
  } 
  else if (error.compare(0, 4, reply) == 0)   // check for "ERRO"
  {
    char b;                                   // The ERROR reply is not static, so we can not use a static crc check.
    m_TrackingDevice->ReceiveByte(&b);        // read next character ("R" from ERROR)
    reply += b;                               // to build complete reply string
    std::string errorcode;
    m_TrackingDevice->Receive(&errorcode, 2); // now read 2 bytes error code
    reply += errorcode;                       // build complete reply string
    /* perform CRC checking */
    std::string expectedCRC = m_TrackingDevice->CalcCRC(&reply);    // calculate crc for received reply string
    std::string readCRC;                      // read attached crc value
    m_TrackingDevice->Receive(&readCRC, 4);   // CRC16 is 2 bytes long, which is transmitted as 4 hexadecimal digits
    if (expectedCRC == readCRC)               // if the read CRC is correct, return normal error code 
      returnValue = this->GetErrorCode(&errorcode);
    else                                      // return error in CRC 
      returnValue = NDICRCERROR;
  } 
  else                                      // something else, that we do not expect
    returnValue = NDIUNEXPECTEDREPLY;

  /* cleanup and return */
  char b; 
  m_TrackingDevice->ReceiveByte(&b);          // read CR character
  m_TrackingDevice->ClearReceiveBuffer();     // flush the buffer to remove the remaining carriage return or unknown/unexpected reply
  return returnValue;
}


mitk::NDIErrorCode mitk::NDIProtocol::GetErrorCode(const std::string* input)
{
  if (input->compare("01") == 0)
    return NDIINVALIDCOMMAND;
  else if (input->compare("02") == 0)
    return NDICOMMANDTOOLONG;
  else if (input->compare("03") == 0)
    return NDICOMMANDTOOSHORT;
  else if (input->compare("04") == 0)
    return NDICRCDOESNOTMATCH;
  else if (input->compare("05") == 0)
    return NDITIMEOUT;
  else if (input->compare("06") == 0)
    return NDIUNABLETOSETNEWCOMMPARAMETERS;
  else if (input->compare("07") == 0)
    return NDIINCORRECTNUMBEROFPARAMETERS;
  else if (input->compare("08") == 0)
    return NDIINVALIDPORTHANDLE;
  else if (input->compare("09") == 0)
    return NDIINVALIDTRACKINGPRIORITY;
  else if (input->compare("0A") == 0)
    return NDIINVALIDLED;
  else if (input->compare("0B") == 0)
    return NDIINVALIDLEDSTATE;
  else if (input->compare("0C") == 0)
    return NDICOMMANDINVALIDINCURRENTMODE;
  else if (input->compare("0D") == 0)
    return NDINOTOOLFORPORT;
  else if (input->compare("0E") == 0)
    return NDIPORTNOTINITIALIZED;

  // ...
  else if (input->compare("10") == 0)
    return NDISYSTEMNOTINITIALIZED;
  else if (input->compare("11") == 0)
    return NDIUNABLETOSTOPTRACKING;
  else if (input->compare("12") == 0)
    return NDIUNABLETOSTARTTRACKING;
  else if (input->compare("13") == 0)
    return NDIINITIALIZATIONFAILED;
  else if (input->compare("14") == 0)
    return NDIINVALIDVOLUMEPARAMETERS;
  else if (input->compare("16") == 0)
    return NDICANTSTARTDIAGNOSTICMODE;
  else if (input->compare("1B") == 0)
    return NDICANTINITIRDIAGNOSTICS;
  else if (input->compare("1F") == 0)
    return NDIFAILURETOWRITESROM;
  else if (input->compare("22") == 0)
    return NDIENABLEDTOOLSNOTSUPPORTED;
  else if (input->compare("23") == 0)
    return NDICOMMANDPARAMETEROUTOFRANGE;
  else if (input->compare("2A") == 0)
    return NDINOMEMORYAVAILABLE;
  else if (input->compare("2B") == 0)
    return NDIPORTHANDLENOTALLOCATED;
  else if (input->compare("2C") == 0)
    return NDIPORTHASBECOMEUNOCCUPIED;
  else if (input->compare("2D") == 0)
    return NDIOUTOFHANDLES;
  else if (input->compare("2E") == 0)
    return NDIINCOMPATIBLEFIRMWAREVERSIONS;
  else if (input->compare("2F") == 0)
    return NDIINVALIDPORTDESCRIPTION;
  else if (input->compare("32") == 0)
    return NDIINVALIDOPERATIONFORDEVICE;
  // ...
  else 
    return NDIUNKNOWNERROR;
}
