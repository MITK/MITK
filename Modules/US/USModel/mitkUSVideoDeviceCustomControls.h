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
#include <itkObjectFactory.h>

namespace mitk {

  class MitkUS_EXPORT USVideoDeviceCustomControls : public USAbstractControlInterface
  {
  public:
    mitkClassMacro(USVideoDeviceCustomControls, USAbstractControlInterface);
    mitkNewMacro1Param(Self, mitk::USImageVideoSource::Pointer);

    virtual void SetIsActive(bool);
    virtual bool GetIsActive();

    /*@brief Sets the area that will be cropped from the US image. Set [0,0,0,0] to disable it, which is also default. */
    void SetCropArea(USImageVideoSource::USImageCropping newArea);

    mitk::USImageVideoSource::USImageCropping GetCropArea();

  protected:
    USVideoDeviceCustomControls(mitk::USImageVideoSource::Pointer);
    virtual ~USVideoDeviceCustomControls();

    bool                          m_IsActive;
    USImageVideoSource::Pointer   m_ImageSource;
  };

} // namespace mitk

#endif // MITKUSVideoDeviceCustomControls_H_HEADER_INCLUDED_