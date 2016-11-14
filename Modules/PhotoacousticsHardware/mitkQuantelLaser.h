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


#ifndef MITKQUANTELLASER_H_HEADER_INCLUDED
#define MITKQUANTELLASER_H_HEADER_INCLUDED

#include "itkObject.h"
#include "mitkCommon.h"

#include <chrono>
#include <thread>
#include <mutex>

#include <usModule.h>
#include <usModuleResource.h>
#include <usGetModuleContext.h>
#include <usModuleContext.h>
#include <usModuleResourceStream.h>

#include "mitkSerialCommunication.h"
#include "MitkPhotoacousticsHardwareExports.h"

namespace mitk {
    
  class MITKPHOTOACOUSTICSHARDWARE_EXPORT QuantelLaser : public itk::LightObject
    {
    public:
      mitkClassMacroItkParent(QuantelLaser, itk::LightObject);
      itkFactorylessNewMacro(Self);

      enum LaserState { UNCONNECTED, STATE0, STATE1, STATE2, STATE3, STATE4, STATE5, STATE6, STATE7 };   ///< Type for STATE variable. The LaserDevice is always in one of these states
      /**
       * \brief Opens a connection to the device
       *
       * This may only be called if there is currently no connection to the device.
       * If OpenConnection() is successful, the object will change from UNCONNECTED state to a STATE state
       */
      virtual bool OpenConnection(std::string configurationFile);

      /**
       * \brief Closes the connection to the device
       *
       * This may only be called if there is currently a connection to the device. (e.g. object is in a STATE state)
       */
      virtual bool CloseConnection(); ///< Closes the connection with the device

      virtual std::string SendAndReceiveLine(const std::string* input, std::string* answer);

      virtual void StayAlive();
      virtual bool StartFlashing();
      virtual bool StopFlashing();

      virtual bool StartQswitching();
      virtual bool StopQswitching();
      virtual bool IsEmitting();
      virtual bool IsFlashing();

      virtual LaserState GetState();

      typedef mitk::SerialCommunication::PortNumber PortNumber; ///< Port number of the serial connection
      typedef mitk::SerialCommunication::BaudRate BaudRate;     ///< Baud rate of the serial connection
      typedef mitk::SerialCommunication::DataBits DataBits;     ///< Number of data bits used in the serial connection
      typedef mitk::SerialCommunication::Parity Parity;         ///< Parity mode used in the serial connection
      typedef mitk::SerialCommunication::StopBits StopBits;     ///< Number of stop bits used in the serial connection
      typedef mitk::SerialCommunication::HardwareHandshake HardwareHandshake; ///< Hardware handshake mode of the serial connection

    private:
      LaserState m_State; ///< current Laser state
    protected:

      QuantelLaser();
      virtual ~QuantelLaser();

      bool m_KeepAlive = false;
      bool m_FlashlampRunning = false;
      bool m_ShutterOpen = false;
      bool m_LaserEmission = false;
      void LoadResorceFile(std::string filename, std::string* lines);
      
      std::string m_DeviceName;///< Device Name
      PortNumber m_PortNumber; ///< COM Port Number
      BaudRate m_BaudRate;     ///< COM Port Baud Rate
      DataBits m_DataBits;     ///< Number of Data Bits per token
      Parity m_Parity;         ///< Parity mode for communication
      StopBits m_StopBits;     ///< number of stop bits per token
      HardwareHandshake m_HardwareHandshake; ///< use hardware handshake for serial port connection

      std::string m_XmlPumpLaserConfiguration;
      mitk::SerialCommunication::Pointer m_SerialCommunication;    ///< serial communication interface
      std::thread m_StayAliveMessageThread;
      std::mutex m_SerialCommunicationMutex; ///< mutex for coordinated access of serial communication interface
    };
} // namespace mitk

#endif /* MITKQUANTELLASER_H_HEADER_INCLUDED */
