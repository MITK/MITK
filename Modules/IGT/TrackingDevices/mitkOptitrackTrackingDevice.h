/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkOptitrackTrackingDevice_h
#define mitkOptitrackTrackingDevice_h

#include <MitkIGTExports.h>
#include <mitkTrackingDevice.h>
#include <mitkTrackingTypes.h>
#include <mitkIGTTimeStamp.h>
#include <itksys/SystemTools.hxx>
#include <string>
#include <cstdlib>
#include <cstdio>
#include <ctime>
#include <thread>
#include <mutex>

/**
* \brief IGT Exceptions
*/
#include "mitkIGTIOException.h"
#include "mitkIGTTimeStamp.h"
#include "mitkIGTException.h"

/**
* \brief OptitrackTrackingTools
*/
#include "mitkOptitrackTrackingTool.h"


namespace mitk
{
  /** Documentation:
  *   \brief An object of this class represents the Optitrack device. You can add tools to this
  *          device, then open the connection and start tracking. The tracking device will then
  *          continuously update the tool coordinates. Remember that it will be necessary to
  *       to have a license for using the Optitrack System.
  *       See  https://www.naturalpoint.com/ for details.
  *   \author E. Marinetto (emarinetto@hggm.es) Instituto de Investigación Sanitaria Gregorio Marañón, Madrid, Spain. & M. Noll (matthias.noll@igd.fraunhofer.de) Cognitive Computing & Medical Imaging | Fraunhofer IGD
  *   \ingroup IGT
  */
  class MITKIGT_EXPORT OptitrackTrackingDevice : public mitk::TrackingDevice
  {

  friend class OptitrackTrackingTool;

  public:
    mitkClassMacro(OptitrackTrackingDevice, mitk::TrackingDevice);
    itkNewMacro(Self);

    /**
    * @returns Returns true if the Optitrack tracker is installed on this build (means activated in CMAKE). False if not.
    */
    bool IsDeviceInstalled() override;

    // Define the Type of Tracker as DefinitionOfTool (MITK)
    typedef mitk::TrackingDeviceType OptiTrackTrackingDeviceType;

    /**
    * \brief Open the Connection with the Tracker. Calls LoadCalibration function and set the system up with the calibration file.
    * Remember that you have to set a calibration file first to open a correct connection to the Optical Tracking System.
    * \return Returns true if the connection is well done. Throws an exception if an error occures related to the Optitrack API messages.
    * @throw mitk::IGTException Throws an exception if InitializeCameras or LoadCalibration failed.
    */
    bool OpenConnection() override;

    /**
    * \brief Close the Connection with the Tracker. Also CleanUp the Optitrack variables using the API: TT_CleanUp and TT_ShutDown.
    * Sometimes API does not work properly and some problems during the Clean Up has been reported.
    * \return Returns true if the cleaning up and shutdown worked correctly. Throws an exception if an error occures related to the Optitrack API messages.
    * @throw mitk::IGTException Throws an exception if the System cannot ShutDown now or was not initialized.
    */
    bool CloseConnection() override;

    /**
    * \brief Start to Track the tools already defined. If no tools are defined for this tracker, it returns an error.
    * Tools can be added using either AddToolByDescriptionFile or AddToolsByConfigurationFiles
    * \return Returns true at least one tool was defined and the tracking is correct
    * @throw mitk::IGTException Throws an exception if the System is not in State Ready .
    */
    bool StartTracking() override;

    /**
    * \brief Stop the Tracking Thread and tools will not longer be updated.
    * \return Returns true if Tracking thread could be stopped.
    * @throw mitk::IGTException Throws an exception if System is not in State Tracking.
    */
    bool StopTracking() override;

    /**
    * \brief Return the tool pointer of the tool number toolNumber
    * \param toolNumber The number of the tool which should be given back.
    * \return Returns the tool which the number "toolNumber". Returns nullptr, if there is
    * no tool with this number.
    */
    TrackingTool* GetTool(unsigned int toolNumber) const override;

