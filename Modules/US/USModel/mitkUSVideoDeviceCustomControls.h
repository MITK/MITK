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
  mitkNewMacro1Param(Self, itk::SmartPointer<USVideoDevice>);

  /**
    * Activate or deactivate the custom controls. This is just for handling
    * widget visibility in a GUI for example. Cropping will not be deactivated
    * if this method is called with false. Use
    * mitk::USVideoDeviceCustomControls::SetCropArea() with an empty are
    * instead.
    */
  virtual void SetIsActive( bool isActive ) override;

  /**
    * \return if this custom controls are currently activated
    */
  virtual bool GetIsActive( ) override;

  /**
    * \brief Sets the area that will be cropped from the US image.
    * Set [0,0,0,0] to disable it, which is also default.
    */
  void SetCropArea( USImageVideoSource::USImageCropping newArea );

  /**
    * \return area currently set for image cropping
    */
  mitk::USImageVideoSource::USImageCropping GetCropArea( );

protected:
  /**
    * Class needs an mitk::USImageVideoSource object for beeing constructed.
    * This object will be manipulated by the custom controls methods.
    */
  USVideoDeviceCustomControls( itk::SmartPointer<USVideoDevice> device );
  virtual ~USVideoDeviceCustomControls( );

  bool                          m_IsActive;
  USImageVideoSource::Pointer   m_ImageSource;
};
} // namespace mitk

#endif // MITKUSVideoDeviceCustomControls_H_HEADER_INCLUDED_