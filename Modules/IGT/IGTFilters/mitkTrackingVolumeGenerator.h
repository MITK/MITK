/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2011-01-18 13:22:38 +0100 (Di, 18 Jan 2011) $
Version:   $Revision: 28959 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#ifndef MITKTRACKINGVOLUMEGENERATOR_H
#define MITKTRACKINGVOLUMEGENERATOR_H

#include "MitkIGTExports.h"

#include <mitkSurfaceSource.h>
#include "mitkTrackingTypes.h"
#include "mitkTrackingDevice.h"

namespace mitk
{
    /**Documentation
    * \brief   An instance of this class represents a generator wich generates the tracking volume of a
    *    given tracking device as a mitk:Surface.
    *
    *  To generate the specific dimensions of the tracking volume of a tracking device
    *  the methods SetTrackingDeviceType(trackingdevicetype) or SetTrackingDevice (tracker) have to be called first. Otherwise
    *  the TrackingDeviceType is set to "TrackingSystemNotSpecified".
    *  After setting the trackingdevice type, the update() method has to be called.
    *  Now the method GetOutput() delivers the generatet TrackingVolume as mitk:Surface
    *
    *  The coordinate system of die TrackingVolume is the same as the coordination system of the tracking device.
    *
    *  For tracking devices that have a modifiable tracking volume (e.g. VirtualTrackingDevice,
    *  this class produces a tracking volume with default values.
    *
    *  \ingroup IGT
    */

class MitkIGT_EXPORT TrackingVolumeGenerator : public mitk::SurfaceSource
  {
  public:

      mitkClassMacro(TrackingVolumeGenerator, mitk::SurfaceSource)
      itkNewMacro(Self);


      /**
      * \brief        Sets the tracking device type of the volume. After doing this
      *               the tracking volume gets generated and set to the correct dimensions in the correct
      *               coordinate system. The TV of a VirtualTrackingDevice is always a 400*400 cube.
      * \param type   The type of the tracking device (currently supported:NDIAurora, NDIPolaris, ClaronMicron, IntuitiveDaVinci and the VirtualTracker).
      */
      void SetTrackingDeviceType(mitk::TrackingDeviceType deviceType);
	  mitk::TrackingDeviceType GetTrackingDeviceType() const;

	  void SetTrackingDeviceData(mitk::TrackingDeviceData deviceData);
	  mitk::TrackingDeviceData GetTrackingDeviceData() const;

	 


      /**
      * \brief        Deprecated! Use set DeviceData instead. Sets the tracking device type of the volume. After doing this
      *               the tracking volume gets generatet and is set to the correct dimensions in the correct
      *               coordinate system. The TV of a VirtualTrackingDevice is always a 400*400 cube.
      * \param tracker  The tracking device the tracking volume has to be created for (currently supported:NDIAurora, NDIPolaris, ClaronMicron, IntuitiveDaVinci and the VirtualTracker).
      */
      void SetTrackingDevice(mitk::TrackingDevice::Pointer tracker);


  protected:
      TrackingVolumeGenerator();
	  mitk::TrackingDeviceData m_Data;

      void GenerateData();
  };
}
#endif // MITKTRACKINGVOLUMEGENERATOR_H


