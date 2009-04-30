/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile: mitkClaronTrackingDevice.h,v $
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
  class MITK_IGT_EXPORT ClaronTrackingDevice : public TrackingDevice
  {
  public:

    mitkClassMacro(ClaronTrackingDevice, TrackingDevice);
    itkNewMacro(Self);

    /**
    * \brief Starts the tracking.
    * \return Returns true if the tracking is started. Returns false if there was an error.
    */
    virtual bool StartTracking();

    /**
    * \brief Stops the tracking.
    * \return Returns true if the tracking is stopped. Returns false if there was an error.
    */
    virtual bool StopTracking();

    /**
    * \brief Opens the connection to the device. This have to be done before the tracking is startet.
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
    TrackingTool* GetTool(unsigned int toolNumber);

    
    /**
    * \brief Create a new Claron tool with toolName and fileName and add it to the list of tools
    *
    * This method will create a new ClaronTool object, load the tool definition file fileName,
    * set the tool name toolName and then add it to the list of tools. 
    * It returns a pointer of type mitk::TrackingTool to the tool
    * that can be used to read tracking data from it.
    * This is the only way to add tools to ClaronTrackingDevice.
    *
    * \WARNING adding tools is not possible in tracking mode, only in setup and ready.
    */
    mitk::TrackingTool* AddTool(const char* toolName, const char* fileName);
      

    /**
    * \return Returns whether the MicronTracker is installed (means whether the C-Make-Variable "MITK_USE_MICRON_TRACKER" is set),
    *         so returns false in this case.
    */
    bool IsMicronTrackerInstalled();
  
  protected:
    ClaronTrackingDevice();
    ~ClaronTrackingDevice();

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
