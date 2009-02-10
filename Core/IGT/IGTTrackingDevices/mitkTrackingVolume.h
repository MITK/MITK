#ifndef MITKTRACKINGVOLUME_H_HEADER_INCLUDED_
#define MITKTRACKINGVOLUME_H_HEADER_INCLUDED_

#include "mitkSurface.h"
#include "itkObject.h"
#include "vtkPolyData.h"
#include "mitkTrackingTypes.h"

namespace mitk
{
  /**
  * \brief An instance of this class represents the tracking volume of
  * a tracking device. As it inherits from mitk::Surface it can be
  * displayed in the 3D-window of MITK. The coordination system is the same
  * as the coordination system of the tracking device.
  * To have the spesific dimensions
  * of the trackingvolume of a trackingdevice you need to call the method SetTrackingDeviceType().
  * If you get your trackingvolume from a trackingdevice-Object (ClaronTrackingDevice for example) this is done for you.
  */

  class TrackingVolume : public mitk::Surface
  {
  public:
    mitkClassMacro(TrackingVolume, mitk::Surface);
    itkNewMacro(Self);

    /**
    * \brief Sets the Trackingdevice of the volume. After doing this
    * the volume is set to the correct dimensions in the correct
    * coordination system.
    */
    bool SetTrackingDeviceType(TrackingDeviceType type);

    /**
    * \brief Sets the volume to manual dimensions. The type is set
    * to TrackingSystemNotSpecified if doing this.
    */
    void SetManualVolume(vtkPolyData* manualVolume);

    /**
    * @return Returns true if the point "punkt" is inside the
    * tracking volume, false if not.
    */
    bool IsInside(mitk::Point3D punkt);

  protected:
    TrackingVolume();
    TrackingDeviceType m_TrackingDeviceType;
  };
}
#endif // MITKTRACKINGVOLUME_H_HEADER_INCLUDED_
