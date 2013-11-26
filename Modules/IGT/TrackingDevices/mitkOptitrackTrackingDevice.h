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

#ifndef OptitrackTrackingDevice_H_HEADER_INCLUDED
#define OptitrackTrackingDevice_H_HEADER_INCLUDED

#include <MitkIGTExports.h>
#include <mitkTrackingDevice.h>
#include <itkMultiThreader.h>
#include <mitkTrackingTypes.h>
#include <mitkIGTTimeStamp.h>
#include <itkFastMutexLock.h>
#include <itksys/SystemTools.hxx>
#include <itkMutexLockHolder.h>
#include <string>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

/**
* \brief IGT Exceptions
*/
#include "mitkIGTIOException.h"
#include "mitkIGTTimeStamp.h"
#include "mitkIGTException.h"



/**
* \brief API library header for Optitrack Systems
*/
#include <NPTrackingTools.h>

/**
* \brief OptitrackTrackingTools
*/
#include "mitkOptitrackTrackingTool.h"

/**
* \brief MutexHolder to keep rest of Mutex
*/
typedef itk::MutexLockHolder<itk::FastMutexLock> MutexLockHolder;


#include <mitkOptitrackErrorMessages.h>

/**
* \brief Maximum number of attempts for Initialization, Shutdown and CleanUp
*/
#ifndef OPTITRACK_ATTEMPTS
#define OPTITRACK_ATTEMPTS 10
#endif

// Time to refresh the tools location (ms)
#ifndef OPTITRACK_FRAME_RATE
#define OPTITRACK_FRAME_RATE 25
#endif


namespace mitk
{
  /** Documentation:
  *   \brief An object of this class represents the Optitrack device. You can add tools to this
  *          device, then open the connection and start tracking. The tracking device will then
  *          continuously update the tool coordinates. Remember that it will be necessary to
  *       to have a license for using the Optitrack System.
  *       See  http://www.naturalpoint.com/ for details.
  *   \ingroup IGT
  */
  class MitkIGT_EXPORT OptitrackTrackingDevice : public mitk::TrackingDevice
  {

  friend class OptitrackTrackingTool;

  public:
    mitkClassMacro(OptitrackTrackingDevice, mitk::TrackingDevice);
        itkNewMacro(Self);


    // Define the Type of Tracker as DefinitionOfTool (MITK)
    typedef mitk::TrackingDeviceType OptiTrackTrackingDeviceType;

    /**
    * \brief Open the Connection with the Tracker. Calls LoadCalibration function and set the system up with the calibration file.
    * Remember that you have to set a calibration file first to open a correct connection to the Optical Tracking System.
    * \return Returns true if the connection is well done. Throws an exception if an error occures related to the Optitrack API messages.
    * @throw mitk::IGTHardwareException Throws an exception if there is an error during the connection with the system.
    */
    virtual bool OpenConnection();

    /**
    * \brief Close the Connection with the Tracker. Also CleanUp the Optitrack variables using the API: TT_CleanUp and TT_ShutDown.
    * Sometimes API does not work properly and some problems during the Clean Up has been reported.
    * \return Returns true if the cleaning up and shutdown worked correctly. Throws an exception if an error occures related to the Optitrack API messages.
    * @throw mitk::IGTHardwareException Throws an exception if there is an error during the cleaning up and shutdown with the system.
    */
    virtual bool CloseConnection();

    /**
    * \brief Start to Track the tools already defined. If no tools are defined for this tracker, it returns an error.
    * Tools can be added using either AddToolByDescriptionFile or AddToolsByConfigurationFiles
    * \return Returns true at least one tool was defined and the tracking is correct
    * @throw mitk::IGTHardwareException Throws an exception if there is an error during the checking of the tools.
    */
    virtual bool StartTracking();

    /**
    * \brief Stop the Tracking Thread and tools will not longer be updated.
    * \return Returns true if Tracking thread could be stopped.
    * @throw mitk::IGTHardwareException Throws an exception if there is an error during thread stopping.
    */
    virtual bool StopTracking();

