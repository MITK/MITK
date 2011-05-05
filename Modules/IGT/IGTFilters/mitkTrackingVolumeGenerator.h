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
#include <mitkSurface.h>
#include <vtkPolyData.h>
#include "mitkTrackingTypes.h"
#include "mitkTrackingDevice.h"

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
    *  After the method GetOutPut(), delivers the generatet TrackingVolume as mitk:Surface
    *
    *  For tracking devices that have a modifiable tracking volume (e.g. VirtualTrackingDevice,
    *  this class produces a tracking volume with default values (e.g. that are set in the class'
    *  constructor
    *
    *  \ingroup IGT
    */

    class MitkIGT_EXPORT TrackingVolumeGenerator : public mitk::SurfaceSource
  {
  public:

      mitkClassMacro(TrackingVolumeGenerator, mitk::SurfaceSource)
      itkNewMacro(Self);


      /**
      * \brief       Sets the trackingvolume explicitly. Only use this method if your specific
      *              trackingvolume is not supported by this class. The type is set to
      *              TrackingSystemNotSpecified if doing this.
      * \param volume  The new tracking volume as vtkPolyData.
      */
      void SetVolumeManually(vtkPolyData* volume);

      /**
      * \brief        Sets the tracking device of the volume. After doing this
      *               the tracking volume gets generatet and set to the correct dimensions in the correct
      *               coordinate system.
      * \param type   The type of the tracking device (something like NDIAurora or NDIPolaris).
      * \return       Returns true if the type was set successfull, false if not.
      */
      bool SetTrackingDeviceType(mitk::TrackingDeviceType type);

      /**
      * \brief        Sets the tracking device of the volume. After doing this
      *               the tracking volume gets generatet and is set to the correct dimensions in the correct
      *               coordinate system.
      * \param tracker  The tracking device the tracking volume have to be created for
      * \return        Returns true if the type was set successfull, false if not.
      */
      bool SetTrackingDevice(mitk::TrackingDevice::Pointer tracker);


      /**
      * \brief      Returns the generatet tracking volume
      * \return     Returns the tracking volume if generated yet, otherwise null
      */
      mitk::Surface* GetOutput();

  protected:
      TrackingVolumeGenerator();
     // mitk::TrackingDevice::Pointer m_TrackingDevice;
      mitk::Surface::Pointer m_Surface;
  };
}
#endif // MITKTRACKINGVOLUMEGENERATOR_H


