/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

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
#include "mitkIGTLTrackingDataDeviceSource.h"

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
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

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
    bool StartTracking() override;

    /**
    * \brief Stops the tracking.
    * \return Returns true if the tracking is stopped.
    */
    bool StopTracking() override;

    /**
    * \brief Opens the connection to the device. This have to be done before the tracking is started.
    * @throw mitk::IGTHardwareException Throws an exception if there is an error during open connection.
    */
    bool OpenConnection() override;

    /**
    * \brief Closes the connection and clears all resources.
    */
    bool CloseConnection() override;

    /**
    * \return Returns the number of tools which have been added to the device.
    */
    unsigned int GetToolCount() const override;

    /**
    * \param toolNumber The number of the tool which should be given back.
    * \return Returns the tool which the number "toolNumber". Returns nullptr, if there is
    * no tool with this number.
    */
    TrackingTool* GetTool(unsigned int toolNumber)  const override;

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

    /** @return Returns true if this device can autodetects its tools. */
    bool AutoDetectToolsAvailable() override;

    /** Autodetects tools from the current OpenIGTLink connection and returns them as a navigation tool storage.
    *  @return Returns the detected tools. Returns an empty storage if no tools are present
    *          or if OpenIGTLink Connection is not possible
    */
    mitk::NavigationToolStorage::Pointer AutoDetectTools() override;

    bool IsDeviceInstalled() override;

    itkSetMacro(UpdateRate, int);               ///< Sets the update rate of the device in fps. Default value is 60 fps.
    itkGetConstMacro(UpdateRate, int);          ///< Returns the update rate of the device in fps

  protected:
    OpenIGTLinkTrackingDevice();
    ~OpenIGTLinkTrackingDevice() override;

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

    /** Receives one message from the OpenIGTLink connection. Starts the tracking stream if required.
     */
    mitk::IGTLMessage::Pointer ReceiveMessage(int waitingTime);

    /**
    * \return Returns all tools of the tracking device.
    */
    std::vector<OpenIGTLinkTrackingTool::Pointer> GetAllTools();

    //OpenIGTLink connection class
    mitk::IGTLClient::Pointer m_OpenIGTLinkClient;

    //OpenIGTLink pipeline
    mitk::IGTLTrackingDataDeviceSource::Pointer m_IGTLDeviceSource;
    mitk::IGTLMessageToNavigationDataFilter::Pointer m_IGTLMsgToNavDataFilter;

    std::vector<OpenIGTLinkTrackingTool::Pointer> m_AllTools; ///< vector holding all tools

    int m_UpdateRate; ///< holds the update rate in FPS (will be set automatically when the OpenIGTLink connection is established)

  private:
    enum TrackingMessageType
    {
      TDATA, TRANSFORM, QTDATA, UNKNOWN
    };

    mitk::OpenIGTLinkTrackingDevice::TrackingMessageType GetMessageTypeFromString(const char* messageTypeString);

    /** Discovers tools from the OpenIGTLink connection and converts them to MITK navigation tool objects.
        @return Returns a navigation tool storage holding all found tools. Returns an empty storage if no tools were found or if there was an error.*/
    mitk::NavigationToolStorage::Pointer DiscoverToolsAndConvertToNavigationTools(mitk::OpenIGTLinkTrackingDevice::TrackingMessageType type, int NumberOfMessagesToWait = 50);

    void AddNewToolForName(std::string name, int i);

    mitk::NavigationTool::Pointer ConstructDefaultOpenIGTLinkTool(std::string name, std::string identifier);
  };
}//mitk
#endif /* MITKOpenIGTLinkTRACKINGDEVICE_H_HEADER_INCLUDED_ */
