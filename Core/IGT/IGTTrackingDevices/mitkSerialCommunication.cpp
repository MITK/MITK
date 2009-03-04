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

#include "mitkSerialCommunication.h"


#ifdef WIN32
  #include <atlstr.h>
  #include <itksys/SystemTools.hxx>
#else // Posix
  #include <sys/types.h>
  #include <sys/stat.h>
  #include <sys/time.h>
  #include <sys/ioctl.h>
  #include <fcntl.h>
  #include <unistd.h>
  #include <termios.h> 
  #include <errno.h>

  #define INVALID_HANDLE_VALUE -1
#endif

#define OK 1
#define ERROR_VALUE 0


mitk::SerialCommunication::SerialCommunication() : itk::Object(),
m_PortNumber(COM1), m_BaudRate(BaudRate9600),
m_DataBits(DataBits8), m_Parity(None), m_StopBits(StopBits1),
m_HardwareHandshake(HardwareHandshakeOff),
m_ReceiveTimeout(500), m_SendTimeout(500), m_Connected(false)
{
#ifdef	WIN32 // Windows
  m_ComPortHandle = INVALID_HANDLE_VALUE;
#else // Posix
  m_FileDescriptor = INVALID_HANDLE_VALUE;
#endif
}


mitk::SerialCommunication::~SerialCommunication()
{
  CloseConnection();
}


int mitk::SerialCommunication::OpenConnection()
{
  if (m_Connected)
    return ERROR_VALUE;

#ifdef WIN32
  //CString port = _T("COM") + static_cast<unsigned int>(m_PortNumber);  // <-- original
  std::stringstream ss;
  ss << "COM" << static_cast<unsigned int>(m_PortNumber);
  std::string port = ss.str(); 

  m_ComPortHandle = CreateFile(port.c_str(), GENERIC_READ | GENERIC_WRITE,
    NULL,    /* no sharing */
    NULL, /* no security flags */
    OPEN_EXISTING, /* open com port, don't create it */
    NULL, /* no flags */
    NULL); /* no template */
  if (m_ComPortHandle == INVALID_HANDLE_VALUE) 
    return ERROR_VALUE;

  GetCommState(m_ComPortHandle, &m_PreviousDeviceControlBlock);
  GetCommTimeouts(m_ComPortHandle, &m_PreviousTimeout);
  GetCommMask(m_ComPortHandle, &m_PreviousMask);

  if (this->ApplyConfiguration() != OK)    // set interface parameters
  {
    CloseHandle(m_ComPortHandle);
    m_ComPortHandle = INVALID_HANDLE_VALUE;
    return ERROR_VALUE;
  }
  m_Connected = true;
  return OK;

#else // Posix
  std::string deviceName;
  if (m_DeviceName.empty())
    deviceName = "/dev/ttyS" + static_cast<unsigned int>(m_PortNumber) - 1; // COM1 = ttyS0
  else
    deviceName = m_DeviceName;

  //m_FileDescriptor = open(deviceName.c_str(), O_RDWR | O_NONBLOCK | O_NDELAY | O_NOCTTY | O_EXCL); // open device file
m_FileDescriptor = open(deviceName.c_str(), O_RDWR|O_NONBLOCK|O_EXCL); // open device file
  if (m_FileDescriptor < 0) 
    return ERROR_VALUE; 

  fcntl(m_FileDescriptor, F_SETFL, 0);    // change to blocking mode
  tcflush(m_FileDescriptor, TCIOFLUSH);   // flush buffers
  if (this->ApplyConfiguration() != OK)    // set interface parameters
  {
    close(m_FileDescriptor);
    m_FileDescriptor = INVALID_HANDLE_VALUE;
    return ERROR_VALUE;
  }
  m_Connected = true;
  return OK;
#endif
}


void mitk::SerialCommunication::CloseConnection()
{
#ifdef WIN32
  if (m_ComPortHandle == INVALID_HANDLE_VALUE)
    return;
  ClearReceiveBuffer();
  ClearSendBuffer();
  SetCommState(m_ComPortHandle, &m_PreviousDeviceControlBlock); // restore previous settings
  SetCommTimeouts(m_ComPortHandle, &m_PreviousTimeout);         // restore previous timeout values
  SetCommMask(m_ComPortHandle, m_PreviousMask);                 // restore previous mask value
  PurgeComm(m_ComPortHandle, PURGE_TXCLEAR | PURGE_RXCLEAR);    // empty buffers
  CloseHandle(m_ComPortHandle);                                 // close handle
  m_ComPortHandle = INVALID_HANDLE_VALUE;
  m_Connected = false;
  return;

#else // Posix
  if (m_FileDescriptor == INVALID_HANDLE_VALUE)
    return;
  ClearReceiveBuffer();
  ClearSendBuffer();
  close(m_FileDescriptor);
  m_FileDescriptor = INVALID_HANDLE_VALUE;
  m_Connected = false;
  return;
#endif
}


