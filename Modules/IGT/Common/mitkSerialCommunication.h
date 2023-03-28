/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkSerialCommunication_h
#define mitkSerialCommunication_h

#include <MitkIGTExports.h>
#include "mitkCommon.h"
#include <itkObject.h>
#include <itkObjectFactory.h>

#ifdef WIN32
#include <windows.h>
#else // Posix
#include <termios.h>
#endif

namespace mitk
{
  /**Documentation
  * \brief serial communication interface
  *
  * This class allows to send and receive data over a serial communication interface (COM Port).
  * Define the serial interface that should be used either with SetPortNumber() or SetDeviceName()
  * Next, define communication parameters: baud rate, number of data bits, number of stop bits,
  * parity mode, usage of hardware handshake and timeout values (in ms).
  * Use OpenConnection() to establish a connection on the serial interface with the selected
  * parameters. While the connection is established, changes to the parameters will not take
  * effect. You have to close the connection using CloseConnection() and then reopen it with
  * the new parameters with OpenConnection().
  *
  * \ingroup IGT
  */
  class MITKIGT_EXPORT SerialCommunication : public itk::Object
  {
  public:
    mitkClassMacroItkParent(SerialCommunication, itk::Object);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

    enum PortNumber
    {
      COM1 = 1,
      COM2 = 2,
      COM3 = 3,
      COM4 = 4,
      COM5 = 5,
      COM6 = 6,
      COM7 = 7,
      COM8 = 8,
      COM9 = 9,
      COM10 = 10,
      COM11 = 11,
      COM12 = 12,
      COM13 = 13
    };

    enum BaudRate
    {
      BaudRate9600    = 9600,
      BaudRate14400   = 14400,
      BaudRate19200   = 19200,
      BaudRate38400   = 38400,
      BaudRate57600   = 57600,
      BaudRate115200  = 115200, // Highest supported rate for NDI Aurora
      BaudRate230400  = 230400,
      BaudRate460800  = 460800,
      BaudRate500000  = 500000,
      BaudRate576000  = 576000,
      BaudRate921600  = 921600,
      BaudRate1000000 = 1000000,
      BaudRate1152000 = 1152000,
      // BaudRate1228739 = 1228739, // Highest supported rate for NDI Polaris According to handbook, unknown value to most compilers though
      BaudRate1500000 = 1500000,
      BaudRate2000000 = 2000000,
      BaudRate2500000 = 2500000,
      BaudRate3000000 = 3000000,
      BaudRate3500000 = 3500000,
      BaudRate4000000 = 4000000
    };

    enum DataBits
    {
      DataBits8 = 8,
      DataBits7 = 7
    };

    enum Parity
    {
      None = 'N',
      Odd  = 'O',
      Even = 'E'
    };

    enum StopBits
    {
      StopBits1 = 1,
      StopBits2 = 2
    };

    enum HardwareHandshake
    {
      HardwareHandshakeOn = 1,
      HardwareHandshakeOff = 0
    };

    /**
    * \brief Returns m_Connected
    *
    */
    bool IsConnected();
    /**
    * \brief Opens connection to the COM port with port number m_PortNumber
    * or the device name m_DeviceName and all port settings.
    *
    */
    int OpenConnection();
    /**
    * \brief Closes the connection
    *
    */
    void CloseConnection();

    /**
    * \brief Read numberOfBytes characters from the serial interface
    *
    * This method tries to read numberOfBytes characters from the serial
    * interface or until an eol byte is received, whichever comes first. If
    * The ReceiveTimeout is set to 0, the Receive() method will wait
    * indefinetly until all characters are received or an eol character is
    * received. If the ReceiveTimeout is set to another value, it will return
    * after m_ReceiveTimeout milliseconds (or after all characters are read or
    * an eol character is received).
    *
    * \param[out] answer  String that stores the received characters. Note
    *                     that this will overwrite the content of answer!
    * \param[in] numberOfBytes  The number of bytes to read. When an eol
    *                           character is used this is interpretted as the
    *                           maximum number of bytes to read.
    * \param[in] eol  Pointer to an End-of-Line character. If this is nullptr
    *                 (the default) then no End-of-Line character is used.
    */
    int Receive(std::string& answer, unsigned int numberOfBytes, const char *eol=nullptr);

    /**
    * \brief Send the string input
    *
    * \param[in] input  The string to send to the serial interface. The string
    *                   termination character \\0 is not sent.
    * \param[in] block  If false, the this method will return immediately. If
    *                   true, this method will block until all bytes have been
    *                   physically transmitted over the serial interface.
    */
    int Send(const std::string& input, bool block = false);

    /**
    * \brief Send the break signal for ms milliseconds
    */
    void SendBreak(unsigned int ms = 400);

    /**
    * \brief erase the receive buffer of the serial interface
    */
    void ClearReceiveBuffer();

    /**
    * \brief erase the send buffer of the serial interface
    */
    void ClearSendBuffer();

    /**
    * \brief Get the port number of the serial interface
    *
    * Returns the port number that will be used in the connection.
    * The port number is only used if the m_DeviceName is empty ("").
    */
    itkGetConstMacro(PortNumber, PortNumber);

