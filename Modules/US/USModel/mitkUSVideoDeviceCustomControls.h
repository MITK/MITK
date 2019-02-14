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

#ifndef MITKUSVideoDeviceCustomControls_H_HEADER_INCLUDED_
#define MITKUSVideoDeviceCustomControls_H_HEADER_INCLUDED_

#include "mitkUSAbstractControlInterface.h"
#include "mitkUSImageVideoSource.h"
#include "mitkUSVideoDevice.h"

#include <itkObjectFactory.h>

namespace mitk {
  /**
    * \brief Custom controls for mitk::USVideoDevice.
    * Controls image cropping of the corresponding mitk::USImageVideoSource.
    */
  class MITKUS_EXPORT USVideoDeviceCustomControls : public USAbstractControlInterface
  {
  public:
    mitkClassMacro(USVideoDeviceCustomControls, USAbstractControlInterface);
    mitkNewMacro1Param(Self, itk::SmartPointer<USDevice>);

    /**
      * Activate or deactivate the custom controls. This is just for handling
      * widget visibility in a GUI for example. Cropping will not be deactivated
      * if this method is called with false. Use
      * mitk::USVideoDeviceCustomControls::SetCropArea() with an empty are
      * instead.
      */
    void SetIsActive(bool isActive) override;

    /**
      * \return if this custom controls are currently activated
      */
    bool GetIsActive() override;

    /**
      * \brief Sets the area that will be cropped from the US image.
      * Set [0,0,0,0] to disable it, which is also default.
      */
    void SetCropArea(USImageVideoSource::USImageCropping newArea);

    /**
      * \return area currently set for image cropping defined by the actual current probe.
      */
    mitk::USProbe::USProbeCropping GetCropArea();

    /**
     * \brief Updates the cropping of the current probe given by the crop area of the
     * USImageVideoSource.
     */
    void UpdateProbeCropping( mitk::USImageVideoSource::USImageCropping cropping );

    /**
      * \brief Sets a new depth value to the current probe.
      */
    void SetNewDepth(double depth);

    /**
      * \ brief Sets new probe identifier
      */
    void SetNewProbeIdentifier(std::string probename);

    /**
    *\brief Get all the probes for the current device
    */
    std::vector<mitk::USProbe::Pointer> GetProbes();

    /**
    * \brief Get the scanning dephts of the given probe
    */
    std::vector<int> GetDepthsForProbe(std::string name);

    /**
    * \brief Sets the first existing probe or the default probe of a USVideoDevice
    * as the current probe of the USVideoDevice.
    */
    void SetDefaultProbeAsCurrentProbe();

  protected:
    /**
      * Class needs an mitk::USImageVideoSource object for beeing constructed.
      * This object will be manipulated by the custom controls methods.
      */
    USVideoDeviceCustomControls(itk::SmartPointer<USDevice> device);
    ~USVideoDeviceCustomControls() override;

    bool                          m_IsActive;
    USImageVideoSource::Pointer   m_ImageSource;
  };
} // namespace mitk

#endif // MITKUSVideoDeviceCustomControls_H_HEADER_INCLUDED_