int mitk::SerialCommunication::Receive(std::string& answer, unsigned int numberOfBytes)
{
  if (numberOfBytes == 0)
    return OK;
  if (m_Connected == false)
    return ERROR_VALUE;

#ifdef WIN32
  if (m_ComPortHandle == INVALID_HANDLE_VALUE)
    return ERROR_VALUE;

  DWORD numberOfBytesRead = 0;
  char* buffer = new char[numberOfBytes];
  if (ReadFile(m_ComPortHandle, buffer, numberOfBytes, &numberOfBytesRead, NULL) != 0)
  {
    if (numberOfBytesRead > 0) // data read
    {
      answer.assign(buffer, numberOfBytesRead); // copy buffer to answer
      if (numberOfBytesRead == numberOfBytes)
        return OK;           // everything was received
      else
        return ERROR_VALUE;  // some data was received, but not as much as expected
    }
    else // error
    {
      answer = "";
      return ERROR_VALUE;
    }
  }
  delete buffer;
  return OK;


#else  // Posix
  if (m_FileDescriptor == INVALID_HANDLE_VALUE)
    return ERROR_VALUE;

  long bytesRead = 0;
  long bytesLeft = numberOfBytes;
  char* buffer = new char[numberOfBytes];

  while ((bytesLeft > 0) && (bytesRead < numberOfBytes))
  {
    int num = read(m_FileDescriptor, &buffer[bytesRead], 1); // read one byte
    if (num == -1) // ERROR_VALUE
    {
      if (errno == EAGAIN) // nonblocking, no byte there right now, but maybe next time
        continue; 
      else
        break; // ERROR_VALUE, stop trying to read
    }
    if (num == 0) // timeout or eof(?)
      break;

    bytesLeft -= num;  // n is number of chars left to read
    bytesRead += num;  // i is the number of chars read
  }
  if (bytesRead > 0)
    answer.assign(buffer, bytesRead); // copy buffer to answer
  delete buffer;
  if (bytesRead == numberOfBytes)
    return OK;           // everything was received
  else
    return ERROR_VALUE;  // some data was received, but not as much as expected
#endif
}


int mitk::SerialCommunication::Send(const std::string& input)
{
  //long retval = E2ERR_OPENFAILED;
  if (input.empty())
    return OK;
  if (m_Connected == false)
    return ERROR_VALUE;

#ifdef WIN32
  if (m_ComPortHandle == INVALID_HANDLE_VALUE)
    return ERROR_VALUE;

  DWORD bytesWritten = 0;
  if (WriteFile(m_ComPortHandle, input.data(), input.size(), &bytesWritten, NULL) == TRUE)
    return OK;
  else
    return GetLastError();

#else // Posix
  if (m_FileDescriptor == INVALID_HANDLE_VALUE)
    return ERROR_VALUE;

  long bytesWritten = 0;
  long bytesLeft = input.size();

  while (bytesLeft > 0)
  {
    bytesWritten = write(m_FileDescriptor, input.data() + bytesWritten, bytesLeft);
    if (bytesWritten <= 0)
      return ERROR_VALUE; //return ERROR_VALUE
    bytesLeft -= bytesWritten;
  }
  return OK;
#endif
}


