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
    class MitkIGT_EXPORT TrackingVolumeGenerator : public mitk::SurfaceSource
  {
  public:

      mitkClassMacro(TrackingVolumeGenerator, mitk::SurfaceSource)
      itkNewMacro(Self);
      itkGetMacro(TrackingDevice, mitk::TrackingDevice::Pointer); //macro for generating getters/setters of members
//      itkSetMacro(TrackingDevice, mitk::TrackingDevice::Pointer);

      mitk::Surface::Pointer SetTrackingDevice(mitk::TrackingDevice::Pointer tracker);

      TrackingVolumeGenerator(int l);

  protected:
      TrackingVolumeGenerator();
      mitk::TrackingDevice::Pointer m_TrackingDevice;
  };
}
#endif // MITKTRACKINGVOLUMEGENERATOR_H


