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


#ifndef MITKNDIPROTOCOL_H_HEADER_INCLUDED_
#define MITKNDIPROTOCOL_H_HEADER_INCLUDED_

#include <itkObject.h>
#include <itkObjectFactory.h>

#include <MitkIGTExports.h>
#include "mitkSerialCommunication.h"
#include "mitkNDIPassiveTool.h"

namespace mitk 
{
  class NDITrackingDevice;

  /**Documentation
  * \brief The NDI Protocol class provides building and parsing of 
  *  command strings and answers to and from a NDI tracking device.
  *
  * \ingroup IGT
  */
  class MitkIGT_EXPORT NDIProtocol : public itk::Object
  {
  public:
    mitkClassMacro(NDIProtocol, itk::Object);
    itkNewMacro(Self);

    itkSetObjectMacro(TrackingDevice, NDITrackingDevice);

    typedef mitk::SerialCommunication::PortNumber PortNumber; ///< Port number of the serial connection
    typedef mitk::SerialCommunication::BaudRate BaudRate;     ///< Baud rate of the serial connection
    typedef mitk::SerialCommunication::DataBits DataBits;     ///< Number of data bits used in the serial connection
    typedef mitk::SerialCommunication::Parity Parity;         ///< Parity mode used in the serial connection
    typedef mitk::SerialCommunication::StopBits StopBits;     ///< Number of stop bits used in the serial connection
    typedef mitk::SerialCommunication::HardwareHandshake HardwareHandshake; ///< Hardware handshake mode of the serial connection
    typedef mitk::NDIPassiveTool::TrackingPriority TrackingPriority; ///< Tracking priority used for tracking a tool
    /** 
    * \brief Gives information about the tool which is assosiated with the port handle. Writes portInfo to the string.
    */
    NDIErrorCode PHINF(std::string portHandle, std::string* portInfo);
    NDIErrorCode PSOUT(std::string portHandle, std::string state); ///< Set GPIO Output (Aurora)
    NDIErrorCode COMM(mitk::SerialCommunication::BaudRate baudRate, mitk::SerialCommunication::DataBits dataBits, mitk::SerialCommunication::Parity parity, mitk::SerialCommunication::StopBits stopBits, mitk::SerialCommunication::HardwareHandshake hardwareHandshake); ///< Change Serial Communication Parameters
    NDIErrorCode INIT();    ///< Initialize the Measurement System
    NDIErrorCode DSTART();  ///< Start the Diagnostic Mode
    NDIErrorCode DSTOP();   ///< Stop the Diagnostic Mode
    NDIErrorCode IRINIT();  ///< Initialize the System to Check for Infrared
    NDIErrorCode IRCHK(bool* IRdetected);         ///< This version of IRCHK uses only the simple "presence of infrared light" call, that returns a binary "IR detected/no IR detected" answer
    NDIErrorCode PHSR(PHSRQueryType queryType, std::string* portHandles);    ///< Port Handle Search. Will write returned port handles to the string portHandles
    NDIErrorCode PHF(std::string* portHandle);    ///< Port Handle Free. Frees the port handle.
    NDIErrorCode PHRQ(std::string* portHandle);   ///< Port Handle Request. Will request a Port Handle for a wireless tool and return it in the string portHandle
    NDIErrorCode PVWR(std::string* portHandle, const unsigned char* sromData, unsigned int sromDataLength);    ///< Port Virtual Write. Writes an SROM Image data to a tool 
    NDIErrorCode PINIT(std::string* portHandle);  ///< Port Initialize. Will initialize a Port that has been acquired with PHRQ and has been assigned a SROM File with PVWR
    NDIErrorCode PENA(std::string* portHandle, TrackingPriority prio); ///< Port Enable. Will enable a port that has been initialized with PINIT.
    NDIErrorCode PDIS(std::string* portHandle);   ///< Port Disable. Will disable a port that has been enabled with PENA
    NDIErrorCode IRATE(IlluminationActivationRate rate); ///< Setting the illuminator rate. Will set the refresh rate for the illuminator for wireless tools
    NDIErrorCode BEEP(unsigned char count);       ///< Sounding the measurement system beeper. The tracking system will beep one to nine times
    NDIErrorCode TSTART(bool resetFrameCounter = false);  ///< Start Tracking Mode. The tracking system must be in setup mode and must be initialized.
    NDIErrorCode TSTOP();                         ///< Stop Tracking Mode. The tracking system must be in Tracking mode.
    NDIErrorCode TX(bool trackIndividualMarkers = false, MarkerPointContainerType* markerPositions = NULL); ///< Report transformations in text mode. Optionally, individual markers can be tracked
    NDIErrorCode BX();                            ///< Report transformations in binary mode.
    NDIErrorCode POS3D(MarkerPointContainerType* markerPositions); ///< Report 3D Positions of single markers. can only be used in diagnostics mode
    NDIErrorCode VER(mitk::TrackingDeviceType& t);                 ///< returns if the tracking device is a Polaris or an Aurora system
    NDIErrorCode TX1000(MarkerPointContainerType* markerPositions);    ///< Report transformations in text mode.
    unsigned int ByteToNbBitsOn(char& c) const; ///<
    itkGetConstMacro(UseCRC, bool); ///< Get whether to append a CRC16 checksum to each message
    itkSetMacro(UseCRC, bool);      ///< Set whether to append a CRC16 checksum to each message
    itkBooleanMacro(UseCRC);        ///< Set whether to append a CRC16 checksum to each message
  protected:
    NDIProtocol();
    virtual ~NDIProtocol();

    /**Documentation
    * Reads the reply from the tracking device and checks if it is either "OKAY" or "ERROR##".
    * if it reads an error, it returns the equivalent NDIErrorCode
    * Replies other than OKAY or ERROR result in an NDIUNEXPECTEDREPLY
    */
    NDIErrorCode ParseOkayError();

    /**Documentation
    * Sends the command command to the tracking system and checks for OKAY and ERROR as replies
    * This is used by commands like INIT, DSTART, DSTOP,... that do not need parameters 
    * or special parsing of replies
    */
    NDIErrorCode GenericCommand(const std::string command, const std::string* parameter = NULL);

    /**Documentation
    * \brief returns the error code for an Error String returned from the NDI tracking device
    */
    NDIErrorCode GetErrorCode(const std::string* input);

    NDITrackingDevice* m_TrackingDevice;  ///< tracking device to which the commands will be send
    bool m_UseCRC;  ///< whether to append a CRC16 checksum to each message
  };
} // namespace mitk
#endif /* MITKNDIPROTOCOL_H_HEADER_INCLUDED_ */
