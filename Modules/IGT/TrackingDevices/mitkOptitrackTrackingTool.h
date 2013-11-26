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
/**
* \brief API library header for Optitrack Systems
*/
#include <NPTrackingTools.h>

#include <mitkOptitrackErrorMessages.h>


namespace mitk
{

  //class OptitrackTragkingDevice;

  /** Documentation:
  *   \brief An object of this class represents the a Tool tracked by Optitrack System. You can define
  *  the tool by the a definition file like in the example in ****. Remember that it will be necessary to
  *  to have a license for using the Optitrack System.
  *  See  http://www.naturalpoint.com/ for details.
    *   \author E. Marinetto (emarinetto@hggm.es) Instituto de Investigación Sanitaria Gregorio Marañón, Madrid, Spain. & M. Noll (matthias.noll@igd.fraunhofer.de) Cognitive Computing & Medical Imaging | Fraunhofer IGD
  *   \ingroup IGT
  */
  class MitkIGT_EXPORT OptitrackTrackingTool : public InternalTrackingTool
  {
    friend class OptitrackTrackingDevice;

  public:
    mitkClassMacro(mitk::OptitrackTrackingTool, mitk::InternalTrackingTool);
    itkNewMacro(Self);



    /**
    * \brief Define the tool by a calibration File.
    * \return Returns true if the connection is well done. Throws an exception if an error occures related to the Optitrack API messages.
    * @throw mitk::IGTException Throws an exception if there is an error during the connection with the system.
    */
    bool SetToolByFileName(std::string nameFile);

    int get_IDnext();
    void DeleteTrackable();
    void SetPosition(mitk::Point3D position);           ///< sets the position
    void SetOrientation(mitk::Quaternion orientation);  ///< sets the orientation as a quaternion
    void GetPosition(mitk::Point3D& position) const;
    void GetOrientation(mitk::Quaternion& orientation) const;
    bool Enable();                                      ///< enablea the tool, so that it will be tracked. Returns true if enabling was successfull
    bool Disable();                                     ///< disables the tool, so that it will not be tracked anymore. Returns true if disabling was successfull
    bool IsEnabled() const;                             ///< returns whether the tool is enabled or disabled
    bool IsDataValid() const;                           ///< returns true if the current position data is valid (no error during tracking, tracking error below threshold, ...)
    float GetTrackingError() const;                     ///< return one value that corresponds to the overall tracking error. The dimension of this value is specific to each tracking device
    void SetTrackingError(float error);                 ///< sets the tracking error
    void SetErrorMessage(const char* _arg);
    void SetDataValid(bool _arg);                       ///< sets if the tracking data (position & Orientation) is valid
    void updateTool();

    //void readPivotPosicion(float seg = 0.5);

    OptitrackTrackingTool();
    ~OptitrackTrackingTool();


    std::string fileConfiguration;
    int ID;
    float* calibrationPoints;
    float* pivotPoint;
    int numPoints;

  private:
    // copy constructors deliberately not implemented
    OptitrackTrackingTool(const OptitrackTrackingTool&);
    const OptitrackTrackingTool& operator=(const OptitrackTrackingTool&);

  };
}
#endif /* OptiTrackTrackingTool_H_HEADER_INCLUDED_ */