    /**
    * \brief Return the tool pointer of the tool number toolNumber
    * \param toolNumber The number of the tool which should be given back.
    * \return Returns the tool which the number "toolNumber". Returns NULL, if there is
    * no tool with this number.
    * @throw mitk::IGTHardwareException Throws an exception if there is an error. /////////////////////////^????????????????
    */
    TrackingTool* GetTool(unsigned int toolNumber) const;

    /**
    * \brief Return the tool pointer of the tool number toolNumber
    * \param toolNumber The number of the tool which should be given back.
    * \return Returns the tool which the number "toolNumber". Returns NULL, if there is
    * no tool with this number.
    * @throw mitk::IGTHardwareException Throws an exception if there is an error. /////////////////////////^????????????????
    */
    OptitrackTrackingTool* OptitrackTrackingDevice::GetOptitrackTool(unsigned int toolNumber) const;

    /**
    * \brief Returns the number of defined tools
    * \return Returns the number of defined tools in the Optitrack device.
    * @throw mitk::IGTHardwareException Throws an exception if there is an error. /////////////////////////^????????????????
    */
    unsigned int GetToolCount() const;

    /** @brief Sets the file where the calibration of the OptitrackTracker can be found. */
    itkSetMacro(calibrationPath,std::string);

    /** @brief Gets the current calibration file. */
    itkGetMacro(calibrationPath,std::string);

    /**
    * \brief Start the Tracking Thread for the tools
    * @throw mitk::IGTHardwareException Throws an exception if there is an error. /////////////////////////^????????????????
    */
    static ITK_THREAD_RETURN_TYPE ThreadStartTracking(void* data);

    /**
    * \brief Update each tool location in the list m_AllTools
    * @throw mitk::IGTHardwareException Throws an exception if there is an error. /////////////////////////^????????????????
    */
    void TrackTools();

    /**
    * \brief Load the Calibration file to the Optitrack System and set the cameras in calibrated locations
    * \return Returns true if the calibration was uploaded correctly
    * @throw mitk::IGTHardwareException Throws an exception if there is an error during the uploading of the calibration, also a message related to the API eror messages.
    */
    bool LoadCalibration();

    /**
    * \brief Set the Cameras Exposure, Threshold and Intensity of IR LEDs. By Default it set the Video type to 4: Precision Mode for tracking
    * //== VideoType:
        * //==     0 = Segment Mode
        * //==     1 = Grayscale Mode
        * //==     2 = Object Mode
        * //==     4 = Precision Mode
        * //==     6 = MJPEG Mode     (V100R2 only)
    * \return Returns true if all cameras were set up correctly
    * @throw mitk::IGTHardwareException Throws an exception if there is an error during the setting up of the cameras, also a message related to the API eror messages. // ????? Better just a message informing of a problem with the set up?
    */
    bool SetCameraParams(int exposure, int threshold, int intensity, int videoType = 4);

    /**
    * \brief Initialize the Optitrack System
    * \return Returns true if system was initialized correctly
    * @throw mitk::IGTHardwareException Throws an exception if there is an error during the Initialization
    */
    bool InitializeCameras();

    /**
    * \brief Add a new tool using a text file which described the tool.
    * The file must to have the next structure
    *    ToolName
    *    #NumberOfMarkers
    *    X Y Z - for the first marker
    *    X Y Z - for the second marker
    *    ...
    *    X Y Z - for the last marker, the number #NumberOfMarkers
    *    X Y Z - for the PIVOT point
    * \return Returns true if system was initialized correctly
    * @throw mitk::IGTHardwareException Throws an exception if there is an error during the Initialization
    */
    bool AddToolByDefinitionFile(std::string fileName); // ^????? We should give an example of defined tool