    /**
    * \brief Set the port number of the serial interface
    *
    * SerialCommunication can either use PortNumber to create serial interface device names
    * COM1 to COM9 for windows and /dev/ttyS0 to /dev/ttyS8 on linux
    * (SetPortNumber(COM1) is mapped to /dev/ttyS0 and so on). Alternatively, use SetDeviceName()
    * to set the device name directly (e.g. "CNCA0" for a com0com virtual com port or
    * "/dev/ttyUSB0" for a USB to serial adapter on linux. If a device name is set (m_DeviceName != "")
    * then OpenConnection() will try to open that device. Otherwise, it will build the device
    * name using the port number
    */
    itkSetMacro(PortNumber, PortNumber);

    /**
    * \brief Get the device name
    *
    * SerialCommunication can either use m_PortNumber to create serial interface device names
    * or use m_DeviceName directly. This method allows to set an arbitrary device name
    * that will be used to connect to the device. Common names are COM1, CNCA0, CNCB9
    * on windows and /dev/ttyS0 or /dev/ttyUSB0 on linux.
    */
    itkGetStringMacro(DeviceName);

    /**
    * \brief Set the device name
    *
    * if the device name is set (!=""), OpenConnection() will try to open the
    * serial device on that device name. Normally, the serial interfaces are named COM1-COM9
    * on windows and /dev/ttyS0 to /dev/ttyS9 on linux, but other names are possible too
    * (e.g. /dev/ttyUSB0).
    */
    itkSetStringMacro(DeviceName);

    /**
    * \brief Get the baud rate of the serial interface
    */
    itkGetConstMacro(BaudRate, BaudRate);

    /**
    * \brief Set the baud rate of the serial interface
    */
    itkSetMacro(BaudRate, BaudRate);

    /**
    * \brief Get the number of data bits of the serial interface
    */
    itkGetConstMacro(DataBits, DataBits);

    /**
    * \brief Set the number of data bits of the serial interface
    */
    itkSetMacro(DataBits, DataBits);

    /**
    * \brief Get the parity mode of the serial interface
    */
    itkGetConstMacro(Parity, Parity);

    /**
    * \brief Set the parity mode of the serial interface
    */
    itkSetMacro(Parity, Parity);

    /**
    * \brief Get number of stop bits of the serial interface
    */
    itkGetConstMacro(StopBits, StopBits);

    /**
    * \brief Set number of stop bits of the serial interface
    */
    itkSetMacro(StopBits, StopBits);

    /**
    * \brief returns true if hardware handshake should is used
    */
    itkGetConstMacro(HardwareHandshake, HardwareHandshake);

    /**
    * \brief Set if hardware handshake should be used
    */
    itkSetMacro(HardwareHandshake, HardwareHandshake);

    /**
    * \brief returns the send timeout in milliseconds
    */
    itkGetConstMacro(SendTimeout, unsigned int);

    /**
    * \brief set the send timeout in milliseconds
    *
    * Only applies to WIN32, not POSIX
    */
    itkSetMacro(SendTimeout, unsigned int);

    /**
    * \brief returns the receive timeout in milliseconds
    */
    itkGetConstMacro(ReceiveTimeout, unsigned int);

    /**
    * \brief set the send timeout in milliseconds
    *
    * Specify the receive timeout in milliseconds.
    * Setting this value to 0 will cause the Receive()
    * method to wait until all expected characters are received.
    */
    itkSetMacro(ReceiveTimeout, unsigned int);

  protected:
    SerialCommunication();
    ~SerialCommunication() override;

    /**
    * \brief configures the serial interface with all parameters
    *
    * This automatically reroutes the call to the hardware specific function
    */
    int ApplyConfiguration();


    #ifdef WIN32
    /**
    * \brief Applies the configuration for Windows
    */
    int ApplyConfigurationWin();

    #else
    /**
    * \brief Applies the configuration for Linux
    */
    int ApplyConfigurationUnix();


    #endif


    std::string m_DeviceName; ///< device name that is used to connect to the serial interface (will be used if != "")
    PortNumber m_PortNumber;  ///< port number of the device
    BaudRate m_BaudRate;      ///< baud rate of the serial interface connection
    DataBits m_DataBits;      ///< number of data bits per symbol
    Parity m_Parity;          ///< parity mode
    StopBits m_StopBits;      ///< number of stop bits per symbol
    HardwareHandshake m_HardwareHandshake;  ///< whether to use hardware handshake for the connection
    unsigned int m_ReceiveTimeout; ///< timeout for receiving data from the serial interface in milliseconds
    unsigned int m_SendTimeout;    ///< timeout for sending data to the serial interface in milliseconds

    bool m_Connected;       ///< is set to true if a connection currently established

#ifdef WIN32
    HANDLE m_ComPortHandle;
    DWORD m_PreviousMask;
    COMMTIMEOUTS m_PreviousTimeout;
    DCB m_PreviousDeviceControlBlock;
#else
    int m_FileDescriptor;
#endif
  };
} // namespace mitk
#endif
