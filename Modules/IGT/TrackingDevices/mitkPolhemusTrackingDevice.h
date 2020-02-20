/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef MITKPolhemusTRACKINGDEVICE_H_HEADER_INCLUDED_
#define MITKPolhemusTRACKINGDEVICE_H_HEADER_INCLUDED_


#include <vector>
#include <mitkIGTConfig.h>
#include <mitkTrackingDevice.h>
#include <mitkPolhemusTool.h>
#include <itkMultiThreader.h>

namespace mitk
{
  /** Documentation:
  *   \brief An object of this class represents Polhemus tracking device. You can add tools to this
  *          device, then open the connection and start tracking. The tracking device will then
  *          continuously update the tool coordinates.
  *          The tools which are used by Polhemus need to be connected to the correct port.
  *          The port of the tool is stored as m_ToolPort in PolhemusTool AND as identifier in the NavigationTool (ToolStorage).
  *   \ingroup IGT
  */
  class MITKIGT_EXPORT PolhemusTrackingDevice : public TrackingDevice
  {
  public:

    mitkClassMacro(PolhemusTrackingDevice, TrackingDevice);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

    /**
    * \brief Starts the tracking.
    * \return Returns true if the tracking is started. Throws an exception if an error occures.
    * @throw mitk::IGTHardwareException Throws an exception if there is an error during start tracking.
    */
    virtual bool StartTracking() override;

    /**
    * \brief Stops the tracking.
    * \return Returns true if the tracking is stopped.
    */
    virtual bool StopTracking() override;

    /**
    * \brief Opens the connection to the device. This have to be done before the tracking is started.
    * @throw mitk::IGTHardwareException Throws an exception if there is an error during open connection.
    */
    virtual bool OpenConnection() override;

    /**
    * \brief Closes the connection and clears all resources.
    */
    virtual bool CloseConnection() override;

    /**
    * \return Returns the number of tools which have been added to the device.
    */
    virtual unsigned int GetToolCount() const override;

    /**
    * \param toolNumber The number of the tool which should be given back.
    * \return Returns the tool which the number "toolNumber". Returns NULL, if there is
    * no tool with this number.
    */
    TrackingTool* GetTool(unsigned int toolNumber)  const override;


    /**
    * \brief Create a new Polhemus tool with toolName and add it to the list of tools
    *
    * This method will create a new PolhemusTool object,
    * set the tool name toolName and then add it to the list of tools.
    * It returns a pointer of type mitk::TrackingTool to the tool
    * that can be used to read tracking data from it.
    * This is the only way to add tools to PolhemusTrackingDevice.
    *
    * \warning adding tools is not possible in tracking mode, only in setup and ready.
    */
    mitk::TrackingTool* AddTool(const char* toolName, int toolPort);



    bool IsDeviceInstalled();

    /** @return Returns true if this device can autodetects its tools. */
    virtual bool AutoDetectToolsAvailable();

    /** Autodetects tools from this device and returns them as a navigation tool storage.
    *  @return Returns the detected tools. Returns an empty storage if no tools are present
    *          or if detection is not possible
    */
    virtual mitk::NavigationToolStorage::Pointer AutoDetectTools();

    /** Enables/disables hemisphere tracking for all sensors. */
    void SetHemisphereTrackingEnabled(bool _HemisphereTrackingEnabled);

    /** Is Hemisphere Tracking Enabled for this tool? */
    bool GetHemisphereTrackingEnabled(int _tool);

    /** Toggles the current hemisphere. Parameter _tool describes, for which tool the hemisphere should change. Default -1 toggles all tools.*/
    void ToggleHemisphere(int _tool = -1);

    /** Sets the Hemisphere of tool _tool to the vector _hemisphere */
    void SetHemisphere(int _tool, mitk::Vector3D _hemisphere);

    /** Get the Hemisphere for _tool as mitk vector */
    mitk::Vector3D GetHemisphere(int _tool);

    /** Adjust the Hemisphere for this tool. User needs to make sure, that the tool is located in hemisphere (1|0|0) when calling this function.
    In contrast to SetHemisphere(1,0,0), this method restores the original HemisphereTracking settings at the end. */
    void AdjustHemisphere(int _tool);

  protected:
    PolhemusTrackingDevice();
    ~PolhemusTrackingDevice();

    /**
    * \brief Adds a tool to the tracking device.
    *
    * \param tool  The tool which will be added.
    * \return Returns true if the tool has been added, false otherwise.
    */
    bool InternalAddTool(PolhemusTool::Pointer tool);

    /**
    * \brief This method tracks tools as long as the variable m_Mode is set to "Tracking".
    * Tracking tools means grabbing frames from the camera an updating the tools.
    * @throw mitk::IGTHardwareException Throws an exception if there is an error during tracking of tools.
    */
    void TrackTools();

    /**
    * \return Returns all tools of the tracking device.
    */
    std::vector<PolhemusTool::Pointer> GetAllTools();

    /**
    * \return Gives back the device which is represented by an object of the class PolhemusInterface.
    */
    PolhemusInterface* GetDevice();

    static ITK_THREAD_RETURN_TYPE ThreadStartTracking(void* data);

    std::vector<PolhemusTool::Pointer> m_AllTools; ///< vector holding all tools
    PolhemusInterface::Pointer m_Device; ///< represents the interface to the tracking hardware
    itk::MultiThreader::Pointer m_MultiThreader;
    int m_ThreadID;
    bool m_HemisphereTrackingEnabled;
  };
}//mitk
#endif /* MITKPolhemusTRACKINGDEVICE_H_HEADER_INCLUDED_ */
