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
#include <itkMultiThreader.h>


namespace mitk
{
  /** Documentation:
  *   \brief An object of this class represents the MicronTracker device. You can add tools to this
  *          device, then open the connection and start tracking. The tracking device will then
  *          continuously update the tool coordinates.
  *   \ingroup IGT
  */
  class MitkIGT_EXPORT OpenIGTLinkTrackingDevice : public TrackingDevice
  {
  public:
    friend class OpenIGTLinkTrackingTrackingTool;
    mitkClassMacro(OpenIGTLinkTrackingDevice, TrackingDevice);
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

    /**
    * @returns Returns true if the MicronTracker is installed on this build (means activated in CMAKE). False if not.
    */
    virtual bool IsDeviceInstalled();

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
    * \brief Create a new OpenIGTLink tool with toolName and fileName and add it to the list of tools
    *
    * This method will create a new OpenIGTLinkTool object, load the tool definition file fileName,
    * set the tool name toolName and then add it to the list of tools.
    * It returns a pointer of type mitk::TrackingTool to the tool
    * that can be used to read tracking data from it.
    * This is the only way to add tools to OpenIGTLinkTrackingDevice.
    *
    * \warning adding tools is not possible in tracking mode, only in setup and ready.
    */
    mitk::TrackingTool* AddTool(const char* toolName, const char* fileName);

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

    /**
    * \brief This method tracks tools as long as the variable m_Mode is set to "Tracking".
    * Tracking tools means grabbing frames from the camera an updating the tools.
    * @throw mitk::IGTHardwareException Throws an exception if there is an error during tracking of tools.
    */
    void TrackTools();

    /**
    * \brief Automatically detects tools in field of measurement of the tracking device.
    * Tools can only be detected if their calibration file is availiable in the directory
    * for calibration files.
    * \return Returns all detected Tools.
    */
    std::vector<OpenIGTLinkTrackingTool::Pointer> DetectTools();

    /**
    * \return Returns all tools of the tracking device.
    */
    std::vector<OpenIGTLinkTrackingTool::Pointer> GetAllTools();


    static ITK_THREAD_RETURN_TYPE ThreadStartTracking(void* data);

    std::vector<OpenIGTLinkTrackingTool::Pointer> m_AllTools; ///< vector holding all tools
    itk::MultiThreader::Pointer m_MultiThreader;
    int m_ThreadID;

  };
}//mitk
#endif /* MITKOpenIGTLinkTRACKINGDEVICE_H_HEADER_INCLUDED_ */
