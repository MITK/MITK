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

#include "mitkSerialCommunication.h"

#ifdef WIN32
//#include <atlstr.h>
#include <itksys/SystemTools.hxx>
#else // Posix
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <cerrno>

#define INVALID_HANDLE_VALUE -1
#endif

#define OK 1
#define ERROR_VALUE 0

mitk::SerialCommunication::SerialCommunication() : itk::Object(),
  m_DeviceName(""), m_PortNumber(COM1), m_BaudRate(BaudRate9600),
  m_DataBits(DataBits8), m_Parity(None), m_StopBits(StopBits1),
  m_HardwareHandshake(HardwareHandshakeOff),
  m_ReceiveTimeout(500), m_SendTimeout(500), m_Connected(false)
{
#ifdef  WIN32 // Windows
  m_ComPortHandle = INVALID_HANDLE_VALUE;
#else // Posix
  m_FileDescriptor = INVALID_HANDLE_VALUE;
#endif
}

mitk::SerialCommunication::~SerialCommunication()
{
  CloseConnection();
}

bool mitk::SerialCommunication::IsConnected()
{
  return m_Connected;
}

int mitk::SerialCommunication::OpenConnection()
{
  if (m_Connected)
    return ERROR_VALUE;

#ifdef WIN32
  std::stringstream ss;
  if (m_DeviceName.empty())
    ss << "\\\\.\\COM" << static_cast<unsigned int>(m_PortNumber); // use m_PortNumber
  else
    ss << "\\\\.\\" << m_DeviceName; // use m_DeviceName

  m_ComPortHandle = CreateFile(ss.str().c_str(), GENERIC_READ | GENERIC_WRITE,
    0,             // no sharing
    0,             // no security flags
    OPEN_EXISTING, // open com port, don't create it
    0,             // no flags
    0);            // no template
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
  std::stringstream ss;
  if (m_DeviceName.empty())
    ss << "/dev/ttyS" << static_cast<unsigned int>(m_PortNumber) - 1; // use m_PortNumber, COM1 = ttyS0
  else
    ss << m_DeviceName; // use m_DeviceName

  //m_FileDescriptor = open(ss.str().c_str(), O_RDWR | O_NONBLOCK | O_NDELAY | O_NOCTTY | O_EXCL); // open device file
  m_FileDescriptor = open(ss.str().c_str(), O_RDWR|O_NONBLOCK|O_EXCL); // open device file
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

int mitk::SerialCommunication::Receive(std::string& answer, unsigned int numberOfBytes, const char *eol)
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
  if (ReadFile(m_ComPortHandle, buffer, numberOfBytes, &numberOfBytesRead, nullptr) != 0)
  {
    if (numberOfBytesRead > 0) // data read
    {
      answer.assign(buffer, numberOfBytesRead); // copy buffer to answer
      delete buffer;
      if (numberOfBytesRead == numberOfBytes)
      {
        return OK;           // everything was received
      }
      else
      {
        return ERROR_VALUE;  // some data was received, but not as much as expected
      }
    }
    else // error
    {
      answer = "";
      delete buffer;
      return ERROR_VALUE;
    }
  }
  delete buffer;
  return OK;

#else  // Posix
  if (m_FileDescriptor == INVALID_HANDLE_VALUE)
    return ERROR_VALUE;

  unsigned long bytesRead = 0;
  unsigned long bytesLeft = numberOfBytes;
  auto  buffer = new char[numberOfBytes];

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

    if (eol && *eol == buffer[bytesRead-1]) // end of line char reached
      break;
  }
  if (bytesRead > 0)
    answer.assign(buffer, bytesRead); // copy buffer to answer

  delete[] buffer;

  if ( bytesRead == numberOfBytes ||                            // everything was received
       (eol && answer.size() > 0 && *eol == answer.at(answer.size()-1)) )  // end of line char reached
    return OK;
  else
    return ERROR_VALUE;  // some data was received, but not as much as expected
#endif
}

