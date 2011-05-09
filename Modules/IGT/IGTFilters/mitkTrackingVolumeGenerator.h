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

class vtkPolyData;

namespace mitk
{
    /**Documentation
    * \brief   An instance of this class represents a generator wich generates the tracking volume of a
    *    given tracking device as a mitk:Surface.
    *
    *  As it inherits from mitk::Surface it can be
    *  displayed in the 3D-window of MITK. The coordinate system is the same
    *  as the coordination system of the tracking device.
    *  To generate the specific dimensions of the tracking volume of a tracking device
    *  the methods SetTrackingDeviceType() or SetTrackingDevice ()needs to be called first.
    *  After the method GetOutput(), delivers the generatet TrackingVolume as mitk:Surface
    *
    *  For tracking devices that have a modifiable tracking volume (e.g. VirtualTrackingDevice,
    *  this class produces a tracking volume with default values
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
      *               the tracking volume gets generatet and set to the correct dimensions in the correct
      *               coordinate system. The TV of a VirtualTrackingDevice is always a 400*400 cube.
      * \param type   The type of the tracking device (currently supported:NDIAurora, NDIPolaris, ClaronMicron, IntuitiveDaVinci and the VirtualTracker).
      */
      itkSetMacro(TrackingDeviceType, mitk::TrackingDeviceType);
      itkGetMacro(TrackingDeviceType, mitk::TrackingDeviceType);


      /**
      * \brief        Sets the tracking device of the volume. After doing this
      *               the tracking volume gets generatet and is set to the correct dimensions in the correct
      *               coordinate system.
      * \param tracker  The tracking device the tracking volume has to be created for (currently supported:NDIAurora, NDIPolaris, ClaronMicron, IntuitiveDaVinci and the VirtualTracker).
      */
      void SetTrackingDevice(mitk::TrackingDevice::Pointer tracker);


  protected:
      TrackingVolumeGenerator();
      mitk::TrackingDeviceType m_TrackingDeviceType;

      void GenerateData();
  };
}
#endif // MITKTRACKINGVOLUMEGENERATOR_H


