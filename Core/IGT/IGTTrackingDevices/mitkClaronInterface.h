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

#ifndef MITKCLARONINTERFACE_H_HEADER_INCLUDED_
#define MITKCLARONINTERFACE_H_HEADER_INCLUDED_
#define MTC(func) {int r = func; if (r!=mtOK) printf("MTC error: %s\n",MTLastErrorString()); };

#include <vector>
#include <string>

typedef int mtHandle;

namespace mitk
{
  typedef int claronToolHandle;

  /** Documentation:
  *   @brief An object of this class represents the interface to the MicronTracker. The methods of this class
  *          are calling the c-functions which are provided by the MTC-library. If the MicronTracker is not in
  *          use, which means the CMake-variable "MITK_USE_MICRON_TRACKER" is set to OFF, this class is replaced
  *          by a stub class called "ClaronInterfaceStub". 
  */
  class ClaronInterface
  {
  public:

    /**
    * @brief Constructor for a claroninterface.
    * @param calibrationDir   The directory where the device can find the camera calibration file.
    * @param toolFilesDir     The directory for the tool files.
    */
    ClaronInterface(std::string calibrationDir, std::string toolFilesDir);

    /**
    * @brief Opens the connection to the device and makes it ready to track tools. 
    * @return Returns true if there is a connection to the device and the device is ready to track tools, false if not.
    */
    bool StartTracking();

    /**
    * @brief Clears all resources. After this method have been called the system isn't ready to track any longer.
    * @return Returns true if the operation was succesful, false if not.
    */
    bool StopTracking();

    /**
    * @return Returns all tools which have been detected at the last frame grab.
    */
    std::vector<claronToolHandle> GetAllActiveTools();

    /**
    * @return Returns the position of the tooltip. If no tooltip is defined the Method returns the position of the tool.
    */
    std::vector<double> GetTipPosition(claronToolHandle c);

    /**
    * @return Returns the quarternions of the tooltip. If no tooltip is defined the Method returns the quarternions of the tool.
    */
    std::vector<double> GetTipQuaternions(claronToolHandle c);

    /**
    * @return Returns the position of the tool
    */
    std::vector<double> GetPosition(claronToolHandle c);

    /**
    * @return Returns the quaternion of the tool.
    */
    std::vector<double> GetQuaternions(claronToolHandle c);

    /**
    * @return Returns the name of the tool. This name is given by the calibration file.
    * @param c The handle of the tool, which name should be given back.
    */
    const char* GetName(claronToolHandle c);

    /**
    * @brief Grabs a frame from the camera.
    */
    void GrabFrame();

    /**
    * @return Returns wether the tracking device is tracking or not.
    */
    bool IsTracking();
    
	  /**
    * @return   Returns wether the MicronTracker is installed (means wether the C-Make-Variable "MITK_USE_MICRON_TRACKER" is set ON),
    *           so returns true in this case. This is because the class mitkClaronInterfaceStub, in which the same Method returns false 
    *		        is used otherways.
    */
    bool IsMicronTrackerInstalled();

  protected:
    
    /** @brief Variable is true if the device is tracking at the moment, false if not.*/
    bool isTracking;

    /** @brief Variable which holds the directory which should contain the file BumbleBee_6400420.calib. This directory is needed by the MTC library.*/
    char calibrationDir[512];
    /** @brief Variable which holds a directory with some tool files in it. All this tools are trackable when the path is given to the MTC library.*/
    char markerDir[512];

    //Some handles to communicate with the MTC library.
    mtHandle IdentifiedMarkers;
    mtHandle PoseXf;
    mtHandle CurrCamera;
    mtHandle IdentifyingCamera;
    //------------------------------------------------

  };
}//mitk
#endif