int mitk::SerialCommunication::Send(const std::string& input, bool block)
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
  if (WriteFile(m_ComPortHandle, input.data(), static_cast<DWORD>(input.size()), &bytesWritten, nullptr) == TRUE)
    return OK;
  else
    return ERROR_VALUE;

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
  if (block)
  {
    // wait for output to be physically sent
    if (tcdrain(m_FileDescriptor) == -1)
      return ERROR_VALUE;
  }
  return OK;
#endif
}

int mitk::SerialCommunication::ApplyConfiguration()
{
#ifdef WIN32 // Windows implementation
  return ApplyConfigurationWin();
#else // Posix
  return ApplyConfigurationUnix();
#endif
}

/**
* The Code for Baud rates is highly platform specific and divided into separate subroutines for readability.
*/

#ifdef WIN32
int mitk::SerialCommunication::ApplyConfigurationWin()
{
   if (m_ComPortHandle == INVALID_HANDLE_VALUE)
    return ERROR_VALUE;

  DCB controlSettings;
  if (GetCommState(m_ComPortHandle, &controlSettings) == 0)
  {
    return ERROR_VALUE;
  }

  std::ostringstream o;
  o << "baud=" << m_BaudRate << " parity=" << static_cast<char>(m_Parity) << " data=" << m_DataBits << " stop=" << m_StopBits;
  if (BuildCommDCBA(o.str().c_str(), &controlSettings)  == 0) // Build device-control block
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
    return ERROR_VALUE;

  COMMTIMEOUTS timeouts;

  timeouts.ReadIntervalTimeout = m_ReceiveTimeout;
  timeouts.ReadTotalTimeoutMultiplier = 0;
  timeouts.ReadTotalTimeoutConstant = m_ReceiveTimeout;
  timeouts.WriteTotalTimeoutMultiplier = 0;
  timeouts.WriteTotalTimeoutConstant = m_SendTimeout;
  if (SetCommTimeouts(m_ComPortHandle, &timeouts) == FALSE)  // set timeout values
    return ERROR_VALUE;

  PurgeComm(m_ComPortHandle, PURGE_TXCLEAR | PURGE_RXCLEAR);  // clear read and write buffers
  return OK;
}

#else
/**
* \brief Applies the configuration for Linux
*/
int mitk::SerialCommunication::ApplyConfigurationUnix()
{
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
  case Odd:
    termIOStructure.c_cflag |= (PARENB|PARODD);
    break;
  case Even:
    termIOStructure.c_cflag |= PARENB;
    termIOStructure.c_cflag &= ~PARODD;
    // TODO: check if this is intended
    // FALLTHRU
  case None:
    // FALLTHRU
  default:
    termIOStructure.c_cflag &= ~PARENB;
    break;
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
  case BaudRate230400:
    baudrate = B230400;
    break;
  // the following baud rates do not work for apple
  #ifndef __APPLE__
  case BaudRate460800:
    baudrate = B460800;
    break;
  case BaudRate500000:
    baudrate = B500000;
    break;
  case BaudRate576000:
    baudrate = B576000;
    break;
  case BaudRate921600:
    baudrate = B921600;
    break;
  case BaudRate1000000:
    baudrate = B1000000;
    break;
  case BaudRate1152000:
    baudrate = B1152000;
    break;
  //case BaudRate1228739:
    //baudrate = B1228739;
    //break;
  case BaudRate1500000:
    baudrate = B1500000;
    break;
  case BaudRate2000000:
    baudrate = B2000000;
    break;
  case BaudRate2500000:
    baudrate = B2500000;
    break;
  case BaudRate3000000:
    baudrate = B3000000;
    break;
  case BaudRate3500000:
    baudrate = B3500000;
    break;
  case BaudRate4000000:
    baudrate = B4000000;
    break;
  #endif
  default:
    MITK_WARN("mitk::SerialCommunication") << "Baud rate not recognized, using default of 9600 Baud.";
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
}

#endif


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
