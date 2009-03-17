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


#ifndef MITKNDITRACKINGDEVICE_H_HEADER_INCLUDED_C1C2FCD2
#define MITKNDITRACKINGDEVICE_H_HEADER_INCLUDED_C1C2FCD2

#include "mitkTrackingDevice.h"

#include <mitkCommon.h>
#include <itkMultiThreader.h>

#include <list>

#include "mitkTrackingTypes.h"
#include "mitkNDIProtocol.h"
#include "mitkNDIPassiveTool.h"
#include "mitkSerialCommunication.h"

namespace mitk 
{
  class NDIProtocol;

  /** Documentation
  * \brief superclass for specific NDI tracking Devices that use serial communication. 
  *
  * implements the TrackingDevice interface for NDI tracking devices (POLARIS, AURORA)
  * 
  * \ingroup IGT
  */
  class NDITrackingDevice : public TrackingDevice
  {
    friend class NDIProtocol;

  public:
    typedef std::vector<NDIPassiveTool::Pointer> Tool6DContainerType;  ///< List of 6D tools of the correct type for this tracking device
    typedef mitk::TrackingDeviceType NDITrackingDeviceType;  ///< This enumeration includes the two types of NDI tracking devices (Polaris, Aurora).

    typedef mitk::SerialCommunication::PortNumber PortNumber; ///< Port number of the serial connection
    typedef mitk::SerialCommunication::BaudRate BaudRate;     ///< Baud rate of the serial connection
    typedef mitk::SerialCommunication::DataBits DataBits;     ///< Number of data bits used in the serial connection
    typedef mitk::SerialCommunication::Parity Parity;         ///< Parity mode used in the serial connection
    typedef mitk::SerialCommunication::StopBits StopBits;     ///< Number of stop bits used in the serial connection
    typedef mitk::SerialCommunication::HardwareHandshake HardwareHandshake; ///< Hardware handshake mode of the serial connection


    mitkClassMacro(NDITrackingDevice, TrackingDevice);
    itkNewMacro(Self);

    /**
    * \brief Set the type of the NDI Tracking Device because it can not jet handle this itself
    */
    itkSetMacro(Type,TrackingDeviceType);

    /**
    * \brief initialize the connection to the tracking device
    * 
    * OpenConnection() establishes the connection to the tracking device by:
    * - initializing the serial port with the given parameters (port number, baud rate, ...)
    * - connection to the tracking device
    * - initializing the device
    * - initializing all manually added passive tools (user supplied srom file)
    * - initializing active tools that are connected to the tracking device
    */
    virtual bool OpenConnection();

    /**
    * \brief Closes the connection
    * 
    * CloseConnection() resets the tracking device, invalidates all tools and then closes the serial port.
    */
    virtual bool CloseConnection();

    /**
    * \brief Start the tracking. 
    * 
    * A new thread is created, which continuously reads the position and orientation information of each tool and stores them inside the tools.
    * Depending on the current operation mode (see SetOperationMode()), either the 6D tools (ToolTracking6D), 5D tools (ToolTracking5D),
    * 3D marker positions (MarkerTracking3D) or both 6D tools and 3D markers (HybridTracking) are updated.
    * Call StopTracking() to stop the tracking thread.
    */
    virtual bool StartTracking();

    /**
    * \brief return the tool with index toolNumber 
    */
    virtual TrackingTool* GetTool(unsigned int toolNumber);
    
    /**
    * \brief return current number of tools
    */
    virtual unsigned int GetToolCount() const;

    /**
    * \brief Add a passive 6D tool to the list of tracked tools.
    *
    * \WARNING adding tools is not possible in tracking mode, only in setup and ready.
    */
    virtual bool Add6DTool(NDIPassiveTool* tool);

    /**
    * \brief Remove a passive 6D tool from the list of tracked tools.
    *
    * \WARNING adding tools is not possible in tracking mode, only in setup and ready modes.
    */
    virtual bool Remove6DTool(NDIPassiveTool* tool);

    /**
    * \brief reloads the srom file and reinitializes the tool
    */
    virtual bool UpdateTool(mitk::NDIPassiveTool* tool);

    virtual void SetPortNumber(const PortNumber _arg); ///< set port number for serial communication
    virtual void SetDeviceName(const char* devName);   ///< set device name (e.g. COM1, /dev/ttyUSB0). If this is set, PortNumber will be ignored
    virtual void SetBaudRate(const BaudRate _arg);     ///< set baud rate for serial communication
    virtual void SetDataBits(const DataBits _arg);     ///< set number of data bits
    virtual void SetParity(const Parity _arg);         ///< set parity mode 
    virtual void SetStopBits(const StopBits _arg);     ///< set number of stop bits 
    virtual void SetHardwareHandshake(const HardwareHandshake _arg);  ///< set use hardware handshake for serial communication
    virtual void SetIlluminationActivationRate(const IlluminationActivationRate _arg); ///< set activation rate of IR illumator for polaris
    virtual void SetDataTransferMode(const DataTransferMode _arg);    ///< set data transfer mode to text (TX) or binary (BX). \warning: only TX is supportet at the moment

    virtual bool Beep(unsigned char count);   ///< Beep the tracking device 1 to 9 times

