#ifndef MITKTRACKINGVOLUME_H_HEADER_INCLUDED_
#define MITKTRACKINGVOLUME_H_HEADER_INCLUDED_

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

#include "mitkSurface.h"
#include "itkObject.h"
#include "vtkPolyData.h"
#include "mitkTrackingTypes.h"

namespace mitk
{
  /**
  * @brief 	An instance of this class represents the tracking volume of
  * 		a tracking device. As it inherits from mitk::Surface it can be
  * 		displayed in the 3D-window of MITK. The coordinate system is the same
  * 		as the coordination system of the tracking device.
  * 		To have the specific dimensions of the trackingvolume of a tracking device 
  *			the method SetTrackingDeviceType() needs to be called. If you get your 
  *			trackingvolume from an instance of the class TrackingDevice 
  *			(ClaronTrackingDevice for example) this is already done for you.
  *     This also means that every tracking device instance has a predefined tracking
  *     volume, which is automatically set when you construct the instance.
  */

  class TrackingVolume : public mitk::Surface
  {
  public:
    mitkClassMacro(TrackingVolume, mitk::Surface);
    itkNewMacro(Self);

    /**
    * @brief 		    Sets the tracking device of the volume. After doing this
    *               the volume is set to the correct dimensions in the correct
    *               coordinate system.
    * @param type   The type of the tracking device (something like NDIAurora or NDIPolaris).
    * @return		    Returns true if the type was set successfull, false if not.
    */
    bool SetTrackingDeviceType(TrackingDeviceType type);

    /**
    * @brief 	    Sets the trackingvolume explicitly. Only use this method if your specific 
    *	            trackingvolume is not supported by this class. The type is set to 
    *			        TrackingSystemNotSpecified if doing this.
    * @param manualVolume	The new tracking volume as vtkPolyData.
    */
    void SetManualVolume(vtkPolyData* manualVolume);

    /**
    * @brief 	Checks wether a point is inside or outside the tracking
    *        	volume.
    * @param p  The point which should be checked.
    * @return 	Returns true if the point "p" is inside the
    *           tracking volume, false if not.
    */
    bool IsInside(mitk::Point3D p);

  protected:
    TrackingVolume();
    /**
     * @brief   The current type of the corresponding tracking device is hold here.
     *          If the type is unknown the value is set to TrackingSystemNotSpecified.
     */
    TrackingDeviceType m_TrackingDeviceType;
  };
}
#endif // MITKTRACKINGVOLUME_H_HEADER_INCLUDED_