    /**
    * \brief Return the tool pointer of the tool number toolNumber
    * \param toolNumber The number of the tool which should be given back.
    * \return Returns the tool which the number "toolNumber". Returns nullptr, if there is
    * no tool with this number.
    * @throw mitk::IGTException Throws an exception if there is the required tool does not exist.
    */
    OptitrackTrackingTool* GetOptitrackTool(unsigned int toolNumber) const;

    /**
    * \brief Returns the number of defined tools
    * \return Returns the number of defined tools in the Optitrack device.
    */
    unsigned int GetToolCount() const override;

/** @brief Sets the directory where the calibration file of the MicronTracker can be found. */
    itkSetMacro(Exp,int);

  /** @brief Gets the current calibration directory. */
    itkGetMacro(Exp,int);

/** @brief Sets the directory where the calibration file of the MicronTracker can be found. */
    itkSetMacro(Led,int);

  /** @brief Gets the current calibration directory. */
    itkGetMacro(Led,int);

/** @brief Sets the directory where the calibration file of the MicronTracker can be found. */
    itkSetMacro(Thr,int);

  /** @brief Gets the current calibration directory. */
    itkGetMacro(Thr,int);

    /** @brief Sets the file where the calibration of the OptitrackTracker can be found. */
   void SetCalibrationPath(std::string calibrationPath);

    /** @brief Gets the current calibration file. */
    itkGetMacro(calibrationPath,std::string);

    /**
    * \brief Start the Tracking Thread for the tools
    */
    void ThreadStartTracking();

    /**
    * \brief Update each tool location in the list m_AllTools
    * @throw mitk::IGTException Throws an exception if the getting data operation failed for a defined tool
    */
    void TrackTools();

    /**
    * \brief Load the Calibration file to the Optitrack System and set the cameras in calibrated locations
    * \return Returns true if the calibration was uploaded correctly
    * @throw mitk::IGTException Throws an exception if Calibration Path is empty, the System cannot load a calibration file or System is not ready for load a calibration file because it has not been initialized yet
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
    * @throw mitk::IGTException Throws an exception if System is not Initialized
    */
    bool SetCameraParams(int exposure, int threshold, int intensity, int videoType = 4);

    /**
    * \brief Initialize the Optitrack System
    * \return Returns true if system was initialized correctly
    * @throw mitk::IGTException Throws an exception if the Optitrack Tracking System cannot be initialized
    */
    bool InitializeCameras();

    /**
    * \brief Add a new tool using a text file which described the tool.
    * The file must to have the next structure
    *    ToolName
    *    \#NumberOfMarkers
    *    X Y Z - for the first marker
    *    X Y Z - for the second marker
    *    ...
    *    X Y Z - for the last marker, the number \#NumberOfMarkers
    *    X Y Z - for the PIVOT point
    * \return Returns true if system was initialized correctly
    * @throw mitk::IGTException Throws an exception if Tool could not be added or System is not Initialized
    */
    bool AddToolByDefinitionFile(std::string fileName); // ^????? We should give an example of defined tool

    /**
    * \brief This function load a file with Tools definitions provided for the software
    * \return Returns true if file is correctly loaded with all the tools
    * @throw mitk::IGTException Throws an exception if there is an error during the Initialization
    */
//    bool AddToolByConfigurationFil(std::string fileName); // For next release....

  protected:
    /**
    * \brief Constructor & Destructor of the class
    */
    OptitrackTrackingDevice();
    ~OptitrackTrackingDevice() override;


  private:
    /**
    * \brief The calibration file path. This file is produced by TrackingTools/Motive software.
    * This variable is used to determine what will be the calibration file absolute path.
    */
    std::string m_calibrationPath;

    /**
    * \brief The Cameras Exposition
    */
    int m_Exp;

    /**
    * \brief The Cameras LED power
    */
    int m_Led;

    /**
    * \brief The Cameras Thr
    */
    int m_Thr;

    /**
    * \brief Described if the system was initialized at least once during execution. This is due
    * to some reported problems during the clean up , shutdown and initialization again.
    */
    bool m_initialized;

    /**
    * \brief Vector of pointers pointing to all defined tools
    */
    std::vector<mitk::OptitrackTrackingTool::Pointer> m_AllTools;

    /**
    * \brief Mutex for coordinated access of tool container
    */
    mutable std::mutex m_ToolsMutex;

    std::thread m_Thread;


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