    NDIErrorCode GetErrorCode(const std::string* input);  ///< returns the error code for a string that contains an error code in hexadecimal format
    itkGetStringMacro(ErrorMessage);  ///< return current error message

    virtual bool SetOperationMode(OperationMode mode);  ///< set operation mode to 6D tool tracking, 3D marker tracking or 6D&3D hybrid tracking (see OperationMode)
    virtual OperationMode GetOperationMode();           ///< get current operation mode
    
    /**
    * \brief Get 3D marker positions (operation mode must be set to MarkerTracking3D or HybridTracking)
    */
    virtual bool GetMarkerPositions(MarkerPointContainerType* markerpositions);

  protected:
    /* Methods for NDIProtocol friend class */
    virtual void InvalidateAll();             ///< invalidate all tools
    NDIPassiveTool* GetTool(std::string* handle); ///< returns the tool object that has been assigned the handle or NULL if no tool can be found
    NDIErrorCode Send(const std::string* message, bool addCRC = true);      ///< Send message to tracking device
    NDIErrorCode Receive(std::string* answer, unsigned int numberOfBytes);  ///< receive numberOfBytes bytes from tracking device
    NDIErrorCode ReceiveByte(char* answer);   ///< lightweight receive function, that reads just one byte
    void ClearSendBuffer();                   ///< empty send buffer of serial communication interface
    void ClearReceiveBuffer();                ///< empty receive buffer of serial communication interface
    const std::string CalcCRC(const std::string* input);  ///< returns the CRC16 for input as a std::string

public://TODO
    NDITrackingDevice();          ///< Constructor
    virtual ~NDITrackingDevice(); ///< Destructor

    /**
    * \brief TrackTools() continuously polls serial interface for new 6d tool positions until StopTracking is called.
    *
    * Continuously tracks the 6D position of all tools until StopTracking() is called. 
    * This function is executed by the tracking thread (through StartTracking() and ThreadStartTracking()). 
    * It should not be called directly.
    */
    virtual void TrackTools();            

    /**
    * \brief continuously polls serial interface for new 3D marker positions until StopTracking is called.
    *
    * Continuously tracks the 3D position of all markers until StopTracking() is called. 
    * This function is executed by the tracking thread (through StartTracking() and ThreadStartTracking()). 
    * It should not be called directly.
    */
    virtual void TrackMarkerPositions();
    
    /**
    * \brief continuously polls serial interface for new 3D marker positions and 6D tool positions until StopTracking is called.
    *
    * Continuously tracks the 3D position of all markers and the 6D position of all tools until StopTracking() is called. 
    * This function is executed by the tracking thread (through StartTracking() and ThreadStartTracking()). 
    * It should not be called directly.
    */
    virtual void TrackToolsAndMarkers();
    
    /**
    * \brief static start method for the tracking thread.
    */
    static ITK_THREAD_RETURN_TYPE ThreadStartTracking(void* data);
    
    /**
    * \brief set current error message
    */
    itkSetStringMacro(ErrorMessage);

  protected:
    std::string m_DeviceName;///< Device Name
    PortNumber m_PortNumber; ///< COM Port Number
    BaudRate m_BaudRate;     ///< COM Port Baud Rate
    DataBits m_DataBits;     ///< Number of Data Bits per token
    Parity m_Parity;         ///< Parity mode for communication
    StopBits m_StopBits;     ///< number of stop bits per token
    HardwareHandshake m_HardwareHandshake; ///< use hardware handshake for serial port connection
    NDITrackingVolume m_NDITrackingVolume; ///< which tracking volume is currently used (if device supports multiple volumes) (\warning This parameter is not used yet)
    IlluminationActivationRate m_IlluminationActivationRate; ///< update rate of IR illuminator for Polaris
    DataTransferMode m_DataTransferMode;  ///< use TX (text) or BX (binary) (\warning currently, only TX mode is supported)
    bool m_AutoRecovery;                  ///< automatically restart tracking after error (\warning currently not used)
    Tool6DContainerType m_6DTools;        ///< list of 6D tools

    itk::FastMutexLock::Pointer m_ToolsMutex; ///< mutex for coordinated access of tool container
    mitk::SerialCommunication::Pointer m_SerialCommunication;    ///< serial communication interface
    itk::FastMutexLock::Pointer m_SerialCommunicationMutex; ///< mutex for coordinated access of serial communication interface
    NDIProtocol::Pointer m_DeviceProtocol;    ///< create and parse NDI protocol strings

    std::string m_ErrorMessage;     ///< contains error message in case of error (as indicated by return value of member functions)
    itk::MultiThreader::Pointer m_MultiThreader;      ///< creates tracking thread that continuously polls serial interface for new tracking data
    int m_ThreadID;                 ///< ID of tracking thread
    OperationMode m_OperationMode;  ///< tracking mode (6D tool tracking, 3D marker tracking,...)
    itk::FastMutexLock::Pointer m_MarkerPointsMutex;  ///< mutex for marker point data container
    MarkerPointContainerType m_MarkerPoints;          ///< container for markers (3D point tracking mode)  
  };
} // namespace mitk
#endif /* MITKNDITRACKINGDEVICE_H_HEADER_INCLUDED_C1C2FCD2 */
