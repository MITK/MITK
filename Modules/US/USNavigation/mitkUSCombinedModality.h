/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkUSCombinedModality_h
#define mitkUSCombinedModality_h

#include <MitkUSNavigationExports.h>
#include "mitkUSDevice.h"
#include "mitkImageSource.h"
#include "mitkAbstractUltrasoundTrackerDevice.h"
#include "mitkNavigationDataSource.h"

// Microservices
#include <mitkServiceInterface.h>
#include <usServiceRegistration.h>

namespace itk {
  template<class T> class SmartPointer;
}

namespace mitk {
  class USControlInterfaceBMode;
  class USControlInterfaceProbes;
  class USControlInterfaceDoppler;

  /**
   * \brief Combination of USDevice and NavigationDataSource.
   * This class can be used as an ImageSource subclass. Additionally tracking data be
   * retrieved from the NavigationDataSource returned by GetTrackingDevice().
   *
   * A calibration of the ultrasound image stream to the navigation datas can be set
   * for the currently active zoom level (of the ultrasound device) by SetCalibration().
   * The ultrasound images are transformed according to this calibration in the
   * GenerateData() method.
   */
  class MITKUSNAVIGATION_EXPORT USCombinedModality : public mitk::AbstractUltrasoundTrackerDevice
  {
  public:

    mitkClassMacro(USCombinedModality, mitk::AbstractUltrasoundTrackerDevice);
    mitkNewMacro3Param(USCombinedModality, USDevice::Pointer, itk::SmartPointer<NavigationDataSource>, bool);
    AffineTransform3D::Pointer GetUSPlaneTransform() override;

  protected:
    USCombinedModality( USDevice::Pointer usDevice,
                        itk::SmartPointer<NavigationDataSource> trackingDevice,
                        bool trackedUltrasoundActive = false );
    ~USCombinedModality() override;

    /**
    * \brief Grabs the next frame from the input.
    * This method is called internally, whenever Update() is invoked by an Output.
    */
    void GenerateData() override;

    /**
    * \brief Freezes or unfreezes the CombinedModality.
    */
    void OnFreeze(bool) override;

  };
} // namespace mitk

#endif
