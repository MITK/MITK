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
    * \brief        Deprecated! Use set DeviceData instead.
    *               Sets the tracking device type of the volume. Warning: there are different possible volumes for some device types.
    *               In this case a default volume is chosen automatically. All tracking volumes are defined by TrackingDeviceData
    *               objects (see file mitkTrackingTypes.h) for a list.
    *
    *               After setting the device type the tracking volume gets generated (by a default volume for this type as mentioned above)
    *               and set to the correct dimensions in the correct coordinate system. The TV of a VirtualTrackingDevice is always a 400*400 cube.
    * \param type   The type of the tracking device (currently supported:NDIAurora, NDIPolaris, ClaronMicron, IntuitiveDaVinci and the VirtualTracker; see file mitkTrackingTypes.h for a always up to date list).
    */
    void SetTrackingDeviceType(mitk::TrackingDeviceType deviceType);

    /**
    * \return       Returns the tracking device type of the current device. Warning: there are different possible volumes for some device types.
    *               Use GetTrackingDeviceData to get a unambiguous assignment to a tracking volume.
    */
    mitk::TrackingDeviceType GetTrackingDeviceType() const;


    /**
    * \brief        Sets the tracking device data object which will be used to generate the volume. Each tracking device data object
    *               has an unambiguous assignment to a tracking volume. See file mitkTrackingTypes.h for a list of all availiable object.
    */
    void SetTrackingDeviceData(mitk::TrackingDeviceData deviceData);

    /**
    * \return       Returns the current tracking device data of the generator. See file mitkTrackingTypes.h for the definition of tracking device data objects.
    */
    mitk::TrackingDeviceData GetTrackingDeviceData() const;


    /**
    * \brief        Deprecated! Use set DeviceData instead. Sets the tracking device type of the volume. After doing this
    *               the tracking volume gets generated and is set to the correct dimensions in the correct
    *               coordinate system. The TV of a VirtualTrackingDevice is always a 400*400 cube.
    * \param tracker  The tracking device the tracking volume has to be created for (currently supported: NDIAurora, NDIPolaris, ClaronMicron, IntuitiveDaVinci and the VirtualTracker; see file mitkTrackingTypes.h for a always up to date list).
    */
    void SetTrackingDevice(mitk::TrackingDevice::Pointer tracker);


  protected:
      TrackingVolumeGenerator();

      /** \brief Holds the current tracking device data object, which is used to generate the volume. */
      mitk::TrackingDeviceData m_Data;

      void GenerateData();
  };
}
#endif // MITKTRACKINGVOLUMEGENERATOR_H


