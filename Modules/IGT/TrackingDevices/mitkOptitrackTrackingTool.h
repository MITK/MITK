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

#ifndef OptiTrackTrackingTool_H_HEADER_INCLUDED_
#define OptiTrackTrackingTool_H_HEADER_INCLUDED_

#include <MitkIGTExports.h>
#include <itkMultiThreader.h>
#include "itkFastMutexLock.h"
#include "mitkTrackingDevice.h"
#include "mitkTrackingTool.h"
#include "mitkIGTTimeStamp.h"
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <itksys/SystemTools.hxx>
#include <itkMutexLockHolder.h>
#include "mitkCommon.h"
#include <mitkInternalTrackingTool.h>
#include <mitkVector.h>
#include <itkFastMutexLock.h>
#include "mitkIGTException.h"



/**
* \brief Function to get the Error messages from API
*/
#include <mitkOptitrackErrorMessages.h>


namespace mitk
{

  //class OptitrackTragkingDevice;

  /** Documentation:
  *   \brief An object of this class represents the a Tool tracked by Optitrack System. You can define
  *  the tool by the a definition file like in the example in ****. Remember that it will be necessary to
  *  to have a license for using the Optitrack System.
  *  See  http://www.naturalpoint.com/ for details.
  *   \author E. Marinetto (emarinetto@hggm.es) Instituto de Investigaci�n Sanitaria Gregorio Mara��n, Madrid, Spain. & M. Noll (matthias.noll@igd.fraunhofer.de) Cognitive Computing & Medical Imaging | Fraunhofer IGD
  *   \ingroup IGT
  */
  class MITKIGT_EXPORT OptitrackTrackingTool : public InternalTrackingTool
  {
  public:
    friend class OptitrackTrackingDevice;
    mitkClassMacro(mitk::OptitrackTrackingTool, mitk::InternalTrackingTool);
    itkNewMacro(Self);



    /**
    * \brief Define the tool by a calibration File.
    * The file must to have the next structure. Makers locations must to have "%fe %fe %fe\n" format and in (mm). See http://www.cplusplus.com/reference/cstdio/fscanf/
    *    ToolName
    *    #NumberOfMarkers
    *    X Y Z - for the first marker
    *    X Y Z - for the second marker
    *    ...
    *    X Y Z - for the last marker, the number #NumberOfMarkers
    *    X Y Z - for the PIVOT point
    * \return Returns true if the tool was set correctly
    * @throw mitk::IGTException Throws an exception if there exist any problem during the configuration file reading.
    */
    bool SetToolByFileName(std::string nameFile);

    /**
    * \brief Ask API the next number of defined tool
    * \return Returns the next ID (int) for a new tool in the device list for API
    * @throw mitk::IGTException Throws an exception if get_IDnext failed
    */
    int get_IDnext();

    /**
    * \brief Delete the tool from the list of tools inside API Optitrack
    * \return Returns true if the deletion was correct
    * @throw mitk::IGTException Throws an exception if
    */
    bool DeleteTrackable();

    /**
    * \brief Set the position to a given one
    * @throw mitk::IGTException Throws an exception if
    */
    void SetPosition(mitk::Point3D position, ScalarType eps=0.0);

    /**
    * \brief Set the orientation to a given one using a quaternion nomenclature
    * @throw mitk::IGTException Throws an exception if
    */
    void SetOrientation(mitk::Quaternion orientation, ScalarType eps=0.0);

    /**
    * \brief Get the position of the tool
    * @throw mitk::IGTException Throws an exception if
    */
    void GetPosition(mitk::Point3D& position) const override;

    /**
    * \brief Get the orientation of the tool using quaternion nomenclature
    * @throw mitk::IGTException Throws an exception if
    */
    void GetOrientation(mitk::Quaternion& orientation) const override;

    /**
    * \brief Set the tool enabled for tracking.
  * \return Return true if the enabling was successfull
    * @throw mitk::IGTException Throws an exception if
    */
    bool Enable() override;

    /**
    * \brief Set the tool disabled for tracking.
  * \return Return true if the disabling was successfull
    * @throw mitk::IGTException Throws an exception if
    */
    bool Disable() override;

    /**
    * \brief Check if the tool is enabled (true) or not.
  * \return Return true if the tool is enabled for tracking
    * @throw mitk::IGTException Throws an exception if
    */
    bool IsEnabled() const override;

    /**
    * \brief Check if the data of the tool is valid.
  * \return Return true if location data is valid
    * @throw mitk::IGTException Throws an exception if
    */
    bool IsDataValid() const override;

    /**
    * \brief Get the expectated error in the tracked tool
  * \return Return the error location
    * @throw mitk::IGTException Throws an exception if
    */
    float GetTrackingError() const override;

    /**
    * \brief Set the FLE (Fiducial Localization Error) for the tool
    * @throw mitk::IGTException Throws an exception if
    */
    void SetTrackingError(float FLEerror) override;

    /**
    * \brief Set the valid flag for tracking data to true
    * @throw mitk::IGTException Throws an exception if
    */
    void SetDataValid(bool _arg) override;

    /**
    * \brief Update location and orientation of the tool
    * @throw mitk::IGTException Throws an exception if
    */
    void updateTool();

    /**
    * \brief Constructor of the class
    */
    OptitrackTrackingTool();

    /**
    * \brief Destructor of the class
    */
    ~OptitrackTrackingTool();

    /**
    * \brief File of the configuration for the tool
    */
    std::string m_fileConfiguration;

    /**
    * \brief ID number from Optitrack API
    */
    int m_ID;

    /**
    * \brief List of Markers locations in calibration position and orientation
    */
    float* m_calibrationPoints;

    /**
    * \brief location of the pivot point during calibration
    */
    float* m_pivotPoint;

    /**
    * \brief Number of Markers that blong to the tool
    */
    int m_numMarkers;

    /**
    * \brief Expected value of the fiducial localization error (rms)
    */
  float m_FLE;

  private:
    OptitrackTrackingTool(const OptitrackTrackingTool&);
    const OptitrackTrackingTool& operator=(const OptitrackTrackingTool&);

  };
}
#endif /* OptiTrackTrackingTool_H_HEADER_INCLUDED_ */
