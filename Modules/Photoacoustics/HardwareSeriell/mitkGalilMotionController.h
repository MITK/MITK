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


#ifndef mitkGalilMotionController_H_HEADER_INCLUDED
#define mitkGalilMotionController_H_HEADER_INCLUDED

#include "itkObject.h"
#include "mitkCommon.h"

#include <chrono>
#include <thread>
#include <mutex>

#include "mitkSerialCommunication.h"
#include "MitkPhotoacousticsExports.h"

namespace mitk {
    
    class MITKPHOTOACOUSTICS_EXPORT GalilMotionController : public itk::LightObject
    {
    public:
      mitkClassMacroItkParent(GalilMotionController, itk::LightObject);
      itkFactorylessNewMacro(Self);

      /**
       * \brief Opens a connection to the device
       *
       * This may only be called if there is currently no connection to the device.
       * If OpenConnection() is successful, the object will change from UNCONNECTED state to a STATE state
       */
      virtual bool OpenConnection();

      /**
       * \brief Closes the connection to the device
       *
       * This may only be called if there is currently a connection to the device. (e.g. object is in a STATE state)
       */
	   
      virtual bool CloseConnection(); ///< Closes the connection with the device

      virtual std::string Send(const std::string* input);

      virtual std::string ReceiveLine(std::string* answer);

      virtual void ClearSendBuffer();
      virtual void ClearReceiveBuffer();
	  
      virtual bool Test();
      virtual bool Home();
      virtual bool Tune();

      typedef mitk::SerialCommunication::PortNumber PortNumber; ///< Port number of the serial connection
      typedef mitk::SerialCommunication::BaudRate BaudRate;     ///< Baud rate of the serial connection
      typedef mitk::SerialCommunication::DataBits DataBits;     ///< Number of data bits used in the serial connection
      typedef mitk::SerialCommunication::Parity Parity;         ///< Parity mode used in the serial connection
      typedef mitk::SerialCommunication::StopBits StopBits;     ///< Number of stop bits used in the serial connection
      typedef mitk::SerialCommunication::HardwareHandshake HardwareHandshake; ///< Hardware handshake mode of the serial connection

    private:
      int m_CurrentPosition; ///< current Laser state
    protected:

      GalilMotionController();
      virtual ~GalilMotionController();
      
      std::string m_DeviceName;///< Device Name
      PortNumber m_PortNumber; ///< COM Port Number
      BaudRate m_BaudRate;     ///< COM Port Baud Rate
      DataBits m_DataBits;     ///< Number of Data Bits per token
      Parity m_Parity;         ///< Parity mode for communication
      StopBits m_StopBits;     ///< number of stop bits per token
      HardwareHandshake m_HardwareHandshake; ///< use hardware handshake for serial port connection
	  
      mitk::SerialCommunication::Pointer m_SerialCommunication;    ///< serial communication interface
      std::mutex m_SerialCommunicationMutex; ///< mutex for coordinated access of serial communication interface
    };
} // namespace mitk

#endif /* mitkGalilMotionController_H_HEADER_INCLUDED */
