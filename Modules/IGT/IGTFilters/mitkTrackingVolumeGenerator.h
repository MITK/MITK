#ifndef MITKTRACKINGVOLUMEGENERATOR_H
#define MITKTRACKINGVOLUMEGENERATOR_H

#include <MitkIGTExports.h>

#include "mitkSurfaceSource.h"
#include "mitkSurface.h"
#include "vtkPolyData.h"
#include "mitkTrackingTypes.h"
#include "mitkTrackingDevice.h"

#endif // MITKTRACKINGVOLUMEGENERATOR_H

namespace mitk
{
    class MitkIGT_EXPORT TrackingVolumeGenerator : public mitk::SurfaceSource
  {
  public:

      mitkClassMacro(TrackingVolumeGenerator, mitk::SurfaceSource)
      itkNewMacro(Self);

      bool SetTrackingDevice(mitk::TrackingDevice::Pointer tracker);

      TrackingVolumeGenerator();
  protected:
      mitk::TrackingDevice::Pointer m_TrackingDevice;
  };
}


