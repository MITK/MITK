/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef MITKCLARONTRACKINGDEVICE_H_HEADER_INCLUDED_
#define MITKCLARONTRACKINGDEVICE_H_HEADER_INCLUDED_


#include <vector>
#include <mitkIGTConfig.h>
#include <mitkTrackingDevice.h>
#include <mitkClaronTool.h>
#include <itkMultiThreader.h>

//only include MicronTracker if cmake Variable is on else the ClaronInterfaceStub is included
#ifdef MITK_USE_MICRON_TRACKER
#include <mitkClaronInterface.h>
#else
#include <mitkClaronInterfaceStub.h>
#endif

namespace mitk
{
  /** Documentation:
  *   \brief An object of this class represents the MicronTracker device. You can add tools to this
  *          device, then open the connection and start tracking. The tracking device will then
  *          continuously update the tool coordinates.
  *   \ingroup IGT
  */
  class MITKIGT_EXPORT ClaronTrackingDevice : public TrackingDevice
  {
  public:

    mitkClassMacro(ClaronTrackingDevice, TrackingDevice);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

    /**
    * @returns Returns true if the MicronTracker is installed on this build (means activated in CMAKE). False if not.
    */
    bool IsDeviceInstalled() override;

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
    * \brief Create a new Claron tool with toolName and fileName and add it to the list of tools
    *
    * This method will create a new ClaronTool object, load the tool definition file fileName,
    * set the tool name toolName and then add it to the list of tools.
    * It returns a pointer of type mitk::TrackingTool to the tool
    * that can be used to read tracking data from it.
    * This is the only way to add tools to ClaronTrackingDevice.
    *
    * \warning adding tools is not possible in tracking mode, only in setup and ready.
    */
    mitk::TrackingTool* AddTool(const char* toolName, const char* fileName);


    /**
    * \return Returns whether the MicronTracker is installed (means whether the C-Make-Variable "MITK_USE_MICRON_TRACKER" is set),
    *         so returns false in this case.
    * \deprecatedSince{2014_03} This method is deprecated, please use the static method IsDeviceInstalled() instead.
    */
    DEPRECATED(bool IsMicronTrackerInstalled());

    /** @brief Sets the directory where the calibration file of the MicronTracker can be found. */
    itkSetMacro(CalibrationDir,std::string);

  /** @brief Gets the current calibration directory. */
    itkGetMacro(CalibrationDir,std::string);

  protected:
    ClaronTrackingDevice();
    ~ClaronTrackingDevice() override;

    /**
    * \brief Adds a tool to the tracking device.
    *
    * \param tool  The tool which will be added.
    * \return Returns true if the tool has been added, false otherwise.
    */
    bool InternalAddTool(ClaronTool::Pointer tool);

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
    std::vector<ClaronTool::Pointer> DetectTools();

    /**
    * \return Returns all tools of the tracking device.
    */
    std::vector<ClaronTool::Pointer> GetAllTools();

    /**
    * \return Gives back the device which is represented by an object of the class ClaronInterface.
    */
    ClaronInterface* GetDevice();

    static ITK_THREAD_RETURN_TYPE ThreadStartTracking(void* data);

    std::vector<ClaronTool::Pointer> m_AllTools; ///< vector holding all tools
    ClaronInterface::Pointer m_Device; ///< represents the interface to the tracking hardware
    itk::MultiThreader::Pointer m_MultiThreader;
    int m_ThreadID;

    /** \brief The directory where the camera calibration files can be found */
    std::string m_CalibrationDir;
    /** \brief The directory where the tool calibration files can be found */
    std::string m_ToolfilesDir;
  };
}//mitk
#endif /* MITKCLARONTRACKINGDEVICE_H_HEADER_INCLUDED_ */
