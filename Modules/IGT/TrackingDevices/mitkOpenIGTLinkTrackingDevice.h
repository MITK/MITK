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

#ifndef MITKOPENIGTLINKTRACKINGDEVICE_H_HEADER_INCLUDED_
#define MITKOPENIGTLINKTRACKINGDEVICE_H_HEADER_INCLUDED_

#include <mitkIGTConfig.h>
#include <mitkTrackingDevice.h>
#include <mitkOpenIGTLinkTrackingTool.h>
#include <mitkIGTLClient.h>
#include <mitkIGTLDeviceSource.h>
#include <mitkIGTLMessageToNavigationDataFilter.h>
#include <itkMultiThreader.h>
#include <igtlQuaternionTrackingDataMessage.h>
#include <igtlTrackingDataMessage.h>
#include <igtlTransformMessage.h>

namespace mitk
{
  /** Documentation:
  *   \brief An object of this class represents the MicronTracker device. You can add tools to this
  *          device, then open the connection and start tracking. The tracking device will then
  *          continuously update the tool coordinates.
  *   \ingroup IGT
  */
  class MITKIGT_EXPORT OpenIGTLinkTrackingDevice : public TrackingDevice
  {
  public:
    mitkClassMacro(OpenIGTLinkTrackingDevice, TrackingDevice);
    itkFactorylessNewMacro(Self)
      itkCloneMacro(Self)

      /** Sets the port number for the Open IGT Link connection. Default value is -1 (invalid). */
      void SetPortNumber(int portNumber);

    /** Sets the hostname for the Open IGT Link connection. Default value is 127.0.0.1 (localhost). */
    void SetHostname(std::string hostname);

    int GetPortNumber();

    std::string GetHostname();

    /**
    * \brief Starts the tracking.
    * \return Returns true if the tracking is started. Throws an exception if an error occures.
    * @throw mitk::IGTHardwareException Throws an exception if there is an error during start tracking.
    */
    virtual bool StartTracking();

    /**
    * \brief Stops the tracking.
    * \return Returns true if the tracking is stopped.
    */
    virtual bool StopTracking();

    /**
    * \brief Opens the connection to the device. This have to be done before the tracking is started.
    * @throw mitk::IGTHardwareException Throws an exception if there is an error during open connection.
    */
    virtual bool OpenConnection();

    /**
    * \brief Closes the connection and clears all resources.
    */
    virtual bool CloseConnection();

    /**
    * \return Returns the number of tools which have been added to the device.
    */
    virtual unsigned int GetToolCount() const;

    /**
    * \param toolNumber The number of the tool which should be given back.
    * \return Returns the tool which the number "toolNumber". Returns NULL, if there is
    * no tool with this number.
    */
    TrackingTool* GetTool(unsigned int toolNumber)  const;

    /**
    * \brief Discover the tools available from the connected OpenIGTLink device and adds these tools to this tracking device. Therefore, a connection
    *        is opened, the tools are discovered and added.
    * \param WaitingTime Defines how long the method waits for an answer from the server (in milliseconds). Default value is 10000 (10 seconds).
    * \return Returns true if the connection was established and the tools were discovered successfully and - if at least one tool was found - were added to this device.
    *         Retruns false if no valid connection is available.
    */
    bool DiscoverTools(int WaitingTime = 10000);

    /**
    * \brief Create a new OpenIGTLink tool with toolName and fileName and add it to the list of tools
    *
    * Note that tools are usually provided by the OpenIGTLink connection. In most cases, the method DiscoverTools() should be used
    * instead which automatically finds the provided tools. If you use this method to manually add tools be sure that you add the
    * same number and type of tools that are provided by the connected device. Otherwise problems might occur when you try to start
    * tracking.
    */
    mitk::TrackingTool* AddTool(const char* toolName, const char* fileName);

    bool IsDeviceInstalled();

  protected:
    OpenIGTLinkTrackingDevice();
    ~OpenIGTLinkTrackingDevice();

    /**
    * \brief Adds a tool to the tracking device.
    *
    * \param tool  The tool which will be added.
    * \return Returns true if the tool has been added, false otherwise.
    */
    bool InternalAddTool(OpenIGTLinkTrackingTool::Pointer tool);

    /** Updates the tools from the open IGT link connection. Is called every time a message received event is invoked.*/
    void UpdateTools();
    unsigned long m_MessageReceivedObserverTag;

    /**
    * \return Returns all tools of the tracking device.
    */
    std::vector<OpenIGTLinkTrackingTool::Pointer> GetAllTools();

    //OpenIGTLink connection class
    mitk::IGTLClient::Pointer m_OpenIGTLinkClient;

    //OpenIGTLink pipeline
    mitk::IGTLDeviceSource::Pointer m_IGTLDeviceSource;
    mitk::IGTLMessageToNavigationDataFilter::Pointer m_IGTLMsgToNavDataFilter;

    std::vector<OpenIGTLinkTrackingTool::Pointer> m_AllTools; ///< vector holding all tools

  private:
    enum TrackingMessageType
    {
      TDATA, TRANSFORM, QTDATA, UNKNOWN
    };

    mitk::OpenIGTLinkTrackingDevice::TrackingMessageType GetMessageTypeFromString(const char* messageTypeString);

    bool DiscoverToolsFromTData(igtl::TrackingDataMessage::Pointer msg);

    bool DiscoverToolsFromQTData(igtl::QuaternionTrackingDataMessage::Pointer msg);

    bool DiscoverToolsFromTransform();

    void AddNewToolForName(std::string name, int i);
  };
}//mitk
#endif /* MITKOpenIGTLinkTRACKINGDEVICE_H_HEADER_INCLUDED_ */