    /**
    * \brief This function load a file with Tools definitions provided for the software
    * \return Returns true if file is correctly loaded with all the tools
    * @throw mitk::IGTHardwareException Throws an exception if there is an error during the Initialization
    */
//    bool AddToolByConfigurationFil(std::string fileName); // For next release....

  protected:
    /**
    * \brief Constructor & Destructor of the class
    */
    OptitrackTrackingDevice();
    ~OptitrackTrackingDevice();

    /**
    * \brief Return the string associated with the error number
    * \return Returns true if system was initialized correctly
    * @throw mitk::IGTHardwareException Throws an exception if there is an error during the Initialization
    */
    std::string GetErrorMessage(int errorNumber);


  private:
    /**
    * \brief The calibration file path. This file is produced by TrackingTools/Motive software.
    * This variable is used to determine what will be the calibration file absolute path.
    */
    std::string m_calibrationPath;

    /**
    * \brief Described if the system was initialized at least once during execution. This is due
    * to some reported problems during the clean up , shutdown and initialization again.
    */
    bool m_initialized;


  std::vector<mitk::OptitrackTrackingTool::Pointer> m_AllTools; ///< container type for tracking tools
    itk::FastMutexLock::Pointer m_ToolsMutex; ///< mutex for coordinated access of tool container
    itk::MultiThreader::Pointer m_MultiThreader;    ///< MultiThreader that starts continuous tracking update
    int m_ThreadID;


/* TODO:

// For Tracking
-bool AddToolByConfigurationFil(std::string fileName);
TTAPI   NPRESULT TT_LoadTrackables (const char *filename); //== Load Trackables ======----
TTAPI   NPRESULT TT_SaveTrackables (const char *filename); //== Save Trackables ======----
TTAPI   NPRESULT TT_AddTrackables  (const char *filename); //== Add  Trackables ======----
TTAPI   void     TT_ClearTrackableList();             //== Clear all trackables   =====---
TTAPI   NPRESULT TT_RemoveTrackable(int Index);       //== Remove single trackable ====---
TTAPI   void     TT_SetTrackableEnabled(int index, bool enabled);    //== Set Tracking   ====---
TTAPI   bool     TT_TrackableEnabled(int index);                     //== Get Tracking   ====---
TTAPI   int      TT_TrackableMarkerCount(int index);             //== Get marker count   ====---
TTAPI   void     TT_TrackableMarker(int RigidIndex,              //== Get Trackable mrkr ====---
                        int MarkerIndex, float *x, float *y, float *z);

//For projects
TTAPI   NPRESULT TT_LoadProject(const char *filename); //== Load Project File ==========--
TTAPI   NPRESULT TT_SaveProject(const char *filename); //== Save Project File ==========--

// For VRPN connection
TTAPI   NPRESULT TT_StreamVRPN(bool enabled, int port);//== Start/stop VRPN Stream ===----

// For frame testing
TTAPI   int      TT_FrameMarkerCount();               //== Returns Frame Markers Count ---
TTAPI   float    TT_FrameMarkerX(int index);          //== Returns X Coord of Marker -----
TTAPI   float    TT_FrameMarkerY(int index);          //== Returns Y Coord of Marker -----
TTAPI   float    TT_FrameMarkerZ(int index);          //== Returns Z Coord of Marker -----
TTAPI   int      TT_FrameMarkerLabel(int index);      //== Returns Label of Marker -------
TTAPI   double   TT_FrameTimeStamp();                 //== Time Stamp of Frame (seconds) -

// For cameras handling
TTAPI   int      TT_CameraCount();                    //== Returns Camera Count =====-----
TTAPI   float    TT_CameraXLocation(int index);       //== Returns Camera's X Coord =-----
TTAPI   float    TT_CameraYLocation(int index);       //== Returns Camera's Y Coord =-----
TTAPI   float    TT_CameraZLocation(int index);       //== Returns Camera's Z Coord =-----
TTAPI   float    TT_CameraOrientationMatrix(int camera, int index); //== Orientation -----


*/


};


}
#endif
