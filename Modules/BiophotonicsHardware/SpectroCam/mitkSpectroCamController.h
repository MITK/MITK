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
#ifndef mitkSpectroCamController_h
#define mitkSpectroCamController_h

#include <string>


#include <MitkSpectroCamExports.h>
#include <mitkImage.h>

namespace mitk
{
  // forward declaration of the implementation for the SpectroCam
  class SpectroCamController_pimpl;

  /**
  * @brief Controller for Pixelteq SpectroCam
  *
  *
  * @ingroup BiophotonicsHardware
  */
  class MITKSPECTROCAM_EXPORT SpectroCamController
  {
  public:
    SpectroCamController();
    ~SpectroCamController();

    bool Ini();
    int OpenCameraConnection();
    int CloseCameraConnection();
    bool isCameraRunning();

    /**
    Returns the current image stack. Is of VectorType, unsigned short
    */
    mitk::Image::Pointer GetCurrentImage();

    void SetCurrentImageAsWhiteBalance();

  private:
    SpectroCamController_pimpl* m_SpectroCamController_pimpl;
  };

}
#endif