int mitk::SerialCommunication::ApplyConfiguration()
{
#ifdef WIN32 // Windows implementation
  if (m_ComPortHandle == INVALID_HANDLE_VALUE)
    return ERROR_VALUE;

  DCB controlSettings;
  if (GetCommState(m_ComPortHandle, &controlSettings) == 0)
  {
    return ERROR_VALUE;
  }

  CString deviceControlString;
  deviceControlString.Format(_T("baud=%u parity=%c data=%d stop=%d"), m_BaudRate, m_Parity, m_DataBits, m_StopBits); // set baudrate, parity, data bits and stop bits setting
  if (BuildCommDCBA(deviceControlString, &controlSettings)  == 0) // Build device-control block
    return ERROR_VALUE;

  if (m_HardwareHandshake == HardwareHandshakeOn) // Modify hardware handshake values
  {
    controlSettings.fDtrControl = DTR_CONTROL_ENABLE;
    controlSettings.fRtsControl = RTS_CONTROL_ENABLE;
    controlSettings.fOutxCtsFlow = TRUE;
    controlSettings.fRtsControl = RTS_CONTROL_HANDSHAKE;
  }
  else
  {
    controlSettings.fDtrControl = DTR_CONTROL_DISABLE;
    controlSettings.fRtsControl = RTS_CONTROL_DISABLE;
    controlSettings.fOutxCtsFlow = FALSE;
    controlSettings.fRtsControl = RTS_CONTROL_DISABLE;
  }
  if (SetCommState(m_ComPortHandle, &controlSettings) == FALSE)    // Configure com port
    return GetLastError();

  COMMTIMEOUTS timeouts;


  timeouts.ReadIntervalTimeout = m_ReceiveTimeout;
  timeouts.ReadTotalTimeoutMultiplier = 0;
  timeouts.ReadTotalTimeoutConstant = m_ReceiveTimeout;
  timeouts.WriteTotalTimeoutMultiplier = 0;
  timeouts.WriteTotalTimeoutConstant = m_SendTimeout;
  if (SetCommTimeouts(m_ComPortHandle, &timeouts) == FALSE)  // set timeout values
    return GetLastError();

  PurgeComm(m_ComPortHandle, PURGE_TXCLEAR | PURGE_RXCLEAR);  // clear read and write buffers
  return OK;

#else // Posix

  if ( m_FileDescriptor == INVALID_HANDLE_VALUE )
    return ERROR_VALUE;

  struct termios termIOStructure;
  if ( tcgetattr(m_FileDescriptor, &termIOStructure) != 0 ) // retrieve parameters from com port
    return ERROR_VALUE;

  cfmakeraw(&termIOStructure);        // set flags to raw mode
  termIOStructure.c_cflag |= CLOCAL;
  if (m_HardwareHandshake == HardwareHandshakeOn)
  { // enable
    termIOStructure.c_cflag |= CRTSCTS;
    termIOStructure.c_iflag &= ~(IXON|IXOFF);
  }
  else
  { // disable
    termIOStructure.c_cflag &= ~CRTSCTS;
    termIOStructure.c_iflag &= ~(IXON|IXOFF);
  }
  termIOStructure.c_cflag &= ~CSIZE;  // set number of data bits
  switch (m_DataBits)
  {
  case DataBits7:
    termIOStructure.c_cflag |= CS7;
    break;
  case DataBits8:
  default:
    termIOStructure.c_cflag |= CS8;
  }
  switch (m_StopBits)  // set number of stop bits
  {
  case StopBits2:
    termIOStructure.c_cflag |= CSTOPB;
    break;
  case StopBits1:
  default:
    termIOStructure.c_cflag &= ~CSTOPB;
  }
  switch (m_Parity)  // set parity
  {
  case None:
    termIOStructure.c_cflag &= ~PARENB;
    break;
  case Odd:
    termIOStructure.c_cflag |= (PARENB|PARODD);
    break;
  case Even:
  default:
    termIOStructure.c_cflag |= PARENB;
    termIOStructure.c_cflag &= ~PARODD;
  }
  speed_t baudrate; // set baudrate
  switch (m_BaudRate)
  {
  case BaudRate9600:
    baudrate = B9600;
    break;
  case BaudRate14400:
    baudrate = B9600;  //14400 is not defined for posix, use 9600 instead
    break;
  case BaudRate19200:
    baudrate = B19200;
    break;
  case BaudRate38400:
    baudrate = B38400;
    break;
  case BaudRate57600:
    baudrate = B57600;
    break;
  case BaudRate115200:
    baudrate = B115200;
    break;
  default:
    baudrate = B9600;
    break;
  }
  cfsetispeed(&termIOStructure, baudrate);
  cfsetospeed(&termIOStructure, baudrate); 

  termIOStructure.c_cc[VMIN] = 0;
  termIOStructure.c_cc[VTIME] = m_ReceiveTimeout / 100; // timeout in 1/10 sec, not in ms. Rounded down.

  if (tcsetattr(m_FileDescriptor, TCSANOW, &termIOStructure) == 0)
    return OK;
  else
    return ERROR_VALUE;
#endif
}


void mitk::SerialCommunication::SendBreak(unsigned int ms)
{
#ifdef WIN32
  if (m_ComPortHandle == INVALID_HANDLE_VALUE)
    return;
  SetCommBreak(m_ComPortHandle);
  itksys::SystemTools::Delay(ms);
  ClearCommBreak(m_ComPortHandle);
  return;

#else  // Posix
  if (m_FileDescriptor == INVALID_HANDLE_VALUE)
    return;
  tcsendbreak(m_FileDescriptor, ms);
  return;
#endif
}
 

void mitk::SerialCommunication::ClearReceiveBuffer()
{
#ifdef WIN32
  if (m_ComPortHandle != INVALID_HANDLE_VALUE)
    PurgeComm(m_ComPortHandle, PURGE_RXCLEAR);
#else // Posix
  if (m_FileDescriptor != INVALID_HANDLE_VALUE)
    tcflush(m_FileDescriptor, TCIFLUSH);
#endif
}


void mitk::SerialCommunication::ClearSendBuffer()
{
#ifdef WIN32
  if ( m_ComPortHandle != INVALID_HANDLE_VALUE )
    PurgeComm(m_ComPortHandle, PURGE_TXCLEAR);
#else // Posix
  if ( m_FileDescriptor != INVALID_HANDLE_VALUE )
    tcflush(m_FileDescriptor, TCOFLUSH);
#endif
}
